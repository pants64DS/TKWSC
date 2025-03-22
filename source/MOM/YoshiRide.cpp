#include "YoshiRide.h"

SharedFilePtr YoshiRide::ridingAnim;

asm("UpdateCharVoice = 0x020e6330");
extern "C" void UpdateCharVoice(Player& player);

namespace
{
	enum Animations
	{
		WAIT,
		RIDE,
		RUN,
		
		NUM_ANIMS
	};
	enum States
	{
		ST_WAIT,
		ST_RIDE,
		ST_RUN
	};
	
	struct State
	{
		using FuncPtr = void(YoshiRide::*)();
		FuncPtr init;
		FuncPtr main;
	};
	const State states[]
	{
		State{ &YoshiRide::StWait_Init, &YoshiRide::StWait_Main },
		State{ &YoshiRide::StRide_Init, &YoshiRide::StRide_Main },
		State{ &YoshiRide::StRun_Init,  &YoshiRide::StRun_Main  }
	};
	
	SharedFilePtr* animFiles[] =
	{
		(SharedFilePtr*)0x0210ec30,
		&YoshiRide::ridingAnim,
		(SharedFilePtr*)0x0210ea28
	};
	SharedFilePtr* headAnimFile = (SharedFilePtr*)0x0210eb88;
	
	constexpr Fix12i RADIUS = 0x63000_f;
	constexpr Fix12i HEIGHT = 0x70000_f;
	
	constexpr Fix12i HORZ_SPEED = 0x14000_f;
	constexpr Fix12i VERT_ACCEL = -0x2000_f;
	constexpr Fix12i TERM_VEL = -0x32000_f;
	
	constexpr uint8_t COOLDOWN_TIME = 30;
	constexpr uint16_t RUN_TIME = 180;
	
	//-1: End ride, do not change character. 1: End ride, change character. 0: Do not end ride. 2: End ride, don't run away.
	int ShouldEndRide(Player& player)
	{
		if (player.isMega)
			return 0;
		
		switch ((unsigned)player.currState)
		{
			// All the different states where the player gets hurt
			case 0x02110094:
			case 0x021100ac:
			case 0x021100c4:
			case 0x021100dc:
			case 0x021100f4:
				return 1;
			// sweep
			case 0x0211061c:
				return 2;
		}
		
		return player.param1 == Player::CH_YOSHI ? 0 : -1;
	}
	
	void ChangeCharacter(Player& player, unsigned character)
	{
		player.SetNewHatCharacter(character, 0, true);
		player.param1 = character;
		player.toonStateAndFlag = 0;

		player.bodyModels[player.GetBodyModelID(character, false)]->Copy(
			*player.bodyModels[player.GetBodyModelID(player.prevHatChar, false)],
			*Player::ANIM_PTRS[player.animID + character]->BCA(),
			0
		);

		PLAYER_ARR[0]->realChar = character;
		UpdateCharVoice(*PLAYER_ARR[0]);
	}
};

SpawnInfo YoshiRide::spawnData =
{
	[] -> ActorBase* { return new YoshiRide; },
	0x0171,
	0x00aa,
	0x00000006,
	0x00032000_f,
	0x00046000_f,
	0x01000000_f,
	0x01000000_f
};

void YoshiRide::UpdateModelTransform()
{
	if (riding)
		rigMdl.mat4x3 = PLAYER_ARR[0]->bodyModels[PLAYER_ARR[0]->param1]->mat4x3 * PLAYER_ARR[0]->bodyModels[PLAYER_ARR[0]->param1]->data.transforms[8];
	else
	{
		rigMdl.mat4x3 = Matrix4x3::RotationY(ang.y);
		rigMdl.mat4x3.c3 = pos >> 3;
	}

	headMdl->mat4x3 = rigMdl.mat4x3 * rigMdl.data.transforms[15];
}

