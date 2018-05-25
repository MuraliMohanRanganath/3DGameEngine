#ifndef EAE6320_GRAPHICS_CONSTANT_BUFFER_DATA_H
#define EAE6320_GRAPHICS_CONSTANT_BUFFER_DATA_H

#include "ConstantBufferData.h"

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
		class ConstantBufferDataManager {
		public:
			bool Initialize(ConstantBufferType bufferType, size_t  bufferSize, void* bufferData);
			bool Bind();  
			bool Update(void* bufferData);
			bool CleanUp();
		private:
			ConstantBufferType s_bufferType;
			size_t s_bufferSize;
#if defined( EAE6320_PLATFORM_D3D )
			ID3D11Buffer* s_constantBufferData;
#elif defined( EAE6320_PLATFORM_GL )
			GLuint s_constantBufferId;
#endif
		};

	} // namespace Graphics
} // namespace eae6320

#endif // EAE6320_GRAPHICS_CONSTANT_BUFFER_DATA_H