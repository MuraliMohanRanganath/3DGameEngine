#ifndef EAE6320_RIGID_BODY_H
#define EAE6320_RIGID_BODY_H

#include"../Math/cVector.h"

namespace eae6320
{
	namespace Physics
	{
		class RigidBody
		{
		public:
			Math::cVector acceleration;
			Math::cVector velocity;
			float drag = 5;
			float height = 70;
			float width = 30;
			float length = 30;
			Math::cVector toVelocityPoint;
			Math::cVector toFloorPoint;
		};
	}
}
#endif // !EAE6320_RIGID_BODY_H
