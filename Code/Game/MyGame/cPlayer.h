#ifndef EAE6320_PLAYER_H
#define EAE6320_PLAYER_H

#include "PlayerController.h"
#include "../../Engine/Graphics/GameObject.h"
#include "../../Engine/Graphics/Camera.h"
#include "../../Engine/Graphics/DebugObject.h"

namespace eae6320
{
	namespace Networking
	{
		enum eSession;
		struct sPlayerData;
	}
	namespace Game
	{
		class cPlayer {
		public:
			bool m_hasFlag;
			int m_score = 0;
			float m_stamina = 100;
			Networking::eSession m_session;
			PlayerController controller;
			Graphics::GameObject gameObject;
			Graphics::GameObject* opponentFlag;
			Graphics::Camera camera;
			//Graphics::DebugObject debugCylinder;
			bool m_myPlayer;
		public:
			bool Initialize(eae6320::Networking::eSession i_sessionType, bool i_myPlayer);
			void Update();
			bool CleanUp();
			void ResetOpponentFlag();
			void UpdateScore();
		private:
			Graphics::DebugObject debugLine;
			//Graphics::DebugObject debugLine2;
			void UpdateStamina();
		};
	}
}
#endif // EAE6320_PLAYER_H