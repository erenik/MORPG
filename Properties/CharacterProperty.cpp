/// Emil Hedemalm
/// 2014-08-01
/// Binds the Entity- and custom MORPG character objects together nicely.

#include "Random/Random.h"
#include "App/MORPG.h"
#include "CharacterProperty.h"

#include "Character/Stats.h"
#include "Character/Character.h"
#include "Character/Skill.h"

#include "Input/InputManager.h"

#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/PhysicsProperty.h"
#include "StateManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMCamera.h"

#include "Model/Model.h"

CharacterProperty::CharacterProperty(Entity * characterEntity, Character * associatedWithCharacter)
: EntityProperty("CharacterProperty", 0, characterEntity), ch(associatedWithCharacter)
{
	inputFocus = false;
	lastRight = 0.f;
	autorun = false;
	healingMs = 0;
	healing = false;
	attacking = false;
	targetLocked = false;
	dead = false;
	deadTimeMs = 0;
	mainTarget = 0;
	attackCooldown = 0;
	deleteMe = false;
	movementEnabled = true;
}

CharacterProperty::~CharacterProperty()
{
}

Random attackRand;

/// Time passed in seconds..! Will steer if inputFocus is true.
void CharacterProperty::Process(int timeInMs)
{
	if (inputFocus)
		ProcessInput();
	// Camera wooo
	UpdateCamera();
	// Dead D:
	if (dead)
	{
		/// Remove from targets n stuffs.
		if (ch->characterType != CT::PLAYER)
		{
			ch->BecomeUntargetable();

			deadTimeMs += timeInMs;
			if (deadTimeMs > 3000) // Remove.
				deleteMe = true;
		}
		// Remain targetable, just dead.. lol
		return;
	}
	/// For regen
	if (ch->stats->regen > 0 || ch->stats->refresh > 0)
	{
		HealHp(ch->stats->regen * timeInMs * 0.001f);
		HealMp(ch->stats->mp += ch->stats->refresh * timeInMs * 0.001f);
	}
	if (healing)
	{
		healingMs += timeInMs;
		if (healingMs > (int) (5000 * testMultiplier))
		{
			healingMs -= (int) (5000 * testMultiplier);
			HealTick();
		}
		return;
	}

	/// Process buffs.
	for (int i = 0; i < ch->buffs.Size(); ++i)
	{
		Buff * buff = ch->buffs[i];
		buff->OnTimeElapsed(timeInMs, ch);
		if (buff->removeThis)
		{
			morpg->Log(ch->name+"\'s \""+GetSkillName(buff->skill)+"\" wears off.");
			ch->RemoveBuff(buff);
			--i;
			delete buff;
		}
	}
	// Cooldown ready?
	if (attackCooldown > 0)
		attackCooldown = (short) MaximumFloat(attackCooldown - timeInMs, 0);
	if (attacking)
	{		
		if (attackCooldown > 1)
			return;
		/// Is in range?
		float dist = (mainTarget->Position() - ch->Position()).LengthSquared();
		if (dist > 4)
		{
			// Wait, display message every once in a while.
			static int distCounter = 0;
			distCounter += timeInMs;
			if (distCounter > 1000)
			{	
				morpg->Log("Enemy out of range");
				distCounter = 0;
			}
			return;
		}
		int numAttacks = 1;
		bool unarmed = ch->statsWithBuffs->weaponType == UNARMED;
		if (unarmed)
			++numAttacks;
		bool targetDead = false;
		for (int i = 0; i < numAttacks && !targetDead; ++i)
		{		
			targetDead = Attack();

			// Check for double attacks.
			if (!targetDead && ch->stats->doubleAttack > 0 && attackRand.Randf(100.f) < ch->stats->doubleAttack)
			{
				targetDead = Attack();
			}
			// Check for kick-attacks.
			if (!targetDead && unarmed && ch->stats->kickAttack > 0 && attackRand.Randf(100.f) < ch->stats->kickAttack)
			{
				// Add temporary bonuses for kick-attacks?
				Stats tmp = *ch->stats;
				ch->stats->damageBonusP += 10.f; // default +10% damage on kick attacks.
				targetDead = Attack();
				*ch->stats = tmp;
			}
		}
		attackCooldown += short(ch->stats->weaponDelayMs * testMultiplier); // Attack cooldown.
	}
}


