/// Emil Hedemalm
/// 2014-07-27
/// A character.

#ifndef CHARACTER_H
#define CHARACTER_H

#include "MathLib.h"
#include "String/AEString.h"
#include "List/List.h"
#include <utility>
#include "Class.h"

class Buff;
class Item;
class Stats;


/// Class used to store both npc- and player data! Any character should be playable, or at least jump-in and playable by e.g. GMs for testing purposes.
class Character 
{
public:
	Character();

	/// Performs an attack on main target (probably)
	void Attack();

	/// To current class and level.
	void UpdateBaseStatsToClassAndLevel();
	void UpdateGearStatsToCurrentGear();
	void UpdateStatsWithBuffs();

	/// Adds buff. Recalcualtes stats.
	Buff * AddBuff(Buff * newBuff);
	void RemoveBuff(Buff * goBuff);
	char SkillLevel(int skill);
	bool HasBuff(int fromSkill);
	void AddEnmityFor(Character & character, int amount);

	/// o.o
	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);
	String name;
	Vector3f position;
	
	Character * mainTarget;
	List<Character *> engagedFoes;
	List<std::pair<int, int>> enmity; // first int is ID of character, second is amount of enmity.  

	/// List of skills, sorted.
	List<std::pair<int,char>> skills;
	List<std::pair<int,char>> availableSkills; // Skills available for current level/class combination, includes both skill # and level of the skill.
	List<std::pair<int,int>> skillEXP; // Array of all skills being trained.
	int skillTraining; // Integer which skill is currently being trained.
	
	/// Skills currently being evaluated each frame.
	List<int> activeSkills;
	List<std::pair<int,int>> skillCooldownsMs; // Cooldown of skills, in milliseconds. If not in array, skill should be executable.
	
	int money; // Money in total on character.
	List<std::pair<Item*, int>> inventory; // Total amount of items.

	/// If unlocked, class id and class lvl.
	List<std::pair<char,char>> classLvls;
	std::pair<char,char> currentClassLvl;

	/// Slot and item.
	List<std::pair<char,Item*>> gear;
	/// Includes debuffs.
	List<Buff*> buffs;

	Stats * baseStats, * statsWithGear, * statsWithBuffs;

};

#endif
