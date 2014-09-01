
#include "mod.h"

namespace MOD
{
	int Read(lua_State* L)
	{
		byte* ptr = Util::CheckHeader(L, "EQGM", ".mod");

		Header* header = (Header*)ptr;
		uint32 pos = sizeof(Header);

		lua_createtable(L, 0, 5); //to return

		const char* string_block = (const char*)&ptr[pos];
		pos += header->strings_len;

		//materials and their properties
		lua_createtable(L, header->material_count, 0);

		for (uint32 i = 1; i <= header->material_count; ++i)
		{
			Material* mat = (Material*)&ptr[pos];
			pos += sizeof(Material);

			lua_pushinteger(L, i);
			lua_createtable(L, mat->property_count, 2); //one table per material

			lua_pushstring(L, &string_block[mat->name_index]);
			lua_setfield(L, -2, "name");
			lua_pushstring(L, &string_block[mat->shader_index]);
			lua_setfield(L, -2, "shader");

			for (uint32 j = 1; j <= mat->property_count; ++j)
			{
				Property* prop = (Property*)&ptr[pos];
				pos += sizeof(Property);

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

		//vertices
		lua_createtable(L, header->vertex_count, 0);

		if (header->version < 3)
		{
			for (uint32 i = 1; i <= header->vertex_count; ++i)
			{
				Vertex* vert = (Vertex*)&ptr[pos];
				pos += sizeof(Vertex);

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
		}
		else
		{
			for (uint32 i = 1; i <= header->vertex_count; ++i)
			{
				VertexV3* vert = (VertexV3*)&ptr[pos];
				pos += sizeof(VertexV3);

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
		}

		lua_setfield(L, -2, "vertices");

		//triangles
		lua_createtable(L, header->triangle_count, 0);

		for (uint32 i = 1; i <= header->triangle_count; ++i)
		{
			Triangle* tri = (Triangle*)&ptr[pos];
			pos += sizeof(Triangle);

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

		//bones, if the model has animations...
		lua_createtable(L, header->bone_count, 0);

		for (uint32 i = 1; i <= header->bone_count; ++i)
		{
			Bone* bone = (Bone*)&ptr[pos];
			pos += sizeof(Bone);

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

		//bone assignments
		if (header->bone_count > 0)
		{
			lua_createtable(L, header->vertex_count, 0);

			for (uint32 i = 1; i<= header->vertex_count; ++i)
			{
				BoneAssignment* ba = (BoneAssignment*)&ptr[pos];
				pos += sizeof(BoneAssignment);

				lua_pushinteger(L, i);
				lua_createtable(L, ba->count * 2, 1);

				lua_pushinteger(L, ba->count);
				lua_setfield(L, -2, "count");

				for (uint32 j = 0; j < ba->count; ++j)
				{
					lua_pushinteger(L, j * 2 + 1);
					lua_pushinteger(L, ba->weights[j].bone);
					lua_settable(L, -3);
					lua_pushinteger(L, j * 2 + 2);
					lua_pushnumber(L, Util::FloatToDouble(ba->weights[j].weight));
					lua_settable(L, -3);
				}

				lua_settable(L, -3);
			}
		}
		else
		{
			lua_newtable(L);
		}
		
		lua_setfield(L, -2, "bone_assignments");

		return 1;
	}

	int Write(lua_State* L)
	{
		//takes a mod data table, returns a .eqg directory entry table
		Util::PrepareWrite(L, ".mod");

		//write .mod data
		Header header;
		header.magic[0] = 'E';
		header.magic[1] = 'Q';
		header.magic[2] = 'G';
		header.magic[3] = 'M';
		header.version = 1;

		Util::Buffer name_buf;
		Util::Buffer data_buf;

		lua_getfield(L, 1, "materials");
		header.material_count = lua_objlen(L, -1);
		for (uint32 i = 1; i <= header.material_count; ++i)
		{
			uint32 len;
			const char* name;
			lua_pushinteger(L, i);
			lua_gettable(L, -2);

			Material mat;
			mat.index = i - 1;
			
			name = Util::GetString(L, -1, "name", len);
			mat.name_index = name_buf.GetLen();
			name_buf.Add(name, len);
			name = Util::GetString(L, -1, "shader", len);
			mat.shader_index = name_buf.GetLen();
			name_buf.Add(name, len);

			mat.property_count = lua_objlen(L, -1);
			data_buf.Add(&mat, sizeof(Material));

			for (uint32 j = 1; j <= mat.property_count; ++j)
			{
				lua_pushinteger(L, j);
				lua_gettable(L, -2);

				Property prop;
				name = Util::GetString(L, -1, "name", len);
				prop.name_index = name_buf.GetLen();
				name_buf.Add(name, len);
				prop.type = Util::GetInt(L, -1, "type");
				if (prop.type == 0)
				{
					prop.value.f = Util::GetFloat(L, -1, "value");
				}
				else
				{
					name = Util::GetString(L, -1, "value", len);
					prop.value.i = name_buf.GetLen();
					name_buf.Add(name, len);
				}

				data_buf.Add(&prop, sizeof(Property));
				lua_pop(L, 1);
			}

			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "vertices");
		header.vertex_count = lua_objlen(L, -1);
		for (uint32 i = 1; i <= header.vertex_count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, -2);

			Vertex vert;
			vert.x = Util::GetFloat(L, -1, "x");
			vert.y = Util::GetFloat(L, -1, "y");
			vert.z = Util::GetFloat(L, -1, "z");
			vert.i = Util::GetFloat(L, -1, "i");
			vert.j = Util::GetFloat(L, -1, "j");
			vert.k = Util::GetFloat(L, -1, "k");
			vert.u = Util::GetFloat(L, -1, "u");
			vert.v = Util::GetFloat(L, -1, "v");

			data_buf.Add(&vert, sizeof(Vertex));
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "triangles");
		header.triangle_count = lua_objlen(L, -1);
		for (uint32 i = 1; i <= header.triangle_count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, -2);

			Triangle tri;
			for (int j = 0; j < 3; ++j)
			{
				lua_pushinteger(L, j + 1);
				lua_gettable(L, -2);
				tri.index[j] = lua_tointeger(L, -1);
				lua_pop(L, 1);
			}
			tri.material = Util::GetInt(L, -1, "material");
			tri.flag = Util::GetInt(L, -1, "flag");

			data_buf.Add(&tri, sizeof(Triangle));
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "bones");
		header.bone_count = lua_objlen(L, -1);
		for (uint32 i = 1; i <= header.bone_count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, -2);

			Bone bone;

			uint32 len;
			const char* name = Util::GetString(L, -1, "name", len);
			bone.name_index = name_buf.GetLen();
			name_buf.Add(name, len);

			bone.x = Util::GetFloat(L, -1, "x");
			bone.y = Util::GetFloat(L, -1, "y");
			bone.z = Util::GetFloat(L, -1, "z");
			bone.rot_w = Util::GetFloat(L, -1, "rot_w");
			bone.rot_x = Util::GetFloat(L, -1, "rot_x");
			bone.rot_y = Util::GetFloat(L, -1, "rot_y");
			bone.rot_z = Util::GetFloat(L, -1, "rot_z");
			bone.scale_x = Util::GetFloat(L, -1, "scale_x");
			bone.scale_y = Util::GetFloat(L, -1, "scale_y");
			bone.scale_z = Util::GetFloat(L, -1, "scale_z");

			lua_getfield(L, -1, "base_bone");
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				bone.base_bone = 0xFFFFFFFF;
			}
			else
			{
				lua_pop(L, 1);
				bone.base_bone = Util::GetInt(L, -1, "base_bone");
			}

			lua_getfield(L, -1, "next_bone");
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				bone.next_bone = 0xFFFFFFFF;
			}
			else
			{
				lua_pop(L, 1);
				bone.next_bone = Util::GetInt(L, -1, "next_bone");
			}

			data_buf.Add(&bone, sizeof(Bone));
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "bone_assignments");
		uint32 count = lua_objlen(L, -1);
		for (uint32 i = 1; i <= count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, -2);

			BoneAssignment ba;

			uint32 count = Util::GetInt(L, -1, "count");
			ba.count = count;

			memset(ba.weights, 0, sizeof(Weight) * 4);

			for (uint32 j = 0; j < count; ++j)
			{
				ba.weights[j].bone = Util::GetInt(L, -2, j * 2 + 1);
				ba.weights[j].weight = Util::GetFloat(L, -2, j * 2 + 2);
			}

			data_buf.Add(&ba, sizeof(BoneAssignment));
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		header.strings_len = name_buf.GetLen();

		Util::Buffer buf;
		buf.Add(&header, sizeof(Header));

		byte* b = name_buf.Take();
		buf.Add(b, name_buf.GetLen());
		delete[] b;

		b = data_buf.Take();
		buf.Add(b, data_buf.GetLen());
		delete[] b;

		return Util::FinishWrite(L, buf);
	}

	static const luaL_Reg funcs[] = {
		{"Read", Read},
		{"Write", Write},
		{nullptr, nullptr}
	};

	void LoadFunctions(lua_State* L)
	{
		luaL_register(L, "mod", funcs);
	}
}
