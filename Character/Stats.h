/// Emil Hedemalm
/// 2016-04-23
/// All stats of something to combat with.

#ifndef STAT_H
#define STAT_H

#include "MathLib/AEMath.h"
#include "MathLib/Vector3f.h"

struct Slayer 
{
	int primary, secondary, teriary;
	// Bonuses.
	// ...
};

struct Stats {
	Stats();
	/// attack taking into consideration percentage boosts.
	int Attack();
	int Defense();
	void ClampHpMp(){
		ClampFloat(hp, 0, (float)maxHp);
		ClampFloat(mp, 0, (float)maxMp);
	};
	// Determines how it is to play them. Used actively in the MORPGCharacterProperty. Default while testing: 2. meters per second.
	float movementSpeed;
	float movementSpeedBonus; // %, 0 default, 25 for monk 2-hour, etc.

	int stunDurationMs; // Currently stunned?! Yes.
	int disarmDurationMs; // Currently disarmed?! Yes.
	int paralyzedMs; // like stun

	float attackRange; // in meters. Default is 2 meters?
	
	float rangedAttackRange; // Max range for ranged attacks. Differentiated from melee weapon equipped. Default 30.f max.
	float rangedDistancePenalty; // Default 3% (3.f) per meter after point-blank range.
	float pointBlankDistance; // Minimum distance for ranged attacks without penalties to accuracy.

	int weaponType; // Depends on equipped gear. Here for simplicities' sake.
	int level; /// Max level, used for some calculations.
	int cls; // class
	int family; // monster family.

	float regen; // Regen HP per second.
	float refresh; // Refresh MP per second.

	float basicSpellEfficiency; // % 0.10f for 10% increase, 1.0 for 100% etc.

	float hp, mp;
	int maxHp, maxMp;
	char str, vit, mag;
	float accuracy, evasion; // 1.0 for 100%, 
	int enmity, enmityDecayReduction;
	int attack, defense, mdef;
	float attackBonus, defenseBonus; // In percent, in-case they want to be additive and not multiplicative.
	
	float parryingRate; // %-based
	float disarmRate; // %-based, 3 seconds usually.
	float criticalHitRate, enemyCriticalHitRate; // base % and reduction -%
	float criticalDmgBonus; // %-based

	float healingReceivedBonus; // %-based

	int damage;
	float damageBonusP; // %-based
	int weaponDelayMs; // Based on weapon, maybe 1 or 2 skills may affect. Milliseconds-based. 
	float attackSpeed; // Has its own forumale to affect weapon delay. 0.0, 1.0 for +100%, etc.

	float counterAttack; // %-based, Fighter stats
	float doubleAttack; // %-based
	float doubleAttackAtkBonus; // %-based.
	float damageReduction; // %-based.

	int resting; // tick-tock.
	float restingIncrement;  // Tick add tock.
	float restingPercent; // %-based bonus. 0.02f for 2%

	
	int itemsSellable;
	float taxReduction; // % for bazaar
	float npcSellPrice; // % bonus, 0.0 default, +0.05 for +5% etc.

	bool shieldEquipped; // Updated from gear.
	float shieldStatBonus; // o-o will affect shield stat effects, 0.01 for +1%

	float dualWield; // % penalty. 1.0 (+100%) by default. Reduces to .5 at first trait.

	float unarmedDMGBonus; // H2H n MNK stats
	float unarmedSelfDamage; // Default 30%, decreases with traits.
	float blockRate; // %, 0.1f for 10%
	float blockDMGReduction; // % 
	float blockSkillCooldownReduction;
	float kickAttack; // %, 0.05 for 5%
	float kickAttackBonus; // o-o

	float stealRate; // Rogue stats, % on attempts, 0.05 for 5%

	float coldResistance;
	float fireResistance;

	Slayer * slayer; // Slayer stats...?
};

#endif
