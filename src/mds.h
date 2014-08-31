
#include <lua.hpp>
#include <cstdio>
#include <cstring>
#include "types.h"
#include "util.h"

//re-ordered MOD, basically
#include "mod.h"

namespace MDS
{
	void LoadFunctions(lua_State* L);

	struct Header
	{
		char magic[4]; //EQGS
		uint32 version;
		uint32 strings_len;
		uint32 material_count;
		uint32 bone_count;
		uint32 sub_count;
		static const uint32 SIZE = sizeof(uint32) * 6;
	};

	struct SubHeader
	{
		uint32 main_name_index;
		uint32 sub_name_index;
		uint32 vertex_count;
		uint32 triangle_count;
		uint32 bone_assignment_count;
		static const uint32 SIZE = sizeof(uint32) * 5;
	};
}
