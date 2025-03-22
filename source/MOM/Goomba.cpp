#include "Goomba.h"

namespace
{
	using StateFuncPtr = void(Goomba::*)();

	const char materialName[] = "kuribo_all";
	u8 aliveMatVals[] = {0x1f, 0x10, 0x00, 0x00};
	BMA_File::MaterialProperties aliveMatProps =
	{
		0xffff, 0,
		&materialName[0],
		0x01, 0x00, 0x0000,    0x01, 0x00, 0x0000,    0x01, 0x00, 0x0000, 
		0x01, 0x00, 0x0001,    0x01, 0x00, 0x0001,    0x01, 0x00, 0x0001,  
		0x01, 0x00, 0x0002,    0x01, 0x00, 0x0002,    0x01, 0x00, 0x0002, 
		0x01, 0x00, 0x0002,    0x01, 0x00, 0x0002,    0x01, 0x00, 0x0002, 
		0x01, 0x00, 0x0000, 
	};
	BMA_File aliveMat = {0x0002, 0x0000, &aliveMatVals[0], 1, &aliveMatProps};

	u8 regurgMatVals[] = {0x00, 0x03, 0x0c, 0x16, 0x1f, 0x1f, 0x1f, 0x1f,
							0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
							0x1f, 0x10, 0x00, 0x00};
	BMA_File::MaterialProperties regurgMatProps =
	{
		0xffff, 0,
		&materialName[0],
		0x01, 0x01, 0x0000,    0x01, 0x00, 0x0004,    0x01, 0x00, 0x0004,
		0x01, 0x00, 0x0011,    0x01, 0x00, 0x0011,    0x01, 0x00, 0x0011, 
		0x01, 0x00, 0x0000,    0x01, 0x00, 0x0000,    0x01, 0x00, 0x0000,
		0x01, 0x00, 0x0000,    0x01, 0x00, 0x0000,    0x01, 0x00, 0x0000,
		0x01, 0x00, 0x0004, 
	};
	BMA_File regurgMat = {0x0011, 0x0000, &regurgMatVals[0], 1, &regurgMatProps};

	constexpr StateFuncPtr stateFuncs[] =
	{
		&Goomba::State0,
		&Goomba::State1,
		&Goomba::State2,
		&Goomba::State3,
		&Goomba::State4,
		&Goomba::State5,
		&Goomba::State6,
	};

	constexpr Fix12i SCALES[]          = { 0x00000555_f,  0x00001000_f,  0x00002555_f,  0x00001000_f}; // 0x02130258
	constexpr Fix12i HORZ_CLSN_SIZES[] = { 0x00028000_f,  0x00064000_f,  0x000e0000_f,  0x00064000_f}; // 0x02130208
	constexpr Fix12i VERT_ACCELS[]     = {-0x00000aaa_f, -0x00002000_f, -0x00005aaa_f, -0x00002000_f}; // 0x02130238
	constexpr Fix12i WALK_SPEEDS[]     = { 0x00000aaa_f,  0x00002000_f,  0x00004aaa_f,  0x00006000_f}; // 0x02130228
	constexpr Fix12i RUN_SPEEDS[]      = { 0x0000a800_f,  0x00015000_f,  0x00015000_f,  0x00015000_f}; // 0x02130268
	constexpr Fix12i JUMP_SPEEDS[]     = { 0x00008000_f,  0x00010000_f,  0x00020000_f,  0x00010000_f}; // 0x02130248
	constexpr unsigned DAMAGES[] = { 0, 1, 2, 1 };	// 0x02130204
	constexpr unsigned DYING_SOUND_IDS[] = {0x00000110, 0x000000d6, 0x00000111};
	
	constexpr Vector3 CAP_OFFSET = {0x00000_f, 0x6c000_f, 0x00000_f};
	
	constexpr Fix12i JUMP_HORZ_SPEED =  0x00010000_f;
	constexpr Fix12i JUMP_VERT_ACCEL = -0x00002000_f;
	constexpr Fix12i JUMP_INIT_VEL   =  0x0001c48c_f;
	constexpr Fix12i JUMP_DIST       =  0x00190000_f;
	constexpr short SPIN_SPEED	     =  0x2000;
	constexpr Fix12i SPIN_TERM_VEL   = -0x00009000_f;
	
	constexpr short SMALL_ID  = 520;
	constexpr short NORMAL_ID = 521;
	constexpr short LARGE_ID  = 522;
	constexpr short SMALL_ID_2  = 533;
	constexpr short NORMAL_ID_2 = 534;
	constexpr short LARGE_ID_2  = 535;
	constexpr short ID_DIFFERENCE  = NORMAL_ID_2 - NORMAL_ID;
}

SharedFilePtr Goomba::modelFile;
SharedFilePtr Goomba::texSeqFile;
SharedFilePtr Goomba::animFiles[NUM_ANIMS];

SpawnInfo Goomba::spawnData =
{
	[] -> ActorBase* { return new Goomba; },
	0x00c8,
	0x0018,
	0x10000006,
	0x00032000_f,
	0x00046000_f,
	0x01000000_f,
	0x01000000_f
};

