/// Emil Hedemalm
/// 2016-04-23
/// 

#include "Stats.h"
#include "Skill.h"
#include "AllSkillsEnum.h"
#include "Character.h"
#include "Properties/CharacterProperty.h"
#include "App/MORPG.h"

String GetSkillName(int skill)
{
	switch (skill)
	{
		// 2-hours
		case I_CAN_DO_IT: return "I can do it!"; case BERSERK: return "Berserk"; case RESTORATION: return "Restoration"; case INNER_POTENTIAL: return "Inner potential";
		// Classless skills. 
		/// Monk skills
		
		// etc.
		default:
			morpg->Log("Add in GetSkillName in MORPG/Character/Skill.cpp");
	}
	return "Not there yet";
}

int GetSkillByName(String name)
{
	for (int i = 0; i < ALL_SKILLS; ++i)
	{
		if (GetSkillName(i) == name)
			return i;
	}
	return -1;
}

char GetSkillRequiredClass(int skill)
{
	if (skill < 100)
		return ANY;
	return skill / 100;
}

char GetSkillRequiredClassLevel(int skill)
{
	int n = skill % 100;
	n /= 5; // Get something dividable by n
	n *= 5;
	return n;
}

int GetPreparationTimeMs(int skill, char lvl, Character * character)
{
	switch(skill)
	{
		case BANISH: return 2200;
	}
	return 0;
}

 bool IsActivatableCombatSkill(int skill)
{
	switch(skill)
	{
		/// CLASSLESS skills
		case I_CAN_DO_IT:
		/// FIGHTER_SKILLS
		case BERSERK: case CHARGE: case PROVOKE: case DEFENDER: case RUSH:  // For rush, only Rush 1 need be activatable, bonuses from other skills will be taken care of later on via search.
		case OFFENSIVE_STANCE: case GOAD: case WEAPON_BASH: 
		/// MONK skills
		case INNER_POTENTIAL: case STUNNING_FIST: case FOCUSED_BLOCK: case COMBO: case STUN_COMBO: 
		case DODGE_FOCUS: case TAKE_DOWN: case STRIKE_FOCUS: case RAGING_FISTS: case ONE_INCH_PUNCH:
		case THIRD_EYE: case BOOST: 
		/// Acolyte skills
		case SEAL_OF_PROTECTION: case SEAL_WOUNDS: case REFRESH: case REMEDY: case BANISH: // Seal of protection and other upgrades are sought up automatically later.
			return true;
	}
 // will be used?
	return false;
}

void AddPassiveStatBonuses(int skill, char L, Stats & baseStats)
{
	switch(skill)
	{
		/// CLASSLESS_SKILLS
//		case 
		//// FIGHTER_SKILLS
		case PROVOKE:	baseStats.enmityDecayReduction += L; break; // 1% per L 
		case DEFENSIVE_TRAINING: baseStats.defense += 2 * L; baseStats.healingReceivedBonus += 0.5f * L; break;
		case OFFENSIVE_TRAINING: baseStats.attack += 2 * L; baseStats.healingReceivedBonus -= 0.5f * L; break;
		case DOUBLE_ATTACK: baseStats.doubleAttack += 0.75f * L; baseStats.doubleAttackAtkBonus += 10 * L; break;
		case DEFENSIVE_TRAINING_II: baseStats.defense += 3 * L; baseStats.damageReduction += 0.5f * L; break;
		case OFFENSIVE_TRAINING_II: baseStats.attack += 3 * L; baseStats.damageReduction -= 0.5f * L; break;
		case TEMPO_TRAINING: baseStats.attackSpeed += L; break;
		case GOAD: baseStats.enmityDecayReduction += L; break; // 1% per L.
		case DOUBLE_ATTACK_II: baseStats.doubleAttack += L; break;
		case STALWARD_DEFENDER: baseStats.enmity += L; baseStats.enmityDecayReduction += L; break;
	}	
}