void MCharProp::HealTick()
{
	int pre = int(ch->stats->hp);
	HealHp(ch->stats->resting + ch->stats->restingPercent * ch->stats->maxHp);
	HealMp(ch->stats->resting + ch->stats->restingPercent * ch->stats->maxMp); 
	int recovered = int(ch->stats->hp - pre);
	if (morpg->HUDCharacter() == ch && recovered > 0)
	{
		morpg->UpdateHPInHUD();
		morpg->Log(Text("Recovered "+String(recovered), 0x00FF00FF));
	}
}

/// Heals, notifies GUI etc. as needed. Float in order to work with regen, etc.
void MCharProp::HealHp(float amount)
{
	int hp = (int) ch->stats->hp;
	ch->stats->hp += amount;
	ClampFloat(ch->stats->hp, 0, (float)ch->stats->maxHp);
	if (int(ch->stats->hp) > hp)
		morpg->UpdateHPInHUD();
}
void MCharProp::HealMp(float amount)
{
	int mp = (int) ch->stats->mp;
	ch->stats->mp += amount;
	ClampFloat(ch->stats->mp, 0, (float)ch->stats->maxMp);
	if (int(ch->stats->mp) > mp)
		morpg->UpdateHUD();
}

/// Removing targets, links, enmities.
void MCharProp::OnCharacterDefeated(Character * c)
{
	for (int i = 0; i < engagedFoes.Size(); ++i) {
		Character * c2 = engagedFoes[i];
		if (c2 == c) {
			engagedFoes.RemoveItem(c);
			--i;
		}
	}
	if (this->mainTarget == c)
	{
		LoseTarget();
	}
}

bool MCharProp::TargetAlive()
{
	if (mainTarget)
	{
		Character * c = (Character*)mainTarget;
		return !c->prop->dead;
	}
	return false;
}

