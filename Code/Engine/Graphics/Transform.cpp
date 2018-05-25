#include "Transform.h"
#include <math.h>
#include "../Math/Functions.h"

void eae6320::Graphics::Transform::Move(Math::cVector i_position)
{
	position = i_position;
	updateTransform();
}

void eae6320::Graphics::Transform::Rotate(Math::cVector i_rotation)
{
	euler = i_rotation;
	updateTransform();
}

void eae6320::Graphics::Transform::updateTransform()
{
	orientation = Math::cQuaternion(Math::ConvertDegreesToRadians(euler.x), Math::cVector(1, 0, 0)) * Math::cQuaternion(Math::ConvertDegreesToRadians(euler.y), Math::cVector(0, 1, 0)) * Math::cQuaternion(Math::ConvertDegreesToRadians(euler.z), Math::cVector(0, 0, 1));
	Math::cVector front;
	Math::cQuaternion inversedOrientation;
	inversedOrientation = orientation.CreateInverse();
	front = Math::QuatVector(inversedOrientation, Math::cVector(0, 0, -1));
	forward = front.CreateNormalized();
	right = Math::Cross(forward, Math::cVector(0, 1, 0)).CreateNormalized();
	up = Math::Cross(right, forward);
}