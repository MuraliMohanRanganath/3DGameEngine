#include "cAudioBuilder.h"
#include "../AssetBuildLibrary/UtilityFunctions.h"
#include "../../Engine/Platform/Platform.h"

bool eae6320::AssetBuild::cAudioBuilder::Build(const std::vector<std::string>& )
{
	eae6320::Platform::CopyFile(m_path_source, m_path_target);
	return true;
}