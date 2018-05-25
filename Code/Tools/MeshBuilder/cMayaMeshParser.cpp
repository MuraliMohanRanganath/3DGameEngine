#include "cMayaMeshParser.h"
#include <stdint.h>
#include <iostream>
#include <math.h>
#include "../../Engine/Asserts/Asserts.h"

bool eae6320::AssetBuild::cMayaMeshParser::LoadAsset(const char* const i_path) {
	bool wereThereErrors = false;
	lua_State* luaState = NULL;
	{
		luaState = luaL_newstate();
		if (!luaState)
		{
			wereThereErrors = true;
			std::cerr << "Failed to create a new Lua state" << std::endl;
			goto OnExit;
		}
	}
	const int stackTopBeforeLoad = lua_gettop(luaState);
	{
		const int luaResult = luaL_loadfile(luaState, i_path);
		if (luaResult != LUA_OK)
		{
			wereThereErrors = true;
			std::cerr << lua_tostring(luaState, -1) << std::endl;
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
					std::cerr << "Asset files must return a table (instead of a " <<
						luaL_typename(luaState, -1) << ")" << std::endl;
					lua_pop(luaState, 1);
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				std::cerr << "Asset files must return a single table (instead of " <<
					returnedValueCount << " values)" << std::endl;
				lua_pop(luaState, returnedValueCount);
				goto OnExit;
			}
		}
		else
		{
			wereThereErrors = true;
			std::cerr << lua_tostring(luaState, -1) << std::endl;
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

		EAE6320_ASSERT(lua_gettop(luaState) == 0);

		lua_close(luaState);
		luaState = NULL;
	}

	return !wereThereErrors;
}

bool eae6320::AssetBuild::cMayaMeshParser::LoadTableValues(lua_State& io_luaState) {
	if (!LoadTableKey_vertices(io_luaState))
	{
		return false;
	}
	if (!LoadTableKey_indices(io_luaState))
	{
		return false;
	}
	return true;
}

bool eae6320::AssetBuild::cMayaMeshParser::LoadTableKey_vertices(lua_State& io_luaState) {
	bool wereThereErrors = false;
	const char* const key = "vertices";
	lua_pushstring(&io_luaState, key);
	lua_gettable(&io_luaState, -2);

	if (lua_istable(&io_luaState, -1))
	{
		if (!LoadTableValues_vertices(io_luaState))
		{
			wereThereErrors = true;
			goto OnExit;
		}
	}
	else
	{
		wereThereErrors = true;
		std::cerr << "The value at \"" << key << "\" must be a table "
			"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << std::endl;
		goto OnExit;
	}

OnExit:
	lua_pop(&io_luaState, 1);

	return !wereThereErrors;
}

bool eae6320::AssetBuild::cMayaMeshParser::LoadTableValues_vertices(lua_State& io_luaState) {
	bool wereThereErrors = false;
	verticesCount = luaL_len(&io_luaState, -1);
	std::cout << "vertices length" << verticesCount;
	vertexData = (eae6320::Graphics::sVertex*)malloc(sizeof(eae6320::Graphics::sVertex) * verticesCount);

	for (int index = 1; index <= (int)verticesCount; ++index)
	{
		lua_pushinteger(&io_luaState, index);
		lua_gettable(&io_luaState, -2);

		if (lua_istable(&io_luaState, -1))
		{
			if (!LoadTableKey_position(io_luaState, index))
			{
				wereThereErrors = true;
				goto OnExit;
			}
			if (!LoadTableKey_color(io_luaState, index))
			{
				wereThereErrors = true;
				goto OnExit;
			}
			if (!LoadTableKey_Texture(io_luaState, index))
			{
				wereThereErrors = true;
				goto OnExit;
			}
		}
		else
		{
			wereThereErrors = true;
			std::cerr << "The value at \"" << index << "\" must be a table "
				"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << std::endl;
			goto OnExit;
		}
		lua_pop(&io_luaState, 1);
	}
OnExit:
	return !wereThereErrors;
}

bool eae6320::AssetBuild::cMayaMeshParser::LoadTableKey_position(lua_State& io_luaState, int index) {
	bool wereThereErrors = false;
	const char* const key = "position";
	lua_pushstring(&io_luaState, key);
	lua_gettable(&io_luaState, -2);

	if (lua_istable(&io_luaState, -1))
	{
		if (!LoadTableValues_position(io_luaState, index))
		{
			wereThereErrors = true;
			goto OnExit;
		}
	}
	else
	{
		wereThereErrors = true;
		std::cerr << "The value at \"" << key << "\" must be a table "
			"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << std::endl;
		goto OnExit;
	}

OnExit:
	lua_pop(&io_luaState, 1);

	return !wereThereErrors;
}

bool eae6320::AssetBuild::cMayaMeshParser::LoadTableValues_position(lua_State& io_luaState, int index) {

	const int numberOfAxis = luaL_len(&io_luaState, -1);
	for (int i = 1; i <= numberOfAxis; ++i)
	{
		lua_pushinteger(&io_luaState, i);
		lua_gettable(&io_luaState, -2);
		if (i == 1)
			vertexData[index - 1].x = (float)lua_tonumber(&io_luaState, -1);
		else if (i == 2)
			vertexData[index - 1].y = (float)lua_tonumber(&io_luaState, -1);
		else if (i == 3)
			vertexData[index - 1].z = (float)lua_tonumber(&io_luaState, -1);
		lua_pop(&io_luaState, 1);
	}
	return true;
}

