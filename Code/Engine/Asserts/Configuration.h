/*
	This file provides configurable settings
	that can be used to control assert behavior
*/

#ifndef EAE6320_ASSERTS_CONFIGURATION_H
#define EAE6320_ASSERTS_CONFIGURATION_H

// By default asserts are only enabled for debug builds,
// but you can #define it differently as necessary
#ifdef _DEBUG
	#define EAE6320_ASSERTS_AREENABLED
#endif

#endif	// EAE6320_ASSERTS_CONFIGURATION_H
