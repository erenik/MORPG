/// Emil Hedemalm
/// 2014-07-27
/// A single zone within the world.

#include "Zone.h"
#include "SpawnArea.h"

#include "Entity/Entity.h"
#include "Entity/CompactEntity.h"
#include "Entity/EntityManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/Messages/GraphicsMessages.h"
#include "Graphics/Messages/GMCamera.h"

#include "Character/Stats.h"
#include "Character/Character.h"
#include "Properties/CharacterProperty.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"

#include "Maps/MapManager.h"
#include "Room.h"

#include "WorldMap.h"

Zone::Zone(String i_name) 
{
	Nullify();
	this->name = i_name;
}

void Zone::Nullify()
{
	isWater = false;
	isMountain = false;
	elevation = 1.f;
	hasSettlement = false;
	inhabitants = 0;
	roomGridSize = 1.f;
	camera = NULL;
}

Zone::~Zone()
{
	buildingSlots.ClearAndDelete();
	buildings.ClearAndDelete();
	entrances.ClearAndDelete();
	rooms.ClearAndDelete();
	// Delete map if not done so already. lolwat
//	if (map)
	//	MapMan.DeleteMap(map);
	/// Contains all foes and characters alike. Maybe check for PCs so they're not deleted?
	interactables.ClearAndDelete();
}

void Zone::SetName(String i_name)
{
	// Check that it doesn't clash with other names, or screw it?
	name = i_name;
}

/// Usually the most important building.
Entity * Zone::CreateWorldMapRepresentation()
{
	// Get the coolest house in town.
	if (buildings.Size() == 0)
		return NULL;
	Building * building = buildings[0];
	Entity * entity = EntityMan.CreateEntity(name, building->model, building->texture);
	if (entity == NULL){
	    std::cout<<"\nERROR: MapManager::CreateEntity:Unable to create entity, returning.";
        return NULL;
	}
	entity->worldPosition = position;
	entity->RecalculateMatrix();
	return entity;
}

BuildingSlot * Zone::GetFreeBuildingSlot()
{
	for (int i = 0; i < buildingSlots.Size(); ++i)
	{
		BuildingSlot * slot = buildingSlots[i];
		if (slot->building == NULL)
			return slot;
	}
	return NULL;
}

/// Places room into the zone, adding it to the grid and list.
bool Zone::Place(Room * room)
{
	// Get elements it would occupy.
	List<Vector3i> spotsToOccupy = room->GetAbsPoints();
	for (int i = 0; i < spotsToOccupy.Size(); ++i)
	{
		Vector3i spot = spotsToOccupy[i];
		if (roomMatrix.At(spot))
			return false;
	}
	for (int i = 0; i < spotsToOccupy.Size(); ++i)
	{
		Vector3i spot = spotsToOccupy[i];
		roomMatrix.Set(spot, room);
	}
	rooms.Add(room);
	return true;
}

/// Simulate it. Clear up old stuff as needed.
void Zone::Process(int timeInMs)
{
	List<Character*> toRemove;
	for (int i = 0; i < characters.Size(); ++i)
	{
		Character * c = characters[i];
		if (!c->prop)
			continue;
		if (c->prop->deleteMe)
		{
			toRemove.AddItem(c);
		}
	}
	for (int i = 0; i < toRemove.Size(); ++i)
	{
		Character * c = toRemove[i];
		MapMan.DeleteEntity(c->prop->owner);
		characters.RemoveItem(c);
		interactables.RemoveItem(c);
		delete c;
	}
	/// Process SpawnAreas if any.
	for (int i = 0; i < spawnAreas.Size(); ++i)
	{
		SpawnArea * sa = spawnAreas[i];
		sa->Process(timeInMs);
	}
}

/// Adds character to zone.
void Zone::AddCharacter(Character * c)
{
	assert(!characters.Exists(c));
	characters.AddItem(c);
	assert(!interactables.Exists(c));
	interactables.AddItem(c);
	/// Move divisions later...
	c->zone = this;
	/// If the zone is active, spawn the actual foe into place too?
	if (this->active)
		c->Spawn();
}

