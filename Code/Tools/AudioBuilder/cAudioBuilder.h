#ifndef EAE6320_AUDIO_BUILDER_H
#define EAE6320_AUDIO_BUILDER_H

#include "../AssetBuildLibrary/cbBuilder.h"

namespace eae6320
{
	namespace AssetBuild
	{
		class cAudioBuilder : public cbBuilder
		{

		private:

			virtual bool Build(const std::vector<std::string>& i_arguments);
		};
	}
}
#endif // EAE6320_AUDIO_BUILDER_H