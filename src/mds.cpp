
#include "mds.h"

using namespace MOD;

namespace MDS
{
	int Read(lua_State* L)
	{
		byte* ptr = Util::CheckHeader(L, "EQGS", ".mds");

		Header* header = (Header*)ptr;
		uint32 pos = Header::SIZE;

		lua_createtable(L, 0, 5); //to return

		const char* string_block = (const char*)&ptr[pos];
		pos += header->strings_len;

		//materials and their properties
		lua_createtable(L, header->material_count, 0);

		for (uint32 i = 1; i <= header->material_count; ++i)
		{
			Material* mat = (Material*)&ptr[pos];
			pos += Material::SIZE;

			lua_pushinteger(L, i);
			lua_createtable(L, mat->property_count, 2); //one table per material

			lua_pushstring(L, &string_block[mat->name_index]);
			lua_setfield(L, -2, "name");
			lua_pushstring(L, &string_block[mat->shader_index]);
			lua_setfield(L, -2, "shader");

			for (uint32 j = 1; j <= mat->property_count; ++j)
			{
				Property* prop = (Property*)&ptr[pos];
				pos += Property::SIZE;

				lua_pushinteger(L, j);
				lua_createtable(L, 0, 3);

				lua_pushstring(L, &string_block[prop->name_index]);
				lua_setfield(L, -2, "name");
				lua_pushinteger(L, prop->type);
				lua_setfield(L, -2, "type");
				if (prop->type == 0)
					lua_pushnumber(L, Util::FloatToDouble(prop->value.f));
				else
					lua_pushstring(L, &string_block[prop->value.i]);
				lua_setfield(L, -2, "value");

				lua_settable(L, -3);
			}

			lua_settable(L, -3);
		}

		lua_setfield(L, -2, "materials");

		//bones
		lua_createtable(L, header->bone_count, 0);

		for (uint32 i = 1; i <= header->bone_count; ++i)
		{
			Bone* bone = (Bone*)&ptr[pos];
			pos += Bone::SIZE;

			lua_pushinteger(L, i);
			lua_createtable(L, 0, 12);

			lua_pushstring(L, &string_block[bone->name_index]);
			lua_setfield(L, -2, "name");
			lua_pushinteger(L, bone->flag);
			lua_setfield(L, -2, "flag");

			lua_pushnumber(L, Util::FloatToDouble(bone->x));
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, Util::FloatToDouble(bone->y));
			lua_setfield(L, -2, "y");
			lua_pushnumber(L, Util::FloatToDouble(bone->z));
			lua_setfield(L, -2, "z");
			lua_pushnumber(L, Util::FloatToDouble(bone->rot_x));
			lua_setfield(L, -2, "rot_x");
			lua_pushnumber(L, Util::FloatToDouble(bone->rot_y));
			lua_setfield(L, -2, "rot_y");
			lua_pushnumber(L, Util::FloatToDouble(bone->rot_z));
			lua_setfield(L, -2, "rot_z");
			lua_pushnumber(L, Util::FloatToDouble(bone->rot_w));
			lua_setfield(L, -2, "rot_w");
			lua_pushnumber(L, Util::FloatToDouble(bone->scale_x));
			lua_setfield(L, -2, "scale_x");
			lua_pushnumber(L, Util::FloatToDouble(bone->scale_y));
			lua_setfield(L, -2, "scale_y");
			lua_pushnumber(L, Util::FloatToDouble(bone->scale_z));
			lua_setfield(L, -2, "scale_z");

			if (bone->base_bone != 0xFFFFFFFF)
			{
				lua_pushinteger(L, bone->base_bone);
				lua_setfield(L, -2, "base_bone");
			}
			if (bone->next_bone != 0xFFFFFFFF)
			{
				lua_pushinteger(L, bone->next_bone);
				lua_setfield(L, -2, "next_bone");
			}

			lua_settable(L, -3);
		}

		lua_setfield(L, -2, "bones");

