
#include <lua.hpp>
#include <cstdio>
#include <cstring>
#include "types.h"
#include "util.h"

namespace LAY
{
	void LoadFunctions(lua_State* L);

	struct Header
	{
		char magic[4]; //EQGL
		uint32 version;
		uint32 strings_len;
		uint32 count;
	};

	struct Entry
	{
		uint32 name_index;
		uint32 texture_index[5]; //only 0 and 1 meaningful(?)
		float unknownA;
		float unknownB;
		static const uint32 SIZE_V2 = sizeof(uint32) * 16;
		static const uint32 SIZE_V3 = sizeof(uint32) * 7;
		static const uint32 SIZE_V4 = sizeof(uint32) * 8;
	};
}
