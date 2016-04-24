/// Emil Hedemalm
/// 2016-04-23
/// Buffs.

#ifndef BUFF_H
#define BUFF_H

struct Stats;
class Character;
class MORPGCharacterProperty;

class Buff 
{
public:
	Buff(int skill, char L);
	static Buff * Attacks(int skill, char L, int attacks){ Buff * b = new Buff(skill, L); b->durationAttacks = attacks; return b;};
	static Buff * Seconds(int skill, char L, int seconds){ Buff * b = new Buff(skill, L); b->durationMs = seconds * 1000; return b;};
	/// Returns true if it was removed from the character due to expiration.
	bool OnAttack(Character * character);
	void OnTimeElapsed(int ms, Character * character);
	
	int skill; // Skill that caused it.
	char L; // Skill-level.
	int durationAttacks; // -1 if not used.
	int durationMs;
	int msSoFar;
	float multiplier;
	bool removeThis;
};

#endif
