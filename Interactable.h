/// Emil Hedemalm
/// 2016-04-23
/// Something one can interact with and target explicitly.

#ifndef INTERACTABLE_H
#define INTERACTABLE_H

#include "String/AEString.h"
#include "MathLib.h"

class Character;
class Shop;
class Dialogues;
class HealingService;

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

	/// Requests from ch
	virtual void RequestHeal(Character *ch);
	virtual void RequestBuy(Character * ch);
	virtual void RequestTalk(Character * ch);

	/// Implies if text for damage etc. should be displayed in a pleasant or scary way (red for allies). Default true for all but Foes.
	bool foe;

	/// If can sell stuff to player.
	bool sell;
	/// If can talk, dialogue.
	bool talk;
	/// If when speaking to, will or can offer healing services.
	bool heal;
	// If sell, or buy, has shop. Includes all inventory, prices, etc.
	Shop * shop; 
	// If talk, has dialogues, including prerequisites, prioritization order, etc.
	Dialogues * diags;
	/// If heal, has some statistics concerning what healing is provided, at what rates, cooldowns, etc.
	HealingService * healService; 
};

class Item;

class Shop
{
public:
	/// Inventory of the shop.
	List<std::pair<Item *, int>> inventory;
	/// Rate at which shop sells items to player. Default 1.f or 100%. May be reduced via skill trainings.
	float sellingRate;
	/// Rate at which shop buys items from player. Default 0.25f or 25% of base value. May be increased via skill trainings.
	float buyingRate;
};
class Dialogues
{
public:
};
class HealingService
{
public:
};


#endif
