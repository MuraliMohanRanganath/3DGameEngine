// Header Files
//=============

#include "cMeshBuilder.h"
#include "cMayaMeshParser.h"
#include <sstream>
#include <fstream> 
#include "../AssetBuildLibrary/UtilityFunctions.h"
#include "../../Engine/Platform/Platform.h"

// Inherited Implementation
//=========================

// Build
//------

bool eae6320::AssetBuild::cMeshBuilder::Build(const std::vector<std::string>&)
{
	bool wereThereErrors = false;
	{
		std::string copyErrorMessage;
		cMayaMeshParser mayaMeshParser;
		if (!mayaMeshParser.LoadAsset(m_path_source)) {
			wereThereErrors = true;
			std::ostringstream errorMessage;
			errorMessage << "Failed to Load lua file \"" << m_path_source;
			eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str(), __FILE__);
			return false;
		}
		
			std::ofstream outfile(m_path_target, std::ofstream::binary);
			outfile.write(reinterpret_cast<char*>(&mayaMeshParser.verticesCount), sizeof(uint32_t));
			outfile.write(reinterpret_cast<char*>(&mayaMeshParser.indicesCount), sizeof(uint32_t));
			outfile.write(reinterpret_cast<char*>(mayaMeshParser.vertexData), sizeof(eae6320::Graphics::sVertex) * mayaMeshParser.verticesCount);
			outfile.write(reinterpret_cast<char*>(mayaMeshParser.indices), sizeof(uint32_t) * mayaMeshParser.indicesCount);
			outfile.close();

			delete[] mayaMeshParser.vertexData;
			delete[] mayaMeshParser.indices;
			mayaMeshParser.vertexData = NULL;
			mayaMeshParser.indices = NULL;
	}
	return !wereThereErrors;
}
