/// Emil Hedemalm
/// 2016-04-23
/// All stats of something to combat with.

#ifndef STAT_H
#define STAT_H

#include "MathLib/AEMath.h"

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

	int stunDurationMs; // Currently stunned?! Yes.
	int disarmDurationMs; // Currently disarmed?! Yes.
	int paralyzedMs; // like stun

	int weaponType; // Depends on equipped gear. Here for simplicities' sake.
	int level; /// Max level, used for some calculations.
	int cls; // class
	int family; // monster family.

	float regen; // Regen HP per second.
	float refresh; // Refresh MP per second.

	float hp, mp;
	int maxHp, maxMp;
	char str, vit, mag;
	float accuracy, evasion;
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
	float attackSpeed; // Has its own forumale to affect weapon delay.

	float counterAttack; // %-based, Fighter stats
	float doubleAttack; // %-based
	float doubleAttackAtkBonus; // %-based.
	float damageReduction; // %-based.

	int resting; // tick-tock.
	float restingPercent; // %-based bonus.

	float dualWield; // % penalty. 1.0 (+100%) by default. Reduces to .5 at first trait.

	float unarmedDMGBonus; // H2H n MNK stats
	float unarmedSelfDamage; // Default 30%, decreases with traits.
	float blockRate; // %
	float blockDMGReduction; // % 

	float stealRate; // Rogue stats, % on attempts

	float coldResistance;
	float fireResistance;

	Slayer * slayer; // Slayer stats...?
};

#endif