/// Performs an attack on main target (probably). Returns true if target died. false if still alive for processing more attacks.
bool MCharProp::Attack()
{
	Character * target = MainTarget();
	if (!target)
		return true;
	// Perform an attack.
	Stats * tStats = target->stats,
		* mStats = ch->stats;

	/// Add to engaged list on attack if not already there.
	if (!engagedFoes.Exists(MainTarget()))
		engagedFoes.AddItem(MainTarget());

	/// LDiff
	int lDiff = tStats->level - mStats->level;
	/// Accuracy first.
	float accuracy = mStats->accuracy - tStats->evasion - lDiff * 3.f;
	ClampFloat(accuracy, 10, 99);
	bool hit = attackRand.Randf(100.f) < accuracy;
	if (!hit){ 
		morpg->Log(ch->name+" misses "+MainTarget()->name+".");
		MainTarget()->prop->AddEnmityFor(ch, 1); // Add smaller amount of enmity.
		return true;
	}
	/// Weapon damage.
	int weaponDamage = mStats->damage;
	/// Scale with STR, vs. VIT.
	int strDiff = (mStats->str - tStats->vit);
	ClampFloat(strDiff, int(-0.5f * weaponDamage), int(0.5f * weaponDamage));
	weaponDamage += strDiff;
	/// Apply attack vs. defense.
	float attack = (float) mStats->Attack();
	// Make bell-curve somehow?
	float attDiff = attack / (float)tStats->Defense();
	int damage = weaponDamage;
	float multiplier = 1.f;
	if (attDiff > 1.5f)
	{
		float lowMult = MinimumFloat(1.5f * (attDiff - 0.5f), 3.75f); // Caps at 3.75 at attDiff-3
		float highMult = MinimumFloat(2.5f * (attDiff - 0.6f), 4.f); // Caps at 4.f mult at attDiff-2.2
		// Random from 1.5*(attDiff-0.5f) to 2.5*(attDiff-0.6) (1.5 to 2.25 at min,  to  3x at cap)
		multiplier = attackRand.Randf(highMult - lowMult) + lowMult;
	}
	else if (attDiff > 0.8f)
	{
		// Random from attDiff to 1.5*attDiff (0.8 to 1.2 at min, 1.5 to 2.25 at cap)
		multiplier = attackRand.Randf(1.5f * attDiff - attDiff) + attDiff;
	}
	else 
	{
		// Random from 0.160+0.8attDiff to 0.4+attDiff (0.160 to 0.4 at min, 0.8 to 1.2 at cap)
		multiplier = attackRand.Randf(0.4f+attDiff - 0.16f+0.8f*attDiff) + 0.16f+0.8f*attDiff;
	}
	ClampFloat(multiplier, 0.1f, attack * attack / 10000.f+1.f); // Can reach 2x @ 100 attack, 4x @ 200 attack, not higher til then.
	/// Check for critical hit.
	bool isCritical = attackRand.Randf(100.f) < (mStats->criticalHitRate - tStats->enemyCriticalHitRate);
	if (isCritical)
	{
		multiplier += 1.f; // Add 1x to multiplier, should be sufficient.
		multiplier *= (100.f +  mStats->criticalDmgBonus) / 100.f; // Add extra percentage based bonuses.
	}
	damage = int(damage * multiplier);
	ClampFloat(damage, 1, 9999); // min/max dmg.
	tStats->hp -= damage;
	if (target == morpg->HUDCharacter())
		morpg->UpdateHPInHUD();
	// Add enmity!
	MainTarget()->prop->AddEnmityFor(ch, 3 + 50 * damage/tStats->maxHp);

	String addedStr;
	if (ch->statsWithBuffs->weaponType == UNARMED)
	{
		int selfDmg = damage * mStats->unarmedSelfDamage;
		mStats->hp -= selfDmg;
		addedStr = " "+String(selfDmg)+" damage taken to self.";
		if (mStats->hp < 0)
		{
			OnDeath();
		}
	}
	// Notify.
	morpg->Log(Text((isCritical? "Critical! " : "") + ch->name+" hits "+target->name+" for "+String(damage)+" damage."+addedStr, 
		(target->foe)? 0xFFFFFFFF : 0xFF7F00FF));
	if (ch == morpg->HUDCharacter()) // Update HP if damaged.
		morpg->UpdateHPInHUD();
	
	/// Process buffs.
	for (int i = 0; i < ch->buffs.Size(); ++i)
		ch->buffs[i]->OnAttack(ch);

	/// Vanquish if applicable.
	if (tStats->hp < 0)
	{
		morpg->Log(Text(ch->name+" defeats "+target->name+"!", 
			target->foe ? 0xFFD700FF : 0x8B0000FF));
		tStats->hp = 0;
		/// Calculate EXP to receive.
		int expGained = 200 * pow(1.2f, (float)lDiff);
		if (ch == morpg->HUDCharacter())
		{
			int lvldUp = ch->GainExp(expGained);
			morpg->Log(Text("Gained "+String(expGained)+" experience points.", 0xE8C782));
			if (lvldUp)
				morpg->Log(Text(ch->name+" reaches level "+lvldUp+"!", 0xD3D3D3FF));
		}
		else
			ch->GainExp(expGained);

		/// Delete other one.
		target->prop->OnDeath();
		// Release target and locks if this was our only target.
		if (ch == morpg->HUDCharacter())
			morpg->CloseInteractionMenu();
		LoseTarget();
		return true;
	}
	return false;
}

/// o-o
void MCharProp::Revive()
{
	ch->stats->hp = ch->stats->maxHp * 0.25f; 
	dead = false;
	targetLocked = false;
	movementEnabled = true;
	if (ch == morpg->HUDCharacter())
	{
		morpg->Log(ch->name+" is revived!");
		morpg->UpdateHUD();
	}
}

void MCharProp::DisableMovement()
{
	movementEnabled = false;
	// Set vel 0.
	QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, Vector3f()));
	QueuePhysics(new PMSetEntity(owner, PT_RELATIVE_VELOCITY, Vector3f()));
}

