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
	Interactable();
	virtual Vector3f Position();

	String name; // Always a name.
	int type; // Character?
	
	Vector3f position; // Position in zone, saved when logging out. May be used to calculate sneak attacks, etc.
	int zoneID;	// ID of current zone character was in when logging out.

	float sortValue;
	bool targetable; // default true. disable before deleting.
	Vector3f representationScale; // For white-boxing, 1.0 default of 2x1 box to represent humanoids.
	
	/// Performs an attack on main target (probably)
	virtual void Attack() {};
	virtual void BecomeUntargetable();

	/// Implies if text for damage etc. should be displayed in a pleasant or scary way (red for allies). Default true for all but Foes.
	bool foe;

	/// If can sell stuff to player.
	bool sell;
	/// If can talk, dialogue.
	bool talk;
	/// If when speaking to, will or can offer healing services.
	bool heal;

};

#endif
