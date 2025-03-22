#include "SaveBlock.h"

using SaveBlockCLPS = StaticCLPS_Block<{
	.camBehavID = CLPS::CA_NORMAL,
	.camGoesThru = true
}>;

SharedFilePtr SaveBlock::modelFile;
SharedFilePtr SaveBlock::texSeqFile;
SharedFilePtr SaveBlock::clsnFile;

SpawnInfo SaveBlock::spawnData =
{
	[] -> ActorBase* { return new SaveBlock; },
	0x0034,
	0x0100,
	0x00000002,
	0x00000000_f,
	0x005dc000_f,
	0x01000000_f,
	0x01000000_f
};

/*
asm("SaveGame = 0x02013b9c");
extern "C" bool SaveGame();
*/

asm("ShowMessage2 = 0x0201eb94");
extern "C" void ShowMessage2(short msgID);

void SaveBlock::JumpedUnderBlock()
{
	if (saveable)
	{
		Sound::Play(2, 94, camSpacePos);
		// SaveGame();
		ShowMessage2(0x295);
		Particle::System::NewSimple(0x09, pos.x, pos.y, pos.z);
		Particle::System::NewSimple(0x09, pos.x, pos.y, pos.z);
		stage = 1;
	}
	return;
}

//Jiggles block up then down.
void SaveBlock::Jiggle()
{
	switch (stage)
	{
		case 1:
			pos.y = pos.y + 0x9600_f;
			if (pos.y > oldPos.y + 0x20202_f) {
				stage = 2;
			}
			saveable = false;
			break;
		
		case 2:
			pos.y = pos.y - 0x6400_f;
			if (pos.y < oldPos.y) {
				stage = 3;
			}
			break;
		
		case 3:
			pos.y = oldPos.y;
			stage = 0;
			saveable = true;
			break;
	}
}

void SaveBlock::UpdateModelTransform()
{
	model.mat4x3 = Matrix4x3::RotationY(ang.y);
	model.mat4x3.c3 = pos >> 3;

	DropShadowScaleXYZ(shadow, model.mat4x3, 0x85000_f, 0x150000_f, 0x85000_f, 0xc);
}

int SaveBlock::InitResources()
{
	BMD_File& modelF = modelFile.LoadBMD();
	model.SetFile(modelF, 1, -1);
	
	BTP_File& texSeqF = texSeqFile.LoadBTP();
	modelF.PrepareAnim(texSeqF);
	texSeq.SetFile(texSeqF, Animation::LOOP, 0x10000_f, 1);
	
	KCL_File& clsnF = clsnFile.LoadKCL();
	clsn.SetFile(&clsnF, clsnNextMat, 0x190_f, ang.y, SaveBlockCLPS::instance<>);
	
	shadow.InitCuboid();
	
	UpdateModelTransform();
	UpdateClsnPosAndRot();
	
	shadowMat = model.mat4x3 * Matrix4x3::IDENTITY;
	shadowMat.c3.y -= 0x14000_f >> 3;
	
	saveable = true;
	stage = 0;
	oldPos = pos;
	
	return 1;
}

int SaveBlock::CleanupResources()
{
	clsn.Disable();
	clsnFile.Release();
	
	modelFile.Release();
	texSeqFile.Release();
	
	return 1;
}

int SaveBlock::Behavior()
{
	UpdateModelTransform();
	
	if(IsClsnInRange(0_f, 0_f))
	{
		Player* player = ClosestPlayer();
		
		if (BumpedUnderneathByPlayer(*player) && player->pos.x < pos.x + 0x53500_f && player->pos.x > pos.x - 0x53500_f && player->pos.z < pos.z + 0x53500_f && player->pos.z > pos.z - 0x53500_f)
		{
			JumpedUnderBlock();
		}
		
		if (stage > 0 && stage < 4) {
			Jiggle();
		}
		
		UpdateClsnPosAndRot();
	}
	
	return 1;
}

int SaveBlock::Render()
{
	model.Render();
	texSeq.Update(model.data);
	texSeq.Advance();
	return 1;
}