		/*int vert_count = 0;
		int tri_count = 0;
		int ba_count = 0;
		lua_newtable(L); //vertices, -3
		lua_newtable(L); //triangles, -2
		lua_newtable(L); //bone assignments, -1

		const int n = header->sub_count;
		for (int i = -1; i < n && i < 1; ++i)
		{
			SubHeader* subheader = (SubHeader*)&ptr[pos];
			pos += SubHeader::SIZE;

			for (uint32 j = 0; j < subheader->vertex_count; ++j)
			{
				Vertex* vert = (Vertex*)&ptr[pos];
				pos += Vertex::SIZE;

				lua_pushinteger(L, ++vert_count);
				lua_createtable(L, 0, 8);

				lua_pushnumber(L, Util::FloatToDouble(vert->x));
				lua_setfield(L, -2, "x");
				lua_pushnumber(L, Util::FloatToDouble(vert->y));
				lua_setfield(L, -2, "y");
				lua_pushnumber(L, Util::FloatToDouble(vert->z));
				lua_setfield(L, -2, "z");
				lua_pushnumber(L, Util::FloatToDouble(vert->i));
				lua_setfield(L, -2, "i");
				lua_pushnumber(L, Util::FloatToDouble(vert->j));
				lua_setfield(L, -2, "j");
				lua_pushnumber(L, Util::FloatToDouble(vert->k));
				lua_setfield(L, -2, "k");
				lua_pushnumber(L, Util::FloatToDouble(vert->u));
				lua_setfield(L, -2, "u");
				lua_pushnumber(L, Util::FloatToDouble(vert->v));
				lua_setfield(L, -2, "v");

				lua_settable(L, -5);
			}

			for (uint32 j = 0; j < subheader->triangle_count; ++j)
			{
				Triangle* tri = (Triangle*)&ptr[pos];
				pos += Triangle::SIZE;

				lua_pushinteger(L, ++tri_count);
				lua_createtable(L, 3, 2);

				lua_pushinteger(L, 1);
				lua_pushinteger(L, tri->index[0]);
				lua_settable(L, -3);
				lua_pushinteger(L, 2);
				lua_pushinteger(L, tri->index[1]);
				lua_settable(L, -3);
				lua_pushinteger(L, 3);
				lua_pushinteger(L, tri->index[2]);
				lua_settable(L, -3);
				lua_pushinteger(L, tri->material);
				lua_setfield(L, -2, "material");
				lua_pushinteger(L, tri->flag);
				lua_setfield(L, -2, "flag");

				lua_settable(L, -4);
			}

			for (uint32 j = 0; j < subheader->bone_assignment_count; ++j)
			{
				BoneAssignment* ba = (BoneAssignment*)&ptr[pos];
				pos += BoneAssignment::SIZE;

				lua_pushinteger(L, ++ba_count);
				lua_createtable(L, ba->count * 2, 1);

				lua_pushinteger(L, ba->count);
				lua_setfield(L, -2, "count");

				if (ba->count >= 1)
				{
					lua_pushinteger(L, 1);
					lua_pushinteger(L, ba->bone1);
					lua_settable(L, -3);
					lua_pushinteger(L, 2);
					lua_pushnumber(L, Util::FloatToDouble(ba->weight1));
					lua_settable(L, -3);

					if (ba->count >= 2)
					{
						lua_pushinteger(L, 3);
						lua_pushinteger(L, ba->bone2);
						lua_settable(L, -3);
						lua_pushinteger(L, 4);
						lua_pushnumber(L, Util::FloatToDouble(ba->weight2));
						lua_settable(L, -3);

						if (ba->count >= 3)
						{
							lua_pushinteger(L, 5);
							lua_pushinteger(L, ba->bone3);
							lua_settable(L, -3);
							lua_pushinteger(L, 6);
							lua_pushnumber(L, Util::FloatToDouble(ba->weight3));
							lua_settable(L, -3);

							if (ba->count >= 4)
							{
								lua_pushinteger(L, 7);
								lua_pushinteger(L, ba->bone4);
								lua_settable(L, -3);
								lua_pushinteger(L, 8);
								lua_pushnumber(L, Util::FloatToDouble(ba->weight4));
								lua_settable(L, -3);
							}
						}
					}
				}

				lua_settable(L, -3);
			}
		}

		lua_setfield(L, -4, "bone_assignments");
		lua_setfield(L, -3, "triangles");
		lua_setfield(L, -2, "vertices");

		return 1;*/
		
		//sub sections -- we only care about the first one (for now?)
		SubHeader* subheader = (SubHeader*)&ptr[pos];
		pos += SubHeader::SIZE;

		//vertices -- always the smaller type?
		lua_createtable(L, subheader->vertex_count, 0);

