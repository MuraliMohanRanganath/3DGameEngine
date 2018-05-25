// Header Files
//=============

#include "../Graphics.h"

#include <cstddef>
#include <cstdint>
#include <D3D11.h>
#include <D3DX11async.h>
#include <D3DX11core.h>
#include <DXGI.h>
#include "../../Asserts/Asserts.h"
#include "../../Logging/Logging.h"
#include "../../Time/Time.h"
#include "Direct3dUtil.h"
#include "../VertexData.h"
#include "../ConstantBufferData.h"
#include "../ConstantBufferDataManager.h"
#include "../../Math/cVector.h"
#include "../../Math/cQuaternion.h"
#include "../../Math/cMatrix_transformation.h"
#include "../../Platform/Platform.h"
#include "../Mesh.h"
#include <vector>
#include "../Effect.h"

// Static Data Initialization
//===========================

namespace
{
	ID3D11RenderTargetView* s_renderTargetView = NULL;
	ID3D11DepthStencilView* s_depthStencilView = NULL;
	eae6320::Graphics::ConstantBufferDataManager * frameConstantBufferManager = new eae6320::Graphics::ConstantBufferDataManager();
	eae6320::Graphics::ConstantBufferDataManager * drawCallConstantBufferManager = new eae6320::Graphics::ConstantBufferDataManager();
	eae6320::Graphics::sFrame frameData;
	eae6320::Graphics::sDrawCall drawCallData;
	std::vector<eae6320::Graphics::GameObject> gameObjects;
	std::vector<eae6320::Graphics::UIObject> uiObjects;
	eae6320::Graphics::Camera *m_camera;
	ID3D11SamplerState*  s_samplerState;
}

// Helper Function Declarations
//=============================

namespace
{
	bool CreateConstantBuffer();
	bool CreateDevice( const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight );
	bool CreateView( const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight );
	bool CreateSamplerState();
}


void eae6320::Graphics::SetMesh(Graphics::GameObject i_gameObject) {
	eae6320::Graphics::GameObject gameObject;
	gameObject.mesh = i_gameObject.mesh;
	gameObject.material = i_gameObject.material;
	gameObject.position = i_gameObject.position;
	gameObject.orientation = i_gameObject.orientation;
	gameObjects.push_back(gameObject);
}

void eae6320::Graphics::SetSprite(UIObject i_uiObject)
{
	eae6320::Graphics::UIObject uiObject;
	uiObject.material = i_uiObject.material;
	uiObject.sprite = i_uiObject.sprite;
	uiObjects.push_back(uiObject);
}


void eae6320::Graphics::SetCamera(Graphics::Camera *i_camera) {
	m_camera = i_camera;
}

// Interface
//==========

// Render
//-------

void eae6320::Graphics::RenderFrame()
{
	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	{
		// Black is usually used
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->ClearRenderTargetView( s_renderTargetView, clearColor );

		const float depthToClear = 1.0f;
		const uint8_t stencilToClear = 0; // Arbitrary until stencil is used
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->ClearDepthStencilView(s_depthStencilView, D3D11_CLEAR_DEPTH,
			depthToClear, stencilToClear);
	}

	frameData.g_transform_worldToCamera = m_camera->CalculateWorldToCameraTransformationMatrix();
	frameData.g_transform_cameraToScreen = m_camera->CalculateCameraToScreenTransformationMatrix();
	frameData.g_elapsedSecondCount_total = Time::GetElapsedSecondCount_total();
	frameConstantBufferManager->Update(&frameData);

	// Draw the geometry
	{
		for (eae6320::Graphics::GameObject currentMeshDetail : gameObjects) {
			currentMeshDetail.material.Bind();
			Math::cMatrix_transformation transformationMatrix = Math::cMatrix_transformation(currentMeshDetail.orientation, currentMeshDetail.position);
			drawCallData.g_transform_localToWorld = transformationMatrix;
			drawCallConstantBufferManager->Update(&drawCallData);
			eae6320::Graphics::Mesh meshTobeDrawn = currentMeshDetail.mesh;
			meshTobeDrawn.DrawFrame();
		}
		gameObjects.clear();
	}

	// Draw UI Objects
	{
		for (eae6320::Graphics::UIObject currentSprite : uiObjects)
		{
			currentSprite.material.Bind();
			currentSprite.sprite->Draw();
		}
		uiObjects.clear();
	}

	// Everything has been drawn to the "back buffer", which is just an image in memory.
	// In order to display it the contents of the back buffer must be "presented"
	// (to the front buffer)
	{
		const unsigned int swapImmediately = 0;
		const unsigned int presentNextFrame = 0;
		const HRESULT result = eae6320::Graphics::Direct3dUtil::getDXGISwapChain()->Present( swapImmediately, presentNextFrame );
		EAE6320_ASSERT( SUCCEEDED( result ) );
	}
}

