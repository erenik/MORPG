/// Emil Hedemalm
/// 2016-04-23
/// Default character you would battle against.

#ifndef FOE_H
#define FOE_H

#include "Character.h"
#include "Family.h"

class SpawnArea;

class Foe : public Character
{
public:
	Foe(int family, int level);
	virtual ~Foe();
	/// See family above. Species are pretty much just to distinguish which level-bracket they should spawn in, which zones, etc.
	int family, species;
	/// If spawned in one.
	SpawnArea * sa;

	/// Called internally, If returns non-0 value, will not add class-specific base stats.
	//	UPDATE_STATS_FOR_CLASS, // Default, will apply current class stats for given level.
	//	UPDATE_TRAITS_FOR_CLASS_ONLY, // If only traits are wanted on e.g. a Shieldling, but not the added stats from the class.
	//	UPDATE_NO_CLASS_STATS, // For 0 class interference. All manual override in *Alternative() functions.
	virtual int UpdateBaseStatsAlternative(); 

};



enum shieldlingSpecies 
{
	TINY_SHIELDLING, // L -2 to 5
	SMALL_SHIELDLING, // L 6-10
	MEDIUM_SHIELDLING, // L 11-25
	/// Different species after L 25, add specific skills or jobs for them.
	IRON_SHIELDLING, // L26-35
	STEEL_SHIELDLING, // L36-45
	MITHRIL_SHIELDLING, // L46-55
	OBSIDIAN_SHIELDLING, // L56-65
	TITANIUM_SHIELDLING, // L66-75
	EMERALD_SHIELDLING, // L76-85
	CHROMIUM_SHIELDLING, // L86-95
	DIAMOND_SHIELDLING, // 96-105
	ADAMANTIUM_SHIELDLING, // L106-115
};

enum spikelingSpecies 
{
	TINY_SPIKELING, // L 3-7
	SMALL_SPIKELING, // L 8-13
	MEDIUM_SPIKELING, // 14-20
};

#endif