/*
	A sprite is a quad (two triangle square) drawn in 2D
*/

#ifndef EAE6320_GRAPHICS_CSPRITE_H
#define EAE6320_GRAPHICS_CSPRITE_H

// Header Files
//=============

#include "Configuration.h"

#include <cstdint>

#ifdef EAE6320_PLATFORM_GL
	#include "OpenGL/Includes.h"
#endif

// Forward Declarations
//=====================

#ifdef EAE6320_PLATFORM_D3D
	struct ID3D11Buffer;
#endif

// Class Declaration
//==================

namespace eae6320
{
	namespace Graphics
	{
		class cSprite
		{
			// Interface
			//==========

		public:

			// Render
			//-------

			void Draw() const;

			// Initialization / Clean Up
			//--------------------------

			static bool Initialize();
			static bool CleanUp();

			cSprite( const float i_position_left, const float i_position_right, const float i_position_top, const float i_position_bottom,
				const float i_texcoord_left, const float i_texcoord_right, const float i_texcoord_top, const float i_texcoord_bottom );

			// Data
			//=====

		private:

			// The positions and texture coordinates are stored as a quad
			// (meaning that only rectangular sprites can be made),
			// and are converted into an sVertex when the draw call is made
			struct
			{
				// Left of screen = -1
				// Right of screen = 1
				// Top of screen = 1
				// Bottom of screen = -1
				float left, right, top, bottom;
			} m_position_screen;
			struct
			{
				// Left of texture = 0
				// Right of texture = 1
				// Top of texture = 1 (OpenGL-style)
				// Bottom of texture = 0 (OpenGL-style)
				float left, right, top, bottom;
			} m_textureCoordinates;

			// A single dynamic vertex buffer is created for _all_ sprites,
			// and its contents are changed every draw call
#if defined( EAE6320_PLATFORM_D3D )
			// A vertex buffer holds the data for each vertex
			static ID3D11Buffer* ms_vertexBuffer;
#elif defined( EAE6320_PLATFORM_GL )
			// A vertex array encapsulates the vertex data and the vertex input layout
			static GLuint ms_vertexArrayId;
			static GLuint ms_vertexBufferId;
#endif
		};
	}
}

#endif	// EAE6320_GRAPHICS_CSPRITE_H
