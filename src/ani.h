
#ifndef ANI_H
#define ANI_H

#include <lua.hpp>
#include <cstdio>
#include <cstring>
#include "types.h"
#include "util.h"

namespace ANI
{
	void LoadFunctions(lua_State* L);

	struct Header
	{
		char magic[4]; //EQGA
		uint32 version;
		uint32 strings_len;
		uint32 count;
		uint32 strict_bone_numbering; //only present if version > 1
		static const uint32 SIZE = sizeof(uint32) * 5;
		static const uint32 SIZE_VERSION1 = sizeof(uint32) * 4;
	};

	struct FramesHeader
	{
		uint32 frame_count;
		uint32 bone_name_index;
	};

	struct Frame
	{
		uint32 milliseconds;
		float x, y, z;
		float rot_x, rot_y, rot_z, rot_w;
		float scale_x, scale_y, scale_z;
	};
}

#endif