SpawnInfo Goomba::spawnDataSmall =
{
	[] -> ActorBase* { return new Goomba; },
	0x00c9,
	0x0019,
	0x10000006,
	0x00032000_f,
	0x00046000_f,
	0x006a4000_f,
	0x00800000_f
};

SpawnInfo Goomba::spawnDataBig =
{
	[] -> ActorBase* { return new Goomba; },
	0x00ca,
	0x001a,
	0x10000006,
	0x00064000_f,
	0x000c8000_f,
	0x01000000_f,
	0x01000000_f
};

[[gnu::target("thumb")]]
int Goomba::InitResources()
{
	if (actorID >= SMALL_ID_2 && actorID <= LARGE_ID_2)
		actorID -= ID_DIFFERENCE;
	
	spawnStar = param1 >> 4 & 0xf;
	starID = 0xff;
	
	spawnCapFlag = param1 >> 8 & 0xf;
	starID_VS = 0; // param1 >> 0xc & 0xf; // this is used for the color now
	color = param1 >> 12 & 0xf;

	extraDamage = color == 2 ? 1 : (color == 5 ? 2 : 0);
	extraSpeed = color == 3 ? 5 : 1;
	
	if (spawnStar == 1)
	{
		starID = TrackStar(0xf, 1);
		LoadSilverStarAndNumber();
	}
		
	BMD_File& modelF = modelFile.LoadBMD();

	for(int i = 0; i < NUM_ANIMS; ++i)
		animFiles[i].LoadBCA();
		
	AddCap((uint8_t)(param1 & 0xf));
	if (capID < 6)
		param1 &= 0xf0ff; // invalid cap, so never spawn cap
	
	if (!DestroyIfCapNotNeeded())
		return 0;
	
	if (!modelAnim.SetFile(modelF, 1, -1))
		return 0;
	
	if (!shadow.InitCylinder())
		return 0;
	
	modelF.PrepareAnim(aliveMat);
	materialChg.SetFile(aliveMat, Animation::NO_LOOP, 1._f, 0);
	
	BTP_File& texSeqF = texSeqFile.LoadBTP();
	modelF.PrepareAnim(texSeqF);
	texSeq.SetFile(texSeqF, Animation::NO_LOOP, 1._f, color < 6 ? color + 1 : 0);
	
	coinType = Enemy::CN_YELLOW;

	if (actorID == SMALL_ID)
	{
		type = Type::SMALL;
	}
	else if (actorID == NORMAL_ID)
	{
		type = Type::NORMAL;
	}
	else
	{
		type = Type::BIG;
		LoadBlueCoinModel();
	}
	
	scale.x = SCALES[type];
	scale.y = SCALES[type];
	scale.z = SCALES[type];
	
	cylClsn.Init(this, scale.x * 0x3c, HORZ_CLSN_SIZES[type], 0x00200000, 0x00a6efe0);
	if (type == Type::BIG)
		cylClsn.vulnerableFlags &= ~CylinderClsn::HIT_BY_YOSHI_TONGUE;
		
	wmClsn.Init(this, scale.x * 0x3c, scale.x * 0x3c, nullptr, nullptr);
	wmClsn.StartDetectingWater();
	
	flags468 = state = 0;
	defeatMethod = Enemy::DF_NOT;
	distToPlayer = 0x7fffffff_f;
	
	targetDir = motionAng.y;
	targetSpeed = WALK_SPEEDS[type];
	movementTimer = changeDirTimer = stuckInSpotTimer = 0;
	backupPos = pos;
	
	UpdateMaxDist();
	
	originalPos = pos;
	
	vertAccel = VERT_ACCELS[type];
	termVel = -0x32000_f;
	
	modelAnim.SetAnim(*animFiles[WALK].BCA(), Animation::LOOP, 1._f, 0);
	
	recoverFlags = 0;

	if (LEVEL_ID == 40 && areaID == 0)
	{
		drawDistAsr3 <<= 1;
		flags |= Actor::NO_BEHAVIOR_IF_OFF_SCREEN;
	}
	
	return 1;
}

int Goomba::CleanupResources()
{
	if (type == Type::BIG)
		UnloadBlueCoinModel();
	
	modelFile.Release();
	for(int i = 0; i < NUM_ANIMS; ++i)
		animFiles[i].Release();
	
	if (spawnStar == 1)
		UnloadSilverStarAndNumber();
	
	UnloadCapModel();
	texSeqFile.Release();
	
	return 1;
}

