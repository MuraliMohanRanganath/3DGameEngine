#ifndef EAE6320_PLAYER_CONTROLLER_H
#define EAE6320_PLAYER_CONTROLLER_H

#include "../../Engine/Math/cVector.h"
#include "../../Engine/Math/cQuaternion.h"
#include "../../Engine/Graphics/GameObject.h"

namespace eae6320
{
	class PlayerController
	{
	public:
		float MAXSPEED;
		void Update(Graphics::GameObject & gameObject, Graphics::Camera &camera);
		void UpdateCamera(Graphics::Camera &camera, Graphics::GameObject gameObject);
	private:
		Math::cVector offset;
		Math::cQuaternion x;
		Math::cQuaternion y;
		Math::cQuaternion z;
	};
}
#endif // !EAE6320_PLAYER_CONTROLLER_H