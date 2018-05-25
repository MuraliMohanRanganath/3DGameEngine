// Header Files
//=============

#include "AssetBuild.h"

#include <iostream>
#include <string>
#include "../AssetBuildLibrary/UtilityFunctions.h"
#include "../../Engine/Asserts/Asserts.h"
#include "../../Engine/Platform/Platform.h"
#include "../../External/Lua/Includes.h"

// Static Data Initialization
//===========================

namespace
{
	lua_State* s_luaState = NULL;
}

// Helper Function Declarations
//=============================

namespace
{
	// Lua Wrapper Functions
	//----------------------

	int luaCopyFile( lua_State* io_luaState );
	int luaCreateDirectoryIfNecessary( lua_State* io_luaState );
	int luaDoesFileExist( lua_State* io_luaState );
	int luaExecuteCommand( lua_State* io_luaState );
	int luaGetEnvironmentVariable( lua_State* io_luaState );
	int luaGetLastWriteTime( lua_State* io_luaState );
	int luaInvalidateLastWriteTime( lua_State* io_luaState );
	int luaOutputErrorMessage( lua_State* io_luaState );
}

// Interface
//==========

bool eae6320::AssetBuild::BuildAssets( const char* const i_path_assetsToBuild )
{
	// Load and execute the build script
	{
		// Get the path
		std::string path;
		{
			std::string errorMessage;
			if ( !GetAssetBuildSystemPath( path, &errorMessage ) )
			{
				OutputErrorMessage( errorMessage.c_str(), __FILE__ );
				return false;
			}
		}
		// Load and execute it
		{
			std::string errorMessage;
			if ( Platform::DoesFileExist( path.c_str(), &errorMessage ) )
			{
				// Load the build script file as a "chunk",
				// meaning there will be a callable function at the top of the stack
				const int luaResult = luaL_loadfile( s_luaState, path.c_str() );
				if ( luaResult == LUA_OK )
				{
					// Execute the "chunk"
					const int argumentCount = 1;
					{
						lua_pushstring( s_luaState, i_path_assetsToBuild );
					}
					const int returnValueCount = 0;
					const int noErrorHandler = 0;
					const int luaResult = lua_pcall( s_luaState, argumentCount, returnValueCount, noErrorHandler );
					if ( luaResult != LUA_OK )
					{
						std::cerr << lua_tostring( s_luaState, -1 ) << "\n";
						// Pop the error message
						lua_pop( s_luaState, 1 );
						return false;
					}
				}
				else
				{
					std::cerr << lua_tostring( s_luaState, -1 ) << "\n";
					// Pop the error message
					lua_pop( s_luaState, 1 );
					return false;
				}
			}
			else
			{
				OutputErrorMessage( errorMessage.c_str(), path.c_str() );
				return false;
			}
		}
	}

	return true;
}

// Initialization / Clean Up
//--------------------------

bool eae6320::AssetBuild::Initialize()
{
	// Create a new Lua state
	{
		s_luaState = luaL_newstate();
		if ( !s_luaState )
		{
			OutputErrorMessage( "Memory allocation error creating Lua state", __FILE__ );
			return false;
		}
	}
	// Open the standard libraries
	luaL_openlibs( s_luaState );
	// Register custom functions
	{
		lua_register( s_luaState, "CopyFile", luaCopyFile );
		lua_register( s_luaState, "CreateDirectoryIfNecessary", luaCreateDirectoryIfNecessary );
		lua_register( s_luaState, "DoesFileExist", luaDoesFileExist );
		lua_register( s_luaState, "ExecuteCommand", luaExecuteCommand );
		lua_register( s_luaState, "GetEnvironmentVariable", luaGetEnvironmentVariable );
		lua_register( s_luaState, "GetLastWriteTime", luaGetLastWriteTime );
		lua_register( s_luaState, "InvalidateLastWriteTime", luaInvalidateLastWriteTime );
		lua_register( s_luaState, "OutputErrorMessage", luaOutputErrorMessage );
	}

	return true;
}

