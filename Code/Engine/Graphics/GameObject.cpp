#include "GameObject.h"
#include "../Math/cVector.h"

void eae6320::Graphics::GameObject::Move(Math::cVector i_position)
{
	transform.Move(i_position);
	meshObject.position = i_position;
	if (Math::cVector(rigidBody.velocity.x, 0, rigidBody.velocity.z).GetLength() > 1.0f)
	{
		float dot = Math::Dot(Math::cVector(rigidBody.velocity.x, 0, rigidBody.velocity.z).CreateNormalized(), transform.getForward());
		float angle = std::acos(dot) *(180.0f / Math::Pi);
		Math::cVector cross = Math::Cross(Math::cVector(rigidBody.velocity.x, 0, rigidBody.velocity.z), transform.getForward());
		if (Math::Dot(transform.getUp(), cross) < 0)
		{
			angle = -angle;
		}
		if ((angle > 1.0f) || (angle < -1.0f))
		{
			Rotate(transform.getRotation() + Math::cVector(0, angle, 0));
		}
	}
}

void eae6320::Graphics::GameObject::Rotate(Math::cVector i_rotation)
{
	transform.Rotate(i_rotation);
	meshObject.rotation = transform.getOrientation();
}

void eae6320::Graphics::GameObject::Initialize(Math::cVector initPosition, Math::cVector initRotation, char* meshFilePath, char* materialFilePath)
{
	meshObject.mesh = new Graphics::Mesh();
	if (meshFilePath != nullptr)
	{
		meshObject.mesh->LoadBinaryFile(meshFilePath);
	}
	meshObject.material = new Graphics::cMaterial();
	if (meshFilePath != nullptr)
	{
		meshObject.material->Load(materialFilePath);
	}
	Move(initPosition);
	Rotate(initRotation);
}

bool eae6320::Graphics::GameObject::cleanUp()
{
	if (meshObject.mesh)
		meshObject.mesh->CleanUp();
	if (meshObject.material)
		meshObject.material->CleanUp();
	return true;
}