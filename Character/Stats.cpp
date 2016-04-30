/// Emil Hedemalm
/// 2016-04-23
/// All stats of something to combat with.

#include "Stats.h"
#include "AllSkillsEnum.h"
#include "Character.h"

Stats::Stats()	
{	// Default values.
	movementSpeed = 2.f; 
	movementSpeedBonus = 0;
	stunDurationMs = disarmDurationMs = paralyzedMs = 0; regen = refresh = 0.f; 
	hp = 13.f;
	maxHp = 13; 
	mp = 0.f;
	maxMp = 0;
	str = vit = mag = 10;
	accuracy = 0.95f; // default 95% acc (60% if 0 weapon skill).
	evasion = 0; // is bonus, default miss is the same as evade.
	attackBonus = 0; 
	defenseBonus = 0.f;
	enmity = enmityDecayReduction = 0; // bonuses compared to default of 100%.
	attack = defense = mdef = 1; // Default values. Minimum L for attack/defense, mdef varies more per class.
	parryingRate = disarmRate = criticalDmgBonus = enemyCriticalHitRate = healingReceivedBonus = damageBonusP = 0;
	criticalHitRate = 0.03f;
	damage = 3; // default minimum for unarmed.
	weaponDelayMs = 6000; // default for unarmed? 600 - 6 seconds.
	attackSpeed = 0; // Default, affects % wise with formulae
	counterAttack = doubleAttack = 0;
	resting = 5; // Default resting, 3 hp/mp per 5 seconds.
	restingIncrement = 0.f;
	restingPercent = 0;
	dualWield = 1.0; // % penalty. 1.0 (+100%) by default. Reduces to .5 at first trait.

	attackRange = 2.f;
	rangedAttackRange = 30.f;
	pointBlankDistance = 3.f;
	rangedDistancePenalty = 3.f;

	basicSpellEfficiency = 0.f;
	npcSellPrice = 0.f;

	itemsSellable = 0;
	taxReduction = 0;
	shieldEquipped = false;
	shieldStatBonus = 0;

	unarmedDMGBonus = 0; // H2H n MNK stats
	unarmedSelfDamage = 0.3f; // Default 30%, decreases with traits.
	blockRate = 0.0f; // % Default 0% for H2H/MNK, 10% (0.10f) for Shields.
	blockDMGReduction = 0.f; // % Reduction default 0% for H2H, 15% (0.15f) for Shields.
	kickAttack = 0.f;
	kickAttackBonus = 0.f;

	stealRate = 0; // Rogue stats, % on attempts, 0.05 for 5%
	coldResistance = fireResistance = 0;
	slayer = 0;
	damageReduction = 0;
	doubleAttackAtkBonus = 0;
};

int Stats::Attack()
{
	return (int)MaximumFloat(attack * (100+attackBonus)/100.f, 1);
}
int Stats::Defense()
{
	return (int)MaximumFloat(defense * (100+defenseBonus)/100.f, 1);
}
