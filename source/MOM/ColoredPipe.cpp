#include "ColoredPipe.h"

using ColoredPipeCLPS = StaticCLPS_Block<{ .camBehavID = CLPS::CA_GO_BEHIND }>;

SharedFilePtr ColoredPipe::modelFile;
SharedFilePtr ColoredPipe::clsnFile;

SpawnInfo ColoredPipe::spawnData =
{
	[] -> ActorBase* { return new ColoredPipe; },
	0x0030,
	0x0100,
	0x00000002,
	0x00000000_f,
	0x005dc000_f,
	0x01000000_f,
	0x01000000_f
};

void ColoredPipe::UpdateModelTransform()
{
	model.mat4x3 = Matrix4x3::RotationY(ang.y);
	model.mat4x3.c3 = pos >> 3;
	
	//Pipe rotation for parameters 2 & 3
	model.mat4x3.RotateX(ang.x);
	model.mat4x3.RotateZ(ang.z);
}

[[gnu::target("thumb")]]
int ColoredPipe::InitResources()
{
	
	BMD_File& modelF = modelFile.LoadBMD();
	model.SetFile(modelF, 1, -1);

	model.data.materials[0].difAmb = 
		model.data.materials[1].difAmb = (param1 & 0x7fff) << 16 | 0x8000;

	KCL_File& clsnF = clsnFile.LoadKCL();
	clsn.SetFile(&clsnF, clsnNextMat, 0x0163_f, ang.y, ColoredPipeCLPS::instance<>); //0x0175_f
	
	clsn.beforeClsnCallback = (decltype(clsn.beforeClsnCallback))0x02039348;

	UpdateModelTransform();
	UpdateClsnPosAndRot();
	
	return 1;
}

int ColoredPipe::CleanupResources()
{
	clsn.Disable();
	modelFile.Release();
	clsnFile.Release();
	return 1;
}

int ColoredPipe::Behavior()
{
	UpdateModelTransform();
	if(IsClsnInRange(0_f, 0_f))
	{
		Player* player = ClosestPlayer();
		
		if(!player)
			return 1;
		
		//If the player jumped into the pipe
		/*if (player->pos.x < pos.x + 0x50000_f && player->pos.x > pos.x - 0x50000_f && player->pos.z < pos.z + 0x50000_f && player->pos.z > pos.z - 0x50000_f && player->pos.y < pos.y + 0x50000_f && player->pos.y > pos.y + 0x30000_f)
		{
			Sound::Play(2, 71, camSpacePos);
		}*/
		
		UpdateClsnPosAndRot();
	}
	
	return 1;
}

int ColoredPipe::Render()
{
	model.Render();
	return 1;
}
