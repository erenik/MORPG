/// Emil Hedemalm
/// 2016-04-23
/// Player-dedicated character.

#include "PlayerCharacter.h"

PC::PC() 
	: Character()
{
	characterType = Character::PLAYER;
}
