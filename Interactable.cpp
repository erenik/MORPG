/// Emil Hedemalm
/// 2016-04-23
/// Something one can interact with and target explicitly.

#include "Interactable.h"

void Interactable::BecomeUntargetable()
{
	targetable = false;
}

Vector3f Interactable::Position()
{
	return position;	
}
