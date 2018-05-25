#include "cOctreeDataBuilder.h"
#include "../../Engine/Physics/Octree.h"
#include "../../External/Lua/Includes.h"
#include "../AssetBuildLibrary/UtilityFunctions.h"
#include <sstream>
#include <iostream>
#include <fstream>

namespace {
	bool LoadMeshScript(const char* i_path, std::vector<eae6320::Physics::Octree::NodeData>& data);
	bool LoadTableValues(lua_State& io_luaState, std::vector<eae6320::Physics::Octree::NodeData>& data);
	bool WriteMemoryToFile(const char* targetPath, std::vector<eae6320::Physics::Octree::NodeData>& data);
}

bool eae6320::AssetBuild::cOctreeDataBuilder::Build(const std::vector<std::string>&)
{
	bool wereThereErrors = false;
	std::vector<eae6320::Physics::Octree::NodeData> data;
	if (!LoadMeshScript(m_path_source, data)) {
		wereThereErrors = true;
	}
	{
		bool writeSuccess = WriteMemoryToFile(m_path_target, data);
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

	bool LoadMeshScript(const char* i_path, std::vector<eae6320::Physics::Octree::NodeData>& data)
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
		if (!LoadTableValues(*luaState, data))
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

	bool LoadTableValues(lua_State& io_luaState, std::vector<eae6320::Physics::Octree::NodeData>& data)
	{
		bool wereThereErrors = false;
		{
			const char* const key = "Nodes";
			lua_pushstring(&io_luaState, key);
			lua_gettable(&io_luaState, -2);
			const int NodeCount = luaL_len(&io_luaState, -1);
			for (int i = 1; i <= NodeCount; ++i)
			{
				eae6320::Physics::Octree::NodeData node;
				lua_pushinteger(&io_luaState, i);
				lua_gettable(&io_luaState, -2);
				{
					{
						const char* const key = "cube";
						lua_pushstring(&io_luaState, key);
						lua_gettable(&io_luaState, -2);

						if (lua_istable(&io_luaState, -1))
						{
							const char* const key2 = "center";
							lua_pushstring(&io_luaState, key2);
							lua_gettable(&io_luaState, -2);

							const int IndexCount = luaL_len(&io_luaState, -1);
							for (int j = 1; j <= IndexCount; ++j) {
								lua_pushinteger(&io_luaState, j);
								lua_gettable(&io_luaState, -2);
								node.m_cube.m_center.x[j - 1] = static_cast<float>(lua_tonumber(&io_luaState, -1));
								lua_pop(&io_luaState, 1);
							}
							lua_pop(&io_luaState, 1);
							const char* const key3 = "width";
							lua_pushstring(&io_luaState, key3);
							lua_gettable(&io_luaState, -2);
							node.m_cube.m_width = static_cast<float>(lua_tonumber(&io_luaState, -1));
							lua_pop(&io_luaState, 1);

						}
						lua_pop(&io_luaState, 1);
					}
					const char* const key = "depth";
					lua_pushstring(&io_luaState, key);
					lua_gettable(&io_luaState, -2);
					node.m_depth = static_cast<uint8_t>(lua_tonumber(&io_luaState, -1));
					lua_pop(&io_luaState, 1);
					
					const char* const tri_key = "triangles";
					lua_pushstring(&io_luaState, tri_key);
					lua_gettable(&io_luaState, -2);

					const int IndexCount = luaL_len(&io_luaState, -1);
					node.m_no_tris = IndexCount;
					node.m_tris = static_cast<uint16_t*>(malloc(sizeof(uint16_t)*IndexCount));
					for (int j = 1; j <= IndexCount; ++j) {
						lua_pushinteger(&io_luaState, j);
						lua_gettable(&io_luaState, -2);
						node.m_tris[j - 1] = static_cast<uint16_t>(lua_tonumber(&io_luaState, -1));
						lua_pop(&io_luaState, 1);
					}
					lua_pop(&io_luaState, 1);
				}
				lua_pop(&io_luaState, 1);
				data.push_back(node);
			}
			lua_pop(&io_luaState, 1);
		}
		return !wereThereErrors;
	}

	bool WriteMemoryToFile(const char* targetPath, std::vector<eae6320::Physics::Octree::NodeData>& i_NodeData) 
	{
		std::ofstream outfile(targetPath, std::ofstream::binary);
		char* buffer = NULL;
		size_t size;
		{
			uint16_t node_data_size = static_cast<uint16_t>(i_NodeData.size());
			size = sizeof(uint16_t);
			buffer = reinterpret_cast<char*>(&node_data_size);
			outfile.write(buffer, size);
		}
		{
			for (auto node : i_NodeData) {
				{size_t data = sizeof(eae6320::Physics::Octree::NodeData) + sizeof(node.m_no_tris)*node.m_no_tris - sizeof(node.m_tris);
				size = sizeof(size_t);
				char* buffer = reinterpret_cast<char*>(&data);
				outfile.write(buffer, size);
				}
				{
					size = sizeof(eae6320::Physics::Octree::NodeData::CubeData);
					char* buffer = reinterpret_cast<char*>(&node.m_cube);
					outfile.write(buffer, size);

				}
				{
					size = sizeof(eae6320::Physics::Octree::NodeData::m_depth);
					char* buffer = reinterpret_cast<char*>(&node.m_depth);
					outfile.write(buffer, size);

				}
				{
					size = sizeof(eae6320::Physics::Octree::NodeData::m_no_tris);
					char* buffer = reinterpret_cast<char*>(&node.m_no_tris);
					outfile.write(buffer, size);
				}
				if (node.m_no_tris > 0) {
					size = sizeof(eae6320::Physics::Octree::NodeData::m_no_tris)*node.m_no_tris;
					char* buffer = reinterpret_cast<char*>(node.m_tris);
					outfile.write(buffer, size);
					free(node.m_tris);
				}
			}
		}
		outfile.close();
		return true;
	}
}