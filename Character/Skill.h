/// Emil Hedemalm
/// 2016-04-23
/// 

#include "Class.h"
#include "String/AEString.h"

struct Stats;
class Character;

/// Enum of all skills. Details are in separate Skill_ClassName.h 
enum {
	CLASS_0_SKILLS = 0,
	CLASS_1_SKILLS = 100,
	CLASS_2_SKILLS = 200,
	CLASS_3_SKILLS = 300,
	CLASS_4_SKILLS = 400,
	CLASS_5_SKILLS = 500,
	CLASS_6_SKILLS = 600,
	CLASS_7_SKILLS = 700,
	CLASS_8_SKILLS = 800,
	CLASS_9_SKILLS = 900,
	CLASS_10_SKILLS = 1000,
	CLASS_11_SKILLS = 1100,
	CLASS_12_SKILLS = 1200,
	CLASS_15_SKILLS = 1500,
	
	// 0 to 99 are classless skills
	CLASSLESS_SKILLS = CLASSLESS * 100,
	FIGHTER_SKILLS = FIGHTER * 100,
	ACOLYTE_SKILLS = ACOLYTE * 100,
	MONK_SKILLS = CLASS_3_SKILLS,
	RANGER_SKILLS = CLASS_4_SKILLS, 
	WIZARD_SKILLS = CLASS_5_SKILLS, 
	DUELIST_SKILLS = CLASS_6_SKILLS,
	ROGUE_SKILLS = CLASS_7_SKILLS, 
	HOLY_PROTECTOR_SKUKKS = CLASS_8_SKILLS,
	SLAYER_SKILLS = CLASS_9_SKILLS,
	DEATHS_DISCIPLE_SKILLS = CLASS_10_SKILLS,
	MAX_SKILLS = CLASS_15_SKILLS
};

/// Returns description of skill.
String GetSkillDescription(int skill);
char GetSkillRequiredClass(int skill);
char GetSkillRequiredClassLevel(int skill);
/// Queries if activatable - at all 
bool IsActivatable(int skill);
/// Yarp.
void AddPassiveStatBonuses(int skill, char lvl, Stats & baseStats);
/// If cooldown is ready.
bool IsReady(int skill, Character & character);
/// Query to ready skill i, or start casting it for magicks. Returns true if ready.
bool Queue(int skill, Character & character);
/// Starts execution of the skill, animations, dealing damage, etc. Returns true if it started successfully. False if bad/failed request.
bool Start(int skill, char L, Character & character);
/// For those skills taking time. Returns true when it should go to End.
bool OnFrame(int skill, Character & character);
bool End(int skill, Character & character);