// Initialization / Clean Up
//--------------------------

bool eae6320::Graphics::Initialize( const sInitializationParameters& i_initializationParameters )
{
	bool wereThereErrors = false;

	eae6320::Graphics::Direct3dUtil::setRenderWindow(i_initializationParameters.mainWindow);
	

	// Create an interface to a Direct3D device
	if ( !CreateDevice( i_initializationParameters.resolutionWidth, i_initializationParameters.resolutionHeight ) )
	{
		wereThereErrors = true;
		goto OnExit;
	}

	if (!CreateSamplerState())
	{
		wereThereErrors = true;
		goto OnExit;
	}

	// Initialize the viewport of the device
	if ( !CreateView( i_initializationParameters.resolutionWidth, i_initializationParameters.resolutionHeight ) )
	{
		wereThereErrors = true;
		goto OnExit;
	}

	// Initialize the graphics objects
	if ( !CreateConstantBuffer() )
	{
		wereThereErrors = true;
		goto OnExit;
	}
OnExit:
	return !wereThereErrors;
}

bool eae6320::Graphics::CleanUp()
{
	bool wereThereErrors = false;

	if (eae6320::Graphics::Direct3dUtil::getDirect3dDevice())
	{
		frameConstantBufferManager->CleanUp();
		drawCallConstantBufferManager->CleanUp();

		if (s_samplerState)
		{
			s_samplerState->Release();
			s_samplerState = NULL;
		}

		if ( s_renderTargetView )
		{
			s_renderTargetView->Release();
			s_renderTargetView = NULL;
		}

		if (s_depthStencilView) {
			s_depthStencilView->Release();
			s_depthStencilView = NULL;
		}

		eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->Release();
		eae6320::Graphics::Direct3dUtil::setDirect3dDevice(NULL);
	}
	if (eae6320::Graphics::Direct3dUtil::getDirect3dContext())
	{
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->Release();
		eae6320::Graphics::Direct3dUtil::setDirect3dContext(NULL);
	}
	if (eae6320::Graphics::Direct3dUtil::getDXGISwapChain() )
	{
		eae6320::Graphics::Direct3dUtil::getDXGISwapChain()->Release();
		eae6320::Graphics::Direct3dUtil::setDXGISwapChain(NULL);
	}
	
	eae6320::Graphics::Direct3dUtil::setRenderWindow(NULL);
	return !wereThereErrors;
}

// Helper Function Definitions
//============================

namespace 
{
	bool CreateConstantBuffer()
	{
		
		frameConstantBufferManager->Initialize(eae6320::Graphics::FRAME_DATA,sizeof(eae6320::Graphics::sFrame),&frameData);
		drawCallConstantBufferManager->Initialize(eae6320::Graphics::DRAW_CALL_DATA, sizeof(eae6320::Graphics::sDrawCall), &drawCallData);
		frameConstantBufferManager->Bind();
		drawCallConstantBufferManager->Bind();
		return true;
	}

	bool CreateDevice( const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight )
	{
		return eae6320::Graphics::Direct3dUtil::InitializeState(i_resolutionWidth,i_resolutionHeight);
	}

