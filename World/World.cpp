/// Emil Hedemalm
/// 2014-07-27
/// Class encompassing an entire world.

#include "World.h"

#include "Nation.h"
#include "Zone.h"
#include "Character/Character.h"
#include "Properties/CharacterProperty.h"
#include "Quest.h"

#include "TextureManager.h"

#include "Model/ModelManager.h"
#include "Model/Model.h"

#include "Mesh/Mesh.h"
#include "Mesh/EMesh.h"
#include "Mesh/EVertex.h"

#include "Graphics/GraphicsManager.h"
#include "Maps/MapManager.h"

World world;
EMesh worldEMesh;


World::World()
{
	empty = true;
	oceanElevation = 0;
	oceanColor = Vector4f(0,0,1,0.5f);
}

void World::Process(short timeInMs)
{
	static int msAccumulated = 0;
	msAccumulated += timeInMs;
	if (msAccumulated < 1000)
		return;
	msAccumulated -= 1000;
	// Look for interactables to remove or respawn? Every second?
	for (int i = 0; i < zones.Size(); ++i)
	{
		zones[i]->Process(1000);
	}
}

void World::ClearSettlementsAndCharacters()
{
	for (int i = 0; i < zones.Size(); ++i)
	{
		Zone * zone = zones[i];
		zone->numInhabitants = 0;
		zone->inhabitants.Clear();
		zone->characters.Clear();
		zone->hasSettlement = false;
	}
	pcs.ClearAndDelete();
	settlements.Clear();
}

/// Deletes all contents in this world. Makes it ready for loading again.
void World::Delete()
{
	// Remove pc's from zones first... or not.
	for (int i = 0; i < zones.Size(); ++i)
	{
		zones[i]->RemoveCharacters(pcs);
	}

	std::cout<<"\nDeleting world...";
	nations.ClearAndDelete();

	// Delete zones..?
	MapMan.MakeActive(0);
	zones.ClearAndDelete();
	
	pcs.ClearAndDelete();
	quests.ClearAndDelete();
	settlements.Clear();
	empty = true;
}


// x = major, y = minor, z = debug fix number
Vector3i currentVersion(0,0,0);

#include "List/ListUtil.h"

/// Saves this world to target file. Will save all zones, characters and quests to the same file.
bool World::WriteTo(std::fstream & file)
{
	currentVersion.WriteTo(file);
	name.WriteTo(file);
	size.WriteTo(file);
	WriteListTo(zones, file);
	WriteListTo(pcs,file);
	oceanColor.WriteTo(file);
	return true;
}
/// Loads from target file. Will load all zones, characters and quests from the same file.
bool World::ReadFrom(std::fstream & file)
{
	// Clear before..?
	settlements.Clear();

	Vector3i version;
	version.ReadFrom(file);
	/// No other version allowed, for now.
	if (version != currentVersion)
		return false;
	name.ReadFrom(file);
	size.ReadFrom(file);
	ReadListFrom(zones, file);
	/// Place zones into matrix again.
	zoneMatrix.Allocate(size);
	for (int i = 0; i < zones.Size(); ++i)
	{
		Zone * zone = zones[i];
		zoneMatrix.Set(zone->position, zone);
	}
	/// Re-connect zones.
	ReconnectZones();
	ReadListFrom(pcs, file);
	oceanColor.ReadFrom(file);
	return true;
}

/// Based on zones.
Texture * World::GeneratePreviewTexture()
{
	std::cout<<"\nGenerating preview texture...";
	if (!texture)
		texture = TexMan.NewDynamic();

	texture->name = "World Preview texture";
	texture->source = "World::GeneratePreviewTexture";
	// Resize it.
	texture->Resize(Vector2i(size.x, size.z));

	texture->SetColor(Vector4f(0.2f,0.2f,0.2f,1));

	// Paint pixels accordingly.
	for (int i = 0; i < zones.Size(); ++i)
	{
		Zone * zone = zones[i];
		Vector4f color = zone->GetColor();
		texture->SetPixel(zone->position.x, zone->position.z, color);
	}
	return texture;
}
/// Based on zones.
Model * World::GenerateWorldModel()
{
	return NULL;
	std::cout<<"\nGenerating world model...";
	// Pause rendering while doing this...
	Graphics.PauseRendering();

	if (!model)
		model = ModelMan.NewDynamic();

	// Create the mesh for it.
	if (!model->mesh)
	{
		Mesh * mesh = new Mesh();
		model->mesh = mesh;
	}
	int tiles = size.x * size.y;
	// If the amount of vertices is not the same as the amount suggested by the amount of zones...
	if (worldEMesh.vertices.Size() != tiles)
	{
		// Then delete the existing data.
		worldEMesh.Delete();

		/// Create a regular plane.
		Vector3f topLeft(0,0,0),
			bottomLeft(0,0,size.y),
			bottomRight(size.x,0,size.y), 
			topRight(size.x,0,0);

		// Add grid of wanted size o-o
		// Since the grid "size" is actually the amount of faces, we will ahve to adjust it so that we instead get 1 vertex per "size"
		Vector2i gridSizeWanted = Vector2i(size.x, size.z) - Vector2i(1,1);
		/// Just take -1 on both and we should get the right amount of vertices! :)
		worldEMesh.AddGrid(topLeft, bottomLeft, bottomRight, topRight, gridSizeWanted);
	}
	
//	zoneMatrix.PrintContents();
	// Fetch the matrix of vertices created with te grid.
	Matrix<EVertex*> & vertices = worldEMesh.vertexMatrix;

	float heightMultiplier = 1.f;
	/// Manipulate them depending on what the tiles were randomized to become!
	for (int x = 0; x < size.x; ++x)
	{
		for (int z = 0; z < size.z; ++z)
		{
			Zone * zone = zoneMatrix.At(Vector3i(x,0,z));
			assert(zone);
			if (!zone)
				continue;
			float elevation = zone->elevation * heightMultiplier;
			// Just set y.
			EVertex * vertex = vertices.At(x,z);
			vertex->y = elevation;
		}
	}

	/// Load the new data from the editable mesh into the renderable/optimized one!
	model->mesh->LoadDataFrom(&worldEMesh);
	model->RegenerateTriangulizedMesh();

	std::cout<<" done.";
	Graphics.ResumeRendering();
	return model;
}



Zone * World::GetZoneByName(String name)
{
	for (int i = 0; i < zones.Size(); ++i)
	{
		Zone * zone = zones[i];
		if (zone->Name() == name)
			return zone;
	}
	return NULL;
}

Zone * World::GetZoneByPosition(ConstVec3fr pos)
{
	Zone * closest = NULL;
	float closestDist = 1000000.f;
	for (int i = 0; i < zones.Size(); ++i)
	{
		Zone * zone = zones[i];
		float dist = (zone->position - pos).Length();
		if (dist < closestDist)
		{
			closestDist = dist;
			closest = zone;
		}
	}
	return closest;
}

void World::ReconnectZones()
{
	ConnectZonesByDistance(1.2f);
}

/// Like a navmesh..
void World::ConnectZonesByDistance(float minDist)
{
	Zone * zone1, * zone2;
	for (int i = 0; i < zones.Size(); ++i)
	{
		zone1 = zones[i];
		for (int j = i + 1; j < zones.Size(); ++j)
		{
			zone2 = zones[j];
			float dist = (zone1->position - zone2->position).Length();
			if (dist < minDist)
			{
				zone1->neighbours.Add(zone2);
				zone2->neighbours.Add(zone1);
			}
		}
	}
}
