#ifndef EAE6320_GRAPHICS_EFFECT_H
#define	EAE6320_GRAPHICS_EFFECT_H

#include "cRenderState.h"

#if defined( EAE6320_PLATFORM_D3D )
#include <D3D11.h>
#endif

#if defined(EAE6320_PLATFORM_GL)
#include "../../External/OpenGlExtensions/OpenGlExtensions.h"
#endif

namespace eae6320
{
	namespace Graphics
	{
		class cEffect
		{
		public:
			bool Load(const char* const i_effectBinaryFilePath);
			void Bind();
			bool CleanUp();

		private:
			bool LoadBinaryFile(const char* const i_effectBinaryFilePath);
			cRenderState m_renderState;
			uint8_t m_renderStateBits;
			char * m_vertexShaderPath;
			char * m_fragmentShaderPath;
#ifdef EAE6320_PLATFORM_D3D
			ID3D11InputLayout* s_vertexLayout;
			ID3D11VertexShader* s_vertexShader;
			ID3D11PixelShader* s_fragmentShader;
#endif
#ifdef EAE6320_PLATFORM_GL
			GLuint s_programId = 0;
#endif
		};
	}
}
#endif // !EAE6320_GRAPHICS_EFFECT_H
