#ifndef EAE6320_MESH_PARSER_H
#define EAE6320_MESH_PARSER_H

#include "../../External/Lua/Includes.h"
#include "../../Engine/Graphics/VertexData.h"

namespace eae6320 
{
	namespace AssetBuild
	{
		class cMayaMeshParser
		{
		public:
			bool LoadAsset(const char* const i_path);
			uint32_t verticesCount;
			uint32_t indicesCount;
			eae6320::Graphics::sVertex* vertexData;
			uint32_t* indices;
		private:
			bool LoadTableValues(lua_State& io_luaState);
			bool LoadTableKey_vertices(lua_State& io_luaState);
			bool LoadTableValues_vertices(lua_State& io_luaState);
			bool LoadTableKey_position(lua_State& io_luaState, int index);
			bool LoadTableValues_position(lua_State& io_luaState, int index);
			bool LoadTableKey_color(lua_State& io_luaState, int index);
			bool LoadTableValues_color(lua_State& io_luaState, int index);
			bool LoadTableKey_Texture(lua_State& io_luaState, int index);
			bool LoadTableValues_Textures(lua_State& io_luaState, int index);
			bool LoadTableKey_indices(lua_State& io_luaState);
			bool LoadTableValues_indices(lua_State& io_luaState);
		};
	} //namespace AssetBuild
} // namespace eae6320
#endif // !EAE6320_MESH_PARSER_H