int Goomba::Behavior()
{
	if (state != 4)
		vertAccel = VERT_ACCELS[type];
	if (state != 5)
		termVel = -50._f;

	TrackVsStarIfNecessary();
	UpdateMaxDist();
	
	int capState = GetCapState();
	if (capState == 0)
		return 1;
	
	if (capState == 1)
	{
		flags |= Actor::AIMABLE_BY_EGG;
		PoofDust();
	}
	
	if (LEVEL_ID != 14 && state != 3 && !isBeingSpit
		&& defeatMethod == Enemy::DF_NOT && IsTooFarAwayFromPlayer(0x5dc000_f) != 0)
	{
		Unk_02005d94();
		return 1;
	}
	
	if (defeatMethod != Enemy::DF_NOT)
	{
		int res = UpdateKillByInvincibleChar(wmClsn, modelAnim, 3);
		if (res == 0) // not killed by mega char
		{
			if (!UpdateIfDying())
				UpdateModelTransform();
		}
		else if (res == 2)  // killed by mega char
		{
			Kill();
			
			ReleaseCap(CAP_OFFSET);
			pos = originalPos;
			ang.x = ang.y = ang.z = 0;
			
			RespawnIfHasCap();
			SpawnStarIfNecessary();
		}
		return 1;
	}
	
	if (UpdateIfEaten())
		return 1;
	
	if (state < 3)
	{
		modelAnim.speed = horzSpeed * extraSpeed / (2 * scale.x);
		if (modelAnim.speed > 3._f)
			modelAnim.speed = 3._f;
	}
	else
		modelAnim.speed = 1._f;
	
	MakeVanishLuigiWork(cylClsn);
	
	if (state != 2)
	{
		PlayMovingSound();
		modelAnim.Advance();
	}
	
	unsigned prevState = state;
	(this->*stateFuncs[state])();
	
	++stateTimer;
	if (prevState != state)
		stateTimer = 0;
	
	GetHurtOrHurtPlayer();
	
	if (capID < 6)
		UpdatePos(nullptr);
	else
		UpdatePos(&cylClsn);
	
	if (defeatMethod == Enemy::DF_NOT && state != 2 && state != 3)
	{
		if (!IsGoingOffCliff(wmClsn, 0x32000_f, 0x1f49, 0, 1, 0x32000_f))
			noCliffPos = pos;
		else
			pos = noCliffPos;
	}
	
	// in BOB and TTC
	UpdateWMClsn(wmClsn, type == Type::SMALL ? 0 : ((LEVEL_ID == 6 || LEVEL_ID == 27) && targetSpeed == WALK_SPEEDS[type] && defeatMethod != 7) ? 3 : 2);
	// UpdateWMClsn(wmClsn, 0);
	
	KillIfTouchedBadSurface();
	
	cylClsn.Clear();
	if (defeatMethod == Enemy::DF_NOT)
		cylClsn.Update();
	
	UpdateModelTransform();
	KillIfIntoxicated();
	
	if (state == 0 && (flags & OFF_SCREEN) == 0)
	{
		if (pos.Dist(backupPos) < 0xa000_f)
		{
			++stuckInSpotTimer;
			if (capID < 6 && stuckInSpotTimer == 0x1e)
			{
				Jump();
				noChargeTimer = 0x5a;
			}
			if (0x12b < stuckInSpotTimer && noChargeTimer == 0)
			{
				SpawnStarIfNecessary();
				SpawnCoin();
				Kill();
				ReleaseCap(CAP_OFFSET);
				pos = originalPos;
				RespawnIfHasCap();
			}
		}
		else
		{
			stuckInSpotTimer = 0;
			backupPos = pos;
		}
	}
	else if (noChargeTimer == 0)
		stuckInSpotTimer = 0;
	
	return 1;
}

int Goomba::Render()
{
	if ((flags & Actor::IN_YOSHI_MOUTH) || hasNotSpawned)
		return 1;

	if (LEVEL_ID == 40 && RUNNING_KUPPA_SCRIPT && KS_FRAME_COUNTER < 10)
		return 1;
	
	Vector3 oldScale = scale;
	
	if (defeatMethod == Enemy::DF_SQUASHED)
	{
		scale.x = SCALES[type] * scale.x;
		scale.y = SCALES[type] * scale.y;
		scale.z = SCALES[type] * scale.z;
	}
	
	modelAnim.Render(&scale);
	
	scale = oldScale;
	
	materialChg.Update(modelAnim.data);
	texSeq.Update(modelAnim.data);
	RenderCapModel(nullptr);
	
	return 1;
}

void Goomba::OnPendingDestroy()
{
	return;
}

unsigned Goomba::OnYoshiTryEat()
{
	switch (actorID)
	{
		case NORMAL_ID:
			return Actor::YE_KEEP_AND_CAN_MAKE_EGG;
		case SMALL_ID:
			return Actor::YE_SWALLOW;
		default:
			return Actor::YE_DONT_EAT;
	}
}

[[gnu::target("thumb")]]
void Goomba::OnTurnIntoEgg(Player& player)
{
	if ((capID & 0xf) < 6 || spawnStar == 2)
	{
		pos = originalPos;
		RespawnIfHasCap();
	}
	
	unsigned res = OnYoshiTryEat();
	if (res == 6)
	{
		if (!player.IsCollectingCap())
		{
			bool starUntracked = false;
			if (spawnStar == 1)
			{
				UntrackStar(starID);
				starUntracked = true;
				Actor::Spawn(0xb4, 0x50, originalPos, nullptr, areaID, -1);
				param1 &= 0xff0f;
			}
			/*else if (spawnStar == 2 && starID_VS == VS_STAR_SPAWN_ORDER[NUM_VS_STARS_COLLECTED])
			{
				UntrackStar(starID);
				spawnStar = 3;
				starUntracked = true;
			}*/
			player.RegisterEggCoinCount(coinType == 1, starUntracked, false);
		}
		else
		{
			if (coinType == 1)
				GivePlayerCoins(player, 1, 0);
			
			SpawnStarIfNecessary();
		}
	}
	else if (res == 4 && coinType == 1)
	{
		GivePlayerCoins(player, 1, 0);
	}
	
	Kill();
	return;
}

