#include "cMaterial.h"
#include "../Platform/Platform.h"
#include "../Logging/Logging.h"
#include "../Asserts/Asserts.h"


bool eae6320::Graphics::cMaterial::Load(const char * i_materialPath)
{
	bool wereThereErrors = false;
	eae6320::Platform::sDataFromFile materialBinaryData;
	{
		std::string errorMessage;
		if (!eae6320::Platform::LoadBinaryFile(i_materialPath, materialBinaryData, &errorMessage))
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, errorMessage.c_str());
			eae6320::Logging::OutputError("Failed to load the binary file \"%s\": %s", i_materialPath, errorMessage.c_str());
		}
		uint8_t* data = reinterpret_cast<uint8_t*>(materialBinaryData.data);

		sMaterial* materialInfo = reinterpret_cast<eae6320::Graphics::sMaterial*>(data);
		const char* const effectPath = reinterpret_cast<char*>(data + sizeof(eae6320::Graphics::sMaterial));
		m_materialConstantBuffer = new eae6320::Graphics::ConstantBufferDataManager();
		m_effect = new eae6320::Graphics::cEffect();
		m_materialConstantBuffer->Initialize(eae6320::Graphics::ConstantBufferType::MATERIAL_DATA, sizeof(eae6320::Graphics::sMaterial), materialInfo);
		m_effect->Load(effectPath);

		const char* const texturePath = reinterpret_cast<char*>(data + sizeof(eae6320::Graphics::sMaterial) + std::strlen(effectPath) + 1);
		m_texture = new eae6320::Graphics::cTexture();
		m_texture->Load(texturePath);
		
		return true;
	}
	materialBinaryData.Free();
	return !wereThereErrors;
}

void eae6320::Graphics::cMaterial::Bind()
{
	if (m_materialConstantBuffer)
		m_materialConstantBuffer->Bind();
	if (m_effect)
		m_effect->Bind();
	if (m_texture)
		m_texture->Bind(0);
}

bool eae6320::Graphics::cMaterial::CleanUp()
{
	m_materialConstantBuffer->CleanUp();
	m_effect->CleanUp();
	m_texture->CleanUp();
	return true;
}