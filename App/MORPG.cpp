/// Emil Hedemalm
/// 2014-07-27
/// Global state used for this multiplayer online RPG game.

#include "MORPG.h"

#include "MHost.h"

#include "MORPGSession.h"

#include "Character/Foe.h"
#include "Character/Stats.h"
#include "Character/PlayerCharacter.h"
#include "Character/Family.h"

#include "Random/Random.h"
#include "Properties/CharacterProperty.h"

#include "World/Zone.h"
#include "World/World.h"

#include "Physics/MORPGIntegrator.h"
#include "Physics/MORPGCD.h"
#include "Physics/MORPGCR.h"

#include "Application/Application.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Camera/Camera.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Input/InputManager.h"
#include "Input/Action.h"

#include "Model/Model.h"

#include "Texture.h"
#include "TextureManager.h"

#include "Model/ModelManager.h"

#include "StateManager.h"

#include "UI/UserInterface.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"

#include "Maps/MapManager.h"

#include "Network/NetworkManager.h"

#include "Script/Script.h"
#include "Script/ScriptManager.h"

/// Only one such session active per application.
MORPGSession * session = NULL;

Camera * mapPreviewCamera = NULL;
Camera * firstPersonCamera = NULL;

MORPG * morpg = 0;

List<Camera*> cameras;

MHost * hostState = NULL;

void RegisterStates()
{
	MORPG * global = new MORPG();
	hostState = new MHost();
	StateMan.RegisterState(global);
	StateMan.RegisterState(hostState);
	StateMan.QueueGlobalState(global);
}

void SetApplicationDefaults()
{
	Application::name = "Time and time again - a MORPG sandbox";
	TextFont::defaultFontSource = "img/fonts/font3.png";
}

Random morpgRand;

MORPG::MORPG()
{
	characterProp = NULL;
	hudCharacter = 0;
	hudOpen = false;
	iMenuOpen = false;
	morpg = this;
	hud = 0;
}

MORPG::~MORPG()
{
	// Delete the world..!
	world.Delete();
}


/// Function when entering this state, providing a pointer to the previous StateMan.
void MORPG::OnEnter(AppState * previousState)
{
	// Set integrator
	Physics.QueueMessage(new PMSet(new MORPGIntegrator()));
	Physics.QueueMessage(new PMSet(new MORPGCD()));
	Physics.QueueMessage(new PMSet(new MORPGCR()));

	if (!mapPreviewCamera)
		mapPreviewCamera = CameraMan.NewCamera("MapPreviewCamera", true);
	if (!firstPersonCamera)
		firstPersonCamera = CameraMan.NewCamera("FirstPersonCamera", true);
	cameras.Add(mapPreviewCamera);
	cameras.Add(firstPersonCamera);

	Graphics.QueueMessage(new GMSetCamera(mapPreviewCamera, CT_ROTATION, Vector3f()));
	Graphics.QueueMessage(new GMSetCamera(mapPreviewCamera, CT_DISTANCE_FROM_CENTER_OF_MOVEMENT, 3.f));
	Graphics.QueueMessage(new GMSetCamera(firstPersonCamera, CT_TRACKING_MODE, TrackingMode::FOLLOW_AND_LOOK_AT));
	QueueGraphics(new GMSetCamera(firstPersonCamera, CT_SMOOTHING, 0.5f)); 
	// Set it to follow and track us too.
	
	session = new MORPGSession();
	NetworkMan.AddSession(session);

	InputMan.ForceNavigateUI(true);

	/// World map... 
//	worldMapEntity = MapMan.CreateEntity("World map entity", NULL, NULL);

	// Set up ui.
	if (!ui)
		CreateUserInterface();

	Graphics.QueueMessage(new GMSetUI(ui));
	Graphics.QueueMessage(new GMSetOverlay(NULL));

	// Run OnEnter.ini if such a file exists.
	Script * script = new Script();
	script->Load("OnEnter.ini");
	ScriptMan.PlayScript(script);
//	MesMan.QueueMessages(lines);
}

