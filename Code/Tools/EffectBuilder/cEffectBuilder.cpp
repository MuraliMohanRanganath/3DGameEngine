// Header Files
//=============

#include "cEffectBuilder.h"
#include "../../Tools/AssetBuildLibrary/UtilityFunctions.h"
#include "../../Engine/Platform/Platform.h"
#include "../../External/Lua/Includes.h"
#include "../../Engine/Logging/Logging.h"
#include "../../Engine/Asserts/Asserts.h"
#include "../../Engine/Graphics/cRenderState.h"
#include <sstream>
#include <fstream>
#include <iostream>

namespace 
{
	char* vertexShaderPath;
	char* fragmentShaderPath;
	uint8_t renderStateBits;
	bool LoadLuaFile(const char* const i_path);
	bool LoadShaderPaths(lua_State& io_luaState);
	bool LoadRenderStateBits(lua_State& io_luaState);
}

// Inherited Implementation
//=========================

// Build
//------
bool eae6320::AssetBuild::cEffectBuilder::Build(const std::vector<std::string>&)
{
	bool wereThereErrors = false;
	{
		if (!LoadLuaFile(m_path_source)) {
			wereThereErrors = true;
			std::ostringstream errorMessage;
			errorMessage << "Failed to Load lua file \"" << m_path_source;
			eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str(), __FILE__);
			return false;
		}

		std::ofstream outputBinEffect(m_path_target, std::ofstream::binary);
		outputBinEffect.write(reinterpret_cast<char*>(&renderStateBits), sizeof(uint8_t));

		std::string errormessage;
		std::string binVertexShaderPath;
		eae6320::AssetBuild::ConvertSourceRelativePathToBuiltRelativePath(vertexShaderPath, "shaders", binVertexShaderPath, &errormessage);
		binVertexShaderPath = "data/" + binVertexShaderPath;
		outputBinEffect.write(binVertexShaderPath.c_str(), std::strlen(binVertexShaderPath.c_str()));
		outputBinEffect.write("\0", 1);

		std::string binFragmentShaderPath;
		eae6320::AssetBuild::ConvertSourceRelativePathToBuiltRelativePath(fragmentShaderPath, "shaders", binFragmentShaderPath, &errormessage);
		binFragmentShaderPath = "data/" + binFragmentShaderPath;
		outputBinEffect.write(binFragmentShaderPath.c_str(), std::strlen(binFragmentShaderPath.c_str()));
		outputBinEffect.write("\0", 1);

		outputBinEffect.close();
	}
	return !wereThereErrors;
}

