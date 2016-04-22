/// Emil Hedemalm
/// 2016-04-23
/// Enum of skills.

#include "Class.h"

enum 
{
	BERSERK = FIGHTER * 100, // available at lvl 0.
	CHARGE,
	PROVOKE,
	DEFENDER,
	DEFENSIVE_TRAINING,
	OFFENSIVE_TRAINING, // all 5 available at lvl 1 to 5.

	DOUBLE_ATTACK = FIGHTER + 15,
	RUSH,
	DEFENSIVE_TRAINING_II,
	OFFENSIVE_TRAINING_II,
	OFFENSIVE_STANCE,

	TEMPO_TRAINING = FIGHTER + 30,
	GOAD,
	DOUBLE_ATTACK_II,
	WEAPON_BASH,
	RUSH_II,

	RUSH_III = FIGHTER + 50,
	STALWARD_DEFENDER, 

};