/// Emil Hedemalm
/// 2016-04-23
/// Player-dedicated character.

#include "PlayerCharacter.h"
#include "Class.h"
#include "Skill.h"

PC::PC() 
	: Character()
{
	characterType = Character::PLAYER;
	// Add some default skills - the 2-hours specifically.
	for (int i = 0; i < CLASSES; ++i)
	{
		skills.AddItem(std::pair<int,char>(i * SPC, 1));
	}
}
