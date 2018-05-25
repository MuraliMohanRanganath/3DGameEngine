#ifndef EAE6320_GRAPHICS_CAMERA_H
#define EAE6320_GRAPHICS_CAMERA_H

#include "../Math/cVector.h"
#include "../Math/cQuaternion.h"
#include "../Math/Functions.h"
#include "../Math/cMatrix_transformation.h"
#include "Transform.h"
#include "../UserSettings/UserSettings.h"

namespace eae6320 {
	namespace Graphics {
		class Camera {
		public:
			Math::cVector cameraPosition;
			Math::cQuaternion cameraOrientation;
			Transform transform;
		public:
			Camera()
			{
				m_fieldOfView= Math::ConvertDegreesToRadians(60.0f);
				m_nearPlane = 0.1f;
				m_farPlane = 10000.0f;
				//m_aspectRatio = ((float)UserSettings::GetResolutionWidth()) / UserSettings::GetResolutionHeight();
				Move(Math::cVector());
				Rotate(Math::cVector());
			};
			Camera(Math::cVector initPosition, Math::cVector initRotation, float i_fieldOfView, float i_nearPlane, float i_farPlane, float i_aspectRatio);
			void Set(Math::cVector i_position, Math::cVector i_rotation);
			void Move(Math::cVector i_position);
			void Rotate(Math::cVector i_orientation);
			Math::cMatrix_transformation CalculateWorldToCameraTransformationMatrix();
			Math::cMatrix_transformation CalculateCameraToScreenTransformationMatrix();
		private:
			float m_fieldOfView;
			float m_nearPlane;
			float m_farPlane;
			float m_aspectRatio;
		};
	} // namespace Graphics
} // namespace eae6320

#endif // EAE6320_GRAPHICS_CAMERA_H