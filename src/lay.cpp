
#include "lay.h"

namespace LAY
{
	int Read(lua_State* L)
	{
		byte* ptr = Util::CheckHeader(L, "EQGL", ".lay");

		Header* header = (Header*)ptr;
		uint32 pos = Header::SIZE;
		uint32 ent_size;
		switch (header->version)
		{
		case 2:
			ent_size = Entry::SIZE_V2;
			break;
		case 3:
			ent_size = Entry::SIZE_V3;
			break;
		case 4:
			ent_size = Entry::SIZE_V4;
			break;
		}

		const char* string_block = (const char*)&ptr[pos];
		pos += header->strings_len;

		lua_createtable(L, header->count, 0); //to return

		for (uint32 i = 1; i <= header->count; ++i)
		{
			Entry* e = (Entry*)&ptr[pos];
			pos += ent_size;

			lua_pushinteger(L, i);
			lua_createtable(L, 0, 3);

			lua_pushstring(L, &string_block[e->name_index]);
			lua_setfield(L, -2, "material_identifier");
			lua_pushstring(L, &string_block[e->texture_index[0]]);
			lua_setfield(L, -2, "diffuse");
			lua_pushstring(L, &string_block[e->texture_index[1]]);
			lua_setfield(L, -2, "normal");

			lua_settable(L, -3);
		}

		return 1;
	}

	int Write(lua_State* L)
	{
		return 0;
	}

	static const luaL_Reg funcs[] = {
		{"Read", Read},
		{"Write", Write},
		{nullptr, nullptr}
	};

	void LoadFunctions(lua_State* L)
	{
		luaL_register(L, "lay", funcs);
	}
}
