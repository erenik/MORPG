/// Emil Hedemalm
/// 2016-04-23
/// Buffs.

#include "Buff.h"
#include "Stats.h"
#include "FighterSkills.h"
#include "Character.h"

bool Buff::OnAttack(Character & character)
{
	if (durationAttacks == -1)
		return false;
	if (durationAttacks > 0)
		--durationAttacks;
	if (durationAttacks <= 0)
	{
		character.RemoveBuff(this);
		return true;
	}
	return false;
}

void Buff::OnTimeElapsed(int ms, Character & character)
{
	if (durationMs == -1)
		return;
	durationMs += ms;
	int maxDurationMs = 30000;
	switch(skill)
	{
		case BERSERK:
			if (durationMs > 30000)
				// Recalc bonuses.
				character.UpdateStatsWithBuffs();
			maxDurationMs = 60000;
		default:
			break;
	}
	if (durationMs > maxDurationMs)
		character.RemoveBuff(this);
}

void Buff::AddActiveBonuses(Stats * buffedStats)
{
	Stats & bs = *buffedStats;
	switch(skill)
	{
		case BERSERK: multiplier = (float) (durationMs < 30000? 1: (60000 - durationMs / 30000)); bs.attackSpeed += 100 * multiplier; bs.damageBonusP += 50 * multiplier; bs.criticalHitRate += 20 * multiplier; bs.doubleAttack += 20 * multiplier; bs.criticalDmgBonus += 20 * multiplier; break;
		case CHARGE: bs.movementSpeed += 10 * L; bs.attackBonus += 5 * L; break;
		case DEFENDER: bs.attackBonus -= 20 + 3 *L; bs.defenseBonus += 20 + 3 *L; bs.healingReceivedBonus += 2 * L; break;
		case RUSH: bs.attackSpeed += 100; bs.criticalHitRate += 3 * L; break;
		case OFFENSIVE_STANCE: bs.attackBonus += 20+ 3 *L; bs.defenseBonus -= 20 + 3 *L ; bs.healingReceivedBonus -= 2 * L; break;
		case GOAD: bs.attackBonus += 10; bs.defenseBonus -= 10; break;
		case WEAPON_BASH: bs.damageBonusP += 10 * L;
		case RUSH_II: bs.criticalDmgBonus += 3 * L; bs.accuracy += 3 * L; break;
		case RUSH_III: bs.criticalHitRate += 2 * L; bs.attackBonus += 2 * L; break;
	}
}