/// Main processing function, using provided time since last frame.
void MORPG::Process(int timeInMs)
{
	world.Process(timeInMs);
	Sleep(5);
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void MORPG::OnExit(AppState * nextState)
{
	// Will never be called.
}

/// Creates the user interface for this state
void MORPG::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/MainMenu.gui");
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void MORPG::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg == "OnReloadUI")
			{
				if (hudOpen)
					UpdateHUD();
				iMenuOpen = false;
			}
			if (msg == "NewWorld")
			{
				hostState->enterMode = MHost::WORLD_CREATION;
				StateMan.QueueState(hostState);
			}
			else if (msg == "ToggleAutorun")
			{
				// Get our player! o.o
				if (characterProp)
					characterProp->ToggleAutorun();
			}
			else if (msg == "ToggleHeal" && characterProp)
				characterProp->ToggleHeal();
			else if (msg == "NextTarget" && characterProp)
				NextTarget();
			else if (msg == "PreviousTarget" && characterProp)
				PreviousTarget();
			else if (msg == "Interact" && hudCharacter)
				OpenInteractionMenu();
			else if (msg == "IMenuClosed")
				iMenuOpen = false;
			else if (msg == "Target skills")
			{
				// Open menu for it.
				OpenSubIMenuMenu(TARGET_SKILLS);
			}
			else if (msg == "Attack")
			{
				// Attack! 
				characterProp->Engage();
			}
			else if (msg == "Check")
			{
				if (characterProp->mainTarget == 0)
					return;
				// For characters..
				int lvlDiff = characterProp->MainTarget()->currentClassLvl.second - character->currentClassLvl.second;
				String end;
				if (lvlDiff == 0)
					end = "like an even match";
				else if (lvlDiff > 0)
					end = "tough";
				else if (lvlDiff > -3) end = "like a decent challenge";
				else end = "like easy prey.";
#define TARGET_NAME characterProp->MainTarget()->name
				Log("The "+TARGET_NAME+" seems "+end+".");
			}
			else if (msg.Contains("NextCamera"))
			{
				// Get active camera.
				CameraMan.NextCamera();		
			}
			else if (msg.Contains("LoadMap"))
			{
				String mapName = msg.Tokenize(":")[1];
				Zone * zone = world.GetZoneByName(mapName);
				EnterZone(zone);
			}
			else if (msg.Contains("EnterRandomZone"))
			{
				EnterZone(world.zones[morpgRand.Randi(world.zones.Size())]);
			}
			else if (msg == "NewGame")
			{
				// Load a zone!
				String username = "UserName1";
				String password = "password";
				session->Login(username, password);
			}
			break;
		}
	}
}

/// Creates default key-bindings for the state.
void MORPG::CreateDefaultBindings()
{
	InputMapping * mapping = &this->inputMapping;
	List<Binding*> & bindings = mapping->bindings;
	mapping->bindings.Add(new Binding(Action::FromString("ToggleAutorun"), KEY::R));
	mapping->bindings.Add(new Binding(Action::FromString("ToggleHeal"), KEY::H));
	bindings.AddItem(new Binding(Action::FromString("NextTarget"), KEY::TAB));
	bindings.AddItem(new Binding(Action::FromString("PreviousTarget"), List<int>(KEY::SHIFT, KEY::TAB)));
	bindings.AddItem(new Binding(Action::FromString("Interact"), KEY::ENTER));
}

List<Interactable*> GetInteractablesOnScreen()
{
	// Get all for now.
	List<Interactable *> ints;
	for (int i = 0; i < world.interactables.Size(); ++i)
	{
		if (world.interactables[i]->targetable)
			ints.AddItem(world.interactables[i]);
	}
	return ints;
}

void MORPG::NextTarget()
{
	// Grab all on screen?
	List<Interactable*> ints = GetInteractablesOnScreen();
	if (ints.Size() == 0)
		this->characterProp->SetTarget(0);
	Interactable * currT = characterProp->GetTarget();
	if (currT == 0)
		characterProp->SetTarget(ints[0]);
	for (int i = 0; i < ints.Size(); ++i)
	{
		if (ints[i] == currT)
		{
			Interactable * inter = ints[(i+1) % ints.Size()];
			characterProp->SetTarget(inter);
		}
	}
	UpdateHUD();
}
void MORPG::PreviousTarget()
{
	// Grab previous?
	List<Interactable*> ints = GetInteractablesOnScreen();
	if (ints.Size() == 0)
		this->characterProp->SetTarget(0);
	Interactable * currT = characterProp->GetTarget();
	if (currT == 0)
		characterProp->SetTarget(ints[0]);
	for (int i = 0; i < ints.Size(); ++i)
	{
		if (ints[i] == currT)
			characterProp->SetTarget(ints[(i-1) % ints.Size()]);
	}
	UpdateHUD();
}


