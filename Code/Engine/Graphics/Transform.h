#ifndef EAE6320_TRANSFORM_H
#define EAE6320_TRANSFORM_H

#include "../../Engine/Math/cVector.h"
#include "../../Engine/Math/cQuaternion.h"
#include "../../Engine/Math/cMatrix_transformation.h"

namespace eae6320
{
	namespace Graphics
	{
		class Transform
		{
		private:
			Math::cVector position;
			Math::cVector euler;
			Math::cQuaternion orientation;
			Math::cVector forward;
			Math::cVector right;
			Math::cVector up;
			void updateTransform();
		public:
			Transform()
			{
				updateTransform();
			}
			Transform(Math::cVector initialPosition, Math::cVector initialRotation)
			{
				position = initialPosition;
				euler = initialRotation;
				updateTransform();
			}
			void Move(Math::cVector);
			void Rotate(Math::cVector);
			Math::cVector getPosition() const { return position; }
			Math::cVector getRotation() { return euler; }

			Math::cQuaternion getOrientation() { return orientation; }
			Math::cVector getForward() const { return forward; }
			Math::cVector getRight() const { return right; }
			Math::cVector getUp() const { return up; }
		};
	}
}
#endif	// EAE6320_TRANSFORM_H