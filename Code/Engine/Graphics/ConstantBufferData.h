#ifndef EAE6320_CONSTANT_BUFFER_DATA_H
#define EAE6320_CONSTANT_BUFFER_DATA_H
#include "../Math/cMatrix_transformation.h"
namespace eae6320 {

	namespace Graphics {

		enum ConstantBufferType { FRAME_DATA, DRAW_CALL_DATA, MATERIAL_DATA };

		struct sFrame
		{
			Math::cMatrix_transformation g_transform_worldToCamera;
			Math::cMatrix_transformation g_transform_cameraToScreen;
			union
			{
				float g_elapsedSecondCount_total;
				float register8[4];
			};
		};

		struct sDrawCall
		{
			Math::cMatrix_transformation g_transform_localToWorld;
		};


		struct sMaterial
		{
			struct
			{
				float r, g, b, a;
			} g_color;
			sMaterial() { g_color.r = 1.0f, g_color.g = 1.0f, g_color.b = 1.0f, g_color.a = 1.0f; }
		};

	} // namespace Graphics
}// namespace eae6320

#endif // EAE6320_CONSTANT_BUFFER_DATA_H