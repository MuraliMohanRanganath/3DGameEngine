#include "Camera.h"
#include "../Math/cMatrix_transformation.h"
#include <math.h>

eae6320::Graphics::Camera::Camera(Math::cVector initPosition, Math::cVector initRotation,float i_fieldOfView, float i_nearPlane, float i_farPlane, float i_aspectRatio) {
	m_fieldOfView = i_fieldOfView;
	m_nearPlane = i_nearPlane;
	m_farPlane = i_farPlane;
	m_aspectRatio = i_aspectRatio;
	Move(initPosition);
	Rotate(initRotation);
}

void eae6320::Graphics::Camera::Set(Math::cVector i_position, Math::cVector i_rotation){
	Move(i_position);
	Rotate(i_rotation);
}

void eae6320::Graphics::Camera::Move(Math::cVector i_position) {
	transform.Move(i_position);
}
void eae6320::Graphics::Camera::Rotate(Math::cVector i_orientation) {
	transform.Rotate(i_orientation);
}

eae6320::Math::cMatrix_transformation eae6320::Graphics::Camera::CalculateWorldToCameraTransformationMatrix() {
	return eae6320::Math::cMatrix_transformation::CreateWorldToCameraTransform(transform.getOrientation(), transform.getPosition());
}
eae6320::Math::cMatrix_transformation eae6320::Graphics::Camera::CalculateCameraToScreenTransformationMatrix() {
	return eae6320::Math::cMatrix_transformation::CreateCameraToScreenTransform_perspectiveProjection(m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane);
}