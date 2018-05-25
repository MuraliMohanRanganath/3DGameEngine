// Header Files
//=============

#include "../cTexture.h"

#include <algorithm>
#include <cstdlib>
// <windows.h> is #inluded before <D3D11.h> so that it is #included consistently.
// I didn't think about this and should change this next year so that D3D has it's own Includes.h file like OpenGL does :(
	#include "../../Windows/Includes.h"
#include <D3D11.h>
#include "../cTexture/Internal.h"
#include "../../Asserts/Asserts.h"
#include "../../Logging/Logging.h"
#include "Direct3dUtil.h"

// Interface
//==========

// Render
//-------

void eae6320::Graphics::cTexture::Bind( const unsigned int i_id ) const
{
	const unsigned int viewCount = 1;
	eae6320::Graphics::Direct3dUtil::getDirect3dContext()->PSSetShaderResources( i_id, viewCount, &m_textureView );
}

// Initialization / Clean Up
//--------------------------

bool eae6320::Graphics::cTexture::CleanUp()
{
	if ( m_textureView )
	{
		m_textureView->Release();
		m_textureView = NULL;
	}

	return true;
}

// Implementation
//===============

// Initialization / Clean Up
//--------------------------

bool eae6320::Graphics::cTexture::Initialize( const char* const i_path, const sDdsInfo& i_ddsInfo )
{
	bool wereThereErrors = false;

	ID3D11Texture2D* resource = NULL;
	D3D11_SUBRESOURCE_DATA* subResourceData = NULL;

	// Allocate data for a "subresource" for each MIP level
	// (Subresources are the way that Direct3D deals with textures that act like a single resource
	// but that actually have multiple textures associated with that single resource
	// (e.g. MIP maps, volume textures, texture arrays))
	{
		const size_t byteCountToAllocate = sizeof(D3D11_SUBRESOURCE_DATA) * i_ddsInfo.mipLevelCount;
		subResourceData = reinterpret_cast<D3D11_SUBRESOURCE_DATA*>( malloc( byteCountToAllocate ) );
		if ( !subResourceData )
		{
			wereThereErrors = true;
			EAE6320_ASSERTF( false, "Failed to allocate %u bytes", byteCountToAllocate );
			Logging::OutputError( "Failed to allocate %u bytes for %u subresources for %s", byteCountToAllocate, i_ddsInfo.mipLevelCount, i_path );
			goto OnExit;
		}
	}
	// Fill in the data for each MIP level
	{
		unsigned int currentWidth = static_cast<unsigned int>( i_ddsInfo.width );
		unsigned int currentHeight = static_cast<unsigned int>( i_ddsInfo.height );
		const uint8_t* currentPosition = reinterpret_cast<const uint8_t*>( i_ddsInfo.imageData );
		const uint8_t* const endOfFile = currentPosition + i_ddsInfo.imageDataSize;
		const unsigned int blockSize = TextureFormats::GetSizeOfBlock( i_ddsInfo.format );
		for ( unsigned int i = 0; i < i_ddsInfo.mipLevelCount; ++i )
		{
			// Calculate how much memory this MIP level uses
			const unsigned int blockCount_singleRow = ( currentWidth + 3 ) / 4;
			const unsigned int byteCount_singleRow = blockCount_singleRow * blockSize;
			const unsigned int rowCount = ( currentHeight + 3 ) / 4;
			const unsigned int byteCount_currentMipLevel = byteCount_singleRow * rowCount;
			// Set the data into the subresource
			{
				D3D11_SUBRESOURCE_DATA& currentSubResourceData = subResourceData[i];
				currentSubResourceData.pSysMem = currentPosition;
				currentSubResourceData.SysMemPitch = byteCount_singleRow;
				currentSubResourceData.SysMemSlicePitch = byteCount_currentMipLevel;
			}
			// Update current data for next iteration
			{
				currentPosition += byteCount_currentMipLevel;
				if (currentPosition <= endOfFile)
				{
					currentWidth = std::max(currentWidth / 2, 1u);
					currentHeight = std::max(currentHeight / 2, 1u);
				}
				else
				{
					wereThereErrors = true;
					EAE6320_ASSERTF( false, "DDS file not big enough" );
					Logging::OutputError( "The DDS file %s isn't big enough to hold the data that its header claims to", i_path );
					goto OnExit;
				}
			}
		}
		EAE6320_ASSERTF( currentPosition == endOfFile, "The DDS file %s has more data than it should", i_path );
	}
	// Create the resource
	DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
	switch ( i_ddsInfo.format )
	{
		case TextureFormats::BC1: dxgiFormat = DXGI_FORMAT_BC1_UNORM; break;
		case TextureFormats::BC3: dxgiFormat = DXGI_FORMAT_BC3_UNORM; break;
	}
	{
		D3D11_TEXTURE2D_DESC textureDescription = { 0 };
		{
			textureDescription.Width = i_ddsInfo.width;
			textureDescription.Height = i_ddsInfo.height;
			textureDescription.MipLevels = i_ddsInfo.mipLevelCount;
			textureDescription.ArraySize = 1;
			textureDescription.Format = dxgiFormat;
			{
				DXGI_SAMPLE_DESC& sampleDescription = textureDescription.SampleDesc;
				sampleDescription.Count = 1;	// No multisampling
				sampleDescription.Quality = 0;	// Doesn't matter when Count is 1
			}
			// sample desc
			textureDescription.Usage = D3D11_USAGE_IMMUTABLE;	// The texture will never change once it's been created
			textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDescription.CPUAccessFlags = 0;	// No CPU access is necessary
			textureDescription.MiscFlags = 0;
		}
		const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateTexture2D( &textureDescription, subResourceData, &resource );
		if ( FAILED( result ) )
		{
			wereThereErrors = true;
			EAE6320_ASSERTF( false, "CreateTexture2D() failed" );
			eae6320::Logging::OutputError( "Direct3D failed to create a texture from %s with HRESULT %#010x", i_path, result );
			goto OnExit;
		}
	}
	// Create the view
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription;
		{
			shaderResourceViewDescription.Format = dxgiFormat;
			shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			{
				D3D11_TEX2D_SRV& shaderResourceView2dDescription = shaderResourceViewDescription.Texture2D;
				shaderResourceView2dDescription.MostDetailedMip = 0;	// Use the highest resolution in the texture resource
				shaderResourceView2dDescription.MipLevels = -1;	// Use all MIP levels
			}
		}
		const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateShaderResourceView( resource, &shaderResourceViewDescription, &m_textureView );
		if ( FAILED( result ) )
		{
			wereThereErrors = true;
			EAE6320_ASSERTF( false, "CreateShaderResourceView() failed" );
			eae6320::Logging::OutputError( "Direct3D failed to create a shader resource view for %s with HRESULT %#010x", i_path, result );
			goto OnExit;
		}
	}

OnExit:

	// The texture resource is always released, even on success
	// (the view will hold its own reference to the resource)
	if ( resource )
	{
		resource->Release();
		resource = NULL;
	}
	if ( subResourceData )
	{
		free( subResourceData );
		subResourceData = NULL;
	}

	return !wereThereErrors;
}
