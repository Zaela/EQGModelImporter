
#include "viewer.h"

using namespace irr;

extern std::thread* gViewerThread;
extern std::atomic<Viewer::Update*> gViewerUpdate;
extern std::atomic_flag gRunThread;

namespace Viewer
{
	void ThreadMain(uint32 w, uint32 h)
	{
		Controller ctrl;

		IrrlichtDevice* device = createDevice(video::EDT_OPENGL,
			core::dimension2du(w, h), 16, false, false, true, &ctrl);

		if (device == nullptr)
		{
			gRunThread.clear();
			return;
		}

		device->setWindowCaption(L"EQG Model Viewer");
		scene::ISceneManager* mgr = device->getSceneManager();
		video::IVideoDriver* driver = device->getVideoDriver();
		gui::IGUIEnvironment* gui = device->getGUIEnvironment();

		ctrl.Init(device);

		mgr->addCameraSceneNodeMaya(nullptr, -100.0f, 10.0f, 2.5f, -1, 5);

		for (;;)
		{
			//check if we should close thread
			if (gRunThread.test_and_set() == false)
			{
				device->drop();
				gRunThread.clear();
				gViewerUpdate.store(nullptr);
				return;
			}

			//check if we have a new model to view
			Update* update = gViewerUpdate.load();
			if (update)
			{
				gViewerUpdate.store(nullptr);
				scene::CSkinnedMesh* mesh = update->mesh;
				if (mesh)
				{
					mgr->clear();
					mgr->setAmbientLight(video::SColorf(1, 1, 1));

					std::vector<ImageFile*>* images = update->images;

					uint32 i = 0;
					for (ImageFile* file : *images)
					{
						if (file)
						{
							video::ITexture* tex = driver->getTexture(file);
							if (tex)
							{
								auto& mat = mesh->getMeshBuffer(i)->getMaterial();
								mat.setTexture(0, tex);
								//if (file->alpha)
								//	mat.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
							}
							file->drop();
						}
						++i;
					}
					delete images;

					scene::CSkinnedMesh* static_mesh = new scene::CSkinnedMesh;
					core::array<scene::SSkinMeshBuffer*>& mbufs = mesh->getMeshBuffers();
					for (uint32 i = 0; i < mbufs.size(); ++i)
					{
						scene::SSkinMeshBuffer* buf = static_mesh->addMeshBuffer();
						*buf = *(mbufs[i]);
					}

					scene::IAnimatedMeshSceneNode* node = mgr->addAnimatedMeshSceneNode(mesh);
					scene::IAnimatedMeshSceneNode* static_node = mgr->addAnimatedMeshSceneNode(static_mesh);

					auto& c = node->getBoundingBox().getCenter();
					scene::ICameraSceneNode* cam = mgr->addCameraSceneNodeMaya(nullptr, -100.0f, 10.0f, 2.5f, -1, 5);
					if (update->isWeapon)
					{
						node->setRotation(core::vector3df(0, 0, 90));
						static_node->setRotation(core::vector3df(0, 0, 90));
						cam->setTarget(node->getAbsolutePosition() + core::vector3df(0, c.X, 0));
					}
					else
					{
						node->setRotation(core::vector3df(-90, 0, 0));
						static_node->setRotation(core::vector3df(-90, 0, 0));
						cam->setTarget(node->getPosition() + core::vector3df(0, c.Y, 0));
					}

					/*scene::SMeshBuffer* mb = new scene::SMeshBuffer;
					video::S3DVertex vert(0, 0, 0, 0, 0, 0, video::SColor(1, 1, 1, 1), 0, 0);
					mb->Vertices.push_back(vert);
					mb->Vertices.push_back(vert);
					mb->Vertices.push_back(vert);
					//mb->Indices.push_back(0);
					//mb->Indices.push_back(1);
					//mb->Indices.push_back(2);
					mb->getMaterial().PointCloud = true;
					mb->getMaterial().Thickness = 3;
					mb->getMaterial().ZBuffer = video::ECFN_NEVER;
					mb->recalculateBoundingBox();
					scene::SMesh* sm = new scene::SMesh;
					sm->addMeshBuffer(mb);
					scene::SAnimatedMesh* am = new scene::SAnimatedMesh(sm);
					mgr->addAnimatedMeshSceneNode(am);//, bone);*/

					node->setAnimationSpeed(1000.0f);
					ctrl.SetNode(node, static_node);
					ctrl.SetAnimArray(update->animations);

					//fix this to not be so terrible
					//gui::IGUIStaticText* txt = gui->addStaticText(L"Text goes here", core::recti(10, 10, 300, 125));
				}

				delete update;
			}

			if (device->run())
			{
				driver->beginScene(true, true, video::SColor(255, 128, 128, 128));
				mgr->drawAll();
				gui->drawAll();
				driver->endScene();
			}
			else
			{
				gViewerUpdate.store(nullptr);
				device->drop();
				gRunThread.clear();
				return;
			}
			
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	}

	bool Controller::OnEvent(const SEvent& ev)
	{
		switch (ev.EventType)
		{
		case EET_KEY_INPUT_EVENT:
		{
			const SEvent::SKeyInput& k = ev.KeyInput;
			switch (k.Key)
			{
			case KEY_KEY_W:
				if (k.PressedDown && mNode)
				{
					for (uint32 i = 0; i < mNode->getMaterialCount(); ++i)
					{
						video::SMaterial& mat = mNode->getMaterial(i);
						mat.Wireframe = !mat.Wireframe;
						mat.BackfaceCulling = !mat.BackfaceCulling;
						video::SMaterial& mat2 = mRagdollNode->getMaterial(i);
						mat2.Wireframe = !mat2.Wireframe;
						mat2.BackfaceCulling = !mat2.BackfaceCulling;
					}
				}
				break;
			case KEY_KEY_B:
				if (k.PressedDown && mNode)
				{
					uint32 d = mNode->isDebugDataVisible();
					d = (d & scene::EDS_BBOX) ? d ^ scene::EDS_BBOX : d | scene::EDS_BBOX;
					mNode->setDebugDataVisible(d);
					//mRagdollNode->setDebugDataVisible(d);
				}
				break;
			case KEY_KEY_S:
				if (k.PressedDown && mNode)
				{
					uint32 d = mNode->isDebugDataVisible();
					d = (d & scene::EDS_SKELETON) ? d ^ scene::EDS_SKELETON : d | scene::EDS_SKELETON;
					mNode->setDebugDataVisible(d);
				}
				break;
			case KEY_RIGHT:
				if (k.PressedDown && mNode && mAnimations)
				{
					SetAnim(mAnimNum + 1);
				}
				break;
			case KEY_LEFT:
				if (k.PressedDown && mNode && mAnimations)
				{
					SetAnim(mAnimNum - 1);
				}
			default:
				break;
			}
			break;
		}
		default:
			break;
		}

		return false;
	}

	void Controller::SetAnim(int num)
	{
		if (num >= (int)mAnimations->size())
			num = -1;
		else if (num < -1)
			num = (int)mAnimations->size() - 1;

		if (num == -1)
		{
			mRagdollNode->setVisible(true);
			mNode->setVisible(false);
			mAnimName = nullptr;
			SetText("Base Pose");
		}
		else
		{
			if (mAnimNum == -1)
			{
				mRagdollNode->setVisible(false);
				mNode->setVisible(true);
			}
			Animation anim = (*mAnimations)[num];
			mNode->setFrameLoop(anim.start_frame, anim.end_frame);
			mAnimName = anim.name;
			SetText(anim.name);
		}
		mAnimNum = num;
	}

	void Controller::SetText(const char* text)
	{
		core::stringw txt = text;
		mText->setText(txt.c_str());
	}

	/*void PointNode::render()
	{
		video::IVideoDriver* driver = SceneManager->getVideoDriver();
		scene::ISceneCollisionManager* colmgr = SceneManager->getSceneCollisionManager();
		if (!driver || !colmgr)
			return;

		driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

		video::SMaterial debug_mat;
		//debug_mat.DiffuseColor = 
		debug_mat.Lighting = false;
		debug_mat.AntiAliasing = 0;
		debug_mat.ZBuffer = video::ECFN_NEVER;
		driver->setMaterial(debug_mat);

		core::vector2di& pos = colmgr->getScreenCoordinatesFrom3DPosition(core::vector3df(-1));
		core::recti rect(pos.X - 20, pos.Y - 20, pos.X + 20, pos.Y + 20);

		driver->draw2DRectangle(video::SColor(1, 1, 1, 1), rect);
	}

	void PointNode::OnRegisterSceneNode()
    {
		SceneManager->registerNodeForRendering(this);
		ISceneNode::OnRegisterSceneNode();
    }*/

	ImageFile* CreateImageFile(lua_State* L, int tbl, int info_tbl, int mat, bool& cur_dds)
	{
		lua_pushinteger(L, mat + 1);
		lua_gettable(L, tbl);

		if (!lua_istable(L, -1))
		{
			lua_pop(L, 1);
			return nullptr;
		}

		lua_getfield(L, -1, "ptr");
		byte* data = (byte*)lua_touserdata(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, -1, "inflated_len");
		uint32 len = lua_tointeger(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, -1, "png_name");
		const char* name = lua_tostring(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, -1, "isDDS");
		cur_dds = lua_toboolean(L, -1);
		lua_pop(L, 1);

		fipMemoryIO mem(data, len);
		fipImage img;
		img.loadFromMemory(mem);
		fipMemoryIO out;
		img.saveToMemory(FIF_PNG, out);

		out.seek(0, SEEK_END);
		unsigned long size = out.tell();
		out.seek(0, SEEK_SET);
		byte* ptr = new byte[size];
		out.read(ptr, 1, size);

		lua_pop(L, 1);
		ImageFile* ret = new ImageFile(name, ptr, size);

		lua_pushinteger(L, mat);
		lua_gettable(L, info_tbl);

		if (!lua_istable(L, -1))
		{
			lua_pop(L, 1);
			return ret;
		}

		lua_getfield(L, -1, "alpha");
		ret->alpha = lua_toboolean(L, -1);
		lua_pop(L, 1);

		return ret;
	}

	void HandleBoneRecurse(lua_State* L, scene::CSkinnedMesh* sk, int i, scene::ISkinnedMesh::SJoint* parent)
	{
		lua_pushinteger(L, i);
		lua_gettable(L, 3);

		lua_getfield(L, -1, "base_bone");
		if (lua_isnumber(L, -1))
		{
			int base = lua_tointeger(L, -1);
			HandleBoneRecurse(L, sk, base + 1, parent);
		}
		lua_pop(L, 1);

		scene::ISkinnedMesh::SJoint* bone = sk->addJoint(parent);
		bone->Name = core::stringc(Util::GetString(L, -1, "name"));
		bone->Name.make_upper();

		bone->Animatedposition = core::vector3df(
			Util::GetFloat(L, -1, "x"),
			Util::GetFloat(L, -1, "y"),
			Util::GetFloat(L, -1, "z")
		);

		bone->Animatedrotation = core::quaternion(
			Util::GetFloat(L, -1, "rot_x"),
			Util::GetFloat(L, -1, "rot_y"),
			Util::GetFloat(L, -1, "rot_z"),
			Util::GetFloat(L, -1, "rot_w")
		);

		bone->Animatedscale = core::vector3df(
			Util::GetFloat(L, -1, "scale_x"),
			Util::GetFloat(L, -1, "scale_y"),
			Util::GetFloat(L, -1, "scale_z")
		);

		core::matrix4 positionMatrix;
		positionMatrix.setTranslation(bone->Animatedposition);
		core::matrix4 scaleMatrix;
		scaleMatrix.setScale(bone->Animatedscale);
		core::matrix4 rotationMatrix;
		bone->Animatedrotation.getMatrix_transposed(rotationMatrix);

		bone->LocalMatrix = positionMatrix * rotationMatrix * scaleMatrix;

		if (parent)
			bone->GlobalMatrix = parent->GlobalMatrix * bone->LocalMatrix;
		else
			bone->GlobalMatrix = bone->LocalMatrix;

		int next = -1;
		lua_getfield(L, -1, "next_bone");
		if (lua_isnumber(L, -1))
		{
			next = lua_tointeger(L, -1) + 1;
		}
		lua_pop(L, 1);

		lua_pop(L, 1); //bone

		if (next != -1)
			HandleBoneRecurse(L, sk, next, bone);
	}

	int LoadModel(lua_State* L)
	{
		//vertices, triangles, bones, bone assignments, animations, decompressed textures, texture info
		luaL_checktype(L, 1, LUA_TTABLE);
		luaL_checktype(L, 2, LUA_TTABLE);
		luaL_checktype(L, 3, LUA_TTABLE);
		luaL_checktype(L, 4, LUA_TTABLE);
		luaL_checktype(L, 5, LUA_TTABLE);
		luaL_checktype(L, 6, LUA_TTABLE);
		luaL_checktype(L, 7, LUA_TTABLE);
		bool isWeapon = lua_toboolean(L, 8);

		scene::CSkinnedMesh* sk = new scene::CSkinnedMesh;

		const int bone_count = lua_objlen(L, 3);
		if (bone_count > 0)
		{
			HandleBoneRecurse(L, sk, 1, nullptr);
		}

		const int tri_count = lua_objlen(L, 2);

		uint16 buffer_id = 0;
		int cur_mat = -9999;
		bool cur_dds = false;
		float fps = 0.0f;

		std::unordered_map<int, ImageFile*> image_by_mat;
		std::vector<ImageFile*>* images = new std::vector<ImageFile*>;
		scene::SSkinMeshBuffer* cur_buf = sk->addMeshBuffer();

		lua_getfield(L, 5, "strict");
		int strict_bone_numbering = lua_toboolean(L, -1);
		lua_pop(L, 1);

		for (int i = 1; i <= tri_count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, 2);

			lua_getfield(L, -1, "material");
			int material = lua_tointeger(L, -1);
			lua_pop(L, 1);

			if (material != cur_mat || cur_buf->Indices.size() > 65530)
			{
				cur_mat = material;
				if (cur_mat >= 0)
				{
					if (cur_buf->getVertexCount() > 0)
					{
						cur_buf->recalculateBoundingBox();
						cur_buf = sk->addMeshBuffer();
						++buffer_id;
					}

					ImageFile* add_image;
					if (image_by_mat.count(cur_mat))
					{
						add_image = image_by_mat[cur_mat];
						if (add_image)
							add_image->grab();
					}
					else
					{
						add_image = CreateImageFile(L, 6, 7, cur_mat, cur_dds);
						image_by_mat[cur_mat] = add_image;
					}

					images->push_back(add_image);
				}
			}

			//make triangle! laziness: no vertices are reused to avoid headache of re-indexing
			auto& indices = cur_buf->Indices;
			uint16 idx = indices.size();
			indices.push_back(idx);
			indices.push_back(idx + 1);
			indices.push_back(idx + 2);

			auto& verts = cur_buf->Vertices_Standard;

			//get vertices based on actual index
			for (int j = 1; j <= 3; ++j)
			{
				lua_pushinteger(L, j);
				lua_gettable(L, -2);

				int index = lua_tointeger(L, -1);
				lua_pop(L, 1);

				//get vertex
				lua_pushinteger(L, ++index);
				lua_gettable(L, 1);

				video::S3DVertex vert;
				vert.Pos.X = Util::GetFloat(L, -1, "x");
				vert.Pos.Y = Util::GetFloat(L, -1, "y");
				vert.Pos.Z = Util::GetFloat(L, -1, "z");
				vert.Normal.X = Util::GetFloat(L, -1, "i");
				vert.Normal.Y = Util::GetFloat(L, -1, "j");
				vert.Normal.Z = Util::GetFloat(L, -1, "k");
				vert.TCoords.X = Util::GetFloat(L, -1, "u");
				float v = Util::GetFloat(L, -1, "v");
				if (cur_dds)
					vert.TCoords.Y = (v > 0) ? -v : v;
				else
					vert.TCoords.Y = v;

				verts.push_back(vert);

				lua_pop(L, 1); //vertex

				//get bone assignments for this vertex, if we have a skeleton
				if (bone_count != 0)
				{
					lua_pushinteger(L, index);
					lua_gettable(L, 4);

					const int assign_count = Util::GetInt(L, -1, "count");
					for (int k = 0; k < assign_count; ++k)
					{
						int n = k * 2 + 1;
						lua_pushinteger(L, n);
						lua_gettable(L, -2);
						int boneid = lua_tointeger(L, -1);
						lua_pop(L, 1);
						lua_pushinteger(L, ++n);
						lua_gettable(L, -2);
						const float weight = static_cast<float>(lua_tonumber(L, -1));
						lua_pop(L, 1);

						if (strict_bone_numbering)
						{
							lua_pushinteger(L, boneid + 1);
							lua_gettable(L, 3);
							const char* name = Util::GetString(L, -1, "name");
							lua_pop(L, 1); //bone

							boneid = sk->getJointNumber(name);
							if (boneid == -1)
								continue;
						}

						scene::ISkinnedMesh::SJoint* bone = sk->getAllJoints()[boneid];
						scene::ISkinnedMesh::SWeight* wt = sk->addWeight(bone);
						wt->buffer_id = buffer_id;
						wt->strength = weight;
						wt->vertex_id = idx;
					}
					++idx;

					lua_pop(L, 1); //bone assignment
				}
			}

			lua_pop(L, 1); //triangle
		}

		std::vector<Animation>* animations = nullptr;
		//animations
		if (bone_count > 0)
		{
			animations = new std::vector<Animation>;
			int mem_frame_id = 0;
			int high_frame = 0;
			//two levels of tables: anim name -> anim data, bone name -> frame data
			lua_pushnil(L);
			while (lua_next(L, 5) != 0)
			{
				//anim name at -2, anim data at -1
				if (!lua_istable(L, -1))
				{
					lua_pop(L, 1);
					continue;
				}
				const char* anim_name = lua_tostring(L, -2);

				lua_pushnil(L);
				while (lua_next(L, -2) != 0)
				{
					//bone name at -2, frame data at -1
					if (!lua_istable(L, -1))
					{
						lua_pop(L, 1);
						continue;
					}

					const char* bone_name = lua_tostring(L, -2);
					const uint32 frame_count = Util::GetInt(L, -1, "count");
					lua_getfield(L, -1, "data");
					const byte* data = (byte*)lua_touserdata(L, -1);
					lua_pop(L, 1);

					int boneid = sk->getJointNumber(bone_name);
					if (boneid != -1)
					{
						scene::ISkinnedMesh::SJoint* bone = sk->getAllJoints()[boneid];

						uint32 pos = 0;
						for (uint32 i = 0; i < frame_count; ++i)
						{
							const ANI::Frame* frame = (const ANI::Frame*)&data[pos];
							pos += ANI::Frame::SIZE;

							const int fr = mem_frame_id + frame->milliseconds;

							scene::ISkinnedMesh::SPositionKey* pos = sk->addPositionKey(bone);
							pos->frame = fr;
							pos->position.X = frame->x;
							pos->position.Y = frame->y;
							pos->position.Z = frame->z;

							scene::ISkinnedMesh::SRotationKey* rot = sk->addRotationKey(bone);
							rot->frame = fr;
							rot->rotation.W = frame->rot_w;
							rot->rotation.X = frame->rot_x;
							rot->rotation.Y = frame->rot_y;
							rot->rotation.Z = frame->rot_z;

							scene::ISkinnedMesh::SScaleKey* scale = sk->addScaleKey(bone);
							scale->frame = fr;
							scale->scale.X = frame->scale_x;
							scale->scale.Y = frame->scale_y;
							scale->scale.Z = frame->scale_z;

							if (fr > high_frame)
								high_frame = fr;
						}
					}

					lua_pop(L, 1); //frame data
				}

				Animation anim;
				snprintf(anim.name, 32, "%s", anim_name);
				anim.start_frame = mem_frame_id;
				anim.end_frame = high_frame;
				mem_frame_id = high_frame + 1;

				animations->push_back(anim);

				lua_pop(L, 1); //anim data
			}
		}

		if (cur_buf->getVertexCount() > 0)
		{
			cur_buf->recalculateBoundingBox();
		}

		sk->finalize();

		Update* up = new Update;
		up->mesh = sk;
		up->images = images;
		up->animations = animations;
		up->isWeapon = isWeapon;

		gViewerUpdate.store(up);

		return 0;
	}

	int Open(lua_State* L)
	{
		//if flag is currently false and thread ptr has a value, it is deleteable
		if (gRunThread.test_and_set() == false && gViewerThread)
			delete gViewerThread;

		uint32 w = 0, h = 0;
		lua_getglobal(L, "settings");
		if (lua_istable(L, -1))
		{
			lua_getfield(L, -1, "viewer");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "width");
				w = lua_tointeger(L, -1);
				lua_pop(L, 1);
				lua_getfield(L, -1, "height");
				h = lua_tointeger(L, -1);
			}
		}

		gViewerThread = new std::thread(ThreadMain, (w >= 200) ? w : 500, (h >= 200) ? h : 500);
		gViewerThread->detach();

		lua_getglobal(L, "viewer");
		lua_pushboolean(L, true);
		lua_setfield(L, -2, "running");

		return 0;
	}

	int Close(lua_State* L)
	{
		gRunThread.clear();
		return 0;
	}

	static const luaL_Reg funcs[] = {
		{"LoadModel", LoadModel},
		{"Open", Open},
		{"Close", Close},
		{nullptr, nullptr}
	};

	void LoadFunctions(lua_State* L)
	{
		luaL_register(L, "viewer", funcs);
	}
}
