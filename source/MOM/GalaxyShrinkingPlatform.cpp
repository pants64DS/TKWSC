#include "GalaxyShrinkingPlatform.h"

using GalaxyShrinkingPlatformCLPS = StaticCLPS_Block<{ .camBehavID = CLPS::CA_NORMAL }>;

SharedFilePtr GalaxyShrinkingPlatform::modelFile;
SharedFilePtr GalaxyShrinkingPlatform::clsnFile;
SharedFilePtr GalaxyShrinkingPlatform::frameModelFile;

SpawnInfo GalaxyShrinkingPlatform::spawnData =
{
	[] -> ActorBase* { return new GalaxyShrinkingPlatform; },
	0x0030,
	0x0100,
	0x00000002,
	0x00000000_f,
	0x005dc000_f,
	0x01000000_f,
	0x01000000_f
};

void GalaxyShrinkingPlatform::OnFloorAfterClsn(MeshColliderBase& clsn, Actor* clsnActor, Actor* otherActor)
{
	static_cast<GalaxyShrinkingPlatform*>(clsnActor)->shrinkActivated = true;
	//static_cast<GalaxyShrinkingPlatform*>(clsnActor)->playSound = true;
}


void GalaxyShrinkingPlatform::Shrink() {

	if (skl.x == 0x1000_f) {
		Sound::Play(4, 1, camSpacePos);
	}

	if (skl.x > 0_f) {

		skl.x = skl.x - 100_f;
		skl.z = skl.z - 100_f;

	} else {

		skl.x = 0_f;
		skl.z = 0_f;
		clsn.Disable();

	}

}

void GalaxyShrinkingPlatform::UnShrink() {

	skl.x = 0x1000_f;
	skl.z = 0x1000_f;
	clsn.Enable(this);
	shrinkActivated = false;

}

void GalaxyShrinkingPlatform::UpdateModelTransform()
{
	model.mat4x3 = Matrix4x3::RotationY(ang.y);
	model.mat4x3.c3 = pos >> 3;

	frameModel.mat4x3 = Matrix4x3::RotationY(ang.y);
	frameModel.mat4x3.c3 = pos >> 3;
}


[[gnu::target("thumb")]]
int GalaxyShrinkingPlatform::InitResources()
{
	BMD_File& modelF = modelFile.LoadBMD();
	model.SetFile(modelF, 1, -1);

	BMD_File& frameModelF = frameModelFile.LoadBMD();
	frameModel.SetFile(frameModelF, 1, -1);

	KCL_File& clsnF = clsnFile.LoadKCL();
	clsn.SetFile(&clsnF, clsnNextMat, 0x1000_f, ang.y, GalaxyShrinkingPlatformCLPS::instance<>);

	clsn.beforeClsnCallback = &MeshCollider::UpdatePosWithTransform;
	clsn.afterClsnCallback = &OnFloorAfterClsn;

	UpdateModelTransform();
	UpdateClsnPosAndRot();
	
	return 1;
}

int GalaxyShrinkingPlatform::CleanupResources()
{
	clsn.Disable();
	modelFile.Release();
	clsnFile.Release();
	frameModelFile.Release();
	return 1;
}

int GalaxyShrinkingPlatform::Behavior()
{
	UpdateModelTransform();
	if(IsClsnInRange(0_f, 0_f))
	{
		//Sound::Play(2, 17, camSpacePos);
		UpdateClsnPosAndRot();

		if (shrinkActivated) {
			Shrink();
		}

		/*
		if (playSound) {
			//Sound::Play(5, 0, camSpacePos);
			//Sound::PlayBank3(2, camSpacePos);
			playSound = false;
		}*/

	} else {
		if (shrinkActivated) { UnShrink(); }
	}

	UpdateModelPosAndRotY();
	
	return 1;
}

int GalaxyShrinkingPlatform::Render()
{
	if (skl.x != 0_f) {
		model.Render(&skl);
	}
	frameModel.Render(nullptr);
	return 1;
}
