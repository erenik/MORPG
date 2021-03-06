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
		case RESTING: return "Resting"; case BAZAAR: return "Bazaar"; case STEAL: return "Steal"; case DUAL_WIELD: return "Dual Wield";
		case CARTOGRAPHY: return "Cartography"; case UNARMED_TRAINING: return "Unarmed training"; case SHIELD_MASTERY: return "Shield Mastery";
		// Classless L15+
		case LONG_DISTANCE_ARCHERY: return "Long-distance Archery"; case BUY_N_SELL: return "Buy-and-Sell"; case POLISH_GEAR: return "Polish gear";
		case DUAL_WIELD_II: return "Dual-wield II"; case MAGIC_AFFINITY: return "Magic affinity"; case UNARMED_TRAINING_II: return "Unarmed training II";
		/// Monk skills
		case BLOCKING:return "Blocking"; 
		case EVASION_TRAINING: return "Evasion training"; 
		case STRENGTHENED_SKIN: return "Strengthened Skin"; 
		case BLOCKING_II: return "Blocking II";
		case KICK_ATTACKS: return "Kick attacks"; 
		case STRENGTHENED_SKIN_II: return "Strengthened skin II";
		
		// etc.
		default:
			std::cout<<"";
	}
	return "";
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

// Traning multiplier
int GetSkillMultiplier(int skill)
{
	switch(skill)
	{
		// Default, check number, modulo SKILLS_PER_CLASS, check the tier it belongs to -> multiplier
		default:
		{
			int modSkill = skill % SKILLS_PER_CLASS;
			if (modSkill < 15)
				return 1;
			if (modSkill < 30)
				return 2;
			if (modSkill < 40)
				return 3;
			if (modSkill < 50)
				return 4;
			if (modSkill < 60)
				return 5;
			if (modSkill < 70)
				return 6;
			if (modSkill < 80)
				return 7;
			if (modSkill < 90)
				return 8;
			return 32;
		}
	}
	return 99;
}

 bool IsActivatableCombatSkill(int skill)
{
	switch(skill)
	{
		/// CLASSLESS skills
		case I_CAN_DO_IT:
		case STEAL:
		case MAGIC_AFFINITY:
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
		/// CLASSLESS_SKILLS 1-15
		case RESTING: baseStats.restingIncrement += 0.1f * L; baseStats.restingPercent += 0.001f * L; break;
		case BAZAAR: baseStats.itemsSellable += L; baseStats.taxReduction += L; break;
		case STEAL: baseStats.stealRate += 0.01f * L; break;
		case DUAL_WIELD: baseStats.dualWield = 0.5f; baseStats.dualWield -= 0.02f * L; break;
//		case CARTOGRAPHY: 
		case UNARMED_TRAINING: baseStats.unarmedDMGBonus += 0.5f * L; baseStats.unarmedSelfDamage -= 0.005f * L; break;
		case SHIELD_MASTERY: if (baseStats.shieldEquipped) baseStats.blockRate += 0.01f * L; baseStats.shieldStatBonus += 0.01f * L; break;
			// L 16-30
		case LONG_DISTANCE_ARCHERY: if (baseStats.weaponType == BOW) baseStats.attackRange += 2.f * L; baseStats.rangedDistancePenalty -= 0.1f * L; baseStats.pointBlankDistance += 1.f * L; break;
		case BUY_N_SELL: baseStats.taxReduction += L; baseStats.npcSellPrice += 0.02f * L; break;
		case DUAL_WIELD_II: baseStats.dualWield -= 0.01f * L; break;
		case MAGIC_AFFINITY: baseStats.maxMp += 5 * L; baseStats.mag += L; baseStats.basicSpellEfficiency += 0.10f * L; break;
		case UNARMED_TRAINING_II: baseStats.unarmedDMGBonus += 0.5f * L; baseStats.unarmedSelfDamage -= 0.005f * L; break;
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
		/// MONK_SKILLS 1-14
		case BLOCKING: baseStats.blockRate += 0.02f * L; baseStats.blockDMGReduction += 0.03f * L; baseStats.blockSkillCooldownReduction += 0.01f * L; break;
		case EVASION_TRAINING: baseStats.evasion += 0.01f * L; break;
		case STRENGTHENED_SKIN: baseStats.defense += 2 * L; baseStats.mdef += 1 * L; break;
		/// MONK_SKILLS 15-30
		case KICK_ATTACKS: baseStats.kickAttack += 0.01f * L; break;
		case BLOCKING_II: baseStats.blockRate += 0.01f * L; baseStats.blockDMGReduction += 0.005f * L; baseStats.blockSkillCooldownReduction += 0.01f * L; break;
		case STRENGTHENED_SKIN_II: baseStats.defense += 2.5f * L; baseStats.mdef += 1.25f * L; break;
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
			stats->str += 10; stats->attack += 10; stats->regen += 10; stats->criticalHitRate += 0.10f; stats->accuracy += 0.10f;
			stats->vit += 10; stats->defense += 10; stats->refresh += 10; stats->evasion += 0.10f;
			break;
		// Fighter ones.
		case BERSERK: 
		{
			float multiplier = (float) (buff->msSoFar < 30000? 1: ((buff->durationMs - buff->msSoFar) / 30000.f)); 
			bs.attackSpeed += 1.f * multiplier; 
			bs.damageBonusP += 0.50f * multiplier; 
			bs.criticalHitRate += 0.20f * multiplier; 
			bs.doubleAttack += 0.20f * multiplier; 
			bs.criticalDmgBonus += 0.20f * multiplier; 
			break;
		}
		case CHARGE: bs.movementSpeed += 0.10f * L; bs.attackBonus += 5 * L; break;
		case DEFENDER: bs.attackBonus -= 0.20f + 0.03f * L; bs.defenseBonus += 0.20f + 0.03f * L; bs.healingReceivedBonus += 0.02f * L; break;
		case RUSH: bs.attackSpeed += 1.00f; bs.criticalHitRate += 0.03f * L; break;
		case OFFENSIVE_STANCE: bs.attackBonus += 0.20f + 0.03f *L; bs.defenseBonus -= 0.20f + 0.03f * L ; bs.healingReceivedBonus -= 0.02f * L; break;
		case GOAD: bs.attackBonus += 0.10f; bs.defenseBonus -= 0.10f; break;
		case WEAPON_BASH: bs.damageBonusP += 0.10f * L;
		case RUSH_II: bs.criticalDmgBonus += 0.03f * L; bs.accuracy += 0.03f * L; break;
		case RUSH_III: bs.criticalHitRate += 0.02f * L; bs.attackBonus += 0.02f * L; break;
		/// MONK SKILLS
		case INNER_POTENTIAL:
			stats->regen += stats->maxHp * 0.0025f; stats->movementSpeedBonus += 0.25f; stats->damageBonusP += 0.25f; stats->accuracy += 0.25f; stats->kickAttack += 0.25f;
			stats->evasion += 0.25f; stats->criticalHitRate += 0.25f; stats->attackSpeed += 0.25f; // stats->stunningFists // Maybe apply later.
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





