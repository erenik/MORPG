/// Emil Hedemalm
/// 2016-04-23
/// Something one can interact with and target explicitly.

#ifndef INTERACTABLE_H
#define INTERACTABLE_H

#include "String/AEString.h"
#include "MathLib.h"

class MORPGCharacterProperty;

class Interactable 
{
public:
	Interactable(){ targetable = true; };
	String name; // Always a name.
	int type; // Character?
	Vector3f position;
	bool targetable; // default true. disable before deleting.
	
	/// Performs an attack on main target (probably)
	virtual void Attack() {};
	virtual void BecomeUntargetable();
};

#endif