	bool CreateView( const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight )
	{
		bool wereThereErrors = false;

		ID3D11Texture2D* backBuffer = NULL;
		ID3D11Texture2D* depthBuffer = NULL;

		// Create a "render target view" of the back buffer
		// (the back buffer was already created by the call to D3D11CreateDeviceAndSwapChain(),
		// but we need a "view" of it to use as a "render target",
		// meaning a texture that the GPU can render to)
		{
			// Get the back buffer from the swap chain
			{
				const unsigned int bufferIndex = 0; // This must be 0 since the swap chain is discarded
				const HRESULT result = eae6320::Graphics::Direct3dUtil::getDXGISwapChain()->GetBuffer(bufferIndex, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
				if (FAILED(result))
				{
					EAE6320_ASSERT(false);
					eae6320::Logging::OutputError("Direct3D failed to get the back buffer from the swap chain with HRESULT %#010x", result);
					goto OnExit;
				}
			}
			// Create the view
			{
				const D3D11_RENDER_TARGET_VIEW_DESC* const accessAllSubResources = NULL;
				const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateRenderTargetView(backBuffer, accessAllSubResources, &s_renderTargetView);
				if (FAILED(result))
				{
					EAE6320_ASSERT(false);
					eae6320::Logging::OutputError("Direct3D failed to create the render target view with HRESULT %#010x", result);
					goto OnExit;
				}
			}
		}
		// Create a depth/stencil buffer and a view of it
		{
			// Unlike the back buffer no depth/stencil buffer exists until and unless we create it
			{
				D3D11_TEXTURE2D_DESC textureDescription = { 0 };
				{
					textureDescription.Width = i_resolutionWidth;
					textureDescription.Height = i_resolutionHeight;
					textureDescription.MipLevels = 1; // A depth buffer has no MIP maps
					textureDescription.ArraySize = 1;
					textureDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24 bits for depth and 8 bits for stencil
					{
						DXGI_SAMPLE_DESC& sampleDescription = textureDescription.SampleDesc;
						sampleDescription.Count = 1; // No multisampling
						sampleDescription.Quality = 0; // Doesn't matter when Count is 1
					}
					textureDescription.Usage = D3D11_USAGE_DEFAULT; // Allows the GPU to write to it
					textureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
					textureDescription.CPUAccessFlags = 0; // CPU doesn't need access
					textureDescription.MiscFlags = 0;
				}
				// The GPU renders to the depth/stencil buffer and so there is no initial data
				// (like there would be with a traditional texture loaded from disk)
				const D3D11_SUBRESOURCE_DATA* const noInitialData = NULL;
				const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateTexture2D(&textureDescription, noInitialData, &depthBuffer);
				if (FAILED(result))
				{
					EAE6320_ASSERT(false);
					eae6320::Logging::OutputError("Direct3D failed to create the depth buffer resource with HRESULT %#010x", result);
					goto OnExit;
				}
			}
			// Create the view
			{
				const D3D11_DEPTH_STENCIL_VIEW_DESC* const noSubResources = NULL;
				const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateDepthStencilView(depthBuffer, noSubResources, &s_depthStencilView);
				if (FAILED(result))
				{
					EAE6320_ASSERT(false);
					eae6320::Logging::OutputError("Direct3D failed to create the depth stencil view with HRESULT %#010x", result);
					goto OnExit;
				}
			}
		}

		// Bind the views
		{
			const unsigned int renderTargetCount = 1;
			eae6320::Graphics::Direct3dUtil::getDirect3dContext()->OMSetRenderTargets(renderTargetCount, &s_renderTargetView, s_depthStencilView);
		}
		// Specify that the entire render target should be visible
		{
			D3D11_VIEWPORT viewPort = { 0 };
			viewPort.TopLeftX = viewPort.TopLeftY = 0.0f;
			viewPort.Width = static_cast<float>(i_resolutionWidth);
			viewPort.Height = static_cast<float>(i_resolutionHeight);
			viewPort.MinDepth = 0.0f;
			viewPort.MaxDepth = 1.0f;
			const unsigned int viewPortCount = 1;
			eae6320::Graphics::Direct3dUtil::getDirect3dContext()->RSSetViewports(viewPortCount, &viewPort);
		}

	OnExit:

		if (backBuffer)
		{
			backBuffer->Release();
			backBuffer = NULL;
		}
		if (depthBuffer)
		{
			depthBuffer->Release();
			depthBuffer = NULL;
		}

		return !wereThereErrors;
	}

	bool CreateSamplerState()
	{
		bool wereThereErrors = false;

		// Create a sampler state object
		{
			D3D11_SAMPLER_DESC samplerStateDescription;
			{
				// Linear filtering
				samplerStateDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				// If UVs go outside [0,1] wrap them around (so that textures can tile)
				samplerStateDescription.AddressU = samplerStateDescription.AddressV
					= samplerStateDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
				// Default values
				samplerStateDescription.MipLODBias = 0.0f;
				samplerStateDescription.MaxAnisotropy = 1;
				samplerStateDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
				samplerStateDescription.BorderColor[0] = samplerStateDescription.BorderColor[1]
					= samplerStateDescription.BorderColor[2] = samplerStateDescription.BorderColor[3] = 1.0f;
				samplerStateDescription.MinLOD = -FLT_MAX;
				samplerStateDescription.MaxLOD = FLT_MAX;
			}
			const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateSamplerState(&samplerStateDescription, &s_samplerState);
			if (FAILED(result))
			{
				EAE6320_ASSERT(false);
				eae6320::Logging::OutputError("Direct3D failed to create a sampler state with HRESULT %#010x", result);
				wereThereErrors = true;
				goto OnExit;
			}
		}
		// Bind the sampler state
		{
			const unsigned int samplerStateRegister = 0; // This must match the SamplerState register defined in the shader
			const unsigned int samplerStateCount = 1;
			eae6320::Graphics::Direct3dUtil::getDirect3dContext()->PSSetSamplers(samplerStateRegister, samplerStateCount, &s_samplerState);
		}

	OnExit:
		return !wereThereErrors;
	}
}
