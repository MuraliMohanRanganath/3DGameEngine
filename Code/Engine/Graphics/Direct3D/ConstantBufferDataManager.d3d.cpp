#include "../ConstantBufferDataManager.h"
#include "../ConstantBufferData.h"
#include "../../Asserts/Asserts.h"
#include "../../Logging/Logging.h"
#include "../../Time/Time.h"
#include "Direct3dUtil.h"
#include "../../Math/Functions.h"


bool eae6320::Graphics::ConstantBufferDataManager::Initialize(ConstantBufferType bufferType, size_t  bufferSize, void * bufferData) {
	s_bufferType = bufferType;
	s_bufferSize = bufferSize;

	D3D11_BUFFER_DESC bufferDescription = { 0 };
	{
		// The byte width must be rounded up to a multiple of 16
		bufferDescription.ByteWidth = eae6320::Math::RoundUpToMultiple_powerOf2(static_cast<unsigned int>(bufferSize), 16u);
		bufferDescription.Usage = D3D11_USAGE_DYNAMIC;	// The CPU must be able to update the buffer
		bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// The CPU must write, but doesn't read
		bufferDescription.MiscFlags = 0;
		bufferDescription.StructureByteStride = 0;	// Not used
	}
	D3D11_SUBRESOURCE_DATA initialData = { 0 };
	{
		if(bufferType==FRAME_DATA)		{	// Fill in the constant buffer
			reinterpret_cast<sFrame*>(bufferData)->g_elapsedSecondCount_total = eae6320::Time::GetElapsedSecondCount_total();
		}
		else if(bufferType==DRAW_CALL_DATA) {

		}
		else if(bufferType== MATERIAL_DATA)
		{

		}
		initialData.pSysMem = &bufferData;
	}

	const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateBuffer(&bufferDescription, &initialData, &s_constantBufferData);
	if (SUCCEEDED(result))
	{
		return true;
	}
	else
	{
		EAE6320_ASSERT(false);
		eae6320::Logging::OutputError("Direct3D failed to create a constant buffer with HRESULT %#010x", result);
		return false;
	}
}

bool eae6320::Graphics::ConstantBufferDataManager::Bind() {
	// Bind the constant buffer to the shader
	const unsigned int bufferCount = 1;
	const unsigned int registerAssignedInShader = static_cast<int>(s_bufferType);
	eae6320::Graphics::Direct3dUtil::getDirect3dContext()->VSSetConstantBuffers(registerAssignedInShader, bufferCount, &s_constantBufferData);
	eae6320::Graphics::Direct3dUtil::getDirect3dContext()->PSSetConstantBuffers(registerAssignedInShader, bufferCount, &s_constantBufferData);
	return true;
}

bool eae6320::Graphics::ConstantBufferDataManager::CleanUp() {
	if (s_constantBufferData)
	{
		s_constantBufferData->Release();
		s_constantBufferData = NULL;
	}
	return true;
}

bool eae6320::Graphics::ConstantBufferDataManager::Update(void* bufferData) {

	// Get a pointer from Direct3D that can be written to
	void* memoryToWriteTo = NULL;
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		{
			// Discard previous contents when writing
			const unsigned int noSubResources = 0;
			const D3D11_MAP mapType = D3D11_MAP_WRITE_DISCARD;
			const unsigned int noFlags = 0;
			const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dContext()->Map(s_constantBufferData, noSubResources, mapType, noFlags, &mappedSubResource);
			if (SUCCEEDED(result))
			{
				memoryToWriteTo = mappedSubResource.pData;
			}
			else
			{
				EAE6320_ASSERT(false);
			}
		}
	}
	if (memoryToWriteTo)
	{
		// Copy the new data to the memory that Direct3D has provided
		memcpy(memoryToWriteTo, bufferData, s_bufferSize);
		// Let Direct3D know that the memory contains the data
		// (the pointer will be invalid after this call)
		const unsigned int noSubResources = 0;
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->Unmap(s_constantBufferData, noSubResources);
		memoryToWriteTo = NULL;
	}
	return true;
}