void MCharProp::OnDeath()
{
	// Release target and locks.
	if (ch == morpg->HUDCharacter())
		morpg->CloseInteractionMenu();
	dead = true;
	LoseTarget();
	DisableMovement();
}

void MCharProp::AddEnmityFor(Character * character, int amount)
{
	/// Add to list if not already there.
	if (!engagedFoes.Exists(character))
		engagedFoes.AddItem(character);

	for (int i = 0; i < enmity.Size(); ++i)
	{
		if (enmity[i].first == character->ID())
		{
			enmity[i].second += amount;
			return;
		}
	}
	/// New one?! Attack 'im!
	enmity.AddItem(std::pair<int,int>(character->id, amount));
	if (!attacking) 
	{
		if (mainTarget == 0)
			mainTarget = character;
		if (ch->characterType != CT::PLAYER) // Auto-engage if non-player
			Engage();
	}
}


/// Engages main target.
void CharacterProperty::Engage()
{
	if (attacking)
		return;
	if (mainTarget == ch)
	{
		morpg->Log("Bad target. Cannot engage self.");
		return;
	}
	Character * mt = MainTarget();
	if (mt->prop->dead){
		morpg->Log("Target already defeated.");
		return;
	}
	attacking = true;
	LockTarget();
	// o-o add initial delay.
	attackCooldown += ch->stats->weaponDelayMs * 0.5f;
	ClampFloat(attackCooldown, 0, ch->stats->weaponDelayMs);
	/// Update menu.
	if (morpg->HUDCharacter())
	{
		morpg->Log("Engaging target...");
		morpg->CloseInteractionMenu();
		morpg->OpenInteractionMenu();
	}
}

void CharacterProperty::Disengage()
{
	attacking = false;
	UnlockTarget();
	attackCooldown += ch->stats->weaponDelayMs;
	/// Update menu.
	if (morpg->HUDCharacter())
	{
		morpg->Log("Disengaging target...");
		morpg->CloseInteractionMenu();
		morpg->OpenInteractionMenu();
	}
}

/// o-o
void CharacterProperty::ToggleAutorun()
{
	autorun = !autorun;
	/// New state.. do stuffelistuff.
	if (autorun)
	{
		// Disable healing if we were.
		healing = false;
		// Do we have a velocity? If not, start running?
		if (!owner->physics->velocity.MaxPart())
		{
//			autorun = false;
	//		return;
		}
	}
	// Leaving autorun-mode.
	else 
	{
	}
	UpdateAutorun();
}

void CharacterProperty::ToggleHeal()
{
	healing = !healing;
	if (healing)
	{
		morpg->Log("Resting...");
		healingMs = 0;
		// Stop movement.
		autorun = false;
		// Disable movement.
		QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.9f));
		QueuePhysics(new PMSetEntity(owner, PT_RELATIVE_VELOCITY, Vector3f()));
		QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, Vector3f())); 
	}
	else {
		// Enable movement.
		morpg->Log("Resting canceled.");
	}
}

// Functions since they may affect such things as camera, auto-run mechanics, etc.
void CharacterProperty::LockTarget()
{
	targetLocked = true;
	if (autorun)
		UpdateAutorun();
}
void CharacterProperty::UnlockTarget()
{
	targetLocked = false;
	if (autorun)
		UpdateAutorun();
}

void CharacterProperty::LoseTarget()
{
	attacking = false;
	if (targetLocked)
		UnlockTarget();
	mainTarget = 0;
	if (ch == morpg->HUDCharacter())
	{
		morpg->UpdateHUD();
		morpg->CloseInteractionMenu();
		morpg->OnTargetUpdated();
	}
}

Interactable * CharacterProperty::GetTarget()
{
	return mainTarget;
}
void CharacterProperty::SetTarget(Interactable * i)
{
	if (targetLocked)
	{
		morpg->Log("Target locked.");
		return;
	}
	mainTarget = i;
	/// Update HUD if relevant
	if (ch == morpg->HUDCharacter())
	{
//		morpg->Log("Target with sortValue: "+String(i->sortValue));
		morpg->UpdateTargetInHUD();
		morpg->OnTargetUpdated();
		// Update menu depending on target?
		if (morpg->InteractionMenuOpen())
		{
			morpg->CloseInteractionMenu();
			morpg->OpenInteractionMenu();
		}
	}
}