Fix12i Goomba::OnAimedAtWithEgg()
{
	switch (actorID)
	{
		case NORMAL_ID:
			return 0x41000_f;
		case SMALL_ID:
			return 0x14000_f;
		default:
			return 0x96000_f;
	}
}

void Goomba::UpdateMaxDist()
{
	if (noChargeTimer != 0)
	{
		if (state == 1)
			noChargeTimer = 0;
		else
			--noChargeTimer;
	}
	else if (capID >= 6 && stuckInSpotTimer > 10)
	{
		maxDist = 0x1f4000_f - ((stuckInSpotTimer - 10) * 0x14000_f);
		if (maxDist < 0xa000_f)
			maxDist = 0xa000_f;
	}
	else
		maxDist = 0x1f4000_f;
}

void Goomba::TrackVsStarIfNecessary()
{
	/*if (spawnStar == 2 && starID < 0 && starID_VS == VS_STAR_SPAWN_ORDER[NUM_VS_STARS_COLLECTED])
		starID = TrackStar(starID_VS, 1);*/
}

void Goomba::Kill()
{
	if ((capID & 0xf) >= 6)
		KillAndTrackInDeathTable();
	else
		MarkForDestruction();
}

[[gnu::target("thumb")]]
void Goomba::SpawnStarIfNecessary()
{
	if (spawnStar == 1)
	{
		UntrackStar(starID);
		Actor* starMarker = Actor::Spawn(0xb4, 0x50, originalPos, nullptr, areaID, -1);
		Actor* silverStar = Actor::Spawn(0xb3, 0x10, pos,         nullptr, areaID, -1);
		
		if (starMarker && silverStar)
		{
			*(int*)((char*)silverStar + 0x434) = starMarker->uniqueID;	// make this use the actual member of the Star Marker when it's decompiled
			LinkSilverStarAndStarMarker(starMarker, silverStar);
			SpawnSoundObj(1);
		}
		
		param1 &= 0xff0f; // rid the silver star part
	}
	else if (spawnStar == 2)
	{
		/*if (starID_VS == VS_STAR_SPAWN_ORDER[NUM_VS_STARS_COLLECTED])
		{
			UntrackStar(starID);
			Actor::Spawn(0xb4, starID_VS | 0x30, pos, nullptr, areaID, -1);
			Actor::Spawn(0xb3, starID_VS | 0x30, pos, nullptr, areaID, -1);
			spawnStar = 3;
			param1 &= 0xff0f;
			SpawnSoundObj(1);
		}*/
	}
}

/*
void Goomba::KillAndSpawnCap()
{
	SpawnCoin();
	Kill();
	
	ReleaseCap(CAP_OFFSET);
	pos = originalPos;
	RespawnIfHasCap();
}
void Goomba::KillAndSpawnSilverStar()
{
	SpawnStarIfNecessary();
	KillAndSpawnCap();
}
*/

static void PlaySound(unsigned soundID, const Goomba& goomba)
{
	if (!CAMERA || CAMERA->flags & 1)
		return;

	if (goomba.areaID != 2 || LEVEL_ID != 40 || CAMERA->pos.z < -3900._f)
		Sound::Play(3, soundID, goomba.camSpacePos);
}

bool Goomba::UpdateIfDying()
{
	bool dying = UpdateDeath(wmClsn);
	if (defeatMethod - 2 <= 4)
	{
		modelAnim.speed = 1._f;
		modelAnim.Advance();
	}
	
	if (dying)
	{
		PlaySound(DYING_SOUND_IDS[type], *this);
		SpawnStarIfNecessary();

		if (capID < 6 || spawnStar == 2)
		{
			pos = originalPos;
			RespawnIfHasCap();
			if (capID < 6)
				UntrackInDeathTable();
		}
	}
	
	return dying;
}

void Goomba::RenderRegurgGoombaHelpless(Player* player)
{
	regurgTimer = 0x3c;
	speed.y = (0xd000_f - vertAccel) / floorNormal.y;
	horzSpeed = -speed.HorzLen();
	
	if (player)
		motionAng.y = pos.HorzAngle(player->pos);
	
	modelAnim.SetAnim(*animFiles[UNBALANCE].BCA(), Animation::LOOP, 1._f, 0);
	state = 3;
	isBeingSpit = false;
	
	wmClsn.SetLimMovFlag();
	wmClsn.Unk_0203589c();
	wmClsn.ClearJustHitGroundFlag();
	wmClsn.ClearGroundFlag();
	
	regurgBounceCount = 0;
	
	PlaySound(0x13a, *this);
}

