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


	void HealTick();
	/// Heals, notifies GUI etc. as needed. Float in order to work with regen, etc.
	void HealHp(float amount);
	void HealMp(float amount);
	bool IsAttacking() const { return attacking; };
	/// o-o
	void Revive();
	/// Removing targets, links, enmities.
	void DisableMovement(); // Handles it.
	void OnDeath(); // On dying, stop somethings, pull down some menus.
	void OnCharacterDefeated(Character * c);
	bool TargetAlive();
	void Engage();
	void Disengage();
	void ToggleAutorun();
	void ToggleHeal();
	void LockTarget(); // Functions since they may affect such things as camera, auto-run mechanics, etc.
	void UnlockTarget(); 

	void LoseTarget();
	Interactable * GetTarget();
	void SetTarget(Interactable * i);
	/// Make sure entity is visible (if it has camera focus).
	void UpdateCamera();

	/// Performs an attack on main target (probably). Returns true if target died. false if still alive for processing more attacks.
	virtual bool Attack();
	virtual void AddEnmityFor(Character * character, int amount);

	/// For when toggling, n target locking, etc.
	void UpdateAutorun();
	// Default false. Enable to steer this entity.
	bool inputFocus;

	int healTicks; // How many times resting has occured in succession.
	Interactable * mainTarget;
	Character * MainTarget() {return (Character*) mainTarget;};
	List<Character *> engagedFoes;
	/// first int is ID of character, second is amount of enmity.
	List<std::pair<int, int>> enmity;   

	Character * ch;

	/// True when should be removed.
	bool deleteMe;

	/// For preparing skills
	int preparationTimeMs;
	int activeSkill; // Active skill being prepared.

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
	bool movementEnabled;
	bool autorun;
	bool healing;
	bool attacking;
	bool targetLocked;
	bool dead;
	short deadTimeMs;
	short healingMs;
	short attackCooldown;
};




