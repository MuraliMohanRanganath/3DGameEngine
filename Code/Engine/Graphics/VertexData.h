#ifndef EAE6320_VERTEX_DATA_H
#define EAE6320_VERTEX_DATA_H

#include <stdint.h>

namespace eae6320
{
	namespace Graphics {
		// This struct determines the layout of the geometric data that the CPU will send to the GPU
		struct sVertex
		{
			float x, y, z;
			uint8_t red, blue, green, alpha;
			float u, v;
			sVertex() { }
			sVertex(float i_x, float i_y, float i_z, uint8_t i_r, uint8_t i_g, uint8_t i_b, uint8_t i_a, float i_u, float i_v)
			{
				x = i_x; y = i_y; z = i_z;
				red = i_r; blue = i_b; green = i_g; alpha = i_a;
				u = i_u; v = i_v;
			}
		};
	}
}
#endif // EAE6320_VERTEX_DATA_H