/// Load map/zone by name
void MORPG::EnterZone(Zone * zone)
{
	// Detach cameras.
	// .. TODO

	/// Decide format and stuff later.
	/// Create a test map for now.
	/// First clear ALL entities.
	MapMan.DeleteAllEntities();

	// Test-level of doom >:)
	bool test = true;
	if (test)
	{
		// Create the test level..!
		MapMan.CreateEntity("Base", ModelMan.GetModel("Zones/Test.obj"), TexMan.GetTexture("White"));

		/// o-o...
		character = new PC();
		world.characters.Add(character);
		world.interactables.AddItem(character);
		// Attach ze propororoty to bind the entity and the player.
		character->Spawn(Vector3f(0,0,0));
		character->SetCameraFocus();

		OpenHUD(character);
		// Enable steering!
		characterProp->inputFocus = true;


		/// Populate with some monsters too?
		for (int i = 0; i < 20; ++i)
		{
			Character * foe = new Foe(SHIELDLING, morpgRand.Randf() * 7 - 4);
			world.characters.Add(foe);
			world.interactables.AddItem(foe);
			Vector3f position = Vector3f(morpgRand.Randf(20)-10,0,morpgRand.Randf(20)-10); 
			foe->Spawn(position);
		}
		

		return;
	}
	/// Load the base/zone model(s).
	if (zone)
	{
		zone->CreateEntities();
	}
	else 
	{
	}
	
	/// Create the characters within.
}

Character * MORPG::HUDCharacter() 
{ 
	if (characterProp)
		return characterProp->ch;
	return 0;
}


/// UI stuffs.
void MORPG::OpenHUD(Character * forCharacter)
{
	hudOpen = true;
	characterProp = forCharacter->prop;
	hud = new UserInterface();
	hud->Load("gui/HUD.gui");
	QueueGraphics(new GMSetUI(hud));
	UpdateHUD();
}

void MORPG::UpdateHUD()
{
	if (!hudCharacter)
		return;
	QueueGraphics(new GMSetUIi("HP", GMUI::INTEGER_INPUT, character->stats->hp));
	QueueGraphics(new GMSetUIi("MP", GMUI::INTEGER_INPUT, character->stats->mp));
	QueueGraphics(new GMSetUIs("MaxHP", GMUI::TEXT, "/ "+String(character->stats->maxHp)));
	QueueGraphics(new GMSetUIs("MaxMP", GMUI::TEXT, "/ "+String(character->stats->maxMp)));
	QueueGraphics(new GMSetUIs("Target", GMUI::TEXT, character->prop->mainTarget? ("Target: "+ character->prop->mainTarget->name) : "No target"));
}

#include "UI/UIButtons.h"

void MORPG::OpenInteractionMenu()
{
	if (hudCharacter == 0 || iMenuOpen)
		return;
	if (hudCharacter->mainTarget == 0)
	{
		NextTarget();
		return;
	}
	iMenuOpen = true;
	// Fill it.
	QueueGraphics(new GMClearUI("IMenu"));
	List<String> buttons;
	if (hudCharacter->MainTarget()->characterType == Character::FOE)
	{
		buttons.AddItem("Attack");
		buttons.AddItem("Target skills");
	}
	buttons.AddItem("Check");
	for (int i = 0; i < buttons.Size(); ++i)
	{
		UIButton * button =  new UIButton(buttons[i]);
		button->sizeRatioY = MaximumFloat(1.f / buttons.Size(),0.2f);
		button->textureSource = hud->defaultTextureSource;
		QueueGraphics(new GMAddUI(button, "IMenu"));
	}
	/// Open iMenu.
	QueueGraphics(new GMPushUI("IMenu", hud));
}
void MORPG::CloseInteractionMenu()
{
	QueueGraphics(new GMPopUI("IMenu", hud));
	iMenuOpen = false;
}
void MORPG::OpenSubIMenuMenu(int whichMenu)
{
	if (subMenuOpen >= 0)
		CloseSubIMenuMenu();
	subMenuOpen = whichMenu;
}
void MORPG::CloseSubIMenuMenu()
{

}


void MORPG::Log(CTextr text)
{
	
	QueueGraphics(new GMSetUIt("ChatLog", GMUI::LOG_APPEND, text));
}
