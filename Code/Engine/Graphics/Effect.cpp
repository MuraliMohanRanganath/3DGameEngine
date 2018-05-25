#include "Effect.h"
#include "../Platform/Platform.h"
#include "../Asserts/Asserts.h"
#include "../Logging/Logging.h"

bool eae6320::Graphics::cEffect::LoadBinaryFile(const char* const i_effectBinaryFilePath)
{
	bool wereThereErrors = false;
	eae6320::Platform::sDataFromFile shaderBinaryData;
	{
		std::string errorMessage;
		if (!eae6320::Platform::LoadBinaryFile(i_effectBinaryFilePath, shaderBinaryData, &errorMessage))
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, errorMessage.c_str());
			eae6320::Logging::OutputError("Failed to load the binary file \"%s\": %s", i_effectBinaryFilePath, errorMessage.c_str());
		}
		else
		{
			uint8_t* data = reinterpret_cast<uint8_t*>(shaderBinaryData.data);
			m_renderStateBits = *reinterpret_cast<uint8_t*>(data);
			m_vertexShaderPath = reinterpret_cast<char*>(data) + sizeof(uint8_t);
			m_fragmentShaderPath = reinterpret_cast<char*>(data) + sizeof(uint8_t) + std::strlen(m_vertexShaderPath) + 1;
		}
	}
	return !wereThereErrors;
}