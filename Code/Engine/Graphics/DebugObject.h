#ifndef EAE6320_DEBUG_OBJECT_H
#define EAE6320_DEBUG_OBJECT_H

#include "../Math/cVector.h"
#include "Transform.h"
#include "Graphics.h"

namespace eae6320
{
	namespace Graphics
	{
		class DebugObject
		{
		public:
			Transform transform;
			Graphics::MeshObject meshObject;
		public:
			void Move(Math::cVector);
			void Rotate(Math::cVector);
			void initializeBoxDebugObject(Math::cVector initPosition, Math::cVector initRotation, float width, float height, float depth, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
			void drawBoxDebugObject(Graphics::cMaterial* material, Math::cVector initPosition, Math::cVector initRotation, float width, float height, float depth, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
			void initializeSphereDebugObject(Math::cVector initPosition, Math::cVector initRotation, float radius, int sliceCount, int stackCount, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
			void initializeCylinderDebugObject(Math::cVector initPosition, Math::cVector initRotation, float bottomRadius, float topRadius, float height, int sliceCount, int stackCount, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
			void initializeLineDebugObject(Math::cVector initPosition, Math::cVector initRotation, eae6320::Math::cVector start, eae6320::Math::cVector end, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
			void updateLine(eae6320::Math::cVector start, eae6320::Math::cVector end, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
			bool cleanUp();
		private:
			const char* const debugshapeMaterialPath = "data/materials/debugshape.material";
		};
	}
}
#endif // !EAE6320_DEBUG_OBJECT_H