/// Emil Hedemalm
/// 2016-04-24
/// o-o

#include "Class.h"

int GetClassByName(String name)
{
	name.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	for (int i = 0; i < CLASSES; ++i)
	{
		if (name == ClassAbrvName(i) ||
			name == ClassFullName(i))
			return i;
	}
	return BAD_CLASS;
}
String ClassAbrvName(int which)
{
	switch(which)
	{
		case CLASSLESS: return "CLS"; case FIGHTER: return "FTR"; case ACOLYTE: return "ACL";
		case MONK: return "MNK";
	}
	return "None";
}
String ClassFullName(int which)
{
	switch(which)
	{
		case CLASSLESS: return "Classless"; case FIGHTER: return "Fighter"; case ACOLYTE: return "Acolyte"; case MONK: return "Monk"; 
	}
	return "None";
}