[[gnu::target("thumb")]]
int YoshiRide::InitResources()
{
	if(!(headMdl = new ModelAnim))
		return 0;
	
	//The player should load his stuff first, so the SharedFilePtr's should be there before now.
	headAnimFile->LoadBCA();

	for(int i = 0; i < NUM_ANIMS; ++i)
		animFiles[i]->LoadBCA();
	
	rigMdl.SetFile(*PLAYER_ARR[0]->bodyModels[Player::CH_YOSHI]->data.modelFile, 1, -1);
	rigMdl.SetAnim(*animFiles[WAIT]->BCA(), Animation::LOOP, 0x1000_f, 0);
	headMdl->SetFile(*PLAYER_ARR[0]->headModels[Player::CH_YOSHI]->data.modelFile, 1, -1);
	static_cast<ModelAnim*>(headMdl)->SetAnim(*headAnimFile->BCA(), Animation::NO_LOOP, 1._f, 0);
	
	cylClsn.Init(this, RADIUS, HEIGHT, 0x04200004, 0x00000000);
	wmClsn.Init(this, RADIUS, RADIUS, nullptr, nullptr);
	shadow.InitCylinder();
	
	origPos = pos;
	
	UpdateModelTransform();
	
	horzSpeed = 0_f;
	vertAccel = VERT_ACCEL;
	termVel = TERM_VEL;
	state = ST_WAIT; //don't call ChangeState to avoid setting the cooldown timer
	
	return 1;
}

[[gnu::target("thumb")]]
int YoshiRide::CleanupResources()
{
	delete headMdl; //using the virtual destructor to our advantage
	
	headAnimFile->Release();
	for(int i = 0; i < NUM_ANIMS; ++i)
		animFiles[i]->Release();
	return 1;
}

void YoshiRide::ChangeState(uint8_t newState)
{
	state = newState;
	(this->*states[state].init)();
}

[[gnu::target("thumb")]]
void YoshiRide::StartRide(int charID)
{
	//Remember to take care of the player-not-wearing-a-cap case
	riderChar = PLAYER_ARR[0]->param1;

	// This needs to be done before calling SetFile to prevent a memory leak
	rigMdl.~ModelAnim();
	new (&rigMdl) ModelAnim();

	rigMdl.SetFile(*PLAYER_ARR[0]->bodyModels[charID]->data.modelFile, 1, -1);
	rigMdl.SetAnim(*animFiles[RIDE]->BCA(), Animation::LOOP, 0x1000_f, 0);
	
	static_cast<ModelAnim*>(headMdl)->~ModelAnim();
	new(headMdl) Model();
	
	headMdl->SetFile(*PLAYER_ARR[0]->headModels[charID]->data.modelFile, 1, -1);
	
	// if we don't do this, Luigi always has a vanish head
	if (riderChar == Player::CH_LUIGI)
		PLAYER_ARR[0]->texSeq268.Update(headMdl->data);
	
	PLAYER_ARR[0]->pos = pos;
	PLAYER_ARR[0]->ang = PLAYER_ARR[0]->motionAng = ang;
	PLAYER_ARR[0]->horzSpeed = 0_f;
	
	PLAYER_ARR[0]->TurnOffToonShading(PLAYER_ARR[0]->prevHatChar);
	PLAYER_ARR[0]->TurnOffToonShading(PLAYER_ARR[0]->currHatChar);
	PLAYER_ARR[0]->TurnOffToonShading(Player::CH_YOSHI);

	ChangeCharacter(*PLAYER_ARR[0], Player::CH_YOSHI);
	Sound::PlayCharVoice(3, 16, camSpacePos);

	riding = true;
}

[[gnu::target("thumb")]]
void YoshiRide::EndRide(bool changeChar)
{
	const Vector3 savedPos = rigMdl.mat4x3.c3 << 3;

	// This needs to be done before calling SetFile to prevent a memory leak
	rigMdl.~ModelAnim();
	new (&rigMdl) ModelAnim();

	rigMdl.SetFile(*PLAYER_ARR[0]->bodyModels[Player::CH_YOSHI]->data.modelFile, 1, -1);
	rigMdl.SetAnim(*animFiles[WAIT]->BCA(), Animation::LOOP, 0x1000_f, 0);
	headMdl->~Model();
	new(headMdl) ModelAnim();
	headMdl->SetFile(*PLAYER_ARR[0]->headModels[Player::CH_YOSHI]->data.modelFile, 1, -1);
	static_cast<ModelAnim*>(headMdl)->SetAnim(*headAnimFile->BCA(), Animation::NO_LOOP, 0x1000_f, 0);
	pos = PLAYER_ARR[0]->pos;
	ang.y = PLAYER_ARR[0]->ang.y;
	PLAYER_ARR[0]->pos = savedPos;
	
	if(changeChar)
		ChangeCharacter(*PLAYER_ARR[0], riderChar);
	
	riding = false;
}

