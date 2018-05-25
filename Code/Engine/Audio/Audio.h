#ifndef EAE6320_AUDIO_H
#define EAE6320_AUDIO_H

#include "../Graphics/Transform.h"

namespace eae6320
{
	namespace Graphics
	{
		class Transform;
	}

	namespace Audio
	{
		void Update();
		bool Initialize();
		bool PlayMusic(const char* const i_path);
		bool PlayEffect(const char* const i_path);
		float GetMusicVolume();
		float GetEffectVolume();
		void ModifyMusicVolume(const float i_value);
		void ModifyEffectVolume(const float i_value);
		void UpdateListener(const Graphics::Transform& i_transform);
		void AddStaticEmitter(const Graphics::Transform& i_transform, const char* const i_path, bool * o_soundPlaying);
		bool CleanUp();
	}

}
#endif // EAE6320_AUDIO_H