		for (uint32 i = 1; i <= subheader->vertex_count; ++i)
		{
			Vertex* vert = (Vertex*)&ptr[pos];
			pos += Vertex::SIZE;

			lua_pushinteger(L, i);
			lua_createtable(L, 0, 8);

			lua_pushnumber(L, Util::FloatToDouble(vert->x));
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, Util::FloatToDouble(vert->y));
			lua_setfield(L, -2, "y");
			lua_pushnumber(L, Util::FloatToDouble(vert->z));
			lua_setfield(L, -2, "z");
			lua_pushnumber(L, Util::FloatToDouble(vert->i));
			lua_setfield(L, -2, "i");
			lua_pushnumber(L, Util::FloatToDouble(vert->j));
			lua_setfield(L, -2, "j");
			lua_pushnumber(L, Util::FloatToDouble(vert->k));
			lua_setfield(L, -2, "k");
			lua_pushnumber(L, Util::FloatToDouble(vert->u));
			lua_setfield(L, -2, "u");
			lua_pushnumber(L, Util::FloatToDouble(vert->v));
			lua_setfield(L, -2, "v");

			lua_settable(L, -3);
		}

		lua_setfield(L, -2, "vertices");

		//triangles
		lua_createtable(L, subheader->triangle_count, 0);

		for (uint32 i = 1; i <= subheader->triangle_count; ++i)
		{
			Triangle* tri = (Triangle*)&ptr[pos];
			pos += Triangle::SIZE;

			lua_pushinteger(L, i);
			lua_createtable(L, 3, 2);

			lua_pushinteger(L, 1);
			lua_pushinteger(L, tri->index[0]);
			lua_settable(L, -3);
			lua_pushinteger(L, 2);
			lua_pushinteger(L, tri->index[1]);
			lua_settable(L, -3);
			lua_pushinteger(L, 3);
			lua_pushinteger(L, tri->index[2]);
			lua_settable(L, -3);
			lua_pushinteger(L, tri->material);
			lua_setfield(L, -2, "material");
			lua_pushinteger(L, tri->flag);
			lua_setfield(L, -2, "flag");

			lua_settable(L, -3);
		}

		lua_setfield(L, -2, "triangles");

		//bone assignments
		lua_createtable(L, subheader->bone_assignment_count, 0);

		for (uint32 i = 1; i<= subheader->bone_assignment_count; ++i)
		{
			BoneAssignment* ba = (BoneAssignment*)&ptr[pos];
			pos += BoneAssignment::SIZE;

			lua_pushinteger(L, i);
			lua_createtable(L, ba->count * 2, 1);

			lua_pushinteger(L, ba->count);
			lua_setfield(L, -2, "count");

			if (ba->count >= 1)
			{
				lua_pushinteger(L, 1);
				lua_pushinteger(L, ba->bone1);
				lua_settable(L, -3);
				lua_pushinteger(L, 2);
				lua_pushnumber(L, Util::FloatToDouble(ba->weight1));
				lua_settable(L, -3);

				if (ba->count >= 2)
				{
					lua_pushinteger(L, 3);
					lua_pushinteger(L, ba->bone2);
					lua_settable(L, -3);
					lua_pushinteger(L, 4);
					lua_pushnumber(L, Util::FloatToDouble(ba->weight2));
					lua_settable(L, -3);

					if (ba->count >= 3)
					{
						lua_pushinteger(L, 5);
						lua_pushinteger(L, ba->bone3);
						lua_settable(L, -3);
						lua_pushinteger(L, 6);
						lua_pushnumber(L, Util::FloatToDouble(ba->weight3));
						lua_settable(L, -3);

						if (ba->count >= 4)
						{
							lua_pushinteger(L, 7);
							lua_pushinteger(L, ba->bone4);
							lua_settable(L, -3);
							lua_pushinteger(L, 8);
							lua_pushnumber(L, Util::FloatToDouble(ba->weight4));
							lua_settable(L, -3);
						}
					}
				}
			}

			lua_settable(L, -3);
		}
		
		lua_setfield(L, -2, "bone_assignments");

		return 1;
	}

	static const luaL_Reg funcs[] = {
		{"Read", Read},
		{nullptr, nullptr}
	};

	void LoadFunctions(lua_State* L)
	{
		luaL_register(L, "mds", funcs);
	}
}
