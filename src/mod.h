
#ifndef MOD_H
#define MOD_H

#include <lua.hpp>
#include <cstdio>
#include <cstring>
#include "types.h"
#include "util.h"

namespace MOD
{
	void LoadFunctions(lua_State* L);

	struct Header
	{
		char magic[4]; //EQGM
		uint32 version;
		uint32 strings_len;
		uint32 material_count;
		uint32 vertex_count;
		uint32 triangle_count;
		uint32 bone_count;
	};

	struct Material
	{
		uint32 index;
		uint32 name_index;
		uint32 shader_index;
		uint32 property_count;
	};

	struct Property
	{
		uint32 name_index;
		uint32 type;
		union
		{
			uint32 i;
			float f;
		} value; //may be int (string index) or float depending on the type
	};

	struct Vertex
	{
		float x, y, z;
		float i, j, k;
		float u, v;
	};

	struct VertexV3
	{
		float x, y, z;
		float i, j, k;
		uint32 unknown_int;
		float u, v;
		float unknown_float[2];
	};

	struct Triangle
	{
		uint32 index[3];
		uint32 material;
		uint32 flag;
	};

	struct Bone
	{
		uint32 name_index;
		uint32 base_bone; //if not 0xFFFFFFFF, linked list of shared parentage
		uint32 flag; //seems to be always 0 on terminal bones
		uint32 next_bone; //if not 0xFFFFFFFF, bone which is child of this bone
		float x, y, z;
		float rot_x, rot_y, rot_z, rot_w; //rotations are in quaternion form
		float scale_x, scale_y, scale_z;
	};

	struct Weight
	{
		uint32 bone;
		float weight;
	};

	struct BoneAssignment
	{
		uint32 count; //max 4
		Weight weights[4];
	};
}

#endif//MOD_H
