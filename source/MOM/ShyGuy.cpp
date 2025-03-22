#include "ShyGuy.h"

namespace
{
	using StateFuncPtr = void(ShyGuy::*)();

	enum Animations
	{
		WAIT,
		WALK,
		RUN,
		FREEZE,
		
		NUM_ANIMS
	};

	const StateFuncPtr stateFuncs[]
	{
		&ShyGuy::State0_Wait,
		&ShyGuy::State1_Turn,
		&ShyGuy::State2_Chase,
		&ShyGuy::State3_Stop,
		&ShyGuy::State4_Warp
	};

	const char material0Name[] = "mat_body-material";
	const char material1Name[] = "color-material"; //silly DAE adding "-material" to the end of the name (didn't fall for it)
	u8 matVals[] = {0x1f, 0x1d, 0x1a, 0x18, 0x15, 0x13, 0x10, 0x00};

	BMA_File::MaterialProperties matProps[] =
	{
		{
			0xffff, 0,
			&material0Name[0],
			0x01, false, 0x0000,   0x01, false, 0x0000,   0x01, false, 0x0000,
			0x01, false, 0x0006,   0x01, false, 0x0006,   0x01, false, 0x0006, 
			0x01, false, 0x0007,   0x01, false, 0x0007,   0x01, false, 0x0007,
			0x01, false, 0x0007,   0x01, false, 0x0007,   0x01, false, 0x0007,
			0x01, true , 0x0000
		},
		{
			0xffff, 0,
			&material1Name[0],
			0x01, false, 0x0000,   0x01, false, 0x0000,   0x01, false, 0x0000,
			0x01, false, 0x0006,   0x01, false, 0x0006,   0x01, false, 0x0006, 
			0x01, false, 0x0007,   0x01, false, 0x0007,   0x01, false, 0x0007,
			0x01, false, 0x0007,   0x01, false, 0x0007,   0x01, false, 0x0007,
			0x01, true , 0x0000
		}
	};

	BMA_File matDef = {7, 0x0000, &matVals[0], 2, &matProps[0]};

	constexpr Fix12i RADIUS = 0x50000_f;
	constexpr Fix12i HEIGHT = 0xa0000_f;
	constexpr Fix12i VERT_ACCEL = -0x2000_f;
	constexpr Fix12i TERM_VEL = -0x32000_f;

	constexpr int DAMAGE = 1;
	constexpr Fix12i PLAYER_KNOCKBACK = 0xc000_f;
	constexpr Vector3_16 KNOCKBACK_ROT = {0x1800, 0, 0};
	constexpr int NUM__COINS_SHY_GUY = 2;
	constexpr Fix12i PLAYER_BOUNCE_INIT_VEL = 0x28000_f;

	constexpr uint16_t WAIT_TIME = 45;
	constexpr short TURN_SPEED = 0x400;
	constexpr Fix12i SIGHT_COS_RADIUS = 0xddb_f; //cos(30 degrees)
	constexpr Fix12i SIGHT_DIST = 0x4b0000_f;

	constexpr Fix12i WALK_SPEED = 0x8000_f;
	constexpr Fix12i WARP_SPEED = 0x20000_f;
	constexpr Fix12i TARGET_POS_TOLERANCE = 0x8000_f;
	constexpr Fix12i TARGET_POS_WARP_TOL = 0x20000_f;
	constexpr Fix12i CLIFF_TOLERANCE = 0x32000_f;
	constexpr Fix12i HEIGHT_TOLERANCE = HEIGHT >> 1;

	constexpr Fix12i CHASE_SPEED = 0x10000_f;
	constexpr Fix12i CHASE_ACCEL = 0x800_f;
	constexpr short CHASE_TURN_SPEED = 0xa00;
	constexpr Fix12i QUIT_CHASE_DIST = 0x4b0000_f;
	constexpr uint8_t CHASE_COOLDOWN = 30;

	constexpr uint8_t GIVE_UP_TIMER = 150;
}

SharedFilePtr ShyGuy::modelFile;
SharedFilePtr ShyGuy::animFiles[4];

