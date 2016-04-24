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
	Interactable(){ targetable = true; representationScale = Vector3f(1,1,1); };
	virtual Vector3f Position();
	String name; // Always a name.
	int type; // Character?
	Vector3f position;
	float sortValue;
	bool targetable; // default true. disable before deleting.
	Vector3f representationScale; // For white-boxing, 1.0 default of 2x1 box to represent humanoids.
	
	/// Performs an attack on main target (probably)
	virtual void Attack() {};
	virtual void BecomeUntargetable();
};

#endif
