/// Emil Hedemalm
/// 2016-04-23
/// 

#ifndef SKILL_H
#define SKILL_H

#include "Class.h"
#include "String/AEString.h"

struct Stats;
class Character;
class Buff;

#define SKILLS_PER_CLASS 120
#define SPC SKILLS_PER_CLASS
/// Enum of all skills. Details are in separate Skill_ClassName.h 
enum classSkills {	
	// 0 to 99 are classless skills
	CLASSLESS_SKILLS = CLASSLESS * SPC,
	FIGHTER_SKILLS = FIGHTER * SPC,
	ACOLYTE_SKILLS = ACOLYTE * SPC,
	MONK_SKILLS = MONK * SPC,
	RANGER_SKILLS = RANGER * SPC, 
	WIZARD_SKILLS = WIZARD * SPC, 
	DUELIST_SKILLS = DUELIST * SPC,
	ROGUE_SKILLS = ROGUE * SPC, 
	HOLY_PROTECTOR_SKUKKS = HOLY_PROTECTOR * SPC,
	SLAYER_SKILLS = SLAYER * SPC,
	DEATHS_DISCIPLE_SKILLS = DEATHS_DISCIPLE * SPC,

	MAX_SKILLS = DEATHS_DISCIPLE * SPC + SPC,
};

/// Returns description of skill.
String GetSkillName(int skill); // Text
String GetSkillDescription(int skill);
int GetSkillByName(String name);
char GetSkillRequiredClass(int skill);
char GetSkillRequiredClassLevel(int skill);
int GetPreparationTimeMs(int skill, char lvl, Character * character);
int GetSkillMultiplier(int skill); // Traning multiplier
/// Queries if activatable - at all 
bool IsActivatableCombatSkill(int skill);
/// Yarp.
void AddPassiveStatBonuses(int skill, char lvl, Stats & baseStats);
void AddStatsFromBuff(Buff * buff, Character * character); // Adds stats from buff, usually applied once, but may be re-applied as fit.
/// If cooldown is ready.
bool IsReady(int skill, Character * character);
/// Query to ready skill i, or start casting it for magicks. Returns true if ready.
bool StartPreparing(int skill, char L, Character * character);
/// Starts execution of the skill, animations, dealing damage, etc. Returns true if it started successfully. False if bad/failed request.
bool Start(int skill, char L, Character * character);
/// For those skills taking time. Returns true when it should go to End.
bool OnFrame(int skill, Character * character);
bool End(int skill, Character * character);



#endif
