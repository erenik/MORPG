/// Emil Hedemalm
/// 2014-07-27
/// Global state used for this multiplayer online RPG game.

#include "AppStates/AppState.h"

class MORPGSession;
class Zone;
class CharacterProperty;
class Character;


extern Camera * mapPreviewCamera;
extern Camera * firstPersonCamera;
extern MORPGSession * session;

extern float testMultiplier;

class MORPG : public AppState 
{
public:
	MORPG();
	virtual ~MORPG();

	/// Function when entering this state, providing a pointer to the previous StateMan.
	void OnEnter(AppState * previousState);
	/// Main processing function, using provided time since last frame.
	void Process(int timeInMs);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	void OnExit(AppState * nextState);

	/// Creates the user interface for this state
	virtual void CreateUserInterface();

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();

	/// Load map/zone. Leave old one. 
	virtual void EnterZone(Zone * zone);

	Character * HUDCharacter();

	/// UI stuffs.
	void OpenHUD(Character * forCharacter);
	void UpdateHUD();
	void UpdateHPInHUD();
	void UpdateTargetInHUD(); // Target sub-section of HUD.
	bool InteractionMenuOpen() { return iMenuOpen;};
	void OpenInteractionMenu();
	void CloseInteractionMenu();
	enum {
		TARGET_SKILLS,	// Also for later.
		SELF_SKILLS,	// Self-only. Added later maybe with config, now now.
		COMBAT_SKILLS, // All skills that are relevant for or near combat, healing, offensive, most non-passive.
	};
	void OpenSubIMenuMenu(int whichMenu);
	void CloseSubIMenuMenu();
	void Log(CTextr text);

	/// Chat or command line.
	void EvaluateLine(String cmd);
	/// Show with cool arrows, yes.
	void OnTargetUpdated();

private:

	UserInterface * hud;
	void NextTarget();
	void PreviousTarget();

	bool hudOpen;
	bool iMenuOpen;
	int subMenuOpen; // -1 when none open.

	/// The character property of the character we are currently controlling (both camera focus and input-focus!)
	CharacterProperty * characterProp;
	Character * character;
#define hudCharacter characterProp
};

extern MORPG * morpg;
