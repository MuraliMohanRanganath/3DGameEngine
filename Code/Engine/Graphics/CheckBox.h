#ifndef EAE6320_GRAPHICS_CHECKBOX_H
#define EAE6320_GRAPHICS_CHECKBOX_H

#include "Graphics.h"
#include "cText.h"
#include "cMaterial.h"
#include <cstdint>

namespace eae6320
{
	namespace Graphics
	{
		class Checkbox
		{
		public:
			Checkbox();
			Checkbox(char * text, int16_t x, int16_t y);
			~Checkbox();
			
			UIText Text;
			bool checked = true;

		private:
			char * text;
			int16_t x;
			int16_t y;
		};
	}
}

#endif	// EAE6320_GRAPHICS_CHECKBOX_H