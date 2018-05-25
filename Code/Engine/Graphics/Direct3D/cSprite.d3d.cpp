// Header Files
//=============

#include "../cSprite.h"

// <windows.h> is #inluded before <D3D11.h> so that it is #included consistently.
// I didn't think about this and should change this next year so that D3D has it's own Includes.h file like OpenGL does :(
	#include "../../Windows/Includes.h"
#include <D3D11.h>
#include "../VertexData.h"
#include "../../Asserts/Asserts.h"
#include "../../Logging/Logging.h"
#include "Direct3dUtil.h"

// Static Data Initialization
//===========================

ID3D11Buffer* eae6320::Graphics::cSprite::ms_vertexBuffer = NULL;

namespace
{
	// When the topology is a quad Direct3D only needs 4 vertices
	const unsigned int s_vertexCount = 4;
}

// Interface
//==========

// Render
//-------

void eae6320::Graphics::cSprite::Draw() const
{
	EAE6320_ASSERT( ms_vertexBuffer != NULL );

	// Update the vertex buffer
	{
		// Get a pointer from Direct3D that can be written to
		void* memoryToWriteTo;
		{
			D3D11_MAPPED_SUBRESOURCE mappedSubResource;
			{
				// Discard previous contents when writing
				const unsigned int noSubResources = 0;
				const D3D11_MAP mapType = D3D11_MAP_WRITE_DISCARD;
				const unsigned int noFlags = 0;
				const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dContext()->Map( ms_vertexBuffer, noSubResources, mapType, noFlags, &mappedSubResource );
				if ( FAILED( result ) )
				{
					EAE6320_ASSERT( false );
					Logging::OutputError( "Direct3D failed to map the sprite vertex buffer" );
					return;
				}
			}
			memoryToWriteTo = mappedSubResource.pData;
		}
		// Fill in the memory that Direct3D has provided
		{
			sVertex* const vertexData = reinterpret_cast<sVertex*>( memoryToWriteTo );
			// Position
			{
				// Lower Left
				{
					sVertex& vertex = vertexData[0];
					vertex.x = m_position_screen.left;
					vertex.y = m_position_screen.bottom;
					vertex.z = 0.0f;
				}
				// Upper Left
				{
					sVertex& vertex = vertexData[1];
					vertex.x = m_position_screen.left;
					vertex.y = m_position_screen.top;
					vertex.z = 0.0f;
				}
				// Lower Right
				{
					sVertex& vertex = vertexData[2];
					vertex.x = m_position_screen.right;
					vertex.y = m_position_screen.bottom;
					vertex.z = 0.0f;
				}
				// Upper Right
				{
					sVertex& vertex = vertexData[3];
					vertex.x = m_position_screen.right;
					vertex.y = m_position_screen.top;
					vertex.z = 0.0f;
				}
			}
			// Texture Coordinates
			{
				// Lower Left
				{
					sVertex& vertex = vertexData[0];
					vertex.u = m_textureCoordinates.left;
					vertex.v = 1.0f - m_textureCoordinates.bottom;
				}
				// Upper Left
				{
					sVertex& vertex = vertexData[1];
					vertex.u = m_textureCoordinates.left;
					vertex.v = 1.0f - m_textureCoordinates.top;
				}
				// Lower Right
				{
					sVertex& vertex = vertexData[2];
					vertex.u = m_textureCoordinates.right;
					vertex.v = 1.0f - m_textureCoordinates.bottom;
				}
				// Upper Right
				{
					sVertex& vertex = vertexData[3];
					vertex.u = m_textureCoordinates.right;
					vertex.v = 1.0f - m_textureCoordinates.top;
				}
			}
			// Colors
			for ( unsigned int i = 0; i < s_vertexCount; ++i )
			{
				sVertex& vertex = vertexData[i];
				vertex.red = vertex.green = vertex.blue = vertex.alpha = 255;
			}
		}
		// Let Direct3D know that the memory contains the data
		// (the pointer will be invalid after this call)
		{
			const unsigned int noSubResources = 0;
			eae6320::Graphics::Direct3dUtil::getDirect3dContext()->Unmap( ms_vertexBuffer, noSubResources );
		}
	}
	// Bind the vertex buffer to the device as a data source
	{
		const unsigned int startingSlot = 0;
		const unsigned int vertexBufferCount = 1;
		// The "stride" defines how large a single vertex is in the stream of data
		const unsigned int bufferStride = sizeof( sVertex );
		// It's possible to start streaming data in the middle of a vertex buffer
		const unsigned int bufferOffset = 0;
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->IASetVertexBuffers( startingSlot, vertexBufferCount, &ms_vertexBuffer, &bufferStride, &bufferOffset );
	}
	// Specify what kind of data the vertex buffer holds
	{
		// Set the topology (which defines how to interpret multiple vertices as a single "primitive";
		// we have defined the vertex buffer as a quad, which means that a triangle "strip" will work
		// (meaning that the first primitive will be a triangle defined by three vertices
		// and the second primitive will be a triangle defined by the two most recent vertices and one new vertex)
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	}
	// Render triangles from the currently-bound vertex buffer
	{
		// It's possible to start rendering primitives in the middle of the stream
		const unsigned int indexOfFirstVertexToUse = 0;
		eae6320::Graphics::Direct3dUtil::getDirect3dContext()->Draw( s_vertexCount, indexOfFirstVertexToUse );
	}
}

