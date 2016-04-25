/// Emil Hedemalm
/// 2016-04-23
/// Something one can interact with and target explicitly.

#include "Interactable.h"

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

Vector3f Interactable::Position()
{
	return position;	
}
