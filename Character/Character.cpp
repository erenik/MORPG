/// Emil Hedemalm
/// 2014-07-27
/// A character.

#include "Character.h"
#include "Skill.h"
#include "AllSkillsEnum.h"
#include "Properties/CharacterProperty.h"
#include "Stats.h"
#include "App/MORPG.h"

#include "World/Zone.h"
#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"
#include "Entity/Entity.h"
#include "Entity/EntityManager.h"

#include "Graphics/Messages/GMCamera.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "StateManager.h"
#include "PhysicsLib/PhysicsType.h"
#include "Family.h"

String WeaponTypeString(int type)
{
	switch(type)
	{
		case UNARMED: return "Unarmed";
		case DAGGER: return "Dagger";
		case SWORD: return "Sword";
		default:
			assert(false && "");
	}
	return "";
}


int TNLClass(int level)
{
	if (level < 16)
		return 400 + (level-1) * 150;
	else if (level < 31)
		return 2500 + (level-15) * 100;
	else if (level < 51)
		return 4000 + (level - 30) * 250;
	else if (level < 60)
		return 9000 + (level - 50) * 1000;
	return 25000;
}

int TNLWeapon(int level)
{
	if (level < 11)
		return 60 + (level-1) * 10;
	return 150 + (level-10) * 150;
}

int Character::idEnumerator = 0;

Character::Character(int type)
	: id(idEnumerator++)
{
	currentClassLvl = std::pair<char, char>(CLASSLESS, 1);

	name = "Character";
	baseStats = new Stats();
	statsWithGear = new Stats();
	statsWithBuffs = new Stats();
	stats = statsWithBuffs;
	prop = 0;
	zone = 0;
	characterType = type;
}

Character::~Character()
{
	delete baseStats;
	delete statsWithGear;
	delete statsWithBuffs;
}

/// Spawns in map. Must have map assigned first.
void Character::Spawn(ConstVec3fr position, Zone * intoZone)
{
	/// Still spawned since earlier. Despawn first?
	if (zone)
	{
		Despawn();	
	}
	assert(characterType != CT::BAD_TYPE);
	if (intoZone && zone == 0)
		zone = intoZone;
	assert(zone);
	zone->AddCharacter(this); // Add character to zone too.

	// update all stats right before spawning.
	UpdateBaseStatsToClassAndLevel();

	/// Copy hp and mp from max, since spawning.
	stats->hp = (float) stats->maxHp;
	stats->mp = (float) stats->maxMp;

	// Select texture based on type.
	String tex = "0xAA";
	switch(characterType)
	{
		case CT::NPC: tex = "0xAA"; break;
		case CT::FOE: tex = " 0xFF7F00"; break;
		case CT::PLAYER: tex = "0xEE"; break;
	default:
		assert(false);
	}

	// Attach ze propororoty to bind the entity and the player.
	Entity * entity = EntityMan.CreateEntity(name, ModelMan.GetModel("Characters/TestCharacter.obj"), TexMan.GetTexture(tex));
	entity->SetPosition(position);
	MapMan.AddEntity(entity, zone);
	// Adjust size of representation.
	QueuePhysics(new PMSetEntity(entity, PT_SET_SCALE, representationScale));
	QueuePhysics(new PMSetEntity(entity, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));

	// Prop
	prop = new CharacterProperty(entity, this);
	entity->properties.Add(prop);
}

// Despawns from previous zone.
void Character::Despawn()
{
	// Remove from zone if not already done so?
	zone->RemoveCharacter(this);
	zone = 0;
	MapMan.DeleteEntity(prop->owner); // Assuming we have an entity from before, re-use it? or not.
}

void Character::SetCameraFocus()
{
	// Attach camera to the player.
	QueueGraphics(new GMSetCamera(firstPersonCamera, CT_ENTITY_TO_TRACK, this->prop->owner));
	QueueGraphics(new GMSetCamera(firstPersonCamera)); // Set as active camera.
}

/// Where.
Vector3f Character::Position()
{
	if (prop)
		return prop->owner->worldPosition;
	return Vector3f();
}

void Character::BecomeUntargetable()
{
	Interactable::BecomeUntargetable();
	/// Remove bindings from other characters.
	for (int i = 0; i < prop->engagedFoes.Size(); ++i)
	{
		Character * c = prop->engagedFoes[i];
		c->prop->OnCharacterDefeated(this);
	}
	prop->engagedFoes.Clear();
}

/// o-o
int Character::WeaponTrainingLevel(int type)
{
	for (int i = 0; i < weaponLvls.Size(); ++i)
	{
		if (weaponLvls[i].first == type)
			return weaponLvls[i].second;
	}
	return 0;
}
void Character::SetWeaponTrainingLevel(int type, int lvl)
{
	for (int i = 0; i < weaponLvls.Size(); ++i)
	{
		if (weaponLvls[i].first == type)
		{
			weaponLvls[i].second = lvl;
			return;
		}
	}
	weaponLvls.AddItem(std::pair<char, int>(type, lvl));
}