bool eae6320::AssetBuild::CleanUp()
{
	bool wereThereErrors = false;

	if ( s_luaState )
	{
		EAE6320_ASSERT( lua_gettop( s_luaState ) == 0 );
		lua_close( s_luaState );
		s_luaState = NULL;
	}

	return !wereThereErrors;
}

// Helper Function Definitions
//============================

namespace
{
	// Lua Wrapper Functions
	//----------------------

	int luaCopyFile( lua_State* io_luaState )
	{
		// Argument #1: The source path
		const char* i_path_source;
		if ( lua_isstring( io_luaState, 1 ) )
		{
			i_path_source = lua_tostring( io_luaState, 1 );
		}
		else
		{
			return luaL_error( io_luaState,
				"Argument #1 must be a string (instead of a %s)",
				luaL_typename( io_luaState, 1 ) );
		}
		// Argument #2: The target path
		const char* i_path_target;
		if ( lua_isstring( io_luaState, 2 ) )
		{
			i_path_target = lua_tostring( io_luaState, 2 );
		}
		else
		{
			return luaL_error( io_luaState,
				"Argument #2 must be a string (instead of a %s)",
				luaL_typename( io_luaState, 2 ) );
		}

		// Copy the file
		{
			std::string errorMessage;
			// There are many reasons that a source should be rebuilt,
			// and so even if the target already exists it should just be written over
			const bool noErrorIfTargetAlreadyExists = false;
			// Since we rely on timestamps to determine when a target was built
			// its file time should be updated when the source gets copied
			const bool updateTheTargetFileTime = true;
			if ( eae6320::Platform::CopyFile( i_path_source, i_path_target, noErrorIfTargetAlreadyExists, updateTheTargetFileTime, &errorMessage ) )
			{
				lua_pushboolean( io_luaState, true );
				const int returnValueCount = 1;
				return returnValueCount;
			}
			else
			{
				lua_pushboolean( io_luaState, false );
				lua_pushstring( io_luaState, errorMessage.c_str() );
				const int returnValueCount = 2;
				return returnValueCount;
			}
		}
	}

	int luaCreateDirectoryIfNecessary( lua_State* io_luaState )
	{
		// Argument #1: The path
		const char* i_path;
		if ( lua_isstring( io_luaState, 1 ) )
		{
			i_path = lua_tostring( io_luaState, 1 );
		}
		else
		{
			return luaL_error( io_luaState,
				"Argument #1 must be a string (instead of a %s)",
				luaL_typename( io_luaState, 1 ) );
		}

		std::string errorMessage;
		if ( eae6320::Platform::CreateDirectoryIfNecessary( i_path, &errorMessage ) )
		{
			const int returnValueCount = 0;
			return returnValueCount;
		}
		else
		{
			return luaL_error( io_luaState, errorMessage.c_str() );
		}
	}

	int luaDoesFileExist( lua_State* io_luaState )
	{
		// Argument #1: The path
		const char* i_path;
		if ( lua_isstring( io_luaState, 1 ) )
		{
			i_path = lua_tostring( io_luaState, 1 );
		}
		else
		{
			return luaL_error( io_luaState,
				"Argument #1 must be a string (instead of a %s)",
				luaL_typename( io_luaState, 1 ) );
		}

		std::string errorMessage;
		if ( eae6320::Platform::DoesFileExist( i_path, &errorMessage ) )
		{
			lua_pushboolean( io_luaState, true );
			const int returnValueCount = 1;
			return returnValueCount;
		}
		else
		{
			lua_pushboolean( io_luaState, false );
			lua_pushstring( io_luaState, errorMessage.c_str() );
			const int returnValueCount = 2;
			return returnValueCount;
		}
	}

	int luaExecuteCommand( lua_State* io_luaState )
	{
		// Argument #1: The command
		const char* i_command;
		if ( lua_isstring( io_luaState, 1 ) )
		{
			i_command = lua_tostring( io_luaState, 1 );
		}
		else
		{
			return luaL_error( io_luaState,
				"Argument #1 must be a string (instead of a %s)",
				luaL_typename( io_luaState, 1 ) );
		}

		int exitCode;
		std::string errorMessage;
		if ( eae6320::Platform::ExecuteCommand( i_command, &exitCode, &errorMessage ) )
		{
			lua_pushboolean( io_luaState, true );
			lua_pushinteger( io_luaState, exitCode );
			const int returnValueCount = 2;
			return returnValueCount;
		}
		else
		{
			lua_pushboolean( io_luaState, false );
			lua_pushstring( io_luaState, errorMessage.c_str() );
			const int returnValueCount = 2;
			return returnValueCount;
		}
	}

