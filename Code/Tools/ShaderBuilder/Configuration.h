/*
This file provides configurable settings
that can be used to modify the ShaderBuilder project
*/

#ifndef EAE6320_SHADER_BUILDER_CONFIGURATION_H
#define EAE6320_SHADER_BUILDER_CONFIGURATION_H

// Usually device debug info is only enabled on debug builds
#ifdef _DEBUG
#define EAE6320_GRAPHICS_AREDEBUGSHADERSENABLED
#endif

#endif	// EAE6320_SHADER_BUILDER_CONFIGURATION_H