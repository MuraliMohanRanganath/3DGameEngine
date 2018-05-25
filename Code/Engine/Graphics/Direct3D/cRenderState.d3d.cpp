// Header Files
//=============

#include "../cRenderState.h"

// <windows.h> is #inluded before <D3D11.h> so that it is #included consistently.
// I didn't think about this and should change this next year so that D3D has it's own Includes.h file like OpenGL does :(
	#include "../../Windows/Includes.h"
#include <D3D11.h>
#include "../../Asserts/Asserts.h"
#include "../../Logging/Logging.h"
#include "Direct3dUtil.h"

// Interface
//==========

// Render
//-------

void eae6320::Graphics::cRenderState::Bind() const
{
	// Alpha Transparency
	{
		EAE6320_ASSERT( m_blendState != NULL );
		const float* const noBlendFactor = NULL;
		const unsigned int defaultSampleMask = 0xffffffff;
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->OMSetBlendState( m_blendState, noBlendFactor, defaultSampleMask );
	}
	// Depth Buffering
	{
		EAE6320_ASSERT( m_depthStencilState != NULL );
		const unsigned int unusedStencilReference = 0;
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->OMSetDepthStencilState( m_depthStencilState, unusedStencilReference );
	}
	// Draw Both Triangle Sides
	{
		EAE6320_ASSERT( m_rasterizerState != NULL );
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->RSSetState( m_rasterizerState );
	}
}

// Initialization / Clean Up
//--------------------------

bool eae6320::Graphics::cRenderState::CleanUp()
{
	if ( m_blendState )
	{
		m_blendState->Release();
		m_blendState = NULL;
	}
	if ( m_depthStencilState )
	{
		m_depthStencilState->Release();
		m_depthStencilState = NULL;
	}
	if ( m_rasterizerState )
	{
		m_rasterizerState->Release();
		m_rasterizerState = NULL;
	}

	return true;
}

// Implementation
//===============

// Initialization / Clean Up
//--------------------------

