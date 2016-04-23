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
#include "Buff.h"
#include "Interactable.h"

class Buff;
class Item;
struct Stats;
class CharacterProperty;

enum weaponTypes 
{
	UNARMED,
	BEAST_WEAPON, // Claws, etc., divide further later.
	DAGGER,
	SWORD,
};

String WeaponTypeString(int type);

int TNLClass(int level);
int TNLWeapon(int level);

/// Class used to store both npc- and player data! Any character should be playable, or at least jump-in and playable by e.g. GMs for testing purposes.
class Character : public Interactable
{
private:
	static int idEnumerator;
public:
	Character(); // Generates unique ID. Host-only constructor.
	virtual ~Character();
	const int id; // Unique identifier.
	int ID() const { return id; };

	int characterType;
	enum {
		PLAYER,
		FOE,
		NPC,
	};
	/// Spawns entity + property.
	void Spawn(ConstVec3fr position);
	void SetCameraFocus();


	void BecomeUntargetable();

	/// Returns level.
	int WeaponTrainingLevel(int type);
	void SetWeaponTrainingLevel(int type, int lvl);
	/// Reached level?
	int GainWeaponExp(int amount);
	/// Returns non-0 if reaching target level. 0 if same level.
	int GainExp(int amount);

	/// To current class and level.
	void UpdateBaseStatsToClassAndLevel();
	/// Called internally, If returns non-0 value, will not add class-specific base stats.
	enum {
		UPDATE_STATS_FOR_CLASS, // Default, will apply current class stats for given level.
		UPDATE_TRAITS_FOR_CLASS_ONLY, // If only traits are wanted on e.g. a Shieldling, but not the added stats from the class.
		UPDATE_NO_CLASS_STATS, // For 0 class interference. All manual override in *Alternative() functions.
	};
	virtual int UpdateBaseStatsAlternative(); 
	virtual void UpdateBaseStatsForClass(); // Called internally for all 
	void UpdateGearStatsToCurrentGear();
	void UpdateStatsWithBuffs();

	Buff * AddBuff(Buff * newBuff);
	void RemoveBuff(Buff * goBuff);
	bool HasBuff(int fromSkill);

	/// Level, part of the 
	int Level() const {return currentClassLvl.second;};
	void HealTick();

	/// Adds buff. Recalcualtes stats. Already created when adding.
	char SkillLevel(int skill);

	/// o.o
	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);
	
	/// List of skills, sorted.
	List<std::pair<int,char>> skills;
	List<std::pair<int,int>> skillEXP; // Array of all skills being trained.
	int skillTraining; // Integer which skill is currently being trained.
		
	int money; // Money in total on character.
	List<std::pair<Item*, int>> inventory; // Total amount of items.

	/// If unlocked, class id and class lvl.
	List<std::pair<char,char>> classLvls;
	List<std::pair<char,int>> classExp; // If unlocked, exp gained in current class towards next level.
	std::pair<char,char> currentClassLvl;

	List<std::pair<char,char>> weaponLvls; // Lvl in weapons. 0 start.
	List<std::pair<char,int>> weaponExp; // exp in weapon type.

	/// Skills currently being evaluated each frame.
	List<int> activeSkills;
	List<std::pair<int,char>> availableSkills; // Skills available for current level/class combination, includes both skill # and level of the skill.

	/// Slot and item.
	List<std::pair<char,Item*>> gear;

	/// Includes debuffs.
	List<Buff*> buffs;
	List<std::pair<int,int>> skillCooldownsMs; // Cooldown of skills, in milliseconds. If not in array, skill should be executable.
	Stats * baseStats, * statsWithGear; // stats should link to statsWithBuffs
	Stats * statsWithBuffs, * stats;

	/// yarr.
	CharacterProperty * prop;
};

#endif
