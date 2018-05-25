#include "../Effect.h"
#include "../../Platform/Platform.h"
#include "../../Asserts/Asserts.h"
#include "Direct3dUtil.h"
#include "../../Logging/Logging.h"
#include "../VertexData.h"


// D3D has an "input layout" object that associates the layout of the struct above
// with the input from a vertex shader
//ID3D11InputLayout* s_vertexLayout = NULL;

// The vertex shader is a program that operates on vertices.
// Its input comes from a C/C++ "draw call" and is:
//	* Position
//	* Any other data we want
// Its output is:
//	* Position
//		(So that the graphics hardware knows which pixels to fill in for the triangle)
//	* Any other data we want
//ID3D11VertexShader* s_vertexShader = NULL;
// The fragment shader is a program that operates on fragments
// (or potential pixels).
// Its input is:
//	* The data that was output from the vertex shader,
//		interpolated based on how close the fragment is
//		to each vertex in the triangle.
// Its output is:
//	* The final color that the pixel should be
//ID3D11PixelShader* s_fragmentShader = NULL;

bool eae6320::Graphics::cEffect::Load(const char* const i_effectBinaryFilePath)
{
	bool wereThereErrors = false;
	eae6320::Platform::sDataFromFile compiledVertexShader;
	eae6320::Platform::sDataFromFile compiledShader;

	if (!LoadBinaryFile(i_effectBinaryFilePath)) {
		wereThereErrors = true;
		goto OnExit;
	}

	// Load the compiled shader
	{
		std::string errorMessage;
		if (!eae6320::Platform::LoadBinaryFile(m_vertexShaderPath, compiledVertexShader, &errorMessage))
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, errorMessage.c_str());
			eae6320::Logging::OutputError("Failed to load the shader file \"%s\": %s", m_vertexShaderPath, errorMessage.c_str());
			goto OnExit;
		}
	}
	// Create the shader object
	{
		ID3D11ClassLinkage* const noInterfaces = NULL;
		const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateVertexShader(compiledVertexShader.data, compiledVertexShader.size, noInterfaces, &s_vertexShader);
		if (FAILED(result))
		{
			wereThereErrors = true;
			EAE6320_ASSERT(false);
			eae6320::Logging::OutputError("Direct3D failed to create the shader %s with HRESULT %#010x", m_vertexShaderPath, result);
			goto OnExit;
		}
	}

	// Load the compiled shader
	{
		std::string errorMessage;
		if (!eae6320::Platform::LoadBinaryFile(m_fragmentShaderPath, compiledShader, &errorMessage))
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, errorMessage.c_str());
			eae6320::Logging::OutputError("Failed to load the shader file \"%s\": %s", m_fragmentShaderPath, errorMessage.c_str());
			goto OnExit;
		}
	}
	// Create the shader object
	{
		ID3D11ClassLinkage* const noInterfaces = NULL;
		const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreatePixelShader(
			compiledShader.data, compiledShader.size, noInterfaces, &s_fragmentShader);
		if (FAILED(result))
		{
			wereThereErrors = true;
			EAE6320_ASSERT(false);
			eae6320::Logging::OutputError("Direct3D failed to create the shader %s with HRESULT %#010x", m_fragmentShaderPath, result);
			goto OnExit;
		}
	}

	// Create the vertex layout
	{
		// These elements must match the VertexFormat::sVertex layout struct exactly.
		// They instruct Direct3D how to match the binary data in the vertex buffer
		// to the input elements in a vertex shader
		// (by using so-called "semantic" names so that, for example,
		// "POSITION" here matches with "POSITION" in shader code).
		// Note that OpenGL uses arbitrarily assignable number IDs to do the same thing.
		const unsigned int vertexElementCount = 3;
		D3D11_INPUT_ELEMENT_DESC layoutDescription[vertexElementCount] = { 0 ,0 };
		{
			{
				D3D11_INPUT_ELEMENT_DESC& positionElement = layoutDescription[0];

				positionElement.SemanticName = "POSITION";
				positionElement.SemanticIndex = 0;	// (Semantics without modifying indices at the end can always use zero)
				positionElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				positionElement.InputSlot = 0;
				positionElement.AlignedByteOffset = offsetof(eae6320::Graphics::sVertex, x);
				positionElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				positionElement.InstanceDataStepRate = 0;	// (Must be zero for per-vertex data)
			}
			{
				D3D11_INPUT_ELEMENT_DESC& colorElement = layoutDescription[1];
				colorElement.SemanticName = "COLOR";
				colorElement.SemanticIndex = 0;	// (Semantics without modifying indices at the end can always use zero)
				colorElement.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				colorElement.InputSlot = 0;
				colorElement.AlignedByteOffset = offsetof(eae6320::Graphics::sVertex, red);
				colorElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				colorElement.InstanceDataStepRate = 0;	// (Must be zero for per-vertex data)
			}
			{
				D3D11_INPUT_ELEMENT_DESC& colorElement = layoutDescription[2];
				colorElement.SemanticName = "TEXCOORD";
				colorElement.SemanticIndex = 0;	// (Semantics without modifying indices at the end can always use zero)
				colorElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				colorElement.InputSlot = 0;
				colorElement.AlignedByteOffset = offsetof(eae6320::Graphics::sVertex, u);
				colorElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				colorElement.InstanceDataStepRate = 0;	// (Must be zero for per-vertex data)
			}
		}

		const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateInputLayout(layoutDescription, vertexElementCount,
			compiledVertexShader.data, compiledVertexShader.size, &s_vertexLayout);
		if (FAILED(result))
		{
			EAE6320_ASSERT(false);
			eae6320::Logging::OutputError("Direct3D failed to create a vertex input layout with HRESULT %#010x", result);
			wereThereErrors = true;
			goto OnExit;
		}
	}

	//Initializing render state when effect is loaded
	m_renderState.Initialize(m_renderStateBits);

OnExit:
	compiledVertexShader.Free();
	compiledShader.Free();
	return !wereThereErrors;
}

void eae6320::Graphics::cEffect::Bind()
{
	// Set the vertex and fragment shaders
	{
		ID3D11ClassInstance** const noInterfaces = NULL;
		const unsigned int interfaceCount = 0;
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->VSSetShader(s_vertexShader, noInterfaces, interfaceCount);
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->PSSetShader(s_fragmentShader, noInterfaces, interfaceCount);
	}
	// Specify what kind of data the vertex buffer holds
	{
		// Set the layout (which defines how to interpret a single vertex)
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->IASetInputLayout(s_vertexLayout);
		// Set the topology (which defines how to interpret multiple vertices as a single "primitive";
		// we have defined the vertex buffer as a triangle list
		// (meaning that every primitive is a triangle and will be defined by three vertices)
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	// Bind the render state of the effect
	{
		m_renderState.Bind();
	}
}

bool eae6320::Graphics::cEffect::CleanUp()
{
	bool wereThereErrors = false;

	if (s_vertexLayout)
	{
		s_vertexLayout->Release();
		s_vertexLayout = NULL;
	}
	if (s_vertexShader)
	{
		s_vertexShader->Release();
		s_vertexShader = NULL;
	}
	if (s_fragmentShader)
	{
		s_fragmentShader->Release();
		s_fragmentShader = NULL;
	}

	if (m_renderState.CleanUp()) {
		wereThereErrors = true;
	}
	return !wereThereErrors;
}