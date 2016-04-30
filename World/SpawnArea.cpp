/// Emil Hedemalm
/// 2016-04-30
/// Area spawning of mobs within zones.

#include "SpawnArea.h"
#include "Zone.h"
#include "Character/Foe.h"
#include "Character/Family.h"
#include "App/MORPG.h"

/// Creates a default spawn area centered on 0,0, spawning in +/-10 in X/Z. Aims to keep population at 10 individuals by default.
SpawnArea::SpawnArea(Zone * zone, int family, int minLvl, int maxLvl)
	: zone(zone), family(family), minLvl(minLvl), maxLvl(maxLvl)
{
	currMinLvl = minLvl;
	currMaxLvl = maxLvl;

	area.type = EmitterType::PLANE_XZ;
	area.SetScale(10.f);
	/// o-o
	desiredPopulation = 10;

	List<Foe*> individuals;
	/// Time since last spawn. Counter that is reset occasionally.
	timeSinceLastSpawnMs = 0;
	msPerSpawn = 300000;
}

Random popRand;

// If true: On server start, spawns all. Else: spawns 1 or a group of individuals to meet desired population.
void SpawnArea::Spawn(bool initial)
{
	int num = 1;
	if (initial)
		num = desiredPopulation;

	/// Populate with some monsters too?
	for (int i = 0; i < num; ++i)
	{
		Foe * foe = new Foe(SHIELDLING, (int) (popRand.Randf() * (currMaxLvl - currMinLvl) + currMinLvl));
		Vector3f position = Vector3f(popRand.Randf(20)-10,0,popRand.Randf(20)-10); 
		foe->position = position;
		zone->AddCharacter(foe);
		foe->sa = this;
		individuals.AddItem(foe);
	}
}

/// Could perhaps be called every minute to keep down processing? No?
void SpawnArea::Process(int timeInMs)
{
	/// If long time no kill, reduce min/maxLvl?
	timeSinceLastSpawnMs += timeInMs;
	if (timeSinceLastSpawnMs > msPerSpawn * 3)
	{
		--currMaxLvl;
		--currMinLvl;
		// Max 10 lvl deviation from base.
		ClampInt(currMaxLvl, maxLvl, maxLvl + 10);
		ClampInt(currMinLvl, minLvl, minLvl + 10);
	}

	/// Spawns below.
	if (individuals.Size() >= desiredPopulation)
		return;
	int individualsLacking = desiredPopulation - individuals.Size();
	int msToNext = msPerSpawn / individualsLacking * testMultiplier;
	// Faster spawn if many individuals lacking.
	if (timeSinceLastSpawnMs > msToNext)
	{
		// Increase max level and min each spawn?
		++currMinLvl;
		++currMaxLvl;
		// Max 10 lvl deviation from base.
		ClampInt(currMaxLvl, maxLvl, maxLvl + 30);
		ClampInt(currMinLvl, minLvl, minLvl + 30);

		Spawn(false);
		timeSinceLastSpawnMs = 0;
	}
}