bool eae6320::Graphics::cRenderState::InitializeFromBits()
{
	bool wereThereErrors = false;

	// Alpha Transparency
	{
		D3D11_BLEND_DESC blendStateDescription = { 0 };
		// Alpha-to-coverage isn't used
		blendStateDescription.AlphaToCoverageEnable = FALSE;
		// All render targets use the same blending
		// (in our class we will only ever use a single render target)
		blendStateDescription.IndependentBlendEnable = FALSE;
		{
			D3D11_RENDER_TARGET_BLEND_DESC& renderTargetBlendDescription = blendStateDescription.RenderTarget[0];
			if ( IsAlphaTransparencyEnabled() )
			{
				renderTargetBlendDescription.BlendEnable = TRUE;

				// result = ( source * source.a ) + ( destination * ( 1 - source.a ) )
				renderTargetBlendDescription.SrcBlend = D3D11_BLEND_SRC_ALPHA;
				renderTargetBlendDescription.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
				renderTargetBlendDescription.BlendOp = D3D11_BLEND_OP_ADD;
				renderTargetBlendDescription.SrcBlendAlpha = D3D11_BLEND_ONE;
				renderTargetBlendDescription.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
				renderTargetBlendDescription.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			}
			else
			{
				renderTargetBlendDescription.BlendEnable = FALSE;

				// (Since blending is disabled the remaining states don't matter,
				// but it doesn't hurt to set them to explicitly disable alpha transparency)
				renderTargetBlendDescription.SrcBlend = D3D11_BLEND_ONE;
				renderTargetBlendDescription.DestBlend = D3D11_BLEND_ZERO;
				renderTargetBlendDescription.BlendOp = D3D11_BLEND_OP_ADD;
				renderTargetBlendDescription.SrcBlendAlpha = D3D11_BLEND_ONE;
				renderTargetBlendDescription.DestBlendAlpha = D3D11_BLEND_ZERO;
				renderTargetBlendDescription.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			}
			renderTargetBlendDescription.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
		const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateBlendState( &blendStateDescription, &m_blendState );
		if ( FAILED( result ) )
		{
			wereThereErrors = true;
			EAE6320_ASSERTF( false, "CreateBlendState() failed" );
			eae6320::Logging::OutputError( "Direct3D failed to create a blend render state object from %u with HRESULT %#010x", m_bits, result );
			goto OnExit;
		}
	}
	// Depth Buffering
	{
		D3D11_DEPTH_STENCIL_DESC depthStateDescription = { 0 };
		// We don't use the stencil buffer in our class
		depthStateDescription.StencilEnable = FALSE;
		depthStateDescription.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		depthStateDescription.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		depthStateDescription.FrontFace.StencilFunc = depthStateDescription.BackFace.StencilFunc =
			D3D11_COMPARISON_ALWAYS;
		depthStateDescription.FrontFace.StencilDepthFailOp = depthStateDescription.BackFace.StencilDepthFailOp =
			depthStateDescription.FrontFace.StencilPassOp = depthStateDescription.BackFace.StencilPassOp =
			depthStateDescription.FrontFace.StencilFailOp = depthStateDescription.BackFace.StencilFailOp =
			D3D11_STENCIL_OP_KEEP;
		if ( IsDepthBufferingEnabled() )
		{
			// The new fragment becomes a pixel if its depth is less than what has previously been written
			depthStateDescription.DepthEnable = TRUE;
			depthStateDescription.DepthFunc = D3D11_COMPARISON_LESS;
			// Write to the depth buffer
			depthStateDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		}
		else
		{
			// Don't test the depth buffer
			depthStateDescription.DepthEnable = FALSE;
			depthStateDescription.DepthFunc = D3D11_COMPARISON_ALWAYS;
			// Don't write to the depth buffer
			depthStateDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		}
		const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateDepthStencilState( &depthStateDescription, &m_depthStencilState );
		if ( FAILED( result ) )
		{
			wereThereErrors = true;
			EAE6320_ASSERTF( false, "CreateDepthStencilState() failed" );
			eae6320::Logging::OutputError( "Direct3D failed to create a depth/stencil render state object from %u with HRESULT %#010x", m_bits, result );
			goto OnExit;
		}
	}
	// Draw Both Triangle Sides
	{
		D3D11_RASTERIZER_DESC rasterizerStateDescription;
		// Draw solid geometry (i.e. not wireframe)
		rasterizerStateDescription.FillMode = D3D11_FILL_SOLID;
		// Triangles use left-handed winding order
		// (opposite from OpenGL)
		rasterizerStateDescription.FrontCounterClockwise = FALSE;
		// No depth bias
		rasterizerStateDescription.DepthBias = 0;
		rasterizerStateDescription.SlopeScaledDepthBias = 0.0f;
		rasterizerStateDescription.DepthBiasClamp = 0.0f;
		// Use default depth clipping
		rasterizerStateDescription.DepthClipEnable = TRUE;
		// Disable "scissoring" (i.e. render all pixels in the render target)
		rasterizerStateDescription.ScissorEnable = FALSE;
		// Disable multisampling/antialiasing
		rasterizerStateDescription.MultisampleEnable = FALSE;
		rasterizerStateDescription.AntialiasedLineEnable = FALSE;
		if ( ShouldBothTriangleSidesBeDrawn() )
		{
			// Don't cull any triangles
			rasterizerStateDescription.CullMode = D3D11_CULL_NONE;
		}
		else
		{
			// Cull triangles that are facing backwards
			rasterizerStateDescription.CullMode = D3D11_CULL_BACK;
		}
		const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateRasterizerState( &rasterizerStateDescription, &m_rasterizerState );
		if ( FAILED( result ) )
		{
			wereThereErrors = true;
			EAE6320_ASSERTF( false, "CreateRasterizerState() failed" );
			eae6320::Logging::OutputError( "Direct3D failed to create a rasterizer render state object from %u with HRESULT %#010x", m_bits, result );
			goto OnExit;
		}
	}

OnExit:

	return !wereThereErrors;
}
