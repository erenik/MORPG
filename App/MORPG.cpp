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
#include "Character/Skill.h"

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
float testMultiplier = 1.f; // Should be 1.f by default, lower speeds up things.

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

Character * MainTargetChar()
{
	Character * character = morpg->HUDCharacter();
	Character * mt = character->prop->MainTarget();
	if (mt == 0)
		return 0;
	return mt;
}

Entity * MainTarget()
{
	Character * character = morpg->HUDCharacter();
	Character * mt = character->prop->MainTarget();
	if (mt == 0)
		return 0;
	return mt->prop->owner;
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
	Graphics.QueueMessage(new GMSetCamera(firstPersonCamera, CT_DISTANCE_FROM_CENTER_OF_MOVEMENT, 0.f));
	QueueGraphics(new GMSetCamera(firstPersonCamera, CT_ROTATIONAL_SMOOTHNESS, 0.000001f));
	QueueGraphics(new GMSetCamera(firstPersonCamera, CT_SMOOTHING, 0.05f)); 
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
		case MessageType::SET_STRING:
		{
			SetStringMessage * ssm = (SetStringMessage*) message;
			if (msg == "SetInputLine")
			{
				std::cout<<"\nSetInputLine: "<<ssm;
				/// Good. Evaluate it.
				String input = ssm->value;
				EvaluateLine(input);
				/// Hide input box thingy.
				QueueGraphics(new GMPopUI("InputLine", hud));
			}
			break;
		};
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
			else if (msg == "Skills" || msg == "Combat skills")
				OpenSubIMenuMenu(COMBAT_SKILLS);
			else if (msg == "Attack") // Attack! 
				characterProp->Engage();
			else if (msg == "Disengage")
				characterProp->Disengage();
			else if (msg == "CloseSubIMenu")
				CloseSubIMenuMenu();
			else if (msg == "TargetSelf")
				HUDCharacter()->prop->SetTarget(HUDCharacter());
			else if (msg.StartsWith("Skill:"))
			{
				// Target?
				String skill = msg - "Skill:";
				int sk = skill.Tokenize("L")[0].ParseInt();
				int level = skill.Tokenize("L")[1].ParseInt();
				// Try use straight away.
				StartPreparing(sk, level, HUDCharacter());
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
			else if (msg == "OpenInputLine")
			{
				QueueGraphics(new GMSetUIs("InputLine", GMUI::STRING_INPUT_TEXT, ""));
				QueueGraphics(new GMPushUI("InputLine", hud));
				/// Make it active for input?
				QueueGraphics(new GMSetUIb("InputLine", GMUI::ACTIVE, true, hud));
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
	bindings.AddItem((new Binding(Action::FromString("NextTarget"), KEY::TAB))->SetActivateOnRepeat(true));
	bindings.AddItem((new Binding(Action::FromString("PreviousTarget"), List<int>(KEY::SHIFT, KEY::TAB)))->SetActivateOnRepeat(true));
	bindings.AddItem(new Binding(Action::FromString("Interact"), KEY::ENTER));
	bindings.AddItem(new Binding(Action::FromString("OpenInputLine"), KEY::SPACE));
	bindings.AddItem(new Binding(Action::FromString("TargetSelf"), KEY::F1));
}

List<Interactable*> GetInteractablesOnScreen()
{
	// Check if in camera frustum.
	Camera * cam = CameraMan.ActiveCamera();
//	std::cout<<"\nCamera: "<<cam->name<<" pos"<<cam->Position();
	Frustum f = cam->GetFrustum();
	Vector3f camLeft = cam->LeftVector();
	Vector3f camPosition = cam->Position();
	// Get all for now.
	List<Interactable *> ints;
	for (int i = 0; i < world.interactables.Size(); ++i)
	{
		// If visible on screen or not.
		Interactable * inter = world.interactables[i];
		if (!inter->targetable)
			continue;
		Vector3f position = inter->Position();
		int inside = f.PointInFrustum(position);
		if (inside != Loc::INSIDE)
			continue;
		// By simple dot-product maths of belonging to left or right side of camera look direction.
		inter->sortValue = camLeft.DotProduct((position - camPosition).NormalizedCopy());

		bool added = false;
		for (int j = 0; j < ints.Size(); ++j)
		{
			if (ints[j]->sortValue < inter->sortValue)
			{
				ints.Insert(inter, j);
				added = true;
				break;
			}
		}
		if (!added)
			ints.AddItem(inter);
	}
	return ints;
}

void MORPG::NextTarget()
{
	// Grab all on screen?
	List<Interactable*> ints = GetInteractablesOnScreen();
	Interactable * currT = characterProp->GetTarget();
	if (ints.Size() == 0)
	{
		this->characterProp->SetTarget(0);
		Log("No targets in sight");
		return;
	}
	else if (ints.Size() == 1 || currT == 0)
	{
		this->characterProp->SetTarget(ints[0]);
		return;
	}
	for (int i = 0; i < ints.Size(); ++i)
	{
		if (ints[i] == currT)
		{
			Interactable * inter = ints[(i+1) % ints.Size()];
			characterProp->SetTarget(inter);
			return;
		}
	}
	// Grab first one or self.
	characterProp->SetTarget(ints[0]);
}

void MORPG::PreviousTarget()
{
	// Grab previous?
	List<Interactable*> ints = GetInteractablesOnScreen();
	Interactable * currT = characterProp->GetTarget();
	if (ints.Size() == 0)
	{
		this->characterProp->SetTarget(0);
		Log("No targets in sight");
		return;
	}
	else if (ints.Size() == 1 || currT == 0)
	{
		this->characterProp->SetTarget(ints[0]);
		return;
	}
	if (currT == 0)
		characterProp->SetTarget(ints[0]);
	for (int i = 0; i < ints.Size(); ++i)
	{
		if (ints[i] == currT)
		{
			characterProp->SetTarget(ints[(i-1 + ints.Size()) % ints.Size()]);
			return;
		}
	}
	// Grab first one or self.
	characterProp->SetTarget(ints[0]);
}

/// Show with cool arrows, yes.
Entity * targetArrowEntity = 0;
void MORPG::OnTargetUpdated()
{
	if (targetArrowEntity == 0)
		targetArrowEntity = MapMan.CreateEntity("TargetArrow", ModelMan.GetModel("TargetArrow.obj"), TexMan.GetTexture("0xAAAA33FF"));
	Character * mtc = MainTargetChar();
	// Attach position to other entity.
	QueuePhysics(new PMSetEntity(targetArrowEntity, PT_PARENT, MainTarget()));
	QueuePhysics(new PMSetEntity(targetArrowEntity, PT_INHERIT_POSITION_ONLY));
	if (mtc)
		QueuePhysics(new PMSetEntity(targetArrowEntity, PT_POSITION, Vector3f(0, 0.5f + mtc->representationScale.y * 2.f, 0)));
	/// Make invisible if no target?
	QueueGraphics(new GMSetEntityb(targetArrowEntity, GT_VISIBILITY, MainTarget() != NULL ? true : false));
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
			Character * foe = new Foe(SHIELDLING, (int) (morpgRand.Randf() * 7.f - 4.f));
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
	QueueGraphics(new GMSetUIi("MP", GMUI::INTEGER_INPUT, (int) character->stats->mp));
	QueueGraphics(new GMSetUIs("MaxMP", GMUI::TEXT, "/ "+String(character->stats->maxMp)));
	UpdateHPInHUD();
	UpdateTargetInHUD();
}

void MORPG::UpdateHPInHUD()
{
	if (!hudCharacter)
		return;
	QueueGraphics(new GMSetUIi("HP", GMUI::INTEGER_INPUT, (int) character->stats->hp));
	QueueGraphics(new GMSetUIs("MaxHP", GMUI::TEXT, "/ "+String(character->stats->maxHp)));
}

void MORPG::UpdateTargetInHUD() // Target sub-section of HUD.
{
	QueueGraphics(new GMSetUIs("Target", GMUI::TEXT, character->prop->mainTarget? ("Target: "+ character->prop->mainTarget->name) : "No target"));
}

#include "UI/UIButtons.h"

void MORPG::OpenInteractionMenu()
{
	if (hudCharacter == 0 || iMenuOpen)
		return;
	// Don't auto grab target. If no target, grab self if anything.
	if (hudCharacter->mainTarget == 0)
	{
		hudCharacter->mainTarget = hudCharacter->ch;
		return;
	}
	iMenuOpen = true;
	// Fill it.
	QueueGraphics(new GMClearUI("IMenu"));
	List<String> buttons;
	if (hudCharacter->MainTarget()->characterType == Character::FOE)
	{
		if (hudCharacter->IsAttacking())
		{
			buttons.AddItem("Disengage");
		}
		else
			buttons.AddItem("Attack");
		// If have any skills, that is.
		if (hudCharacter->ch->activatableCombatSkills.Size())
			buttons.AddItem("Skills");
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
	if (hudCharacter == 0)
		return;
	// Don't auto grab target. If no target, grab self if anything.
	if (subMenuOpen >= 0)
		CloseSubIMenuMenu();
	subMenuOpen = whichMenu;
	Character * ch = hudCharacter->ch;
	/// Populate it.
	QueueGraphics(new GMClearUI("SubIMenu", hud));
	for (int i = 0; i < ch->activatableCombatSkills.Size(); ++i)
	{
		std::pair<int,char> skill = ch->activatableCombatSkills[i];
		String text = GetSkillName(skill.first);
		UIButton * button =  new UIButton("CloseSubIMenu&Skill:"+String(skill.first)+"L"+String(skill.second));
		button->text = text;
		button->sizeRatioY = MaximumFloat(1.f / ch->activatableCombatSkills.Size(), 0.2f);
		button->textureSource = hud->defaultTextureSource;
		QueueGraphics(new GMAddUI(button, "SubIMenu"));		
	}
	QueueGraphics(new GMPushUI("SubIMenu", hud));
}
void MORPG::CloseSubIMenuMenu()
{
	QueueGraphics(new GMPopUI("SubIMenu", hud));
	subMenuOpen = -1;
}


void MORPG::Log(CTextr text)
{
	
	QueueGraphics(new GMSetUIt("ChatLog", GMUI::LOG_APPEND, text));
}

/// Chat or command line.
void MORPG::EvaluateLine(String cmd)
{
	Character * c =	HUDCharacter();
	std::cout<<"\nCmd: "<<cmd;
	if (!cmd.StartsWith("/"))
		// Just send it to log for now.
		Log(HUDCharacter()? HUDCharacter()->name+": "+cmd : cmd);
	// OK, starts with /, cool.
	if (cmd.StartsWith("/setlvl"))
	{
		int lvl = cmd.Tokenize(" ")[1].ParseInt(); HUDCharacter()->currentClassLvl.second = lvl;	HUDCharacter()->UpdateBaseStatsToClassAndLevel(); Log("Level set to "+String(lvl)+".");
	} // Set weapon level
	if (cmd.StartsWith("/setwpl"))
	{
		int lvl = cmd.Tokenize(" ")[1].ParseInt(); HUDCharacter()->SetWeaponTrainingLevel(HUDCharacter()->stats->weaponType, lvl);	HUDCharacter()->UpdateGearStatsToCurrentGear(); Log("Weapon traning level set to "+String(lvl)+".");
	}
	if (cmd.StartsWith("/setcls"))
	{
		int which = GetClassByName(cmd.Tokenize(" ")[1]); if (which < 0) {Log("Bad class given. Retry."); return; }HUDCharacter()->currentClassLvl.first = which; HUDCharacter()->UpdateBaseStatsToClassAndLevel(); Log("Class set to "+ClassFullName(which)+". Level retained.");
	}
	if (cmd.StartsWith("/revive"))
	{
		c->prop->Revive();
	}
	if (cmd.StartsWith("/testspd") || cmd.StartsWith("/setspd"))
	{
		testMultiplier = cmd.Tokenize(" ")[1].ParseFloat();
		ClampFloat(testMultiplier, 0.1f, 3.f);
	}
}
