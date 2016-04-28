/// Emil Hedemalm
/// 2016-04-23
/// Something one can interact with and target explicitly.

#include "Interactable.h"
#include "App/MORPG.h"
#include "Character/Character.h"
#include "Properties/CharacterProperty.h"

Interactable::Interactable()
{ 
	targetable = true; representationScale = Vector3f(1,1,1); 
	sell = talk = heal = false;
	foe = false;
};


void Interactable::BecomeUntargetable()
{
	targetable = false;
}

/// Requests from ch
void Interactable::RequestHeal(Character *ch)
{
	morpg->Log(name+": Heal!");
	ch->prop->HealHp(9999);
	ch->prop->HealMp(9999);
}
void Interactable::RequestBuy(Character * ch)
{
	morpg->Log(name+": Hi. I sell some stuff.");
	morpg->OpenShop(this->shop);
}
void Interactable::RequestTalk(Character * withChar)
{
	// Check dialogue thingy.
	// Placehold.
	morpg->Log(name+": Hi "+withChar->name);
	// morpg->StartConversation();
}

Vector3f Interactable::Position()
{
	return position;	
}