void Goomba::KillIfTouchedBadSurface()
{
	if (wmClsn.IsOnGround())
		return;
	
	// remade this part by taking inspiration from the Bully code so it also works on lava
	Vector3 raycastPos = { pos.x, pos.y + 150._f, pos.z };
	
	RaycastGround raycaster;
	raycaster.SetObjAndPos(raycastPos, this);
	
	if (raycaster.DetectClsn() && pos.y <= raycaster.clsnPosY + 20._f)
	{
		CLPS& clps = raycaster.result.surfaceInfo.clps;
		
		if (clps.isWater)
		{
			SpawnStarIfNecessary();
			SpawnCoin();
			Kill();
			ReleaseCap(CAP_OFFSET);
			pos = originalPos;
			RespawnIfHasCap();
			return;
		}
		
		unsigned behav = clps.behaviorID;
		
		if ((clps.textureID == CLPS::TX_SAND &&
			(behav == CLPS::BH_LOW_JUMPS || behav == CLPS::BH_SLOW_SHALLOW_QUICKSAND || behav == CLPS::BH_SLOW_DEEP_QUICKSAND || behav == CLPS::BH_INSTANT_QUICKSAND)) ||
			behav == CLPS::BH_WIND_GUST || behav == CLPS::BH_LAVA || behav == CLPS::BH_DEATH || behav == CLPS::BH_DEATH_2)
		{
			SpawnCoin();
			Kill();
			if ((capID & 0xf) < 6 || spawnStar == 2)
			{
				pos = originalPos;
				ReleaseCap(CAP_OFFSET);
				RespawnIfHasCap();
			}
		}
	}
	
	wmClsn.sphere.floorResult.Reset(); // I don't know what this does, the original code does this so I kept it
}

void Goomba::UpdateModelTransform()
{
	modelAnim.mat4x3 = Matrix4x3::RotationY(ang.y);
	modelAnim.mat4x3.c3 = pos >> 3;
	
	if (!(flags & 0x40000))
		DropShadowRadHeight(shadow, modelAnim.mat4x3, scale.x * 0x50, wmClsn.IsOnGround() ? 0x1e000_f : 0x96000_f, 0xf);
	
	UpdateCapPos(Vector3{Sin(ang.y) * 0xa, CAP_OFFSET.y, Cos(ang.y) * 0xa}, ang);
}

bool Goomba::UpdateIfEaten()
{
	unsigned eatState = UpdateYoshiEat(wmClsn); //r4
	if (eatState == 0) return false;
	else if (eatState == 1)
	{
		if (GetCapEatenOffIt(CAP_OFFSET))
		{
			RenderRegurgGoombaHelpless(eater);
			horzSpeed = -0xf000_f;
			speed.y = 0x14000_f;
			modelFile.BMD()->PrepareAnim(regurgMat);
			materialChg.SetFile(regurgMat, Animation::NO_LOOP, 1._f, 0);
			materialChg.currFrame = 0x0_f;
			cylClsn.Clear();
			return false;
		}
	}
	else if (eatState == 3)
	{
		if (wmClsn.IsOnGround())
			horzSpeed >>= 1;
	}
	
	if (SpawnParticlesIfHitOtherObj(cylClsn))
	{
		Actor* other = Actor::FindWithID(cylClsn.otherObjID); // guaranteed to exist by condition
		defeatMethod = Enemy::DF_HIT_REGURG;
		KillByAttack(*other);
		modelAnim.SetAnim(*animFiles[ROLLING].BCA(), Animation::NO_LOOP, 1._f, 0);
		cylClsn.flags1 |= CylinderClsn::DISABLED;
		return true;
	}
	
	UpdateModelTransform();
	cylClsn.Clear();
	if (isBeingSpit)
	{
		KillIfTouchedBadSurface();
		if (spitTimer == 0)
			cylClsn.Update();
		else if (spitTimer == 5)
		{
			modelAnim.SetAnim(*animFiles[ROLLING].BCA(), Animation::NO_LOOP, 1._f, 0);
			motionAng.y += 0x8000;
			horzSpeed = -horzSpeed;
			
			modelFile.BMD()->PrepareAnim(regurgMat);
			materialChg.SetFile(regurgMat, Animation::NO_LOOP, 1._f, 0);
			materialChg.currFrame = 0x0_f;
		}
		
		modelAnim.Advance();
		if (wmClsn.JustHitGround())
			RenderRegurgGoombaHelpless(nullptr);
	}
	
	return true;
}

void Goomba::PlayMovingSound()
{
	if (state != 0)
		return;
		
	if (!wmClsn.IsOnGround())
		return;
		
	unsigned currFrameInt = (int)modelAnim.currFrame;
	if ((modelAnim.file == animFiles[WALK].BCA() && (currFrameInt <= 4 || (currFrameInt >= 12 && currFrameInt <= 16))) ||
		(modelAnim.file == animFiles[RUN ].BCA() && (currFrameInt <= 3 || (currFrameInt >= 10 && currFrameInt <= 13))))
	{
		if (flags468 & 2)
			return;
			
		PlaySound(0xd0, *this);
		flags468 |= 2;	
	}
	else
		flags468 = flags468 & ~2;
}

static void Bounce(Player& player, Fix12i bounceInitVel)
{
	if (player.currState == &Player::ST_LONG_JUMP)
		player.speed.y = bounceInitVel;
	else
		player.Bounce(bounceInitVel);
}

