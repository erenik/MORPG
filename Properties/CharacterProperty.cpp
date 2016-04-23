/// Emil Hedemalm
/// 2014-08-01
/// Binds the Entity- and custom MORPG character objects together nicely.

#include "Random/Random.h"
#include "App/MORPG.h"
#include "CharacterProperty.h"

#include "Character/Stats.h"
#include "Character/Character.h"

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
	/// Associate property too.
	ch->prop = this;
}

CharacterProperty::~CharacterProperty()
{
}



/// Time passed in seconds..! Will steer if inputFocus is true.
void CharacterProperty::Process(int timeInMs)
{
	if (inputFocus)
		ProcessInput();

	if (dead)
	{
		/// Remove from targets n stuffs.
		if (ch->characterType != Character::PLAYER)
		{
			ch->BecomeUntargetable();

			deadTimeMs += timeInMs;
			if (deadTimeMs > 3000) // Remove.
				deleteMe = true;
		}
		// Remain targetable, just dead.. lol
		return;
	}
	if (healing)
	{
		healingMs += timeInMs;
		if (healingMs > 500)
		{
			healingMs -= 500;
			// Actually heal.
			ch->HealTick();
		}
		return;
	}
	if (attacking)
	{
		// Cooldown ready?
		attackCooldown = (short) MaximumFloat(attackCooldown - timeInMs, 0);
		
		if (attackCooldown > 1)
			return;
		/// Is in range?
		Attack();
		/// If unarmed, swing again.
		if (ch->statsWithBuffs->weaponType == UNARMED && TargetAlive())
			Attack();
		attackCooldown += ch->stats->weaponDelay; // Attack cooldown.
	}
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

Random attackRand;
void MCharProp::Attack()
{
	// Perform an attack.
	Stats * tStats = MainTarget()->stats,
		* mStats = ch->stats;

	/// Add to engaged list on attack if not already there.
	if (!engagedFoes.Exists(MainTarget()))
		engagedFoes.AddItem(MainTarget());

	/// LDiff
	int lDiff = tStats->level - mStats->level;
	/// Accuracy first.
	float accuracy = mStats->accuracy - tStats->evasion - lDiff * 3.f;
	bool hit = attackRand.Randf(100.f) < accuracy;
	if (!hit){ 
		morpg->Log(ch->name+" misses "+MainTarget()->name+".");
		MainTarget()->prop->AddEnmityFor(*ch, 1); // Add smaller amount of enmity.
		return;
	}
	/// Weapon damage.
	int weaponDamage = mStats->damage;
	/// Scale with STR, vs. VIT.
	int strDiff = (mStats->str - tStats->vit);
	ClampFloat(strDiff, -0.5 * weaponDamage, 0.5 * weaponDamage);
	weaponDamage += strDiff;
	/// Apply attack vs. defense.
	float attack = mStats->Attack();
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
	damage *= multiplier;
	ClampFloat(damage, 1, 9999); // min/max dmg.
	tStats->hp -= damage;
	// Add enmity!
	MainTarget()->prop->AddEnmityFor(*ch, 3 + 50 * damage/tStats->maxHp);

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
	morpg->Log(Text(ch->name+" hits "+MainTarget()->name+" for "+String(damage)+" damage."+addedStr, 
		(MainTarget()->characterType == Character::FOE)? 0xFFFFFFFF : 0xFF7F00FF));
	if (ch == morpg->HUDCharacter()) // Update HP if damaged.
		morpg->UpdateHUD();
	if (tStats->hp < 0)
	{
		morpg->Log(Text(ch->name+" defeats "+MainTarget()->name+"!", 
			(MainTarget()->characterType == Character::FOE) ? 0xFFD700FF : 0x8B0000FF));
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
		MainTarget()->prop->OnDeath();
		// Release target and locks if this was our only target.
		if (ch == morpg->HUDCharacter())
			morpg->CloseInteractionMenu();
		LoseTarget();
	}
	for (int i = 0; i < ch->buffs.Size(); ++i)
		ch->buffs[i]->OnAttack(*ch);

	// Add delay.
	mStats->weaponDelay = 600;
}

void MCharProp::OnDeath()
{
	// Release target and locks.
	if (ch == morpg->HUDCharacter())
		morpg->CloseInteractionMenu();
	dead = true;
	LoseTarget();
}

void MCharProp::AddEnmityFor(Character & character, int amount)
{
	/// Add to list if not already there.
	if (!engagedFoes.Exists(&character))
		engagedFoes.AddItem(&character);

	for (int i = 0; i < enmity.Size(); ++i)
	{
		if (enmity[i].first == character.ID())
		{
			enmity[i].second += amount;
			return;
		}
	}
	/// New one?! Attack 'im!
	enmity.AddItem(std::pair<int,int>(character.id, amount));
	if (!attacking)
	{
		mainTarget = &character;
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
	targetLocked = true;
	// o-o add initial delay.
	attackCooldown += ch->stats->weaponDelay;
	ClampFloat(attackCooldown, 0, ch->stats->weaponDelay * 10);
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
		// Set relative velocity. It will solve the issue of direction by using the current rotation :)
		Vector3f velocity(0, 0, -ch->stats->movementSpeed);
		QueuePhysics(new PMSetEntity(owner, PT_RELATIVE_VELOCITY, velocity));
		/// Disable regular velocity.
		QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, Vector3f())); 
		/// Set damping in case the regular velocity persist somehow.
		QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.5f));
	}
	// Leaving autorun-mode.
	else 
	{
		QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.9f));
		QueuePhysics(new PMSetEntity(owner, PT_RELATIVE_VELOCITY, Vector3f()));
	}
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

