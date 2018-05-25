#ifndef EAE6320_NETWORKING_H
#define EAE6320_NETWORKING_H

#include "../Math/cVector.h"
#include <functional>

namespace eae6320 {
	namespace Networking {
		enum eSession {
			CLIENT = 0,
			SERVER = 1
		};
		struct sPlayerData {
			eae6320::Math::cVector m_position;
			eae6320::Math::cVector m_rotation;
			bool m_hasFalg;
			int m_score;
			float m_speed;
		};

		bool Initialize();
		void Load(const char* const i_path, std::function<void(eae6320::Networking::eSession, bool)> i_callback);
		void Update();
		bool CleanUp();
		void SubmitMainPlayerData(sPlayerData* i_playerData);
		void UpdateRemotePlayerData(std::function<void(eae6320::Networking::sPlayerData*)> i_update_callback);
	}
}
#endif // EAE6320_NETWORKING_H