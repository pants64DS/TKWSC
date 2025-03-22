#include "TreeShadow.h"

SpawnInfo TreeShadow::spawnData =
{
	[] -> ActorBase* { return new TreeShadow; },
	0x0034,
	0x0100,
	0x00000002,
	0x00000000_f,
	0x005dc000_f,
	0x01000000_f,
	0x01000000_f
};

void TreeShadow::DropShadow()
{
	shadowMat = Matrix4x3::RotationY(ang.y);
	shadowMat.c3 = pos >> 3;

	DropShadowRadHeight(shadow, shadowMat, 0x150000_f, 0x137000_f, 0xc);
}

[[gnu::target("thumb")]]
int TreeShadow::InitResources()
{
	opacity = param1 & 0xf;
	
	shadow.InitCylinder();
	
	DropShadow();
	
	shadowMat = Matrix4x3::IDENTITY;
	shadowMat.c3.y -= 0x14000_f >> 3;
	
	return 1;
}

int TreeShadow::CleanupResources()
{
	return 1;
}

int TreeShadow::Behavior()
{
	DropShadow();
	return 1;
}
