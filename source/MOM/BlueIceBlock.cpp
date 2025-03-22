#include "BlueIceBlock.h"

using IceBlockCLPS = StaticCLPS_Block<
{
	.textureID = CLPS::TX_ICE,
	.tractionID = CLPS::TR_SLIPPERY,
	.camBehavID = CLPS::CA_GO_BEHIND_8,
	.behaviorID = CLPS::BH_NO_GETTING_STUCK,
	.camGoesThru = true,
}>;

SharedFilePtr BlueIceBlock::modelFile;
SharedFilePtr BlueIceBlock::clsnFile;

SpawnInfo BlueIceBlock::spawnData =
{
	[] -> ActorBase* { return new BlueIceBlock; },
	0x0034,
	0x0100,
	0x00000002,
	0x00000000_f,
	0x005dc000_f,
	0x01000000_f,
	0x01000000_f
};

void BlueIceBlock::UpdateModelTransform()
{
	model.mat4x3 = Matrix4x3::RotationY(ang.y);
	model.mat4x3.c3 = pos >> 3;
}

int BlueIceBlock::InitResources()
{
	BMD_File& modelF = modelFile.LoadBMD();
	model.SetFile(modelF, 1, -1);

	KCL_File& clsnF = clsnFile.LoadKCL();
	clsn.SetFile(&clsnF, clsnNextMat, 1._f, ang.y, IceBlockCLPS::instance<>);
	
	UpdateModelTransform();
	UpdateClsnPosAndRot();

	return 1;
}

int BlueIceBlock::CleanupResources()
{
	clsn.Disable();
	clsnFile.Release();
	modelFile.Release();
	return 1;
}

int BlueIceBlock::Behavior()
{
	UpdateModelTransform();
	
	if (IsClsnInRange(0_f, 0_f))
		UpdateClsnPosAndRot();
	
	return 1;
}

int BlueIceBlock::Render()
{
	model.Render();
	return 1;
}
