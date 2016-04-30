/// Emil Hedemalm
/// 2016-04-30
/// Area spawning of mobs within zones.

#ifndef SPAWN_AREA_H
#define SPAWN_AREA_H

#include "MathLib/Emitter.h"

class Foe;
class Zone;

/// A spawn area for a particular type of foe.
class SpawnArea 
{
public:
	/// Creates a default spawn area centered on 0,0, spawning in +/-10 in X/Z. Aims to keep population at 10 individuals by default.
	SpawnArea(Zone * zone, int family, int minLvl, int maxLvl);
	// If true: On server start, spawns all. Else: spawns 1 or a group of individuals to meet desired population.
	void Spawn(bool initial); 
	/// Could perhaps be called every minute to keep down processing? No?
	void Process(int timeInMs);
	/// o-o
	List<Foe*> individuals;
	int desiredPopulation; // Desired?
	int minLvl, maxLvl, currMinLvl, currMaxLvl; // For adapting to players..?
	int family;
	/// Time since last spawn. Counter that is reset occasionally.
	int timeSinceLastSpawnMs;
	/// MS between spawns when under desired population. Default 300s (300000)
	int msPerSpawn;
	/// Center position -> add into emitter?
	Vector3f pos;
	/// Area defined how? Emitter? Replace with list of emitters later perhaps. Or have an emitter containing emitters.
	Emitter area;
	/// Zone it's in.
	Zone * zone;
};

#endif