void CharacterProperty::LoseTarget()
{
	attacking = false;
	targetLocked = false;
	mainTarget = 0;
	if (ch == morpg->HUDCharacter())
	{
		morpg->UpdateHUD();
		morpg->CloseInteractionMenu();
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
	// Update menu depending on target?
	morpg->CloseInteractionMenu();
	morpg->OpenInteractionMenu();
}


/** Checks states via InputManager. Regular key-bindings should probably still be defined in the main game state 
	and then passed on as messages to the character with inputFocus turned on.
*/
void CharacterProperty::ProcessInput()
{
	float forward = 0.f;
	// Should probably check some lexicon of key-bindings here too. or?
	if (InputMan.KeyPressed(KEY::W))
		forward -= 1.f;
	if (InputMan.KeyPressed(KEY::S))
		forward += 1.f;
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

//	Vector3f rot;
//	rot.y = right;

	// 
	// Auto-running,.
	if (autorun)
	{
		if (lastAcc != acc)
		{
		}
		if (right != lastRight)
		{
			// Rotate int Y..
			Quaternion q = Quaternion(Vector3f(0,1,0), right);
			QueuePhysics(new PMSetEntity(owner, PT_ROTATIONAL_VELOCITY, q));
			lastRight = right;
		}
	}
			
	/// o-o cameraaaa focsuuuuuu!
	if (owner->cameraFocus)
	{

		// Free-form running (relative to camera)
		if (!autorun)
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


		/// Make sure the camera is rotating around the center of the entity.
		float height = 1.7f;
		if (owner->cameraFocus->relativePosition.y != height)
		{
			Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_RELATIVE_POSITION_Y, height));
			Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_TRACKING_POSITION_OFFSET, Vector3f(0,height,0)));
		}
		/// Camera Control, Booyakasha!
		float cameraRight = 0.f;
		if (InputMan.KeyPressed(KEY::LEFT))
			cameraRight += 1.f;
		if (InputMan.KeyPressed(KEY::RIGHT))
			cameraRight -= 1.f;

		// Set it! :D
		static float pastCameraRight = 0.f;
		if (cameraRight != pastCameraRight)
		{
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
}
