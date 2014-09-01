
#include "ani.h"

namespace ANI
{
	int Read(lua_State* L)
	{
		byte* ptr = Util::CheckHeader(L, "EQGA", ".ani");

		Header* header = (Header*)ptr;
		uint32 pos;
		pos = (header->version > 1) ? Header::SIZE : Header::SIZE_VERSION1;

		lua_createtable(L, 0, header->count); //to return

		const char* string_block = (const char*)&ptr[pos];
		pos += header->strings_len;

		lua_pushboolean(L, true);
		lua_setfield(L, -2, "binary");

		lua_pushboolean(L, (header->version > 1 && header->strict_bone_numbering));
		lua_setfield(L, -2, "strict");

		for (uint32 i = 0; i < header->count; ++i)
		{
			FramesHeader* fh = (FramesHeader*)&ptr[pos];
			pos += sizeof(FramesHeader);

			lua_pushstring(L, &string_block[fh->bone_name_index]);
			lua_createtable(L, 0, 2);

			lua_pushinteger(L, fh->frame_count);
			lua_setfield(L, -2, "count");

			uint32 len = sizeof(Frame) * fh->frame_count;
			byte* data = (byte*)lua_newuserdata(L, len);
			memcpy(data, &ptr[pos], len);
			ptr += len;
			lua_setfield(L, -2, "data");

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
		luaL_register(L, "ani", funcs);
	}
}