[[gnu::target("thumb")]]
void Goomba::GetHurtOrHurtPlayer()
{
	if (cylClsn.otherObjID == 0)
		return;
	
	Player* player = (Player*)Actor::FindWithID(cylClsn.otherObjID);
	if (player == nullptr)
		return;
	
	hitFlags = cylClsn.hitFlags;
	bool rotate = false;
	
	if (actorID != SMALL_ID && (hitFlags & CylinderClsn::HIT_BY_MEGA_CHAR) != 0)
	{
		ReleaseCap(CAP_OFFSET);
		KillByInvincibleChar(Vector3_16{(short)(actorID == NORMAL_ID ? -0x2000 : -0x1800), 0, 0}, *player);
		return;
	}
	else if ((hitFlags & CylinderClsn::HIT_BY_SPIN_OR_GROUND_POUND) != 0)
	{
		defeatMethod = Enemy::DF_SQUASHED;
		if (type == Type::BIG)
			coinType = Enemy::CN_BLUE;
		
		scale.x = scale.y = scale.z = 1._f;
		PlaySound(0xe0, *this);
	}
	else if ((hitFlags & CylinderClsn::HIT_BY_FIRE) != 0)
	{
		rotate = true;
		modelAnim.SetAnim(*animFiles[STRETCH].BCA(), Animation::NO_LOOP, 1._f, 0);
		defeatMethod = Enemy::DF_BURNED;
	}
	else if (actorID == LARGE_ID)
	{
		if (player->actorID == 0xbf)
		{
			// Vector3 playerPos = player->pos;
			if (JumpedOnByPlayer(cylClsn, *player))
			{
				Bounce(*player, 0x28000_f);
				PlaySound(0xe0, *this);
				defeatMethod = Enemy::DF_SQUASHED;
				scale.x = scale.y = scale.z = 1._f;
			}
			else
			{
				if (player->isVanishLuigi)
					return;
				
				if (state == 0)
				{
					state = 1;
					if ((cylClsn.hitFlags & CylinderClsn::HIT_BY_PLAYER) != 0)
						player->Hurt(pos, DAMAGES[type], 0xc000_f + 0x6000_f * std::min<int>(extraDamage, 2), 1, 0, 1);
					
					return;
				}
			}
		}
	}
	else if ((hitFlags & CylinderClsn::HIT_BY_REGURG_GOOMBA) != 0)
	{
		defeatMethod = Enemy::DF_HIT_REGURG;
		modelAnim.SetAnim(*animFiles[ROLLING].BCA(), Animation::NO_LOOP, 1._f, 0);
		cylClsn.flags1 |= CylinderClsn::DISABLED;
	}
	else if ((hitFlags & (CylinderClsn::HIT_BY_DIVE | CylinderClsn::HIT_BY_EGG)) != 0)
	{
		defeatMethod = Enemy::DF_DIVED;
		modelAnim.SetAnim(*animFiles[ROLLING].BCA(), Animation::NO_LOOP, 1._f, 0);
	}
	else if ((hitFlags & CylinderClsn::HIT_BY_EXPLOSION) != 0)
	{
		defeatMethod = Enemy::DF_UNK_6;
		modelAnim.SetAnim(*animFiles[STRETCH].BCA(), Animation::NO_LOOP, 1._f, 0);
	}
	else if ((hitFlags & (CylinderClsn::HIT_BY_KICK | CylinderClsn::HIT_BY_BREAKDANCE | CylinderClsn::HIT_BY_SLIDE_KICK)) != 0)
	{
		rotate = true;
		modelAnim.SetAnim(*animFiles[STRETCH].BCA(), Animation::NO_LOOP, 1._f, 0);
		defeatMethod = Enemy::DF_KICKED;
	}
	else if ((hitFlags & CylinderClsn::HIT_BY_PUNCH) != 0)
	{
		modelAnim.SetAnim(*animFiles[ROLLING].BCA(), Animation::NO_LOOP, 1._f, 0);
		defeatMethod = Enemy::DF_PUNCHED;
		rotate = true;
	}
	else if ((hitFlags & CylinderClsn::HIT_BY_YOSHI_TONGUE) == 0 && player->actorID == 0xbf)
	{
		if (player->isMetalWario)
		{
			ReleaseCap(CAP_OFFSET);
			KillByInvincibleChar(Vector3_16{0x2000, 0, 0}, *player);
			return;
		}
		
		// Vector3 playerPos = player->pos;
		
		if (player->IsOnShell())
		{
			defeatMethod = Enemy::DF_DIVED;
			rotate = true;
			modelAnim.SetAnim(*animFiles[ROLLING].BCA(), Animation::NO_LOOP, 1._f, 0);
		}
		else if (JumpedOnByPlayer(cylClsn, *player))
		{
			Bounce(*player, 0x28000_f);
			PlaySound(0xe0, *this);
			defeatMethod = Enemy::DF_SQUASHED;
			scale.x = scale.y = scale.z = 1._f;
		}
		else if (player->isVanishLuigi)
				return;
		else if (state == 0)
		{
			if (type == Type::SMALL)
			{
				SmallPoofDust();
				
				if (color == BLUE)
					player->Shock(DAMAGES[type] + extraDamage);
				else if (color == ORANGE)
					player->Burn();
				else
					player->Hurt(pos, DAMAGES[0] + extraDamage, 0xc000_f + 0x6000_f * std::min<int>(extraDamage, 2), 1, 0, 1);
				
				Kill();
				PlaySound(0x110, *this);
			}
			else if ((cylClsn.hitFlags & CylinderClsn::HIT_BY_PLAYER) != 0)
			{
				if (color == BLUE)
					player->Shock(DAMAGES[type] + extraDamage);
				else if (color == ORANGE)
					player->Burn();
				else
					player->Hurt(pos, DAMAGES[type] + extraDamage, 0xc000_f + 0x6000_f * std::min<int>(extraDamage, 2), 1, 0, 1);
				
				state = 1;
			}
			
			return;
		}
	}
	
	if (defeatMethod != Enemy::DF_NOT)
		ReleaseCap(CAP_OFFSET);
	
	KillByAttack(*player);
	
	if (rotate)
		ang.y = motionAng.y + 0x8000;
}