SpawnInfo ShyGuy::spawnData =
{
	[] -> ActorBase* { return new ShyGuy; },
	0x016b,
	0x0018,
	0x10000006,
	0x00032000_f,
	0x00046000_f,
	0x01000000_f,
	0x01000000_f
};

void ShyGuy::UpdateModelTransform()
{
	Matrix4x3& modelMat = rigMdl.mat4x3;
	
	modelMat = Matrix4x3::RotationY(ang.y);
	modelMat.c3 = pos >> 3;
	
	if(!(flags & 0x40000))
		DropShadowRadHeight(shadow, modelMat, RADIUS, wmClsn.IsOnGround() ? 0x1e000_f : 0x96000_f, 0xf);
}

Fix12i ShyGuy::FloorY(const Vector3& pos)
{
	Vector3 raycasterPos = {pos.x, pos.y + 0x14000_f, pos.z};
	RaycastGround raycaster;
	raycaster.SetObjAndPos(raycasterPos, nullptr);
	Fix12i res;
	if(raycaster.DetectClsn())
		res = raycaster.clsnPosY;
	else
		res = pos.y;
	return res;
}

void ShyGuy::SetTargetPos()
{
	pathPtr.GetNode(targetPos, nextPathPt);
	targetPos.y = FloorY(targetPos);
	targetAngle = pos.HorzAngle(targetPos);
}

int ShyGuy::InitResources()
{
	BMD_File& modelF = modelFile.LoadBMD();

	for(int i = 0; i < NUM_ANIMS; ++i)
		animFiles[i].LoadBCA();
	
	if(!rigMdl.SetFile(modelF, 1, -1))
		return 0;
	
	if(!shadow.InitCylinder())
		return 0;
	
	rigMdl.SetAnim(*animFiles[WAIT].BCA(), Animation::LOOP, 1._f, 0);
	modelF.PrepareAnim(matDef);
	matChg.SetFile(matDef, Animation::NO_LOOP, 1._f, 0);
		
	cylClsn.Init(this, RADIUS, HEIGHT, 0x00200000, 0x00a6efe0);
	wmClsn.Init(this, RADIUS, HEIGHT >> 1, nullptr, nullptr);
	
	UpdateModelTransform();
	
	scale = Vector3{0x1000_f, 0x1000_f, 0x1000_f};
	
	coinType = Enemy::CN_YELLOW;
	numCoinsMinus1 = NUM__COINS_SHY_GUY - 1;
	
	targetAngle = ang.y;
	vertAccel = VERT_ACCEL;
	termVel = TERM_VEL;
	
	state = chaseCooldown = 0;
	targetPlayer = nullptr;
	
	offTrack = false;
	backAndForth = (ang.x & 0xf) > 0;
	customColor = (ang.x >> 4 & 0xf) > 0;

	if(param1 != 0xffff)
	{
		reverse = false;
		pathPtr.FromID(param1);
		numPathPts = pathPtr.NumNodes();
		nextPathPt = 1;
		pathPtr.GetNode(pos, 0);
		SetTargetPos();
	}
	else
	{
		targetPos = Vector3{pos.x, FloorY(pos), pos.z};
	}
	
	color = (uint16_t)ang.z | 0x8000 | (ang.z >>  0 & 0x1f) >> 1 << 16
									 | (ang.z >>  5 & 0x1f) >> 1 << 21
									 | (ang.z >> 10 & 0x1f) >> 1 << 26;
	
	return 1;
}

int ShyGuy::CleanupResources()
{
	for(int i = 0; i < NUM_ANIMS; ++i)
		animFiles[i].Release();
	modelFile.Release();
	return 1;
}

void ShyGuy::Kill()
{
	//The coins have already spawned.
	Sound::PlayArchive3(0xd6, camSpacePos);
	KillAndTrackInDeathTable();
}

