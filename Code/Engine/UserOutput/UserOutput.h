/*
	This file's functions are used to display information to the user

	A real game would have a fully-implemented UI system,
	and this class should be seen as the simplest possible "stub" for such a system
	and _not_ as a good example of how user output would really be handled.
*/

#ifndef EAE6320_USEROUTPUT_H
#define EAE6320_USEROUTPUT_H

// Header Files
//=============

#if defined( EAE6320_PLATFORM_WINDOWS )
	#include "../Windows/Includes.h"
#endif

// Interface
//==========

namespace eae6320
{
	namespace UserOutput
	{
		// Output
		//-------

		void Print( const char* const i_message, ... );

		// Initialization / Clean Up
		//--------------------------

		struct sInitializationParameters
		{
#if defined( EAE6320_PLATFORM_WINDOWS )
			HWND mainWindow;
#endif
		};

		bool Initialize( const sInitializationParameters& i_initializationParameters );
		bool CleanUp();
	}
}

#endif	// EAE6320_USEROUTPUT_H
