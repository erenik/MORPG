/// Emil Hedemalm
/// 2016-04-25
/// 

#include "NPC.h"

NPC::NPC()
	: Character(CharacterType::NPC)
{

}
// Default shop-npc
NPC * NPC::Shop()
{
	NPC * n = new NPC();
	n->name = "Shop";
	n->sell = true;
	return n;
}
// Default healer-npc. May later pay to get healing or debuffs removed. Could also be used for skill-resets.
NPC * NPC::Healer()
{
	NPC * n = new NPC();
	n->name = "Healer";
	n->heal = true;
	return n;
}
// Dialogue only? perhaps quests?
NPC * NPC::Talker()
{
	NPC * n = new NPC();
	n->name = "Talker";
	n->talk = true;
	return n;
}