void ShyGuy::HandleClsn()
{
	Actor* other = Actor::FindWithID(cylClsn.otherObjID);
	if(!other)
		return;
	
	unsigned hitFlags = cylClsn.hitFlags;
	
	if(hitFlags & 0x000667c0)
	{
		if(other->actorID == 0x00bf)
			KillByInvincibleChar(KNOCKBACK_ROT, *(Player*)other);
		else
		{
			SpawnCoin();
			Kill();
		}
		return;
	}
	else if(hitFlags & CylinderClsn::HIT_BY_SPIN_OR_GROUND_POUND)
	{
		defeatMethod = Enemy::DF_SQUASHED;
		KillByAttack(*other);
		scale.y = 0x0555_f;
		Sound::PlayArchive3(0xe0, camSpacePos);
		return;
	}
	
	if(other->actorID != 0x00bf)
		return;
	
	Player* player = (Player*)other;
	if(JumpedOnByPlayer(cylClsn, *player))
	{
		defeatMethod = Enemy::DF_SQUASHED;
		KillByAttack(*other);
		scale.y = 0x0555_f;
		Sound::PlayArchive3(0xe0, camSpacePos);
		player->Bounce(PLAYER_BOUNCE_INIT_VEL);
	}
	else if(hitFlags & CylinderClsn::HIT_BY_MEGA_CHAR)
	{
		SpawnMegaCharParticles(*player, nullptr);
		Sound::PlayArchive3(0x001d, camSpacePos);
		KillByInvincibleChar(KNOCKBACK_ROT, *player);
	}
	else if(player->IsOnShell() || player->isMetalWario)
	{
		KillByInvincibleChar(KNOCKBACK_ROT, *player);
	}
	else
	{
		player->Hurt(pos, DAMAGE, PLAYER_KNOCKBACK, 1, 0, 1);
		if(state == 2)
		{
			state = 0;
			chaseCooldown = CHASE_COOLDOWN;
		}
		else if((state == 0 || state == 1) && chaseCooldown == 0)
		{
			state = 1;
			alarmed = true;
			targetAngle = pos.HorzAngle(player->pos);
		}
	}
	
}

Player* ShyGuy::PlayerVisibleToThis(Player* player)
{
	if(!player)
		player = ClosestPlayer();
	if(!player)
		return nullptr;
	
	Vector3 eyePos = {pos.x, pos.y + (HEIGHT >> 1), pos.z};
	Vector3 playerPos = {player->pos.x, player->pos.y + 0x48000_f, player->pos.z};
	
	Vector3 forward = {0_f, 0_f, 0x1000_f};
	Vector3 toPlayer = playerPos - eyePos;
	Fix12i dist = toPlayer.Len();
	if(dist > SIGHT_DIST)
		return nullptr;
	
	MATRIX_SCRATCH_PAPER = Matrix4x3::RotationY(ang.y);
	forward *= MATRIX_SCRATCH_PAPER;
	if(toPlayer.Dot(forward) < dist * SIGHT_COS_RADIUS)
		return nullptr;
	
	RaycastLine raycaster;
	raycaster.SetObjAndLine(eyePos, playerPos, nullptr);
	return !raycaster.DetectClsn() ? player : nullptr;
}

bool ShyGuy::KillIfTouchedBadSurface()
{
	if(wmClsn.IsOnGround())
		return false;
	
	CLPS& clps = wmClsn.sphere.floorResult.surfaceInfo.clps;
	int behav = clps.behaviorID;
	
	
	if((clps.textureID == CLPS::TX_SAND &&
		(behav == CLPS::BH_LOW_JUMPS || behav == CLPS::BH_SLOW_SHALLOW_QUICKSAND || behav == CLPS::BH_SLOW_DEEP_QUICKSAND || behav == CLPS::BH_INSTANT_QUICKSAND)) ||
		clps.isWater ||
	   behav == CLPS::BH_WIND_GUST || behav == CLPS::BH_LAVA || behav == CLPS::BH_DEATH || behav == CLPS::BH_DEATH_2)
	{
		SpawnCoin();
		Kill();
		return true;
	}
	return false;
}

int ShyGuy::GetClosestPathPtID()
{
	Fix12i closestDist = 0x7fffffff_f;
	int closestPt = 0;
	Vector3 pathPt;
	for(int i = 0; i < numPathPts; ++i)
	{
		pathPtr.GetNode(pathPt, i);
		Fix12i dist = pos.Dist(pathPt);
		if(dist < closestDist)
		{
			closestDist = dist;
			closestPt = i;
		}
	}
	return closestPt;
}

