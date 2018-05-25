#ifndef EAE6320_PHYSICS_H
#define EAE6320_PHYSICS_H

#include "../Math/cVector.h"
#include "../Graphics/GameObject.h"
#include <vector>

namespace eae6320
{
	namespace Physics
	{
		bool Initialize();
		void Update();
		bool Load(const char* const i_path);
		void CheckCollision(Graphics::GameObject* gameObject);
		int IntersectSegmentTriangle(const Math::cVector& p, const Math::cVector& q, const Math::cVector& a, const Math::cVector& b, const Math::cVector& c, float *o_u, float *o_v, float *o_w, float *o_t);
		bool CleanUp();
	}
}
#endif // !EAE6320_PHYSICS_H