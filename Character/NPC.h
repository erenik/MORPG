/// Emil Hedemalm
/// 2016-04-23
/// stuff o-o

#include "Character.h"

class NPC : public Character
{
public:
	NPC();
	static NPC * Shop(); // Default shop-npc
	static NPC * Healer(); // Default healer-npc. May later pay to get healing or debuffs removed. Could also be used for skill-resets.
	static NPC * Talker(); // Dialogue only? perhaps quests?
//	static NPC * 
};
