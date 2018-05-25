#ifndef EAE6320_GAMEOBJECT_H
#define EAE6320_GAMEOBJECT_H

#include "Transform.h"
#include "../../Engine/Math/cVector.h"
#include "Graphics.h"
#include "../Physics/RigidBody.h"

namespace eae6320
{
	namespace Graphics
	{
		class GameObject
		{
		public:
			Graphics::MeshObject meshObject;
			Physics::RigidBody rigidBody;
			Transform transform;
		public:
			void Move(Math::cVector);
			void Rotate(Math::cVector);
			void Initialize(Math::cVector, Math::cVector, char*, char*);
			bool cleanUp();
		};
	}
}
#endif	// EAE6320_GAMEOBJECT_H