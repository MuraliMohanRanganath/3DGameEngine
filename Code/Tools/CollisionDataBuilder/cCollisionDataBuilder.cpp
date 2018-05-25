#include "cCollisionDataBuilder.h"
#include "../AssetBuildLibrary/UtilityFunctions.h"
#include "../../External/Lua/Includes.h"
#include "../../Engine/Physics/TriangleData.h"
#include <sstream>
#include <iostream>
#include <fstream>

namespace {

	bool LoadMeshScript(const char* const i_path, std::vector<eae6320::Physics::sTriangle>* o_tris);
	bool LoadTableValues(lua_State& io_luaState, std::vector<eae6320::Physics::sTriangle>* o_tris);
	bool WriteToBinaryFile(const char* const targetPath, std::vector<eae6320::Physics::sTriangle>* i_tris);
}

bool eae6320::AssetBuild::cCollisionDataBuilder::Build(const std::vector<std::string>&)
{
	bool wereThereErrors = false;
	std::vector<eae6320::Physics::sTriangle> pos;
	if (!LoadMeshScript(m_path_source, &pos)) {
		wereThereErrors = true;
	}
	{
		bool writeSuccess = WriteToBinaryFile(m_path_target, &pos);
		if (!writeSuccess)
		{
			wereThereErrors = true;
			std::ostringstream errorMessage;
			errorMessage << "Failed to copy \"" << m_path_source << "\" to \"" << m_path_target << "\": ";
			eae6320::AssetBuild::OutputErrorMessage(errorMessage.str().c_str(), __FILE__);
		}
	}

	return !wereThereErrors;
}

namespace {

	bool LoadMeshScript(const char* const i_path, std::vector<eae6320::Physics::sTriangle>* o_pos)
	{
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
			const int returnValueCount = LUA_MULTRET;	// Return _everything_ that the file returns
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
						std::cerr << "Asset files must return a table (instead of a " << luaL_typename(luaState, -1) << ")" << std::endl;
						lua_pop(luaState, 1);
						goto OnExit;
					}
				}
				else
				{
					wereThereErrors = true;
					std::cerr << "Asset files must return a single table (instead of " << returnedValueCount << " values)" << std::endl;
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

		if (!LoadTableValues(*luaState, o_pos))
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

	bool LoadTableValues(lua_State& io_luaState, std::vector<eae6320::Physics::sTriangle>* o_pos)
	{
		bool wereThereErrors = false;
		{
			const char* const key = "triangles";
			lua_pushstring(&io_luaState, key);
			lua_gettable(&io_luaState, -2);
			if (lua_istable(&io_luaState, -1))
			{
				const size_t triangle_count = static_cast<size_t>(luaL_len(&io_luaState, -1));
				for (size_t index = 1; index <= triangle_count; ++index) {
					lua_pushinteger(&io_luaState, index);
					lua_gettable(&io_luaState, -2);
					eae6320::Physics::sTriangle triangle;
					{
						const char* const vertexkey = "A";
						lua_pushstring(&io_luaState, vertexkey);
						lua_gettable(&io_luaState, -2);

						const int PositionCount = luaL_len(&io_luaState, -1);
						float holder[3];
						for (int i = 1; i <= PositionCount; ++i)
						{
							lua_pushinteger(&io_luaState, i);
							lua_gettable(&io_luaState, -2);
							holder[i - 1] = static_cast<float>(lua_tonumber(&io_luaState, -1));
							lua_pop(&io_luaState, 1);
						}
						triangle.A.x = holder[0];
						triangle.A.y = holder[1];
						triangle.A.z = holder[2];
						lua_pop(&io_luaState, 1);
					}
					{
						const char* const vertexkey = "B";
						lua_pushstring(&io_luaState, vertexkey);
						lua_gettable(&io_luaState, -2);

						const int PositionCount = luaL_len(&io_luaState, -1);
						float holder[3];
						for (int i = 1; i <= PositionCount; ++i)
						{
							lua_pushinteger(&io_luaState, i);
							lua_gettable(&io_luaState, -2);
							holder[i - 1] = static_cast<float>(lua_tonumber(&io_luaState, -1));
							lua_pop(&io_luaState, 1);
						}
						triangle.B.x = holder[0];
						triangle.B.y = holder[1];
						triangle.B.z = holder[2];
						lua_pop(&io_luaState, 1);
					}
					{
						const char* const vertexkey = "C";
						lua_pushstring(&io_luaState, vertexkey);
						lua_gettable(&io_luaState, -2);

						const int PositionCount = luaL_len(&io_luaState, -1);
						float holder[3];
						for (int i = 1; i <= PositionCount; ++i)
						{
							lua_pushinteger(&io_luaState, i);
							lua_gettable(&io_luaState, -2);
							holder[i - 1] = static_cast<float>(lua_tonumber(&io_luaState, -1));
							lua_pop(&io_luaState, 1);
						}
						lua_pop(&io_luaState, 1);
						triangle.C.x = holder[0];
						triangle.C.y = holder[1];
						triangle.C.z = holder[2];
					}
					o_pos->push_back(triangle);
					lua_pop(&io_luaState, 1);
				}
				lua_pop(&io_luaState, 1);
			}
		}
		return !wereThereErrors;
	}

	bool WriteToBinaryFile(const char* const targetPath, std::vector<eae6320::Physics::sTriangle>* i_tris) {
		std::ofstream outfile(targetPath, std::ofstream::binary);
		char* buffer = NULL;
		size_t size;
		uint32_t noOfTris = static_cast<uint32_t>(i_tris->size());
		size = sizeof(noOfTris);
		buffer = reinterpret_cast<char*>(&noOfTris);
		outfile.write(buffer, size);
		size = sizeof(eae6320::Physics::sTriangle)*i_tris->size();
		buffer = reinterpret_cast<char*>(i_tris->data());
		outfile.write(buffer, size);
		outfile.close();
		return true;
	}
}