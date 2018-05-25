#ifndef EAE6320_GRAPHICS_CTEXT_H
#define EAE6320_GRAPHICS_CTEXT_H

#include "Configuration.h"
#include <fstream>
#include <cstdint>

#ifdef EAE6320_PLATFORM_GL
#include "OpenGL/Includes.h"
#endif

#ifdef EAE6320_PLATFORM_D3D
struct ID3D11Buffer;
#endif

namespace eae6320
{
	namespace Graphics
	{
		class cText
		{
		public:
			void Draw() const;
			static bool Initialize();
			static bool CleanUp();
			static bool LoadFontData(char* filename);
			cText(char * text, int x, int y);
		
			struct sScreenPosition
			{
				float left, right, top, bottom;
			};
		
			struct sFont
			{
				float left, right;
				int size;
			};

		private:
			static sFont* m_Font;
			char * m_text;
			int16_t m_x;
			int16_t m_y;

#if defined( EAE6320_PLATFORM_D3D )

#elif defined( EAE6320_PLATFORM_GL )
			// A vertex array encapsulates the vertex data and the vertex input layout
			static GLuint ms_vertexArrayId;
			static GLuint ms_vertexBufferId;
			static GLuint ms_indexBufferId;
#endif
		};
	}
}

#endif	// EAE6320_GRAPHICS_CTEXT_H