/** Checks states via InputManager. Regular key-bindings should probably still be defined in the main game state 
	and then passed on as messages to the character with inputFocus turned on.
*/
void CharacterProperty::ProcessInput()
{
	if (!movementEnabled)
		return;
	float forward = 0.f;
	// Should probably check some lexicon of key-bindings here too. or?
	if (InputMan.KeyPressed(KEY::W))
		forward -= 1.f;
	if (InputMan.KeyPressed(KEY::S))
	{
		forward += 1.f;
		if (autorun) // Stop autorun if S is pressed too.
			ToggleAutorun();
	}
	float right = 0.f;
	if (InputMan.KeyPressed(KEY::A))
		right -= 1.f;
	if (InputMan.KeyPressed(KEY::D))
		right += 1.f;
	float movementSpeed = ch->stats->movementSpeed;
	forward *= movementSpeed;

	if (forward != 0 || right != 0)
	{
		if (healing)
			ToggleHeal();
	}
	float rotationSpeed = 1.2f;
	right *= rotationSpeed;

	Vector3f acc;
	acc.z = forward;

	// Auto-running,.
	if (autorun)
	{
		/// Update auto-run for left/right at least.
		if (targetLocked && right != lastRight)
		{
			// Go towards or uh.. side-ways from.
			Vector3f dirToT = mainTarget->Position() - owner->worldPosition;
			dirToT.Normalize();
			Vector3f perp = dirToT.CrossProduct(Vector3f(0,1,0)).NormalizedCopy();
			Vector3f onwardsToGlory = dirToT + perp * right;
			onwardsToGlory.Normalize();
			onwardsToGlory *= ch->MovementSpeed();
			QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, onwardsToGlory));
		}
		else 
		{
			if (right != lastRight)
			{
				// Rotate int Y..
				Quaternion q = Quaternion(Vector3f(0,1,0), right);
				QueuePhysics(new PMSetEntity(owner, PT_ROTATIONAL_VELOCITY, q));
			}
		}
	}
	// If has target, go towards.
	else if (targetLocked)
	{
		// Go towards or uh.. side-ways from.
		Vector3f dirToT = mainTarget->Position() - owner->worldPosition;
		dirToT.Normalize();
		Vector3f perp = dirToT.CrossProduct(Vector3f(0,1,0)).NormalizedCopy();
		QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, - dirToT * forward + perp * right));
	}
	
	// Free-form running (relative to camera)
	if (!autorun && owner->cameraFocus)
	{
		/// Get camera transform.
		Vector3f forwardVector = -forward * owner->cameraFocus->LookingAt();
		forwardVector.Normalize();
		Vector3f rightwardVector = -right * owner->cameraFocus->LeftVector();
		rightwardVector.Normalize();
		Vector3f newVelocity = forwardVector + rightwardVector;
		// Remove Y-component.
		newVelocity.y = 0;
		Vector3f normalizedVelocity = newVelocity.NormalizedCopy();
		// Multiply movement speed.
		newVelocity = normalizedVelocity * movementSpeed;
			
		static Vector3f lastVelocity;
		if (lastVelocity != newVelocity)
		{
			// And set it!
			QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, newVelocity)); 
			lastVelocity = newVelocity;
				
			if (newVelocity.MaxPart())
			{
				// Set our rotation toward this new destination too!
				float yaw = atan2(normalizedVelocity.z, normalizedVelocity.x) + PI * 0.5f;
				QueuePhysics(new PMSetEntity(owner, PT_ROTATION_YAW, yaw));
			}
		}
	}
	lastRight = right;
}

void CharacterProperty::UpdateCamera()
{
	/// o-o cameraaaa focsuuuuuu! Walk based on what we see with the current camera.
	if (!owner->cameraFocus)
		return;

	/// Make sure the camera is rotating around the center of the entity.
	float height = 2.2f;
	if (owner->cameraFocus->relativePosition.y != height)
	{

		Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_DESIRED_MINIMUM_Y_DIFF, height));