namespace
{
	bool LoadLuaFile(const char* const i_path)
	{
		bool wereThereErrors = false;
		lua_State* luaState = NULL;
		{
			luaState = luaL_newstate();
			if (!luaState)
			{
				wereThereErrors = true;
				eae6320::AssetBuild::OutputErrorMessage("Failed to create a new Lua state");
				goto OnExit;
			}
		}
		{
			const int luaResult = luaL_loadfile(luaState, i_path);
			if (luaResult != LUA_OK)
			{
				wereThereErrors = true;
				eae6320::AssetBuild::OutputErrorMessage(lua_tostring(luaState, -1));
				lua_pop(luaState, 1);
				goto OnExit;
			}
		}
		{
			const int argumentCount = 0;
			const int returnValueCount = 1;
			const int noErrorHandler = 0;
			const int luaResult = lua_pcall(luaState, argumentCount, returnValueCount, noErrorHandler);
			if (luaResult == LUA_OK)
			{
				if (!lua_istable(luaState, -1))
				{
					wereThereErrors = true;
					std::stringstream cerr;
					cerr << "Asset files must return a table instead of a " << luaL_typename(luaState, -1) << std::endl;
					eae6320::AssetBuild::OutputErrorMessage(cerr.str().c_str());
					lua_pop(luaState, 1);
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				eae6320::AssetBuild::OutputErrorMessage(lua_tostring(luaState, -1));
				lua_pop(luaState, 1);
				goto OnExit;
			}
		}

		if (!LoadRenderStateBits(*luaState)) {
			wereThereErrors = true;
		}

		if (!LoadShaderPaths(*luaState))
		{
			wereThereErrors = true;
		}

		lua_pop(luaState, 1);
	OnExit:
		if (luaState)
		{
			if (lua_gettop(luaState) != 0) {
				eae6320::AssetBuild::OutputErrorMessage("Lua stack is not cleared");
			}
			lua_close(luaState);
			luaState = NULL;
		}
		return !wereThereErrors;
	}

	bool LoadShaderPaths(lua_State& io_luaState)
	{
		bool wereThereErrors = false;
		{
			const char* const key = "vertex";
			lua_pushstring(&io_luaState, key);
			lua_gettable(&io_luaState, -2);

			if (!lua_isstring(&io_luaState, -1)) {
				lua_pop(&io_luaState, 1);
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "Asset files must return a string instead of a " << luaL_typename(&io_luaState, -1) << std::endl;
				eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str());
				goto OnExit;
			}

			char* tempBuffer = const_cast<char*>(lua_tostring(&io_luaState, -1));
			size_t bufferLength = std::strlen(tempBuffer) + 1;
			vertexShaderPath = new char[bufferLength];
			strcpy_s(vertexShaderPath, bufferLength, tempBuffer);
			vertexShaderPath[bufferLength] = '\0';
			lua_pop(&io_luaState, 1);
		}
		{
			const char* const key = "fragment";
			lua_pushstring(&io_luaState, key);
			lua_gettable(&io_luaState, -2);
			if (!lua_isstring(&io_luaState, -1)) {
				lua_pop(&io_luaState, 1);
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "Asset files must return a string instead of a " << luaL_typename(&io_luaState, -1) << std::endl;
				eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str());
				goto OnExit;
			}
			char* tempBuffer = const_cast<char*>(lua_tostring(&io_luaState, -1));
			size_t bufferLength = std::strlen(tempBuffer) + 1;
			fragmentShaderPath = new char[bufferLength];
			strcpy_s(fragmentShaderPath, bufferLength, tempBuffer);
			fragmentShaderPath[bufferLength] = '\0';
			lua_pop(&io_luaState, 1);
		}
	OnExit:
		return !wereThereErrors;
	}

	bool LoadRenderStateBits(lua_State& io_luaState)
	{
		const char* const key = "renderStates";
		lua_pushstring(&io_luaState, key);
		lua_gettable(&io_luaState, -2);

		if (lua_istable(&io_luaState, -1))
		{
			const char* const aphaTransparencyKey = "alphaTransparency";
			lua_pushstring(&io_luaState, aphaTransparencyKey);
			lua_gettable(&io_luaState, -2);

			if (!lua_isnil(&io_luaState, -1) && lua_isboolean(&io_luaState, -1))
			{
				if (lua_toboolean(&io_luaState, -1))
					eae6320::Graphics::RenderStates::EnableAlphaTransparency(renderStateBits);
				else
					eae6320::Graphics::RenderStates::DisableAlphaTransparency(renderStateBits);
			}
			else
			{
				eae6320::Graphics::RenderStates::DisableAlphaTransparency(renderStateBits);
			}
			lua_pop(&io_luaState, 1);


			const char* const depthBufferingKey = "depthBuffering";
			lua_pushstring(&io_luaState, depthBufferingKey);
			lua_gettable(&io_luaState, -2);

			if (!lua_isnil(&io_luaState, -1) && lua_isboolean(&io_luaState, -1))
			{
				if (lua_toboolean(&io_luaState, -1))
					eae6320::Graphics::RenderStates::EnableDepthBuffering(renderStateBits);
				else
					eae6320::Graphics::RenderStates::DisableDepthBuffering(renderStateBits);
			}
			else
			{
				eae6320::Graphics::RenderStates::EnableDepthBuffering(renderStateBits);
			}
			lua_pop(&io_luaState, 1);


			const char* const drawBothTriangleSidesKey = "drawBothTriangleSides";
			lua_pushstring(&io_luaState, drawBothTriangleSidesKey);
			lua_gettable(&io_luaState, -2);

			if (!lua_isnil(&io_luaState, -1) && lua_isboolean(&io_luaState, -1))
			{
				if (lua_toboolean(&io_luaState, -1))
					eae6320::Graphics::RenderStates::EnableDrawingBothTriangleSides(renderStateBits);
				else
					eae6320::Graphics::RenderStates::DisableDrawingBothTriangleSides(renderStateBits);
			}
			else
			{
				eae6320::Graphics::RenderStates::DisableDrawingBothTriangleSides(renderStateBits);
			}
			lua_pop(&io_luaState, 1);

			const char* const wireFrameKey = "wireFrame";
			lua_pushstring(&io_luaState, wireFrameKey);
			lua_gettable(&io_luaState, -2);

			if (!lua_isnil(&io_luaState, -1) && lua_isboolean(&io_luaState, -1))
			{
				if (lua_toboolean(&io_luaState, -1))
					eae6320::Graphics::RenderStates::EnableWireFrame(renderStateBits);
				else
					eae6320::Graphics::RenderStates::DisableWireFrame(renderStateBits);
			}
			else
			{
				eae6320::Graphics::RenderStates::DisableWireFrame(renderStateBits);
			}
			lua_pop(&io_luaState, 1);
		}
		else {
			eae6320::Graphics::RenderStates::DisableAlphaTransparency(renderStateBits);
			eae6320::Graphics::RenderStates::EnableDepthBuffering(renderStateBits);
			eae6320::Graphics::RenderStates::DisableDrawingBothTriangleSides(renderStateBits);
			eae6320::Graphics::RenderStates::DisableWireFrame(renderStateBits);
		}
		lua_pop(&io_luaState, 1);
		return true;
	}
}