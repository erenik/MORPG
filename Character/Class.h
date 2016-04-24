/// Emil Hedemalm
/// 2016-04-22
/// o-o

#ifndef CLASS_H
#define CLASS_H

enum classes
{
	BAD_CLASS = -2,
	ANY = -1,
	CLASSLESS = 0, // CLS
	FIGHTER,	// FTR
	ACOLYTE,	// ACL
	MONK,		// MNK
	CLASSES,
};

#include "String/AEString.h"

/// See list above. May be both regular list or abbreviated versions.
int GetClassByName(String name);
String ClassAbrvName(int which);
String ClassFullName(int which);


class Class 
{
public:
	int cls;
};


#endif