void Goomba::KillIfIntoxicated()
{
	if (wmClsn.IsOnGround())
		return;
		
	RaycastGround raycaster;
	raycaster.StartDetectingWater();
	raycaster.StartDetectingToxic();
	raycaster.StopDetectingOrdinary();
	
	raycaster.SetObjAndPos(Vector3{pos.x, pos.y + 0x190000_f, pos.z}, this);
	if (raycaster.DetectClsn() && raycaster.result.surfaceInfo.clps.isToxic &&
	   raycaster.clsnPosY != 0x80000000_f && pos.y < raycaster.clsnPosY)
	{
		SpawnCoin();
		PoofDust();
		Kill();
		ReleaseCap(CAP_OFFSET);
		pos = originalPos;
		RespawnIfHasCap();
	}
}

void Goomba::Jump()
{
	PlaySound(0x118, *this);
	state = 2;
	horzSpeed = 0x0_f;
	speed.y = JUMP_SPEEDS[type];
	wmClsn.ClearGroundFlag();
	cylClsn.flags1 |= 4;
}

void Goomba::UpdateTargetDirAndDist(Fix12i theMaxDist)
{
	Player* player = ClosestPlayer();
	
	if (pos.Dist(originalPos) > theMaxDist || !player)
	{
		targetDir = pos.HorzAngle(originalPos);
		distToPlayer = 0x061a8000_f;
		return;
	}
	
	Vector3 playerPos = player->pos;
	
	if (originalPos.Dist(playerPos) > theMaxDist)
	{
		distToPlayer = 0x061a8000_f;
		return;
	}
	
	distToPlayer = pos.Dist(playerPos);
	targetDir = pos.HorzAngle(playerPos);
}

void Goomba::State0()
{
	State0_NormalGoomba();

	ang.y = motionAng.y;
}

void Goomba::State0_NormalGoomba()
{
	short angAccel = 0x200;
	UpdateTargetDirAndDist(0x3e8000_f);
	horzSpeed.ApproachLinear(targetSpeed, 0x500_f);
	
	if (flags468 & 1)
	{
		if (ApproachLinear(motionAng.y, targetDir, 0x200) != 0)
			flags468 &= ~1;
		
		return;
	}
	
	if (noChargeTimer == 0)
	{
		if (0x61a7fff_f < distToPlayer)
		{
			targetDir2 = targetDir;
			movementTimer = 0x19;
		}
		
		bool redirected = AngleAwayFromWallOrCliff(wmClsn, targetDir2);
		flags468 = (flags468 & ~1) | ((uint8_t)redirected & 1);
		
		if (!redirected)
		{
			if (distToPlayer < maxDist || (capID < 6 && 0x3e8000_f < pos.Dist(originalPos)))
			{
				if (WALK_SPEEDS[type] < targetSpeed)
					modelAnim.SetAnim(*animFiles[RUN].BCA(), Animation::LOOP, 1._f, 0);
				else
					Jump();
				
				if (capID < 6 && noChargeTimer == 0)
					angAccel = 0x600;
				
				targetDir2 = targetDir;
				targetSpeed = RUN_SPEEDS[type] * extraSpeed;
				
				// colors that jump: RED, GREEN, ORANGE
				if ((color <= GREEN || color == ORANGE) && distToPlayer <= JUMP_DIST && modelAnim.file == animFiles[RUN].BCA())
				{
					state = 5;
					horzSpeed = JUMP_HORZ_SPEED;
					speed.y = JUMP_INIT_VEL;
					vertAccel = JUMP_VERT_ACCEL;
					modelAnim.SetAnim(*animFiles[WAIT].BCA(), Animation::Flags::LOOP, 1._f, 0);
					PlaySound(0x118, *this);
				}
			}
			else
			{
				targetSpeed = WALK_SPEEDS[type] * extraSpeed;
				modelAnim.SetAnim(*animFiles[WALK].BCA(), Animation::LOOP, 1._f, 0);
				
				if (movementTimer == 0)
				{
					if (((RandomIntInternal(&RNG_STATE) >> 0x10) & 3) == 0)
					{
						targetDir2 = RandomIntInternal(&RNG_STATE);
						Jump();
					}
					else
					{
						RandomIntInternal(&RNG_STATE);
						targetDir2 = motionAng.y + RandomIntInternal(&RNG_STATE);
						movementTimer = 100;
					}
				}
				else
				{
					--movementTimer;
				}
			}
		}
		
		if (5 < capID && (0x1e < stuckInSpotTimer))
			noChargeTimer = stuckInSpotTimer;
		
		ApproachLinear(motionAng.y, targetDir2, angAccel);
		return;
	}
	
	if (capID < 6)
	{
		if (WALK_SPEEDS[type] < targetSpeed)
			modelAnim.SetAnim(*animFiles[RUN].BCA(), Animation::LOOP, 1._f, 0);
		else
			Jump();
		
		angAccel = 0x800;
		targetSpeed = RUN_SPEEDS[type];
		targetDir2 = targetDir;
	}
	else
	{
		targetSpeed = WALK_SPEEDS[type] * extraSpeed;
		modelAnim.SetAnim(*animFiles[WALK].BCA(), Animation::LOOP, 1._f, 0);
		targetDir2 = pos.HorzAngle(originalPos);
		angAccel = 0x400;
	}
	
	ApproachLinear(motionAng.y, targetDir2, angAccel);
	return;
}