void AddStatsFromBuff(Buff * buff, Character * ch)
{
	Stats * stats = ch->stats;
	Stats & bs = *stats;
	int L = buff->L;
	switch(buff->skill)
	{
		// Classless
		case I_CAN_DO_IT:
			stats->str += 10; stats->attack += 10; stats->regen += 10; stats->criticalHitRate += 10; stats->accuracy += 10;
			stats->vit += 10; stats->defense += 10; stats->refresh += 10; stats->evasion += 10;
			break;
		// Fighter ones.
		case BERSERK: 
		{
			float multiplier = (float) (buff->msSoFar < 30000? 1: ((buff->durationMs - buff->msSoFar) / 30000.f)); 
			bs.attackSpeed += 100 * multiplier; 
			bs.damageBonusP += 50 * multiplier; 
			bs.criticalHitRate += 20 * multiplier; 
			bs.doubleAttack += 20 * multiplier; 
			bs.criticalDmgBonus += 20 * multiplier; 
			break;
		}
		case CHARGE: bs.movementSpeed += 10 * L; bs.attackBonus += 5 * L; break;
		case DEFENDER: bs.attackBonus -= 20 + 3 *L; bs.defenseBonus += 20 + 3 *L; bs.healingReceivedBonus += 2 * L; break;
		case RUSH: bs.attackSpeed += 100; bs.criticalHitRate += 3 * L; break;
		case OFFENSIVE_STANCE: bs.attackBonus += 20+ 3 *L; bs.defenseBonus -= 20 + 3 *L ; bs.healingReceivedBonus -= 2 * L; break;
		case GOAD: bs.attackBonus += 10; bs.defenseBonus -= 10; break;
		case WEAPON_BASH: bs.damageBonusP += 10 * L;
		case RUSH_II: bs.criticalDmgBonus += 3 * L; bs.accuracy += 3 * L; break;
		case RUSH_III: bs.criticalHitRate += 2 * L; bs.attackBonus += 2 * L; break;
		/// MONK SKILLS
		case INNER_POTENTIAL:
			stats->regen += stats->maxHp * 0.0025f; stats->movementSpeedBonus += 25.0f; stats->damageBonusP += 25.f; stats->accuracy += 25.f;stats->kickAttack += 25.f;
			stats->evasion += 25.f; stats->criticalHitRate += 25.f; stats->attackSpeed += 25.f; // stats->stunningFists // Maybe apply later.
			break;
	}
}


/// Query to ready skill i, or start casting it for magicks. Returns true if ready.
bool StartPreparing(int skill, char L, Character * ch)
{
	/// Only skills with actual cooldowns need to anything in the switch-clause.
	switch(skill)
	{
		/// Cast-times here, yo!
		case BANISH:
			ch->prop->preparationTimeMs = GetPreparationTimeMs(skill, L, ch); 
			ch->prop->activeSkill = skill;
			break;
		default:
			break;
	}
	/// Default immediately here for the rest.
	Start(skill, L, ch);
	return true;
}

/// Starts execution of the skill, animations, dealing damage, etc. Returns true if it started successfully. False if bad/failed request.
bool Start(int skill, char L, Character * ch)
{
	// If close-by, add to chat.
	morpg->Log(ch->name+" uses "+GetSkillName(skill));
	switch(skill)
	{	
		/// Classless skills
		case I_CAN_DO_IT:
			ch->AddBuff(Buff::Seconds(skill, L, 30)); 
			break;
		/// Fighter skills
		case BERSERK:
			ch->AddBuff(Buff::Seconds(skill, L, 60));
			break;
		case DEFENDER:	
		case OFFENSIVE_STANCE:
			ch->AddBuff(Buff::Seconds(skill, L, 30)); 
			break;
		case CHARGE:
			ch->AddBuff(Buff::Attacks(skill, L, 1)); 
			break;
		case RUSH:	
		case RUSH_II:	
		case RUSH_III:
#define ifHasThenActivate(a,b) if (ch->SkillLevel(a) > 0) ch->AddBuff(Buff::Attacks(a, ch->SkillLevel(a), b)) 
			ifHasThenActivate(RUSH,5);
			ifHasThenActivate(RUSH_II,5);
			ifHasThenActivate(RUSH_III,5);
			break;
		case PROVOKE: for (int i = 0; i < ch->prop->engagedFoes.Size(); ++i) ch->prop->engagedFoes[i]->prop->AddEnmityFor(ch, 5 + 2 * L); break;
		case GOAD: ch->prop->MainTarget()->prop->AddEnmityFor(ch, 5+2*L); ch->prop->MainTarget()->AddBuff(Buff::Seconds(skill, L, 30)); break;
		case WEAPON_BASH: ch->AddBuff(Buff::Attacks(skill, L, 1)); ch->Attack(); break;
		/// Monk skills
		case INNER_POTENTIAL:
			ch->AddBuff(Buff::Seconds(skill, L, 60));
			break;
	}
	return true;
}

/// For those skills that do compelx thingies, like a series of attacks and crazy shit. Hm. Returns true when it should go to End.
bool OnFrame(int skill, Character & character)
{
	switch(skill)
	{
		// Mainly attacks that should not allow other actions at the same time, like combos.
		case RUSH:	case RUSH_II:	case RUSH_III:
		case WEAPON_BASH:
			if (character.HasBuff(skill))
				return false;

	}
	return true;
}

/// o-o
bool End(int skill, Character & character)
{
	return true;
}





