/// Emil Hedemalm
/// 2014-08-01
/// Binds the Entity- and custom MORPG character objects together nicely.

#include "Entity/EntityProperty.h"
#include "MathLib.h"

class Buff;
struct Stats;
class Interactable;
class Character;

#define CharProp CharacterProperty
#define MCharProp CharacterProperty
class CharProp : public EntityProperty 
{
public:
	CharProp(Entity * characterEntity, Character * associatedWithCharacter);
	virtual ~CharProp();
	/// Time passed in seconds..! Will steer if inputFocus is true.
	virtual void Process(int timeInMs);

	/// Removing targets, links, enmities.
	void OnDeath(); // On dying, stop somethings, pull down some menus.
	void OnCharacterDefeated(Character * c);
	bool TargetAlive();
	void Engage();
	void ToggleAutorun();
	void ToggleHeal();

	void LoseTarget();
	Interactable * GetTarget();
	void SetTarget(Interactable * i);

	/// Performs an attack on main target (probably)
	virtual void Attack();
	virtual void AddEnmityFor(Character & character, int amount);

	// Default false. Enable to steer this entity.
	bool inputFocus;

	Interactable * mainTarget;
	Character * MainTarget() {return (Character*) mainTarget;};
	List<Character *> engagedFoes;
	/// first int is ID of character, second is amount of enmity.
	List<std::pair<int, int>> enmity;   

	Character * ch;

	/// True when should be removed.
	bool deleteMe;
private:
	/** Checks states via InputManager. Regular key-bindings should probably still be defined in the main game state 
		and then passed on as messages to the character with inputFocus turned on.
	*/
	void ProcessInput(); 

	// Moved to character.
//	Interactable * target;

	/// For handling movement.
	Vector3f lastAcc;
	float lastRight;
	bool autorun;
	bool healing;
	bool attacking;
	bool targetLocked;
	bool dead;
	short deadTimeMs;
	short healingMs;
	short attackCooldown;
};




