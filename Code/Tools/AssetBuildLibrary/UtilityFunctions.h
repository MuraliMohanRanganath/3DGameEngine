/*
	This file contains utility functions for other asset build tools
*/

#ifndef EAE6320_ASSETBUILD_UTILITYFUNCTIONS_H
#define EAE6320_ASSETBUILD_UTILITYFUNCTIONS_H

// Header Files
//=============

#include <string>

// Interface
//==========

namespace eae6320
{
	namespace AssetBuild
	{
		// If an asset ("A") references another asset ("B")
		// then that reference to B must be converted from a source path to a built path
		// if it is included in the built A asset
		bool ConvertSourceRelativePathToBuiltRelativePath( const char* const i_sourceRelativePath, const char* const i_assetType,
			std::string& o_builtRelativePath, std::string* const o_errorMessage );

		bool GetAssetBuildSystemPath( std::string& o_path, std::string* const o_errorMessage );

		// Errors can be formatted a specific way so that they show up
		// in Visual Studio's "Error List" tab
		void OutputErrorMessage( const char* const i_errorMessage, const char* const i_optionalFileName = NULL );
	}
}

#endif	// EAE6320_ASSETBUILD_UTILITYFUNCTIONS_H
