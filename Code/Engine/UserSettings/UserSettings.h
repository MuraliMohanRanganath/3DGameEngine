/*
	This file manages user-configurable settings
	(ones that the user can set in an INI file)
*/

#ifndef EAE6320_USERSETTINGS_H
#define EAE6320_USERSETTINGS_H

// Interface
//==========

namespace eae6320
{
	namespace UserSettings
	{
		unsigned int GetResolutionHeight();
		unsigned int GetResolutionWidth();
	}
}

#endif	// EAE6320_USERSETTINGS_H
