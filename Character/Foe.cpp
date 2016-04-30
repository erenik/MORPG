/// Emil Hedemalm
/// 2016-04-23
/// Default character you would battle against.

#include "Foe.h"
#include "Stats.h"
#include "Family.h"
#include "World/SpawnArea.h"

Foe::Foe(int family, int level)
	: Character(CharacterType::FOE), family(family)
{
	characterType = CT::FOE;
	/// Inherit name from family or species.
	name = "Foe";
	currentClassLvl.second = level;
	foe = true;
	sa = 0;
}

Foe::~Foe()
{
	// Notify spawnArea.
	if (sa)
		sa->individuals.RemoveItem(this);
}



/// Called internally, If returns non-0 value, will not add class-specific base stats.
//	UPDATE_STATS_FOR_CLASS, // Default, will apply current class stats for given level.
//	UPDATE_TRAITS_FOR_CLASS_ONLY, // If only traits are wanted on e.g. a Shieldling, but not the added stats from the class.
//	UPDATE_NO_CLASS_STATS, // For 0 class interference. All manual override in *Alternative() functions.
int Foe::UpdateBaseStatsAlternative()
{
	Stats & bs = *baseStats;
	bs.level = currentClassLvl.second;
	bs.cls = currentClassLvl.first;
	bs.family = family;
	bs.weaponType = BEAST_WEAPON;

	int l = Level();
	/// Base stats.
	float strL, vitL, hpL, mpL, magL = 0.5f, MdefL = 0.5f, hppL = 0, mppL = 0, dmgL = 0.5f, attL = 1.f, defL = 1.f;
	switch(family)
	{
		case SHIELDLING: bs.str = 3; bs.vit = 5; bs.damage = 1; bs.maxHp = 20; bs.attack = 6; bs.defense = 8;// Initial
			strL = 1.1f; vitL = 2.2f; dmgL = 1.f; attL = 2.f; hpL = mpL = 3; hppL = 1.3f; mppL = 0; defL = 3.7f; 
			if (l < 3) { species = TINY_SHIELDLING; name = "Tiny shieldling";}
			else if (l < 11) {	species = SMALL_SHIELDLING; name = "Small shieldling"; }
			representationScale.x = representationScale.z = 0.5f + 0.01f*l;
			representationScale.y = 0.1f + 0.02f * MaximumFloat(l, 0);
			break; // Per L
		default:
			assert(false);
	}
	int L = currentClassLvl.second;
	bs.str += int(strL * L);
	bs.vit += int(vitL * L);
	bs.maxHp += int(hpL * L);
	bs.mag += int(magL * L);
	bs.mdef += int(MdefL * L);
	bs.maxMp += int(mpL * L);
	bs.maxHp = (int) ((1.0f + hppL * 0.01f * (float)L) * bs.maxHp);
	bs.maxMp = (int) ((1.0f + mppL * 0.01f * (float)L) * bs.maxMp);
	bs.damage += (int) (dmgL * L);
	bs.attack += int(attL * L);
	bs.defense += int(defL * L);
	std::cout<<"\nHP: "<<bs.maxHp;


	/// Clamp minimum/max values.
	ClampFloat(bs.damage, 1, 255);
	ClampFloat(bs.maxHp, 1,99999);
	ClampFloat(bs.defense, 1, 999);
	ClampFloat(bs.attack, 1, 999);
	// Add some base attack/defense based on level and str/vit?

	/// Passive Traits
	int tier = currentClassLvl.second / 5;

	// Copy stats onto gear preemtively.
	*statsWithGear = bs;

	return UPDATE_NO_CLASS_STATS;
}