void ShyGuy::AimAtClosestPathPt()
{
	if(pathPtr.ptr)
	{
		nextPathPt = GetClosestPathPtID();
		SetTargetPos();
	}
}

void ShyGuy::PlayMovingSoundEffect()
{
	if(rigMdl.file != animFiles[WALK].BCA() && rigMdl.file != animFiles[RUN].BCA())
		return;
	
	int animLen = (int)rigMdl.GetFrameCount();
	int currFrame = (int)rigMdl.currFrame;

	if(currFrame == 0 || currFrame == animLen / 2)
		Sound::PlayArchive3(0xd0, camSpacePos);
}

void ShyGuy::State0_Wait()
{
	if(offTrack)
	{
		Vector3 targetDir = targetPos - pos;
		
		if((wmClsn.IsOnWall() && targetDir.Dot(wallNormal) < 0_f) ||
			IsGoingOffCliff(wmClsn, 0x32000_f, 0x1f49, 0, 1, CLIFF_TOLERANCE) ||
			(pos.HorzDist(targetPos) <= TARGET_POS_TOLERANCE && Abs(targetPos.y - pos.y) > HEIGHT_TOLERANCE))
		{
			state = 4;
			return;
		}
	}
		
	if(pos.HorzDist(targetPos) > TARGET_POS_TOLERANCE)
	{
		rigMdl.SetAnim(*animFiles[WALK].BCA(), Animation::LOOP, 0x1000_f, 0);
		horzSpeed = WALK_SPEED;
		motionAng.y = ang.y = pos.HorzAngle(targetPos);
	}
	else if(pathPtr.ptr)
	{
		nextPathPt += reverse ? -1 : 1;
		if(backAndForth)
		{
			if(nextPathPt == numPathPts - 1)
				reverse = true;
			else if(nextPathPt == 0)
				reverse = false;
		}
		else if(nextPathPt >= numPathPts)
			nextPathPt = 0;
		
		SetTargetPos();
		state = 1;
	}
	else
	{
		rigMdl.SetAnim(*animFiles[WAIT].BCA(), Animation::LOOP, 0x1000_f, 0);
		horzSpeed = 0_f;
		if(stateTimer >= WAIT_TIME)
		{
			targetAngle += 0x4000;
			state = 1;
		}
	}
}

void ShyGuy::State1_Turn()
{
	horzSpeed = 0_f;
	rigMdl.SetAnim(*animFiles[WAIT].BCA(), Animation::LOOP, 0x1000_f, 0);
	if(ApproachLinear(ang.y, targetAngle, TURN_SPEED))
		state = alarmed ? 2 : 0;
}

void ShyGuy::State2_Chase()
{
	//if(rigMdl.currFrame >= 0x1000)
	//	rigMdl.currFrame -= 0x1000;
	rigMdl.SetAnim(*animFiles[RUN].BCA(), Animation::LOOP, 0x1000_f, 0);
	
	offTrack = true;
	if(flags & Actor::OFF_SCREEN)
	{
		state = 0;
		chaseCooldown = CHASE_COOLDOWN;
		AimAtClosestPathPt();
	}

	if (!targetPlayer)
	{
		state = 3;
		return;
	}

	if(pos.Dist(targetPlayer->pos) >= QUIT_CHASE_DIST)
		state = 3;
	
	targetAngle = pos.HorzAngle(targetPlayer->pos);
	
	horzSpeed.ApproachLinear(CHASE_SPEED, CHASE_ACCEL);
	ApproachLinear(ang.y, targetAngle, CHASE_TURN_SPEED);
	motionAng.y = ang.y;
}

void ShyGuy::State3_Stop()
{
	AimAtClosestPathPt();	
	rigMdl.SetAnim(*animFiles[FREEZE].BCA(), Animation::LOOP, 0x1000_f, 0);
	offTrack = true;
	if(horzSpeed.ApproachLinear(0_f, CHASE_ACCEL))
	{
		state = 0;
		chaseCooldown = CHASE_COOLDOWN;
	}
}