void Goomba::State1()
{
	Jump();
	if (actorID == LARGE_ID)
		speed.y *= 0x1800_f;
	
	targetDir2 = targetDir;
	flags468 &= ~1;
	ang.y = motionAng.y;
}

void Goomba::State2()
{
	if (wmClsn.JustHitGround())
	{
		if (type == Type::NORMAL)
			LandingDust(true);
		else if (type == Type::BIG)
			BigLandingDust(true);
	}
	
	if (wmClsn.IsOnGround())
	{
		state = 0;
		cylClsn.flags1 &= ~4;
	}
	else
		ApproachLinear(motionAng.y, targetDir2, 0x800);
	
	ang.y = motionAng.y;
}

void Goomba::State3()
{
	if (regurgTimer == 0) // never gets executed
	{
		materialChg.Advance();
		if (!modelAnim.Finished())
			return;
		
		flags = recoverFlags;
		state = 0;
		modelAnim.SetAnim(*animFiles[WALK].BCA(), Animation::LOOP, 1._f, 0);
		targetSpeed = WALK_SPEEDS[type];
		wmClsn.ClearLimMovFlag();
		motionAng.y = ang.y;
		cylClsn.flags1 &= ~0x00020000;
		modelFile.BMD()->PrepareAnim(aliveMat);
		materialChg.SetFile(aliveMat, Animation::NO_LOOP, 1._f, 0);
		materialChg.currFrame = 0x0_f;
		return;
	}
	
	if (regurgTimer < 0x3d && --regurgTimer == 0)
	{
		SpawnStarIfNecessary();
		SpawnCoin();
		Kill();
		ReleaseCap(CAP_OFFSET);
		pos = originalPos;
		RespawnIfHasCap();
	}
	
	if (wmClsn.JustHitGround() == 0)
	{
		if (wmClsn.IsOnGround() != 0)
		{
			wmClsn.ClearLimMovFlag();
			speed.y = horzSpeed = 0x0_f;
			
			if (0x3c < regurgTimer)
				regurgTimer = 0x1e;
		}
	}
	else
	{
		if (0x3c < regurgTimer)
			regurgTimer = 0x1e;
		
		if (Abs(speed.y) <= 0x500_f * regurgTimer)
			speed.y = regurgTimer * 0x400_f - vertAccel;
		else
			speed.y = -0x50 * speed.y / (0x64 * floorNormal.y);
		
		horzSpeed >>= 1;
		
		if (Abs(horzSpeed) < 0x5000_f)
		{
			if (horzSpeed < 0_f)
				horzSpeed = -0x5000_f;
			else
				horzSpeed = 0x5000_f;
		}
		
		if (regurgBounceCount == 0)
			PlaySound(0x13a, *this);
		else if (regurgBounceCount < 3)
			PlaySound(0x13b, *this);
		
		++regurgBounceCount;
	}
	
	isBeingSpit = true;
	if (SpawnParticlesIfHitOtherObj(cylClsn))
	{
		defeatMethod = Enemy::DF_HIT_REGURG;
		KillByAttack(*Actor::FindWithID(cylClsn.otherObjID));
		modelAnim.SetAnim(*animFiles[ROLLING].BCA(), Animation::NO_LOOP, 1._f, 0);
		cylClsn.flags1 |= 1;
	}
	isBeingSpit = false;
	
	return;
}

void Goomba::State4()
{
	horzSpeed = 0x0_f;
	if (changeDirTimer == 0 || --changeDirTimer != 0)
		return;
	
	state = 0;
	modelAnim.SetAnim(*animFiles[WALK].BCA(), 0, 1._f, 0);
}

void Goomba::State5()
{
	if ((color == GREEN || color == ORANGE) && speed.y < 0_f)
	{
		state = 6;
		termVel = SPIN_TERM_VEL;
	}
	
	if (wmClsn.IsOnGround())
		state = 0;
}

void Goomba::State6()
{
	ang.y += SPIN_SPEED;
	
	UpdateTargetDirAndDist(0x3e8000_f);
	ApproachLinear(motionAng.y, targetDir, 0x800);
	
	if (wmClsn.IsOnGround())
		state = 0;
}