// Initialization / Clean Up
//--------------------------

bool eae6320::Graphics::cSprite::Initialize()
{
	D3D11_BUFFER_DESC bufferDescription = { 0 };
	{
		const size_t bufferSize = static_cast<size_t>( s_vertexCount ) * sizeof( sVertex );
		EAE6320_ASSERT( bufferSize < ( uint64_t( 1u ) << ( sizeof( bufferDescription.ByteWidth ) * 8 ) ) );
		bufferDescription.ByteWidth = static_cast<unsigned int>( bufferSize );
		bufferDescription.Usage = D3D11_USAGE_DYNAMIC;	// The buffer will change frequently
		bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// The CPU will write to the buffer
		bufferDescription.MiscFlags = 0;
		bufferDescription.StructureByteStride = 0;	// Not used
	}
	sVertex vertexData[s_vertexCount];
	{
		// Lower Left
		{
			sVertex& vertex = vertexData[0];
			vertex.x = -1.0f;
			vertex.y = -1.0f;
			vertex.z = 0.0f;
			vertex.u = 0.0f;
			vertex.v = 1.0f;
			vertex.red = vertex.green = vertex.blue = vertex.alpha = 255;
		}
		// Upper Left
		{
			sVertex& vertex = vertexData[1];
			vertex.x = -1.0f;
			vertex.y = 1.0f;
			vertex.z = 0.0f;
			vertex.u = 0.0f;
			vertex.v = 0.0f;
			vertex.red = vertex.green = vertex.blue = vertex.alpha = 255;
		}
		// Lower Right
		{
			sVertex& vertex = vertexData[2];
			vertex.x = 1.0f;
			vertex.y = -1.0f;
			vertex.z = 0.0f;
			vertex.u = 1.0f;
			vertex.v = 1.0f;
			vertex.red = vertex.green = vertex.blue = vertex.alpha = 255;
		}
		// Upper Right
		{
			sVertex& vertex = vertexData[3];
			vertex.x = 1.0f;
			vertex.y = 1.0f;
			vertex.z = 0.0f;
			vertex.u = 1.0f;
			vertex.v = 0.0f;
			vertex.red = vertex.green = vertex.blue = vertex.alpha = 255;
		}
	}
	D3D11_SUBRESOURCE_DATA initialData = { 0 };
	{
		initialData.pSysMem = vertexData;
		// (The other data members are ignored for non-texture buffers)
	}
	const HRESULT result = eae6320::Graphics::Direct3dUtil::getDirect3dDevice()->CreateBuffer( &bufferDescription, &initialData, &ms_vertexBuffer );
	if ( SUCCEEDED( result ) )
	{
		return true;
	}
	else
	{
		EAE6320_ASSERT( false );
		eae6320::Logging::OutputError( "Direct3D failed to create the vertex buffer for sprites with HRESULT %#010x", result );
		return false;
	}
}

bool eae6320::Graphics::cSprite::CleanUp()
{
	if ( ms_vertexBuffer )
	{
		ms_vertexBuffer->Release();
		ms_vertexBuffer = NULL;
	}

	return true;
}
