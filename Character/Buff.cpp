/// Emil Hedemalm
/// 2016-04-23
/// Buffs.

#include "Buff.h"
#include "Stats.h"
#include "AllSkillsEnum.h"
#include "Character.h"

Buff::Buff(int skill, char L) : skill(skill), L(L), durationAttacks(-1), durationMs(-1)
{
	removeThis = false;
	msSoFar = 0;
}

bool Buff::OnAttack(Character * character)
{
	if (durationAttacks == -1)
		return false;
	if (durationAttacks > 0)
		--durationAttacks;
	if (durationAttacks <= 0)
	{
		removeThis = true;
		return true;
	}
	return false;
}

void Buff::OnTimeElapsed(int ms, Character * ch)
{
	if (durationMs == -1)
		return;
	msSoFar += ms;
	switch(skill)
	{
		case BERSERK:
			if (msSoFar > 30000)
				// Recalc bonuses... every second?
				ch->UpdateStatsWithBuffs();
			break;
		default:
			break;
	}
	if (msSoFar > durationMs)
		removeThis = true;
}