	int luaGetEnvironmentVariable( lua_State* io_luaState )
	{
		// Argument #1: The key
		const char* i_key;
		if ( lua_isstring( io_luaState, 1 ) )
		{
			i_key = lua_tostring( io_luaState, 1 );
		}
		else
		{
			return luaL_error( io_luaState,
				"Argument #1 must be a string (instead of a %s)",
				luaL_typename( io_luaState, 1 ) );
		}

		std::string value;
		std::string errorMessage;
		if ( eae6320::Platform::GetEnvironmentVariable( i_key, value, &errorMessage ) )
		{
			lua_pushstring( io_luaState, value.c_str() );
			const int returnValueCount = 1;
			return returnValueCount;
		}
		else
		{
			lua_pushboolean( io_luaState, false );
			lua_pushstring( io_luaState, errorMessage.c_str() );
			const int returnValueCount = 2;
			return returnValueCount;
		}
	}

	int luaGetLastWriteTime( lua_State* io_luaState )
	{
		// Argument #1: The path
		const char* i_path;
		if ( lua_isstring( io_luaState, 1 ) )
		{
			i_path = lua_tostring( io_luaState, 1 );
		}
		else
		{
			return luaL_error( io_luaState,
				"Argument #1 must be a string (instead of a %s)",
				luaL_typename( io_luaState, 1 ) );
		}

		// Get the last time that the file was written to
		uint64_t lastWriteTime;
		std::string errorMessage;
		if ( eae6320::Platform::GetLastWriteTime( i_path, lastWriteTime, &errorMessage ) )
		{
			lua_pushnumber( io_luaState, static_cast<lua_Number>( lastWriteTime ) );
			const int returnValueCount = 1;
			return returnValueCount;
		}
		else
		{
			return luaL_error( io_luaState, errorMessage.c_str() );
		}
	}

	int luaInvalidateLastWriteTime( lua_State* io_luaState )
	{
		// Argument #1: The path
		const char* i_path;
		if ( lua_isstring( io_luaState, 1 ) )
		{
			i_path = lua_tostring( io_luaState, 1 );
		}
		else
		{
			return luaL_error( io_luaState,
				"Argument #1 must be a string (instead of a %s)",
				luaL_typename( io_luaState, 1 ) );
		}

		// Invalidate the last time that the file was written to
		std::string errorMessage;
		if ( eae6320::Platform::InvalidateLastWriteTime( i_path, &errorMessage ) )
		{
			const int returnValueCount = 0;
			return returnValueCount;
		}
		else
		{
			return luaL_error( io_luaState, errorMessage.c_str() );
		}
	}

	int luaOutputErrorMessage( lua_State* io_luaState )
	{
		// Argument #1: The error message
		const char* i_errorMessage;
		if ( lua_isstring( io_luaState, 1 ) )
		{
			i_errorMessage = lua_tostring( io_luaState, 1 );
		}
		else
		{
			return luaL_error( io_luaState,
				"Argument #1 must be a string (instead of a %s)",
				luaL_typename( io_luaState, 1 ) );
		}
		// Argument #2: An optional file name
		const char* i_optionalFileName = NULL;
		if ( !lua_isnoneornil( io_luaState, 2 ) )
		{
			if ( lua_isstring( io_luaState, 2 ) )
			{
				i_optionalFileName = lua_tostring( io_luaState, 2 );
			}
			else
			{
				return luaL_error( io_luaState,
					"Argument #2 must be a string (instead of a %s)",
					luaL_typename( io_luaState, 2 ) );
			}
		}

		// Output the error message
		eae6320::AssetBuild::OutputErrorMessage( i_errorMessage, i_optionalFileName );

		const int returnValueCount = 0;
		return returnValueCount;
	}
}
