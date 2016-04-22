/// Emil Hedemalm
/// 2014-07-27
/// A character.

#include "Character.h"
#include "Stat.h"

Character::Character()
{
	currentClassLvl = std::pair<char, char>(CLASSLESS, 1);
}

/// To current class and level.
void Character::UpdateBaseStatsToClassAndLevel()
{
	*baseStats = Stats(); // Refresh from base.
	availableSkills.Clear();
	for (int i = 0; i < skills.Size(); ++i)
	{
		std::pair<char, char> skill = skills[i];
		if (skill.first < currentClassLvl.first * 100 || skill.first >= (currentClassLvl.first+1) * 100)
			continue;
		availableSkills.AddItem(skill);
	}
	/// Base stats.
	float strL, vitL, hpL, mpL, magL, MdefL, hppL = 0, mppL = 0;
	switch(currentClassLvl.first)
	{
		case CLASSLESS: strL = vitL = magL = MdefL = 0.7; hpL = mpL = 3; hppL = mppL = 0; break;
		case FIGHTER: strL = 1.1; vitL = 1.2; magL = 0.5; MdefL = 0.8; hpL = 11; hppL = 2.7; break;
		default:
			assert(false);
	}
	int L = currentClassLvl.second;
	Stats & bs = *baseStats;
	bs.str += strL * L;
	bs.vit += vitL * L;
	bs.maxHp += hpL * L;
	bs.mag += magL * L;
	bs.mdef += MdefL * L;
	bs.maxMp += mpL * L;
	bs.maxHp *= (1.0 + hppL * L);
	bs.maxMp *= (1.0 + mppL * L);
	/// Passive Traits
	int tier = currentClassLvl.second / 5;
	switch(currentClassLvl.first)
	{
		case CLASSLESS:
		{
			switch(tier)
			{
			case 0: case 1: case 2: // up to 14.
				bs.str += 1;
				bs.vit += 1;
				bs.mag += 1;
				bs.mdef += 1;
			}
			break;	
		}
		case FIGHTER:
		{
			switch(tier)
			{
				case 3: case 4: // lvl 15, 20, 25 to 29
					bs.counterAttack += 2;
					bs.attack += 5;
					bs.defense += 5;
				case 0: case 1: case 2: // Up to lvl 14
					bs.attack += 10;
					bs.defense += 10;
					bs.criticalHitRate += 3;
					break;
			}
			break;
		}
	}
	/// Review skills.
	for (int i = 0; i < availableSkills.Size(); ++i)
	{
		
	}
	/// Update gear data.
	UpdateGearStatsToCurrentGear();
}

void Character::UpdateGearStatsToCurrentGear()
{
	/// Copy.
	*statsWithGear = *baseStats;
	UpdateStatsWithBuffs();
}

void Character::UpdateStatsWithBuffs()
{
	*this->statsWithBuffs = *this->statsWithGear; // Copy over gear data.
	for (int i = 0; i < buffs.Size(); ++i)
	{
		Buff * buff = buffs[i];
		
	}
}


/// Adds buff. Recalcualtes stats.
Buff * Character::AddBuff(Buff * newBuff)
{
	buffs.AddItem(newBuff);
	UpdateStatsWithBuffs();
	return newBuff;
}

void Character::RemoveBuff(Buff * goBuff)
{
	buffs.RemoveItem(goBuff);
	UpdateStatsWithBuffs();
}	

char Character::SkillLevel(int skill)
{
	for (int i = 0; i < availableSkills.Size(); ++i)
	{
		if (availableSkills[i].first == skill)
			return availableSkills[i].second;
	}
	return -1;
}


bool Character::HasBuff(int fromSkill)
{
	for (int i = 0; i < buffs.Size(); ++i)
	{
		if (buffs[i]->skill == fromSkill)
			return true;
	}
	return false;
}	


/// o.o
bool Character::WriteTo(std::fstream & file)
{
	name.WriteTo(file);
	return true;	
}

bool Character::ReadFrom(std::fstream & file)
{
	name.ReadFrom(file);
	return true;
}