//			Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_TRACKING_POSITION_OFFSET, Vector3f(0,height,0)));
	}
	/// Camera Control, Booyakasha!
	float cameraRight = 0.f;
	float cameraYawSpeed = 2.f;
	if (InputMan.KeyPressed(KEY::LEFT))
		cameraRight += cameraYawSpeed;
	if (InputMan.KeyPressed(KEY::RIGHT))
		cameraRight -= cameraYawSpeed;

	// Set it! :D
	static float pastCameraRight = 0.f;
	if (cameraRight != pastCameraRight)
	{
		/// Move position rightward?
		Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_ROTATION_SPEED_YAW, -cameraRight));
		pastCameraRight = cameraRight;
	}

	/// Camera updown
	float cameraUp = 0.f;
	if (InputMan.KeyPressed(KEY::UP))
		cameraUp += 1.f;
	if (InputMan.KeyPressed(KEY::DOWN))
		cameraUp -= 1.f;
	static float pastCameraUp = 0.f;
	if (cameraUp != pastCameraUp)
	{
		Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_ROTATION_SPEED_PITCH, -cameraUp)); 
		pastCameraUp = cameraUp;
	}


	float cameraZoom = 0.f;
	float cameraZoomMultiplier = 1.f;
#define CONSTANT_ZOOM_SPEED 1.f
#define ZOOM_MULTIPLIER_SPEED 1.3f
	if (InputMan.KeyPressed(KEY::PG_DOWN))
	{
		cameraZoomMultiplier *= ZOOM_MULTIPLIER_SPEED;
		cameraZoom = CONSTANT_ZOOM_SPEED;
	}
	if (InputMan.KeyPressed(KEY::PG_UP))
	{
		cameraZoomMultiplier /= ZOOM_MULTIPLIER_SPEED;
		cameraZoom = - CONSTANT_ZOOM_SPEED;
	}
	static float pastCameraZoom = 1.f;
	if (cameraZoom != pastCameraZoom)
	{
		Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_DISTANCE_FROM_CENTER_OF_MOVEMENT_SPEED, cameraZoom));
		Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_DISTANCE_FROM_CENTER_OF_MOVEMENT_SPEED_MULTIPLIER, cameraZoomMultiplier));
		pastCameraZoom = cameraZoom;
	}
}

void CharacterProperty::UpdateAutorun()
{
	if (autorun)
	{
		/// Target-locked directed auto-run
		if (targetLocked)
		{
			Vector3f toTarget = mainTarget->Position() - owner->worldPosition;
			toTarget.Normalize();
			toTarget *= ch->stats->movementSpeed;
			// Set relative velocity. It will solve the issue of direction by using the current rotation :)
//			Vector3f velocity(0, 0, -ch->stats->movementSpeed);
			/// Set damping in case the regular velocity persist somehow.
			QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, 1.0f));
			QueuePhysics(new PMSetEntity(owner, PT_RELATIVE_VELOCITY, Vector3f()));
			/// Disable regular velocity.
			QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, toTarget)); 


			/// 
	/*					// Go towards or uh.. side-ways from.
			Vector3f dirToT = mainTarget->Position() - owner->worldPosition;
			dirToT.Normalize();
			Vector3f perp = dirToT.CrossProduct(Vector3f(0,1,0)).NormalizedCopy();
			float forward = 1.f, right = 0.f;
			QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, - dirToT * forward + perp * right));
*/
		}
		/// Free-form autorun.
		else {
			// Set relative velocity. It will solve the issue of direction by using the current rotation :)
			Vector3f velocity(0, 0, -ch->stats->movementSpeed);
			QueuePhysics(new PMSetEntity(owner, PT_RELATIVE_VELOCITY, velocity));
			/// Disable regular velocity.
			QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, Vector3f())); 
			/// Set damping in case the regular velocity persist somehow.
			QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.5f));
		}
	}	
	else 
	{		
		QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.9f));
		QueuePhysics(new PMSetEntity(owner, PT_RELATIVE_VELOCITY, Vector3f()));
	}
}