int Character::GainWeaponExp(int amount)
{
	/// Get weapon level.
	int wtl = WeaponTrainingLevel(stats->weaponType);
	// Gain exp into weapon.
	for (int i = 0; i < this->weaponExp.Size(); ++i)
	{
		std::pair<char, int> & exp = weaponExp[i];	
		if (exp.first == stats->weaponType)
		{
			// enter exp into skill training in this class, and class itself.
			exp.second += amount;
			int tnl = TNLWeapon(wtl+1);
			if (exp.second > tnl)
			{
				SetWeaponTrainingLevel(stats->weaponType, wtl+1);
				exp.second -= tnl;
				// Update stats immediately.
				this->UpdateGearStatsToCurrentGear();
				// Mention it.
				if (this == morpg->HUDCharacter())
					morpg->Log(Text(WeaponTypeString(stats->weaponType)+" training skill rises to level "+String(wtl+1)+".", 0xFFFFFFFF));
				return wtl+1;
			}
			return 0;
		}
	}
	// Add to list, re-call.
	weaponExp.AddItem(std::pair<char, int>(stats->weaponType, 0));
	return GainWeaponExp(amount);	
}

/// Returns non-0 if reaching target level. 0 if same level.
int Character::GainExp(int amount)
{
	GainWeaponExp(amount);

	// Gain exp into equipped weapon, class level, and class training skill, yes?
	for (int i = 0; i < classExp.Size(); ++i)
	{
		std::pair<char, int> & exp = classExp[i];
		if (exp.first == currentClassLvl.first)
		{
			// enter exp into skill training in this class, and class itself.
			exp.second += amount;
			int tnl = TNLClass(currentClassLvl.second);
			if (exp.second > tnl)
			{
				++currentClassLvl.second;
				exp.second -= tnl;
				// Update stats immediately.
				this->UpdateBaseStatsToClassAndLevel();
				return currentClassLvl.second;
			}
			return 0;
		}
	}
	// Add to list, re-call.
	classExp.AddItem(std::pair<char, int>(currentClassLvl.first, 0));
	return GainExp(amount);
}

/// To current class and level.
void Character::UpdateBaseStatsToClassAndLevel()
{
	*baseStats = Stats(); // Refresh from base.

	int result = UpdateBaseStatsAlternative();
	if (result == UPDATE_STATS_FOR_CLASS)
		UpdateBaseStatsForClass();

	/// Update gear data.
	if (characterType == CT::FOE)
		UpdateStatsWithBuffs();
	else
		UpdateGearStatsToCurrentGear();
	if (this == morpg->HUDCharacter())
		morpg->UpdateHUD();
}

int Character::UpdateBaseStatsAlternative()
{
	return UPDATE_STATS_FOR_CLASS;
}
void Character::UpdateBaseStatsForClass()
{
	availableSkills.Clear(); // Clear lists of available ones.
	activatableCombatSkills.Clear();
	for (int i = 0; i < skills.Size(); ++i)
	{
		std::pair<int, char> skill = skills[i];
		// Diss those outside of range (120 skills per class).
		if (skill.first > CLASSLESS_SKILLS * SPC &&
				(
				skill.first < currentClassLvl.first * SPC || skill.first >= (currentClassLvl.first+1) * SPC
				)
			)
			continue;
		// Exception to the rule.
		if (skill.first == 0 && currentClassLvl.first != CLASSLESS)
			continue;
		availableSkills.AddItem(skill);
		if (IsActivatableCombatSkill(skill.first))
			activatableCombatSkills.AddItem(skill);
	}
	/// Base stats.
	float strL, vitL, hpL, mpL, magL, MdefL, hppL = 0, mppL = 0;
	switch(currentClassLvl.first)
	{
		case CLASSLESS: strL = vitL = magL = MdefL = 0.7f; hpL = mpL = 3; hppL = mppL = 0; break;
		case FIGHTER: strL = 1.1f; vitL = 1.2f; magL = 0.5f; MdefL = 0.8f; hpL = 11.f; hppL = 2.7f; break;
		case MONK: strL = 1.2f; vitL = 1.1f; magL = 0.8f; MdefL = 0.4f; hpL = 10.f; hppL = 2.6f; break;
		default:
			assert(false);
	}
	int L = currentClassLvl.second;
	Stats & bs = *baseStats;
	bs.level = currentClassLvl.second;
	bs.cls = currentClassLvl.first;
	bs.family = HUMANOID;
	bs.weaponType = UNARMED;

	bs.str += (int) strL * L;
	bs.vit += (int) vitL * L;
	bs.maxHp += (int) hpL * L;
	bs.mag += (int) magL * L;
	bs.mdef += (int) MdefL * L;
	bs.maxMp += (int) mpL * L;
	bs.maxHp = int(bs.maxHp * (1.0 + hppL * L * 0.01f));
	bs.maxMp = int(bs.maxHp * (1.0 + mppL * L * 0.01f));

	/// Passive Traits
	int tier = currentClassLvl.second / 5;
	switch(currentClassLvl.first)
	{
		case CLASSLESS:
		{
			switch(tier)
			{
			case 0: case 1: case 2: // up to 14.
				bs.str += 1;
				bs.vit += 1;
				bs.mag += 1;
				bs.mdef += 1;
			}
			break;	
		}
		case FIGHTER:
		{
			switch(tier)
			{
				case 14: case 15:
				case 12: case 13:
				case 10: case 11:
				case 6: case 7: case 8: case 9:
				case 3: case 4: case 5: // lvl 15, 20, 25 to 29
					bs.counterAttack += 2;
					bs.attack += 5;
					bs.defense += 5;
				case 0: case 1: case 2: // Up to lvl 14
					bs.attack += 10;
					bs.defense += 10;
					bs.criticalHitRate += 3;
					break;
			}
			break;
		}
		case MONK:
		{	
			switch(tier)
			{
				case 10: // Lvl 50+
				case 6: case 7: case 8: case 9:
					bs.unarmedDMGBonus += 3.f;
					bs.regen += .5f;
					bs.attackSpeed += 3.f;
				case 3: case 4: case 5: // Up to 30
					bs.unarmedDMGBonus += 2.f;
					bs.regen += .5f;
					bs.attackSpeed += 5.f;
				case 0: case 1: case 2: // lvl 0 to 14
					bs.unarmedDMGBonus += 5.f;
					bs.regen += 1.f;
					break;
			}
			break;
		}
	}
	/// Review skills.
	for (int i = 0; i < availableSkills.Size(); ++i)
	{
		
	}
}

