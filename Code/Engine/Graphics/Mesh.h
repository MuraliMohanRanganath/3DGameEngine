#ifndef EAE6320_MESH_H
#define EAE6320_MESH_H

// Header Files
//=============

#include "Configuration.h"
#include "VertexData.h"
#include <stdint.h>
#include <vector>
#include "../Math/cVector.h"
#include "../../External/Lua/Includes.h"

#if defined( EAE6320_PLATFORM_WINDOWS )
#include "../Windows/Includes.h"
#endif

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
		class Mesh {
		public:
			bool LoadBinaryFile(const char* const i_path);
			bool Initialize();
			void DrawFrame();
			bool CleanUp();
			uint32_t getIndicesCount() {
				return indicesCount;
			}

			sVertex * getVertexData() {
				return vertexData;
			}

			uint32_t *getIndices() {
				return indices;
			}

#ifdef _DEBUG
			bool DrawLine(Math::cVector start, Math::cVector end, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
			bool DrawCube(float width, float height, float depth, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
			bool DrawSphere(float radius, int sliceCount, int stackCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
			bool DrawCylinder(float bottomRadius, float topRadius, float height, int sliceCount, int stackCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
#endif // DEBUG
		
		private:
			uint32_t verticesCount;
			sVertex* vertexData;
			uint32_t indicesCount;
			uint32_t* indices;

			void DrawCylinderTopCap(float topRadius, float height, int sliceCount, std::vector<sVertex> &verticies, std::vector<uint32_t> &indicies, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
			void DrawCylinderBottomCap(float bottomRadius, float height, int sliceCount, std::vector<sVertex> &verticies, std::vector<uint32_t> &indicies, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

#if defined( EAE6320_PLATFORM_WINDOWS )
			HWND mainWindow;
#if defined( EAE6320_PLATFORM_D3D )
			ID3D11Buffer* s_vertexBuffer=NULL;
			ID3D11Buffer* s_indexBuffer=NULL;
#elif defined( EAE6320_PLATFORM_GL )
			GLuint s_vertexArrayId;
#ifndef EAE6320_GRAPHICS_ISDEVICEDEBUGINFOENABLED
			GLuint s_vertexBufferId;
			GLuint s_indexBufferId;
#endif // EAE6320_GRAPHICS_ISDEVICEDEBUGINFOENABLED

#endif
#endif
		};
	}
}
#endif // EAE6320_MESH_H