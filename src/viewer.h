
#include <cstdio>
#include <cstring>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <lua.hpp>
#include <irrlicht.h>
#include <FreeImagePlus.h>
#include "types.h"
#include "util.h"
#include "ani.h"
#include "irrlicht/CSkinnedMesh.h"

using namespace irr;

namespace Viewer
{
	void LoadFunctions(lua_State* L);

	struct Animation;

	class Controller : public IEventReceiver
	{
	public:
		Controller() : mNode(nullptr), mRagdollNode(nullptr), mAnimations(nullptr), mAnimNum(0), mAnimName(nullptr)
		{
			
		}

		void Init(IrrlichtDevice* device)
		{
			mText = device->getGUIEnvironment()->addStaticText(L"Base Pose", core::recti(10, 10, 300, 125));
		}

		void SetNode(scene::IAnimatedMeshSceneNode* node, scene::IAnimatedMeshSceneNode* static_node)
		{
			mNode = node;
			mRagdollNode = static_node;
		}

		void SetAnimArray(std::vector<Animation>* anims)
		{
			if (mAnimations)
				delete mAnimations;
			mAnimations = anims;
			if (anims)
				SetAnim(-1);
		}

		void SetAnim(int num);
		const char* GetAnimName() const
		{
			if (mAnimName)
				return mAnimName;
			return nullptr;
		}

		void SetText(const char* text);

		virtual bool OnEvent(const SEvent& ev) override;

	private:
		scene::IAnimatedMeshSceneNode* mNode;
		scene::IAnimatedMeshSceneNode* mRagdollNode;
		gui::IGUIStaticText* mText;
		std::vector<Animation>* mAnimations;
		int mAnimNum;
		const char* mAnimName;
	};

	class ImageFile : public irr::io::IReadFile
	{
	public:
		ImageFile(const char* name, byte* data, uint32 len) :
			alpha(false),
			mName(name), mData(data), mLength(len), mPos(0)
		{

		}

		~ImageFile()
		{
			if (mData)
				delete[] mData;
		}

		virtual const io::path& getFileName() const
		{
			return mName;
		}

		virtual long getPos() const
		{
			return mPos;
		}

		virtual long getSize() const
		{
			return mLength;
		}

		virtual int32 read(void* buffer, uint32 sizeToRead)
		{
			long read = mPos + sizeToRead;
			if (read >= mLength)
				sizeToRead = mLength - mPos;
			memcpy(buffer, &mData[mPos], sizeToRead);
			mPos = read;
			return sizeToRead;
		}

		virtual bool seek(long finalPos, bool relativeMovement = false)
		{
			if (relativeMovement)
				finalPos += mPos;
			if (finalPos >= mLength)
				return false;
			mPos = finalPos;
			return true;
		}

	public:
		bool alpha;

	private:
		irr::io::path mName;
		byte* mData;
		long mLength;
		long mPos;
	};

	struct Animation
	{
		char name[32];
		int start_frame;
		int end_frame;
	};

	struct Update
	{
		scene::CSkinnedMesh* mesh;
		std::vector<ImageFile*>* images;
		std::vector<Animation>* animations;
		bool isWeapon;

		Update() : mesh(nullptr), images(nullptr), animations(nullptr), isWeapon(false) { }
	};

	/*class PointNode : public scene::ISceneNode
	{
	public:
		PointNode(scene::ISceneNode* parent, scene::ISceneManager* mgr) 
			: scene::ISceneNode(parent, mgr) { }

		virtual void render() override;
		virtual void OnRegisterSceneNode() override;
		virtual const core::aabbox3d<f32>& getBoundingBox() const override
		{
			return core::aabbox3df();
		}
	};*/
}
