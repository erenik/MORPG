/// Emil Hedemalm
/// 2016-04-23
/// 

#include "Stat.h"
#include "Skill.h"
#include "FighterSkills.h"
#include "Character.h"

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

 bool IsActivatable(int skill)
{
 // will be used?
	return false;
}

void AddPassiveStatBonuses(int skill, char L, Stats & baseStats)
{
	switch(skill)
	{
		//// FIGHTER_SKILLS
		case PROVOKE:	baseStats.enmityDecayReduction += L; break; // 1% per L 
		case DEFENSIVE_TRAINING: baseStats.defense += 2 * L; baseStats.healingReceivedBonus += 0.5 * L; break;
		case OFFENSIVE_TRAINING: baseStats.attack += 2 * L; baseStats.healingReceivedBonus -= 0.5 * L; break;
		case DOUBLE_ATTACK: baseStats.doubleAttack += 0.75 * L; baseStats.doubleAttackAtkBonus += 10 * L; break;
		case DEFENSIVE_TRAINING_II: baseStats.defense += 3 * L; baseStats.damageReduction += 0.5 * L; break;
		case OFFENSIVE_TRAINING_II: baseStats.attack += 3 * L; baseStats.damageReduction -= 0.5 * L; break;
		case TEMPO_TRAINING: baseStats.attackSpeed += L; break;
		case GOAD: baseStats.enmityDecayReduction += L; break; // 1% per L.
		case DOUBLE_ATTACK_II: baseStats.doubleAttack += L; break;
		case STALWARD_DEFENDER: baseStats.enmity += L; baseStats.enmityDecayReduction += L; break;
	}	
}


/// Query to ready skill i, or start casting it for magicks. Returns true if ready.
bool Queue(int skill, Character & character)
{
	/// Only skills with actual cooldowns need to anything in the switch-clause.
	switch(skill)
	{
		break;
	}
	return true;
}

/// Starts execution of the skill, animations, dealing damage, etc. Returns true if it started successfully. False if bad/failed request.
bool Start(int skill, char L, Character & character)
{
	switch(skill)
	{
		/// Fighter skills
		case BERSERK:
		case DEFENDER:	
		case OFFENSIVE_STANCE:
			character.AddBuff(new Buff(skill, L)); 
			break;
		case CHARGE:
			character.AddBuff(new Buff(skill, L))->durationAttacks = 1; 
			break;
		case RUSH:	
		case RUSH_II:	
		case RUSH_III:
#define ifHasThenActivate(a,b) if (character.SkillLevel(a) > 0) character.AddBuff(Buff::Attacks(a, character.SkillLevel(a), b)) 
			ifHasThenActivate(RUSH,5);
			ifHasThenActivate(RUSH_II,5);
			ifHasThenActivate(RUSH_III,5);
			break;
		case PROVOKE: for (int i = 0; i < character.engagedFoes.Size(); ++i) character.engagedFoes[i]->AddEnmityFor(character, 5 + 2 * L); break;
		case GOAD: character.mainTarget->AddEnmityFor(character, 5+2*L); character.mainTarget->AddBuff(new Buff(skill, L)); break;
		case WEAPON_BASH: character.AddBuff(new Buff(skill, L)); character.Attack(); break;
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





