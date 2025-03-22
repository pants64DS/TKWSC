#include "Noteblock.h"

using NoteblockCLPS = StaticCLPS_Block<{
	.camBehavID = CLPS::CA_NORMAL,
	.camGoesThru = true
}>;

SharedFilePtr Noteblock::modelFile;
SharedFilePtr Noteblock::clsnFile;

SpawnInfo Noteblock::spawnData =
{
	[] -> ActorBase* { return new Noteblock; },
	0x0034,
	0x0100,
	0x00000002,
	0x00000000_f,
	0x005dc000_f,
	0x01000000_f,
	0x01000000_f
};

void Noteblock::OnFloorAfterClsn(MeshColliderBase& clsn, Actor* clsnActor, Actor* otherActor)
{
	static_cast<Noteblock*>(clsnActor)->stage = 1;
	static_cast<Noteblock*>(clsnActor)->oldPos = static_cast<Noteblock*>(clsnActor)->pos;
	static_cast<Noteblock*>(clsnActor)->isLaunching = true;
}

void Noteblock::UpdateModelTransform()
{
	model.mat4x3 = Matrix4x3::RotationY(ang.y);
	model.mat4x3.c3 = pos >> 3;

	DropShadowScaleXYZ(shadow, model.mat4x3, 0x60000_f, 0x150000_f, 0x60000_f, 0xc);
}

//Jiggles block down then up.
void Noteblock::Jiggle()
{
	switch (stage)
	{
		case 1:
			pos.y = pos.y - 0x9600_f;

			if (pos.y < oldPos.y - 0x20202_f)
				stage = 2;

			break;

		case 2:
			pos.y = pos.y + 0x6400_f;

			if (pos.y > oldPos.y)
				stage = 3;

			break;

		case 3:
			pos.y = oldPos.y;
			stage = 0;
			break;
	}
}

//Launches player.
void Noteblock::Launch()
{
	PLAYER_ARR[0]->ChangeState(Player::ST_FALL);

	if (boost)
	{
		PLAYER_ARR[0]->speed.y = 0x1000_f * param1;
		boost = false;
	}
	else
		PLAYER_ARR[0]->speed.y = 0x37000_f;

	isLaunching = false;
}

int Noteblock::InitResources()
{
	BMD_File& modelF = modelFile.LoadBMD();
	model.SetFile(modelF, 1, -1);

	KCL_File& clsnF = clsnFile.LoadKCL();
	clsn.SetFile(&clsnF, clsnNextMat, 1._f, ang.y, NoteblockCLPS::instance<>);
	
	clsn.beforeClsnCallback = &MeshColliderBase::UpdatePosWithTransform;
	clsn.afterClsnCallback = &OnFloorAfterClsn;
	
	shadow.InitCuboid();
	
	UpdateModelTransform();
	UpdateClsnPosAndRot();
	
	shadowMat = model.mat4x3 * Matrix4x3::IDENTITY;
	shadowMat.c3.y -= 0x14000_f >> 3;

	return 1;
}

int Noteblock::CleanupResources()
{
	clsn.Disable();
	modelFile.Release();
	clsnFile.Release();

	return 1;
}

int Noteblock::Behavior()
{
	UpdateModelTransform();

	if(IsClsnInRange(0_f, 0_f))
	{
		if (stage > 0 && stage < 4)
			Jiggle();

		if (isLaunching)
		{
			Launch();
			Sound::Play(4, 2, camSpacePos);
		}

		if (PLAYER_ARR[0]->pos.y >= pos.y && INPUT_ARR[0].buttonsPressed & Input::B && PLAYER_ARR[0]->currState == &Player::ST_FALL && PLAYER_ARR[0]->pos.y <= pos.y + 0x3E8000_f && PLAYER_ARR[0]->speed.y < 0_f)
			boost = true;

		UpdateClsnPosAndRot();
	}
	
	return 1;
}

bool Noteblock::BeforeBehavior()
{
	IMMUNE_TO_DAMAGE = PLAYER_ARR[0]->pos.y >= pos.y && IsClsnInRange(0_f, 0_f);

	return Actor::BeforeBehavior();
}

int Noteblock::Render()
{
	model.Render(nullptr);

	return 1;
}

Noteblock::~Noteblock()
{
	IMMUNE_TO_DAMAGE = false;
}