bool eae6320::AssetBuild::cMayaMeshParser::LoadTableKey_color(lua_State& io_luaState, int index) {
	bool wereThereErrors = false;

	const char* const key = "color";
	lua_pushstring(&io_luaState, key);
	lua_gettable(&io_luaState, -2);

	if (lua_istable(&io_luaState, -1))
	{
		if (!LoadTableValues_color(io_luaState, index))
		{
			wereThereErrors = true;
			goto OnExit;
		}
	}
	else
	{
		wereThereErrors = true;
		std::cerr << "The value at \"" << key << "\" must be a table "
			"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << std::endl;
		goto OnExit;
	}
OnExit:
	lua_pop(&io_luaState, 1);

	return !wereThereErrors;
}

bool eae6320::AssetBuild::cMayaMeshParser::LoadTableValues_color(lua_State& io_luaState, int index) {
	const int colorCount = luaL_len(&io_luaState, -1);
	for (int i = 1; i <= colorCount; ++i)
	{
		lua_pushinteger(&io_luaState, i);
		lua_gettable(&io_luaState, -2);
		lua_Number colorValue = lua_tonumber(&io_luaState, -1);

		uint8_t floatingPointValue = static_cast<uint8_t>(nearbyint(colorValue * 255.0f));
		if (i == 1)
			vertexData[index - 1].red = floatingPointValue;
		else if (i == 2)
			vertexData[index - 1].green = floatingPointValue;
		else if (i == 3)
			vertexData[index - 1].blue = floatingPointValue;
		else if (i == 4)
			vertexData[index - 1].alpha = floatingPointValue;

		lua_pop(&io_luaState, 1);
	}
	return true;
}

bool eae6320::AssetBuild::cMayaMeshParser::LoadTableKey_indices(lua_State& io_luaState) {
	bool wereThereErrors = false;

	const char* const key = "indices";
	lua_pushstring(&io_luaState, key);
	lua_gettable(&io_luaState, -2);
	if (lua_istable(&io_luaState, -1))
	{
		if (!LoadTableValues_indices(io_luaState))
		{
			wereThereErrors = true;
			goto OnExit;
		}
	}
	else
	{
		wereThereErrors = true;
		std::cerr << "The value at \"" << key << "\" must be a table "
			"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << std::endl;
		goto OnExit;
	}

OnExit:
	lua_pop(&io_luaState, 1);

	return !wereThereErrors;
}

bool eae6320::AssetBuild::cMayaMeshParser::LoadTableValues_indices(lua_State& io_luaState) {
	indicesCount = luaL_len(&io_luaState, -1);
	indices = (uint32_t*)malloc(sizeof(uint32_t) * indicesCount);
	for (unsigned int i = 1; i <= indicesCount; ++i)
	{
		lua_pushinteger(&io_luaState, i);
		lua_gettable(&io_luaState, -2);
#if defined( EAE6320_PLATFORM_GL )
		indices[i - 1] = (uint32_t)lua_tonumber(&io_luaState, -1);
#elif defined( EAE6320_PLATFORM_D3D )
		indices[indicesCount - i] = (uint32_t)lua_tonumber(&io_luaState, -1);
#endif
		lua_pop(&io_luaState, 1);
	}
	return true;
}

bool eae6320::AssetBuild::cMayaMeshParser::LoadTableKey_Texture(lua_State& io_luaState, int index)
{
	bool wereThereErrors = false;
	const char* const key = "texture";
	lua_pushstring(&io_luaState, key);
	lua_gettable(&io_luaState, -2);
	
	if (lua_istable(&io_luaState, -1))
	{
		if (!LoadTableValues_Textures(io_luaState, index))
		{
			wereThereErrors = true;
			goto OnExit;
		}
	}
	else
	{
		wereThereErrors = true;
		std::cerr << "The value at \"" << key << "\" must be a table "
			"(instead of a " << luaL_typename(&io_luaState, -1) << ")" << std::endl;
		goto OnExit;
	}
OnExit:
	lua_pop(&io_luaState, 1);
	return !wereThereErrors;
}

bool eae6320::AssetBuild::cMayaMeshParser::LoadTableValues_Textures(lua_State& io_luaState, int index)
{
	float  texture[2] = { 0 };
	const int textureCount = luaL_len(&io_luaState, -1);
	for (int i = 1; i <= textureCount; ++i)
	{
		lua_pushinteger(&io_luaState, i);
		lua_gettable(&io_luaState, -2);

		if (i == 1)
		{
			vertexData[index - 1].u = (float)lua_tonumber(&io_luaState, -1);
		}
		else if (i == 2)
		{
#if defined(EAE6320_PLATFORM_GL)
			vertexData[index - 1].v = (float)lua_tonumber(&io_luaState, -1);
#elif defined(EAE6320_PLATFORM_D3D)
			vertexData[index - 1].v = 1.0f - (float)lua_tonumber(&io_luaState, -1);
#endif
		}
		lua_pop(&io_luaState, 1);
	}
	return true;
}