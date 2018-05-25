#ifndef EAE6320_MATERIAL_H
#define EAE6320_MATERIAL_H

#include "Effect.h"
#include "cTexture.h"
#include "ConstantBufferDataManager.h"

namespace eae6320
{
	namespace Graphics
	{

		class cMaterial
		{
		public:
			bool Load(const char* i_materialPath);
			void Bind();
			bool CleanUp();
		private:
			ConstantBufferDataManager* m_materialConstantBuffer;
			cEffect* m_effect;
			cTexture* m_texture;
		};
		
	} // namespace Graphics

} // namespace eae6320
#endif // EAE6320_MATERIAL_H