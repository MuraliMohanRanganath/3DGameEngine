#include "cMaterialBuilder.h"
#include "../../Tools/AssetBuildLibrary/UtilityFunctions.h"
#include "../../External/Lua/Includes.h"
#include "../../Engine/Graphics/ConstantBufferData.h"
#include <sstream>
#include <fstream>
#include <iostream>

namespace
{
	eae6320::Graphics::sMaterial* materialInfo;
	char* effectFilePath;
	char* textureFilePath = "textures/default.dds";
	bool LoadLuaFile(const char* const i_path);
	bool LoadTableValues(lua_State& io_luaState);
	bool LoadConstantData(lua_State& io_luaState);
	bool LoadConstantDataColor(lua_State& io_luaState);
	bool LoadColorValues(lua_State& io_luaState);
	bool LoadEffectsPath(lua_State& io_luaState);
	bool LoadTexturePath(lua_State& io_luaState);
}

bool eae6320::AssetBuild::cMaterialBuilder::Build(const std::vector<std::string>&)
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
		
		std::ofstream outputBinMaterial(m_path_target, std::ofstream::binary);
		outputBinMaterial.write(reinterpret_cast<char*>(materialInfo), sizeof(eae6320::Graphics::sMaterial));
		
		std::string builtEffectPath;
		std::string errormessage;
		eae6320::AssetBuild::ConvertSourceRelativePathToBuiltRelativePath(effectFilePath, "effects", builtEffectPath, &errormessage);
		builtEffectPath = "data/" + builtEffectPath;

		outputBinMaterial.write(builtEffectPath.c_str(), std::strlen(builtEffectPath.c_str()));
		outputBinMaterial.write("\0", 1);

		std::string builtTexturePath;
		std::cout << " Texture Path " << textureFilePath << "\n";
		eae6320::AssetBuild::ConvertSourceRelativePathToBuiltRelativePath(textureFilePath, "textures", builtTexturePath, &errormessage);
		builtTexturePath = "data/" + builtTexturePath;
		std::cout << " Built Texture Path " << builtTexturePath<<"\n";
		outputBinMaterial.write(builtTexturePath.c_str(), std::strlen(builtTexturePath.c_str()));
		outputBinMaterial.write("\0", 1);
		outputBinMaterial.close();
		free(materialInfo);
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
				std::ostringstream errorMessage;
				errorMessage << "Failed to create a new Lua state" << errorMessage.str();
				eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str());
				goto OnExit;
			}
		}
		const int stackTopBeforeLoad = lua_gettop(luaState);
		{
			const int luaResult = luaL_loadfile(luaState, i_path);
			if (luaResult != LUA_OK)
			{
				wereThereErrors = true;
				std::ostringstream errorMessage;
				errorMessage << lua_tostring(luaState, -1) << errorMessage.str();
				eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str());
				lua_pop(luaState, 1);
				goto OnExit;
			}
		}
		{
			const int argumentCount = 0;
			const int returnValueCount = LUA_MULTRET;
			const int noMessageHandler = 0;
			const int luaResult = lua_pcall(luaState, argumentCount, returnValueCount, noMessageHandler);
			if (luaResult == LUA_OK)
			{
				const int returnedValueCount = lua_gettop(luaState) - stackTopBeforeLoad;
				if (returnedValueCount == 1)
				{
					if (!lua_istable(luaState, -1))
					{
						wereThereErrors = true;
						std::ostringstream errorMessage;
						errorMessage << "Asset files must return a table (instead of a" << luaL_typename(luaState, -1) << ")" << errorMessage.str();
						eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str());
						lua_pop(luaState, 1);
						goto OnExit;
					}
				}
				else
				{
					wereThereErrors = true;
					std::ostringstream errorMessage;
					errorMessage << "Asset files must return a table (instead of a" << returnedValueCount << " values)" << errorMessage.str();
					eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str());
					lua_pop(luaState, returnedValueCount);
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				std::ostringstream errorMessage;
				errorMessage << lua_tostring(luaState, -1) << errorMessage.str();
				eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str());
				lua_pop(luaState, 1);
				goto OnExit;
			}
		}
		if (!LoadTableValues(*luaState))
		{
			wereThereErrors = true;
		}
		lua_pop(luaState, 1);
	
	OnExit:
		if (luaState)
		{
			lua_close(luaState);
			luaState = NULL;
		}

		return !wereThereErrors;
	}

	bool LoadTableValues(lua_State& io_luaState)
	{
		if (!LoadConstantData(io_luaState))
		{
			return false;
		}
		if (!LoadEffectsPath(io_luaState))
		{
			return false;
		}
		if (!LoadTexturePath(io_luaState))
		{
			return false;
		}
		return true;
	}

	bool LoadConstantData(lua_State& io_luaState)
	{
		bool wereThereErrors = false;
		const char* const key = "ConstantData";
		lua_pushstring(&io_luaState, key);
		lua_gettable(&io_luaState, -2);
		if (lua_istable(&io_luaState, -1))
		{
			if (!LoadConstantDataColor(io_luaState))
			{
				wereThereErrors = true;
				goto OnExit;
			}
		}
		else
		{
			wereThereErrors = true;
			std::ostringstream errorMessage;
			errorMessage << "The value at \"" << key << "\" must be a table "
				"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << errorMessage.str();
			eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str());

			goto OnExit;
		}

	OnExit:
		lua_pop(&io_luaState, 1);
		return !wereThereErrors;
	}

	bool LoadConstantDataColor(lua_State& io_luaState)
	{
		bool wereThereErrors = false;

		const char* const key = "g_color";
		lua_pushstring(&io_luaState, key);
		lua_gettable(&io_luaState, -2);

		if (lua_istable(&io_luaState, -1))
		{
			if (!LoadColorValues(io_luaState))
			{
				wereThereErrors = true;
				goto OnExit;
			}
		}
		else
		{
			if (lua_isnil(&io_luaState, -1))
			{
				std::ostringstream Message;
				Message << "No Table found at " << key << " Default Values will be used for Color" << Message.str();
				goto OnExit;
			}
			wereThereErrors = true;
			std::ostringstream errorMessage;
			errorMessage << "The value at \"" << key << "\" must be a table "
				"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << errorMessage.str();
			eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str());
			goto OnExit;
		}

	OnExit:
		lua_pop(&io_luaState, 1);
		return !wereThereErrors;
	}

	bool LoadColorValues(lua_State& io_luaState)
	{
		bool wereThereErrors = false;
		
		materialInfo = static_cast<eae6320::Graphics::sMaterial*>(malloc(sizeof(eae6320::Graphics::sMaterial)));

		const int colorCount = luaL_len(&io_luaState, -1);
		for (int i = 1; i <= colorCount; i++)
		{
			lua_pushinteger(&io_luaState, i);
			lua_gettable(&io_luaState, -2);
			float o_value = (float)lua_tonumber(&io_luaState, -1);
			if (i == 1)
				materialInfo->g_color.r = o_value;
			else if (i == 2)
				materialInfo->g_color.g = o_value;
			else if (i == 3)
				materialInfo->g_color.b = o_value;
			else if (i == 4)
				materialInfo->g_color.a = o_value;
			lua_pop(&io_luaState, 1);
		}
		return !wereThereErrors;
	}

	bool LoadEffectsPath(lua_State& io_luaState)
	{
		bool wereThereErrors = false;
		{
			const char* key = "effectPath";
			lua_pushstring(&io_luaState, key);
			lua_gettable(&io_luaState, -2);

			if (!lua_isstring(&io_luaState, -1))
			{
				lua_pop(&io_luaState, 1);
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "Asset files must return a string instead of a " <<
					luaL_typename(&io_luaState, -1) << ")\n";
				eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str());
				goto OnExit;
			}
			const char* buffer = lua_tostring(&io_luaState, -1);
			size_t bufferLength = std::strlen(buffer) + 1;
			effectFilePath = new char[bufferLength];
			strcpy_s(effectFilePath, bufferLength, buffer);
			effectFilePath[bufferLength] = '\0';
			lua_pop(&io_luaState, 1);
		}

	OnExit:
		return !wereThereErrors;
	}

	bool LoadTexturePath(lua_State& io_luaState)
	{
		bool wereThereErrors = false;
		{
			const char* key = "texturePath";
			lua_pushstring(&io_luaState, key);
			lua_gettable(&io_luaState, -2);
			if (lua_isnil(&io_luaState, -1))
			{
				goto OnExit;
			}
			if (!lua_isstring(&io_luaState, -1))
			{
				lua_pop(&io_luaState, 1);
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "Asset files must return a string instead of a " <<
					luaL_typename(&io_luaState, -1) << ")\n";
				eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str());
				goto OnExit;
			}
			const char* buffer = lua_tostring(&io_luaState, -1);
			size_t bufferLength = std::strlen(buffer) + 1;
			textureFilePath = new char[bufferLength];
			strcpy_s(textureFilePath, bufferLength, buffer);
			textureFilePath[bufferLength] = '\0';
			lua_pop(&io_luaState, 1);
		}
	OnExit:
		return !wereThereErrors;
	}
}