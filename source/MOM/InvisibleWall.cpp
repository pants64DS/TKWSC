#include "InvisibleWall.h"

using InvisibleWallCLPS = StaticCLPS_Block<{ .tractionID = CLPS::TR_SLIP_NO_WALL_JUMP_ABOVE }>;

SharedFilePtr InvisibleWall::clsnFile;

SpawnInfo InvisibleWall::spawnData =
{
	[] -> ActorBase* { return new InvisibleWall; },
	0x0000,
	0x0100,
	0x00000003,
	0x00064000_f,
	0x000c8000_f,
	0x01000000_f,
	0x01000000_f
};

[[gnu::target("thumb")]]
void InvisibleWall::UpdateModelTransform()
{
	model.mat4x3 = Matrix4x3::RotationY(ang.y);
	model.mat4x3.c3 = pos >> 3;
}


[[gnu::target("thumb")]]
int InvisibleWall::InitResources()
{
	UpdateModelTransform();
	UpdateClsnPosAndRot();
	
	KCL_File& clsnF = clsnFile.LoadKCL();
	clsn.SetFile(&clsnF, clsnNextMat, 1._f, ang.y, InvisibleWallCLPS::instance<>);
	clsn.beforeClsnCallback = &MeshColliderBase::UpdatePosWithTransform;
	
	return 1;
}

int InvisibleWall::CleanupResources()
{
	clsn.Disable();
	clsnFile.Release();
	return 1;
}

int InvisibleWall::Behavior()
{
	IsClsnInRange(0_f, 0_f);
	
	return 1;
}

int InvisibleWall::Render()
{
	return 1;
}