void Zone::RemoveCharacter(Character * c)
{
	characters.RemoveItem(c);
	interactables.RemoveItem(c);
	// If active zone, remove from simulation here too. or elsewhere?
}

void Zone::RemoveCharacters(List<Character *> cs)
{
	characters.Remove(cs);
	List<Interactable*>  ics = ConvertList<Character*, Interactable*>(cs);
	interactables.Remove(ics);
}

// Registers all entities for display and makes the world-map camera active.
void Zone::MakeActive()
{
	// Unregister all current entities from graphics rendering. <- why?
//	GraphicsMan.QueueMessage(new GraphicsMessage(GM_UNREGISTER_ALL_ENTITIES));
	// Create camera if needed.
	if (!camera){
		camera = CameraMan.NewCamera("ZoneCamera", true);
		// One-time setup of camera.
		camera->movementType = CAMERA_MOVEMENT_ABSOLUTE;
		camera->absForward = Vector3f(0,0,-1);
		camera->absRight = Vector3f(1,0,0);
		camera->absUp = Vector3f(0,1,0);
		// Create the default camera-position too.
		Camera * resetCamera = CameraMan.NewCamera("ResetCamera", true);
		camera->resetCamera = resetCamera;
	}
	/// Why set the camera? MakeActive should load the Zone, but where to look will be dealt with later.
//	GraphicsMan.QueueMessage(new GMSetCamera(camera));
	// o.o
	if (entities.Size() == 0)
	{
		CreateEntities();
		// Update the reset-camera's position and stuffs! o.o
		Vector3f position = FromWorldToWorldMap(roomMatrix.Size() * 0.5f, 0.f);
		camera->resetCamera->position = position + Vector3f(0, 20.f, 20.f);	 
		camera->resetCamera->rotation = Vector3f(0.9f, 0, 0);
		camera->resetCamera->flySpeed = roomMatrix.Size().Length() * 0.1f;
		// And reset it.
		camera->Reset();
	}
	// Make whole map active?
	MapMan.MakeActive(this);
//	GraphicsMan.QueueMessage(new GMRegisterEntities(entities));
}

void Zone::MakeInactive()
{
	MapMan.MakeInactive(this);
}

/// Called host-side. Creates map with all entities n properties inside for simulation. Does not register for simulation.
void Zone::CreateMap()
{
	/// 
//	map = MapMan.CreateMap(name);
//	assert(map);

	MapMan.MakeActive(this);
	/// Default, make one flat space for floor first.
	Entity * entity = EntityMan.CreateEntity("Room placeholder plate", ModelMan.GetModel("Plane.obj"), TexMan.GenerateTexture(Vector4f(1,1,1,1)));
//	entity->position = worldPos * roomGridSize + Vector3f(0.5f,0,0.5f); // Offset so that it is inside the grid correctly.
	entity->RecalculateMatrix();
	entities.Add(entity);
	MapMan.AddEntity(entity);
	/// For each building or obstacle, create it.

	/// Create each zone.


	/// For each character inside, create its entity.
	for (int i = 0; i < characters.Size(); ++i)
	{
		Character * c = characters[i];
		c->Spawn(c->position, this);
	}
	/// Should only create, not necessarily make visible or register for physics.


	// Create all entities as the rooms and character data specify them at the moment!
	for (int i = 0; i < rooms.Size(); ++i)
	{
		Room * room = rooms[i];
		if (room->model)
		{/*
			Entity * entity = EntityMan.CreateEntity("Room", room->model, TexMan.GenerateTexture(Vector4f(1,1,1,1)));
			Vector3f worldPos = FromWorldToWorldMap(room->position);
			entity->worldPosition = worldPos * roomGridSize;
			entity->scale = FromWorldToWorldMap(room->scale);
			entity->RecalculateMatrix();
			entities.Add(entity);
			*/
		}
		else 
		{
			/*
			// Default, just create a bunch of planes.
			List<Vector3i> points = room->GetAbsPoints();
			for (int j = 0; j < points.Size(); ++j)
			{
				Entity * entity = EntityMan.CreateEntity("Room placeholder plate", ModelMan.GetModel("Plane.obj"), TexMan.GenerateTexture(Vector4f(1,1,1,1)));
				Vector3f rawPosition = points[j];
				Vector3f worldPos = FromWorldToWorldMap(rawPosition);
				entity->position = worldPos * roomGridSize + Vector3f(0.5f,0,0.5f); // Offset so that it is inside the grid correctly.
				entity->RecalculateMatrix();
				entities.Add(entity);
			}*/
		}
		// Visualize the room's entry-points.
		for (int j = 0; j < room->entryPoints.Size(); ++j)
		{
			/*
			EntryPoint & point = room->entryPoints[j];
			Vector3f dir = point.direction;
			Texture * tex = TexMan.GenerateTexture(Vector4f(dir.Abs(), 1.f));
			Entity * entry = EntityMan.CreateEntity("Entry point "+String(j), ModelMan.GetModel("sphere.obj"), tex);
			Vector3f worldPos = FromWorldToWorldMap(point.position);
			// Plus some offset.
			Vector3f offset = FromWorldToWorldMap(point.direction) * 0.2f;
			entry->worldPosition = worldPos + offset + Vector3f(0.5f,0.5f,0.5f);
			entry->scale = Vector3f(1,1,1) * 0.2f;
			entry->RecalculateMatrix();
			entities.Add(entry);
			*/
		}
		// Visualize building-slots
		for (int j = 0; j < room->buildingSlots.Size(); ++j)
		{
			/*
			BuildingSlot * bs = room->buildingSlots[j];
			Entity * bse = EntityMan.CreateEntity("Building slot", ModelMan.GetModel("plane.obj"), TexMan.GenerateTexture(Vector4f(0.2f,0.5f,1,1.f)));
			bse->scale = FromWorldToWorldMap(bs->size);
			bse->scale.y = 1.f;
			bse->position = FromWorldToWorldMap(bs->position) + Vector3f(0,1,0);
			bse->RecalculateMatrix();
			entities.Add(bse);*/
		}
	}
	// Deletes and re-creates entities as needed.
//	LoadFromCompactData();
}

