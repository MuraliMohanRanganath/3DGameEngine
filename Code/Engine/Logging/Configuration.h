/*
	This file provides configurable settings
	that can be used to control logging behavior
*/

#ifndef EAE6320_LOGGING_CONFIGURATION_H
#define EAE6320_LOGGING_CONFIGURATION_H

// By default the log uses a relative path which will be generated
// in the same directory as the game's executable
// (which is nice because it is easy for a user to find)
#define EAE6320_LOGGING_DEFAULTPATH "eae6320.log"

// Flushing the logging buffer to disk is expensive,
// but it can be done after every message is output during development
// if an application is crashing so that no messages are lost
#ifdef _DEBUG
	#define EAE6320_LOGGING_FLUSHBUFFERAFTEREVERYMESSAGE
#endif

#endif	// EAE6320_LOGGING_CONFIGURATION_H