[[gnu::target("thumb")]]
void YoshiRide::StWait_Init()
{
	rigMdl.SetAnim(*animFiles[WAIT]->BCA(), Animation::LOOP, 0x1000_f, 0);
	cooldown = COOLDOWN_TIME;
	horzSpeed = 0_f;
}

[[gnu::target("thumb")]]
void YoshiRide::StWait_Main()
{
	UpdatePos(nullptr);
	Actor* actor = Actor::FindWithID(cylClsn.otherObjID);
	if(!CountDownToZero(cooldown) && actor && actor->actorID == 0x00bf)
	{
		Player& player = *(Player*)actor;
		if (player.param1 != 3 &&
			!player.isMetalWario &&
			!player.isVanishLuigi &&
			JumpedOnByPlayer(cylClsn, player))
		{
			ChangeState(ST_RIDE);
		}
	}
	
	MakeVanishLuigiWork(cylClsn);
	cylClsn.Clear();
	if(state != ST_RIDE)
	{
		cylClsn.Update();
		wmClsn.UpdateDiscreteNoLava();
	}
}

[[gnu::target("thumb")]]
void YoshiRide::StRide_Init()
{
	StartRide(PLAYER_ARR[0]->param1);
}

[[gnu::target("thumb")]]
void YoshiRide::StRide_Main()
{
	pos = PLAYER_ARR[0]->pos;
	int newArea = 8;
	for(int i = 0; i < 8; ++i)
		if(AREAS[i].showing)
		{
			if(newArea == 8)
				newArea = i;
			else
				newArea = -1;
		}
	if(newArea != 8)
		areaID = newArea;
	
	int endRideState = ShouldEndRide(*PLAYER_ARR[0]);
	if (endRideState != 0)
	{
		EndRide(endRideState != -1);

		// Play the right char voice
		if (PLAYER_ARR[0]->currState == &Player::ST_SWEEP_KICK)
			Sound::PlayCharVoice(PLAYER_ARR[0]->realChar, 10, PLAYER_ARR[0]->camSpacePos);

		else if (PLAYER_ARR[0]->currState == &Player::ST_BURN_FIRE ||
		         PLAYER_ARR[0]->currState == &Player::ST_BURN_LAVA)
			Sound::PlayCharVoice(PLAYER_ARR[0]->realChar, 35, PLAYER_ARR[0]->camSpacePos);

		if (endRideState == 2)
			Sound::PlayCharVoice(PLAYER_ARR[0]->realChar, 10, PLAYER_ARR[0]->camSpacePos);

		ChangeState(endRideState == 1 ? ST_RUN : ST_WAIT);
	}
}

[[gnu::target("thumb")]]
void YoshiRide::StRun_Init()
{
	rigMdl.SetAnim(*animFiles[RUN]->BCA(), Animation::LOOP, 0x1900_f, 0);
	cooldown = COOLDOWN_TIME;
	runTimer = RUN_TIME;
	motionAng.y = ang.y = PLAYER_ARR[0]->ang.y;
}

[[gnu::target("thumb")]]
void YoshiRide::StRun_Main()
{
	horzSpeed = HORZ_SPEED;
	StWait_Main();
	
	if(state == ST_RUN && !CountDownToZero(runTimer))
	{
		DisappearPoofDustAt(Vector3{pos.x, pos.y + 0x50000_f, pos.z});
		pos = origPos;
		PoofDustAt(Vector3{pos.x, pos.y + 0x50000_f, pos.z});
		ChangeState(ST_WAIT);
	}
}

int YoshiRide::Behavior()
{
	rigMdl.Advance();
	
	(this->*states[state].main)();

	if (!riding)
	{
		shadowMat = rigMdl.mat4x3;
		shadowMat.c3.y += 0x14000_f >> 3;
		DropShadowRadHeight(shadow, shadowMat, RADIUS, 0x37000_f, 0xf); //radius and height are (C) Yoshi the Player.
	}
	
	return 1;
}

int YoshiRide::Render()
{
	UpdateModelTransform();
	
	if (!riding)
	{
		rigMdl.data.materials[0].paletteInfo =
		headMdl->data.materials[0].paletteInfo =
		headMdl->data.materials[1].paletteInfo = PLAYER_ARR[0]->bodyModels[Player::CH_YOSHI]->data.materials[0].paletteInfo;
	}
	
	rigMdl.Render();
	headMdl->Render();
	return 1;
}
