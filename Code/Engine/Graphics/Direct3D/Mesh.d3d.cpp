#include "../Mesh.h"
#include <D3D11.h>
#include <D3DX11async.h>
#include <D3DX11core.h>
#include <DXGI.h>
#include "../../Asserts/Asserts.h"
#include "../../Logging/Logging.h"
#include "../VertexData.h"
#include <algorithm>
#include "Direct3dUtil.h"

bool eae6320::Graphics::Mesh::Initialize() {
	s_vertexBuffer = NULL;
	s_indexBuffer = NULL;
		
	const unsigned int bufferSize = verticesCount * sizeof(sVertex);
	const unsigned int indexBufferSize = indicesCount*sizeof(uint32_t);
	
	D3D11_BUFFER_DESC bufferDescription = { 0 };
	{
		bufferDescription.ByteWidth = bufferSize;
		bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
		bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDescription.CPUAccessFlags = 0;	// No CPU access is necessary
		bufferDescription.MiscFlags = 0;
		bufferDescription.StructureByteStride = 0;	// Not used
	}
	D3D11_SUBRESOURCE_DATA initialData = { 0 };
	{
		initialData.pSysMem = vertexData;
		// (The other data members are ignored for non-texture buffers)
	}

	const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateBuffer(&bufferDescription, &initialData, &s_vertexBuffer);
	if (FAILED(result))
	{
		EAE6320_ASSERT(false);
		eae6320::Logging::OutputError("Direct3D failed to create the vertex buffer with HRESULT %#010x", result);
		return false;
	}

	D3D11_BUFFER_DESC indexBufferDescription = { 0 };
	{
		indexBufferDescription.ByteWidth = indexBufferSize;
		indexBufferDescription.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
		indexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDescription.CPUAccessFlags = 0;	// No CPU access is necessary
		indexBufferDescription.MiscFlags = 0;
		indexBufferDescription.StructureByteStride = 0;	// Not used
	}
	D3D11_SUBRESOURCE_DATA indexInitialData = { 0 };
	{
		indexInitialData.pSysMem = indices;
		// (The other data members are ignored for non-texture buffers)
	}

	const HRESULT indexBufferCreationResult = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateBuffer(&indexBufferDescription, &indexInitialData, &s_indexBuffer);
	if (FAILED(indexBufferCreationResult))
	{
		EAE6320_ASSERT(false);
		eae6320::Logging::OutputError("Direct3D failed to create the index buffer with HRESULT %#010x", indexBufferCreationResult);
		return false;
	}
	return true;
}

void eae6320::Graphics::Mesh::DrawFrame() {
	// Bind a specific vertex buffer to the device as a data source
	{
		const unsigned int startingSlot = 0;
		const unsigned int vertexBufferCount = 1;
		// The "stride" defines how large a single vertex is in the stream of data
		const unsigned int bufferStride = sizeof(sVertex);
		// It's possible to start streaming data in the middle of a vertex buffer
		const unsigned int bufferOffset = 0;
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->IASetVertexBuffers(startingSlot, vertexBufferCount, &s_vertexBuffer, &bufferStride, &bufferOffset);
	}

	/* Render triangles from the currently-bound vertex buffer
	{
		// As of this comment we are only drawing a single triangle
		// (you will have to update this code in future assignments!)
		const unsigned int triangleCount = 2;
		const unsigned int vertexCountPerTriangle = 3;
		const unsigned int vertexCountToRender = triangleCount * vertexCountPerTriangle;
		// It's possible to start rendering primitives in the middle of the stream
		const unsigned int indexOfFirstVertexToRender = 0;
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->Draw(vertexCountToRender, indexOfFirstVertexToRender);
	}*/
	// Bind a specific vertex buffer to the device as a data source
	{
		EAE6320_ASSERT(s_indexBuffer != NULL);
		// Every index is a 16 bit unsigned integer
		const DXGI_FORMAT format = DXGI_FORMAT_R32_UINT;
		// The indices start at the beginning of the buffer
		const unsigned int offset = 0;
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->IASetIndexBuffer(s_indexBuffer, format, offset);
	}
	// Render triangles from the currently-bound vertex and index buffers
	{
		// It's possible to start rendering primitives in the middle of the stream
		const unsigned int indexOfFirstIndexToUse = 0;
		const unsigned int offsetToAddToEachIndex = 0;
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->DrawIndexed(indicesCount, indexOfFirstIndexToUse, offsetToAddToEachIndex);
	}
}

bool eae6320::Graphics::Mesh::CleanUp() {
	bool wereThereErrors = false;
	if (s_vertexBuffer)
	{
		s_vertexBuffer->Release();
		s_vertexBuffer = NULL;
	}
	if (s_indexBuffer) {
		s_indexBuffer->Release();
		s_indexBuffer = NULL;
	}

	return !wereThereErrors;
}