/// Takes all models this zone is composed of and creates it for you. Will also create all characters within (hopefully including you!)
void Zone::CreateEntities()
{
}

/// If this zone is to be painted on a map, what color would it be?
Vector4f Zone::GetColor()
{
	/// o.o 0 = water-line, 1 = land, 0.1 to 0.9 = Beach?, 2 = hills, 3+ = mountains
	if (elevation <= -1.f)
	{
		return Vector3f(0,0, elevation * 0.5f + 1.f);
	}
	/// Shallow waters
	else if (elevation < 0.f)
	{
		// Lagoons?
		if (elevation > -0.1f)
			return Vector3f(0,1,1);
		// Regular waters.
		return Vector3f(0.2f, 0.5f, 1.f) * (1 + elevation) + Vector3f(0,0,1);
	}
	// Beach.
	else if (elevation < 0.5f)
	{
		return Vector3f(1.f, 1.f, 0.f);
	}
	// Grass to hills?
	else if (elevation < 2.f)
	{
		return Vector3f(0,1.f,0.f);
	}
	/// Hills to Mountains o.o
	else if (elevation < 3.f)
		return Vector3f(1,1,1) * 0.5f;
	// Mountains and snowy peaks!
	else 
	{
		return Vector3f(1,1,1);
	}
	return Vector3f(0,1,0);
}

bool Zone::IsWater()
{
	return isWater;
};

bool Zone::IsMountain()
{
	return isMountain;
}

void Zone::SetWater(bool w)
{ 
	isWater = w;
	elevation = -2.f;
};

void Zone::SetMountain(bool m)
{
	isMountain = m;
	elevation = 3.f;
}



bool Zone::WriteTo(std::fstream & file)
{
	name.WriteTo(file);
	position.WriteTo(file);
	file.write((char*) &elevation, sizeof(float));
//	std::cout<<"\nWrite zone "<<name<<" with elevation "<<elevation;
	return true;
}
bool Zone::ReadFrom(std::fstream & file)
{
	name.ReadFrom(file);
	position.ReadFrom(file);
	file.read((char*) &elevation, sizeof(float));
//	std::cout<<"\nRead zone "<<name<<" with elevation "<<elevation;
	return true;
}
