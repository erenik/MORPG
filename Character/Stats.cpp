/// Emil Hedemalm
/// 2016-04-23
/// All stats of something to combat with.

#include "Stats.h"
#include "FighterSkills.h"
#include "Character.h"

Stats::Stats()	
{	// Default values.
	movementSpeed = 2.f; stunDurationMs = disarmDurationMs = paralyzedMs = 0; regen = refresh = 0.f; hp = maxHp = 13; mp = maxMp = 0;
	str = vit = mag = 10;
	accuracy = 95.f; // default 95% acc (60% if 0 weapon skill).
	evasion = 0; // is bonus, default miss is the same as evade.
	attackBonus = 0; 
	defenseBonus = 0.f;
	enmity = enmityDecayReduction = 0; // bonuses compared to default of 100%.
	attack = defense = mdef = 1; // Default values. Minimum L for attack/defense, mdef varies more per class.
	parryingRate = disarmRate = criticalDmgBonus = enemyCriticalHitRate = healingReceivedBonus = damageBonusP = 0;
	criticalHitRate = 3;
	damage = 3; // default minimum for unarmed.
	weaponDelay = 600; // default for unarmed? 600 - 6 seconds.
	attackSpeed = 0; // Default, affects % wise with formulae
	counterAttack = doubleAttack = 0;
	resting = 3; // Default resting, 3 hp/mp per 5 seconds.
	restingPercent = 0;
	dualWield = 1.0; // % penalty. 1.0 (+100%) by default. Reduces to .5 at first trait.
	unarmedDMGBonus = 0; // H2H n MNK stats
	unarmedSelfDamage = 0.3f; // Default 30%, decreases with traits.
	blockRate = 0.1f; // %
	blockDMGReduction = 0.15f; // % 
	stealRate = 0.03f; // Rogue stats, % on attempts
	coldResistance = fireResistance = 0;
	slayer = 0;
	damageReduction = 0;
	doubleAttackAtkBonus = 0;
};

int Stats::Attack()
{
	return MaximumFloat(attack * (100+attackBonus)/100.f, 1);
}
int Stats::Defense()
{
	return MaximumFloat(defense * (100+defenseBonus)/100.f, 1);
}
