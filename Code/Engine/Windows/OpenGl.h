/*
	This file helps with working with OpenGL under Windows
*/

#ifndef EAE6320_WINDOWS_OPENGL_H
#define EAE6320_WINDOWS_OPENGL_H

// Header Files
//=============

#include "Includes.h"

#include <string>

// Interface
//==========

namespace eae6320
{
	namespace Windows
	{
		namespace OpenGl
		{
			// Initializing OpenGL in Windows requires a device context,
			// and so a hidden window must be created and a device context created
			// before any OpenGL functions can called

			struct sHiddenWindowInfo
			{
				HWND window;
				ATOM windowClass;
				HDC deviceContext;
				HGLRC openGlRenderingContext;

				sHiddenWindowInfo() : window( NULL ), windowClass( NULL ), deviceContext( NULL ), openGlRenderingContext( NULL ) {}
			};

			// It is ok if the HINSTANCE that is passed to CreateHiddenContextWindow() is NULL,
			// but if it is then the valid HINSTANCE that CreateHiddenContextWindow() assigns _must_ be passed to FreeHiddenContextWindow()
			bool CreateHiddenContextWindow( HINSTANCE& io_hInstance, sHiddenWindowInfo& o_info, std::string* const o_errorMessage = NULL );
			bool FreeHiddenContextWindow( HINSTANCE& i_hInstance, sHiddenWindowInfo& io_info, std::string* const o_errorMessage = NULL );
		}
	}
}

#endif	// EAE6320_WINDOWS_OPENGL_H