void ShyGuy::State4_Warp()
{
	if(stateTimer == 0)
	{
		vertAccel = 0_f;
		termVel = 0x80000000_f;
		flags &= ~Actor::AIMABLE_BY_EGG;
	}
	
	if(pos.HorzDist(targetPos) > TARGET_POS_WARP_TOL)
	{
		int vertAngle = targetPos.VertAngle(pos);
		horzSpeed  = WARP_SPEED * Cos(vertAngle);
		speed.y = WARP_SPEED * Sin(vertAngle);
		motionAng.y = ang.y = pos.HorzAngle(targetPos);
	}
	else
	{
		state = 0;
		chaseCooldown = CHASE_COOLDOWN;
		
		speed.y = 0_f;
		vertAccel = VERT_ACCEL;
		termVel = TERM_VEL;
		flags |= Actor::AIMABLE_BY_EGG;
		
		offTrack = false;
	}
	
	matChg.currFrame.ApproachLinear(0x6000_f, 0x1000_f);
}

int ShyGuy::Behavior()
{
	if(defeatMethod != Enemy::DF_NOT)
	{
		int res = UpdateKillByInvincibleChar(wmClsn, rigMdl, 3);
		if(res == 2) //finished kill
		{
			Kill();
		}
		else if(res == 0) //not yet
		{
			UpdateDeath(wmClsn);
			UpdateModelTransform();
		}
		return 1;
	}
	
	int eatState = UpdateYoshiEat(wmClsn);
	if(eatState == Enemy::UY_NOT || isBeingSpit)
	{
		if(KillIfTouchedBadSurface())
			return 1;
	}
	
	if(eatState != Enemy::UY_NOT)
	{
		cylClsn.Clear();
	
		if(isBeingSpit && spitTimer == 0)
			cylClsn.Update();
		UpdateModelTransform();
		
		if(isBeingSpit && wmClsn.JustHitGround())
		{
			isBeingSpit = false;
			state = 3;
		}
		return 1;
	}
	
	int prevState = state;
	
	//Dun, dun, dun!
	//----------------------------//
	(this->*stateFuncs[state])(); //dat syntax tho
	//----------------------------//
	
	++stateTimer;
	if(state != prevState)
		stateTimer = 0;
	
	bool cooledDown = CountDownToZero(chaseCooldown) == 0;
	if((state == 0 || state == 1) && cooledDown && !(flags & Actor::OFF_SCREEN))
	{
		targetPlayer = PlayerVisibleToThis(nullptr);
		if(targetPlayer)
			state = 2;
	}
	if(state != 1)
		alarmed = false;
	
	
	UpdatePos(nullptr);
	UpdateModelTransform();
	
	if(state == 4)
	{
		cylClsn.Clear();
		return 1;
	}
	
	matChg.currFrame.ApproachLinear(0_f, -1._f);
	
	UpdateWMClsn(wmClsn, 2);
	HandleClsn(); //must be done before clearing collision, of course
	
	cylClsn.Clear();
	if(defeatMethod == Enemy::DF_NOT)
		cylClsn.Update();
	
	MakeVanishLuigiWork(cylClsn);
	
	PlayMovingSoundEffect();
	rigMdl.Advance();
	
	return 1;
}

int ShyGuy::Render()
{
	if(flags & Actor::IN_YOSHI_MOUTH)
		return 1;
		
	rigMdl.Render(&scale);
	
	matChg.Update(rigMdl.data);
	
	if (customColor)
		rigMdl.data.materials[1].difAmb = color; //custom color
	else if (pathPtr.ptr)
		rigMdl.data.materials[1].difAmb = 0x0010801f; //red
	else
		rigMdl.data.materials[1].difAmb = 0x4100fe00; //blue
	
	return 1;
}

void ShyGuy::OnPendingDestroy() {}

unsigned ShyGuy::OnYoshiTryEat()
{
	return state == 4 ? Actor::YE_DONT_EAT : Actor::YE_KEEP_AND_CAN_MAKE_EGG;
}

void ShyGuy::OnTurnIntoEgg(Player& player)
{
	if(player.IsCollectingCap())
		GivePlayerCoins(player, NUM__COINS_SHY_GUY, 0);
	else
		player.RegisterEggCoinCount(NUM__COINS_SHY_GUY, false, false);
	
	MarkForDestruction();
}

Fix12i ShyGuy::OnAimedAtWithEgg()
{
	return HEIGHT >> 1;
}
