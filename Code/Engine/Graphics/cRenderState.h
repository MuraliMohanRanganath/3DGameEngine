/*
	Render state is the fixed-function state
	that can be configured on the GPU
*/

#ifndef EAE6320_GRAPHICS_CRENDERSTATE_H
#define EAE6320_GRAPHICS_CRENDERSTATE_H

// Header Files
//=============

#include "Configuration.h"

#include <cstdint>

// Render State Bits
//==================

namespace eae6320
{
	namespace Graphics
	{
		namespace RenderStates
		{
			// The values below are used as bit masks
			enum eRenderState
			{
				// Set to enable alpha transparency
				AlphaTransparency = 1 << 0,
				// Set to enable both depth testing and writing to the depth buffer
				DepthBuffering = 1 << 1,
				// Set to draw triangles regardless of whether they are front-facing or back-facing
				// (set to disable culling)
				DrawBothTriangleSides = 1 << 2,
				
				WireFrame = 1 << 3,
			};

			// Alpha Transparency
			bool IsAlphaTransparencyEnabled( const uint8_t i_renderStateBits );
			void EnableAlphaTransparency( uint8_t& io_renderStateBits );
			void DisableAlphaTransparency( uint8_t& io_renderStateBits );
			// Depth Buffering
			bool IsDepthBufferingEnabled( const uint8_t i_renderStateBits );
			void EnableDepthBuffering( uint8_t& io_renderStateBits );
			void DisableDepthBuffering( uint8_t& io_renderStateBits );
			// Draw Both Triangle Sides
			bool ShouldBothTriangleSidesBeDrawn( const uint8_t i_renderStateBits );
			void EnableDrawingBothTriangleSides( uint8_t& io_renderStateBits );
			void DisableDrawingBothTriangleSides( uint8_t& io_renderStateBits );
			// WireFrame
			bool IsWireFrameEnabled(const uint8_t i_renderStateBits );
			void EnableWireFrame(uint8_t& io_renderStateBits);
			void DisableWireFrame(uint8_t& io_renderStateBits);
		}
	}
}

// Forward Declarations
//=====================

#ifdef EAE6320_PLATFORM_D3D
	struct ID3D11BlendState;
	struct ID3D11DepthStencilState;
	struct ID3D11RasterizerState;
#endif

// Class Declaration
//==================

namespace eae6320
{
	namespace Graphics
	{
		class cRenderState
		{
			// Interface
			//==========

		public:

			// Render
			//-------

			void Bind() const;

			// Access
			//-------

			bool IsAlphaTransparencyEnabled() const;
			bool IsDepthBufferingEnabled() const;
			bool ShouldBothTriangleSidesBeDrawn() const;
			bool IsWireFrameEnabled() const;

			uint8_t GetRenderStateBits() const;

			// Initialization / Clean Up
			//--------------------------

			// The input parameter is a concatenation of RenderState::eRenderState bits
			// which define which render states should be enabled
			bool Initialize( const uint8_t i_renderStateBits );
			bool CleanUp();

			cRenderState();
			~cRenderState();

			// Data
			//=====

		private:

#if defined( EAE6320_PLATFORM_D3D )
			ID3D11BlendState* m_blendState;
			ID3D11DepthStencilState* m_depthStencilState;
			ID3D11RasterizerState* m_rasterizerState;
#endif
			uint8_t m_bits;

			// Implementation
			//===============

			// Initialization / Clean Up
			//--------------------------

			bool InitializeFromBits();
		};
	}
}

#include "cRenderState/cRenderState.inl"

#endif	// EAE6320_GRAPHICS_CRENDERSTATE_H
