/// Emil Hedemalm
/// 2016-04-23
/// All stats of something to combat with.

#ifndef STAT_H
#define STAT_H

struct Stats;
class Character;

struct Buff 
{
	Buff(int skill, char L) : skill(skill), L(L), durationAttacks(-1), durationMs(-1) {};
	static Buff * Attacks(int skill, char L, int attacks){ Buff * b = new Buff(skill, L); b->durationAttacks = attacks; return b;};
	void OnAttack(Character & character);
	void OnTimeElapsed(int ms, Character & character);
	void AddActiveBonuses(Stats * buffedStats);
	
	int skill; // Skill that caused it.
	char L; // Skill-level.
	int durationAttacks; // -1 if not used.
	int durationMs;
	float multiplier;
};

struct Slayer 
{
	int primary, secondary, teriary;
	// Bonuses.
	// ...
};

struct Stats {
	Stats()
	{	// Default values.
		movementSpeed = 2.f; stunDurationMs = disarmDurationMs = paralyzedMs = 0; regen = refresh = 0; hp = mp = 13; maxHp = maxMp = 13;
		str = vit = mag = 10;
		accuracy = 0.6; // default 60% acc with 0 weapon skills.
		evasion = 0; // is bonus, default miss is the same as evade.
		attackBonus = defenseBonus = enmity = enmityDecayReduction = 0; // bonuses compared to default of 100%.
		attack = defense = mdef = 1; // Default values. Minimum L for attack/defense, mdef varies more per class.
		parryingRate = disarmRate = criticalDmgBonus = enemyCriticalHitRate = healingReceivedBonus = damageBonusP = 0;
		criticalHitRate = 3;
		damage = 1; // default minimum for unarmed.
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
	// Determines how it is to play them. Used actively in the MORPGCharacterProperty. Default while testing: 2. meters per second.
	float movementSpeed;

	int stunDurationMs; // Currently stunned?! Yes.
	int disarmDurationMs; // Currently disarmed?! Yes.
	int paralyzedMs; // like stun

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
	int weaponDelay; // Based on weapon, maybe 1 or 2 skills may affect.
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