void Character::UpdateGearStatsToCurrentGear()
{
	/// Copy.
	*statsWithGear = *baseStats;
	/// Check equipped weapon..
	/// Or if unarmed...
	// Check skill level for given weapon.
	int level = WeaponTrainingLevel(statsWithGear->weaponType);
	/// Add bonuses accordingly.
	/*	+2% hit-rate and damage per level after L10, Level 0 - no proficiency, Attack, acc, atk speed (60+8L)%  up to L5
		+1 Attack, +1 Accuracy (%?) per L after L5, +0.1 damage per L after L5, +0.4% attack speed per level after L5
		+0.06125% critical hit rate per L after L100, +0.5% critical hit damage per L after L150
		Max lvl in weapons training is 255?	Max bonuses: +250 attack, +250% accuracy, +9.6875% critical hit rate, +52.5% cdamage, +25 damage, +125% attack speed*/
	// -35% by default when lvl 0 in given weapon skill.
	// To L5
	int to5 = MaximumFloat(5 - level, 0);
	Stats & s = *statsWithGear;
	if (to5 > 0)
	{
		s.accuracy -= to5 * 7;
		s.attackBonus -= to5 * 7;
	}
	/// Above 5, bonuses
	else {
		s.accuracy += level - 5;
		s.attack += level - 5;
		s.attackSpeed += (level - 5) * 0.4f;
		s.damage += int((level - 5) * 0.1f);
		if (level > 100)
			s.criticalHitRate += (level - 100) * 0.06125f;
		if (level > 150)
			s.criticalDmgBonus += (level - 150) * 0.5f;
	}
	/// Now that we know what weapon we have, apply unarmed amage bonus straight to damage.
	if (s.weaponType == UNARMED)
		s.damage += (int) s.unarmedDMGBonus;

	UpdateStatsWithBuffs();
}

void Character::UpdateStatsWithBuffs()
{
	// Copy first HP and MP before over-writing shits, yo.
	statsWithGear->hp = stats->hp;
	statsWithGear->mp = stats->mp;
	*this->statsWithBuffs = *this->statsWithGear; // Copy over gear data.

	/// Review skills
	for (int i = 0; i < buffs.Size(); ++i)
	{
		Buff * buff = buffs[i];
		AddStatsFromBuff(buff, this);
	}


	// Add some base attack/defense based on level and str/vit? <- depending on weapon too?
	stats->attack += stats->str / 2;
	stats->defense += stats->vit / 2;
}


/// Adds buff. Recalcualtes stats.
Buff * Character::AddBuff(Buff * newBuff)
{
	buffs.AddItem(newBuff);
	UpdateStatsWithBuffs();
	return newBuff;
}

void Character::RemoveBuff(Buff * goBuff)
{
	buffs.RemoveItem(goBuff);
	UpdateStatsWithBuffs();
}	

char Character::SkillLevel(int skill)
{
	for (int i = 0; i < availableSkills.Size(); ++i)
	{
		if (availableSkills[i].first == skill)
			return availableSkills[i].second;
	}
	return -1;
}


bool Character::HasBuff(int fromSkill)
{
	for (int i = 0; i < buffs.Size(); ++i)
	{
		if (buffs[i]->skill == fromSkill)
			return true;
	}
	return false;
}	

/// Taking into consideration bonuses.
float Character::MovementSpeed()
{
	return stats->movementSpeed * (1.f + stats->movementSpeedBonus);
}

/// o.o
bool Character::WriteTo(std::fstream & file)
{
	name.WriteTo(file);
	return true;	
}

bool Character::ReadFrom(std::fstream & file)
{
	name.ReadFrom(file);
	return true;
}


