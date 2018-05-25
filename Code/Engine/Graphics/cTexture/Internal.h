/*
	This file contains information used internally by the cTexture class
*/

#ifndef EAE6320_GRAPHICS_CTEXTURE_INTERNAL_H
#define EAE6320_GRAPHICS_CTEXTURE_INTERNAL_H

namespace eae6320
{
	namespace Graphics
	{
		namespace TextureFormats
		{
			enum eFormat
			{
				// Block Compression types
				BC1,
				BC2,
				BC3,
				BC4,
				BC5,
				BC6h,
				BC7,

				Unknown
			};
			// Returns the size in bytes of a single 4x4 (16 pixel) block
			inline unsigned int GetSizeOfBlock( const eFormat i_format )
			{
				switch ( i_format )
				{
					// RGB (with an optional binary alpha)
					case BC1: return 8; break;
					// RGB + A (BC1 for RGB and 4 bits for each alpha pixel)
					case BC2: return 8 + ( 4 * 16 );
					// RGB + A (BC1 for RGB and BC4 for alpha)
					case BC3: return 8 + 8; break;
					// Single Channel
					case BC4: return 8; break;
					// Two Channels (2 BC4s)
					case BC5: return 8 + 8; break;
					// RGB 16-bit ("h" for "half precision") floating point
					case BC6h: return 16; break;
					// RGBA
					case BC7: return 16; break;
				}

				// The format doesn't use block compression
				return 0;
			}
		}

		// Information extracted from a DDS file at load-time
		// (using this saves the platform-specific texture-loading implementations
		// from having to worry about parsing the DDS header)
		struct sDdsInfo
		{
			const void* imageData;
			size_t imageDataSize;
			unsigned int width, height;
			unsigned int mipLevelCount;
			TextureFormats::eFormat format;

			sDdsInfo()
				:
				imageData( nullptr ), imageDataSize( 0 ),
				width( 0 ), height( 0 ), mipLevelCount( 0 ),
				format( TextureFormats::Unknown )
			{

			}
		};
	}
}

#endif	// EAE6320_GRAPHICS_CTEXTURE_INTERNAL_H
