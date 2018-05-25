// Header Files
//=============

#include "../cRenderState.h"

// Interface
//==========

// Initialization / Clean Up
//--------------------------

bool eae6320::Graphics::cRenderState::Initialize( const uint8_t i_renderStateBits )
{
	m_bits = i_renderStateBits;
	return InitializeFromBits();
}

eae6320::Graphics::cRenderState::cRenderState()
	:
#if defined( EAE6320_PLATFORM_D3D )
	m_blendState( NULL ), m_depthStencilState( NULL ), m_rasterizerState( NULL ),
#endif
	// A reasonable default could be set here in the constructor,
	// but choosing a recognizable impossible value can help in debugging
	// (if a cRenderState ever has these bits it shows that it wasn't initialized)
	m_bits( 0xff )
{

}

eae6320::Graphics::cRenderState::~cRenderState()
{
	CleanUp();
}
