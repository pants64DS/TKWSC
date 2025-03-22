#include "Magikoopa.h"

//#define __aeabi_idivmod  Math_IDivMod
//#define __aeabi_uidivmod Math_UDivMod
namespace
{
	using StateFuncPtr = void(Magikoopa::*)();
	
	/*const, but rodata not supported in dynamic libraries yet*/ StateFuncPtr stateFuncs[]
	{
		&Magikoopa::State0_Appear,
		&Magikoopa::State1_Wave,
		&Magikoopa::State2_Shoot,
		&Magikoopa::State3_Poof,
		&Magikoopa::State4_Teleport,
		&Magikoopa::State5_Hurt,
		&Magikoopa::State6_Wait,
		&Magikoopa::State7_Defeat
	};
	
	enum Animations
	{
		APPEAR,
		WAVE,
		SHOOT,
		POOF,
		WAIT,
		HURT,
		DEFEAT,
		
		NUM_ANIMS
	};
	
	constexpr Fix12i SCALES[] = {0x1000_f, 0x4000_f};
	
	constexpr Fix12i RADIUSES[] = {0x68000_f, 4 * 0x58000_f}; //sic
	constexpr Fix12i HEIGHTS [] = {0x90000_f, 4 * 0x90000_f};
	constexpr Fix12i VERT_ACCEL = 0_f;
	constexpr Fix12i TERM_VEL = 0x80000000_f;
	constexpr Fix12i SPIT_DECEL = 0x1000_f;

	constexpr Fix12i TP_PLAYER_DIST_TOL = 0x1e0000_f;
	constexpr Fix12i TP_PLAYER_DIST_MAX = 0xc00000_f;
	constexpr Fix12i BOSS_START_BATTLE_RADIUS = 1775._f;
	constexpr Fix12i BOSS_STOP_BATTLE_RADIUS = 0x1600000_f;
	
	constexpr uint16_t waitMsgIDs  [] = {0x0128, 0x0128, 0x0128, 0x0128};
	constexpr uint16_t defeatMsgIDs[] = {0x0129, 0x0129, 0x0129, 0x0129};
	constexpr Fix12i TALK_HEIGHT = 0x1d74a6_f;

	constexpr int DAMAGE = 1;
	constexpr Fix12i PLAYER_KNOCKBACK = 0xc000_f;
	constexpr Vector3_16 KNOCKBACK_ROT = {0x1800, 0x0000, 0x0000};
	constexpr int NUM__COINS_MAGIKOOPA = 3;
	constexpr Fix12i PLAYER_BOUNCE_INIT_VEL = 0x28000_f;

	constexpr uint16_t APPEAR_TIME = 30;
	constexpr uint16_t WAVE_TIME = 32;
	constexpr uint16_t SHOOT_TIME = 12;
	constexpr uint16_t POOF_TIME = 30;
	constexpr uint16_t TELEPORT_TIME = 45;
	constexpr uint16_t HURT_TIME = 32;

	constexpr short int KAMEK_SHOT = 0x20F;
	constexpr short int KAMEK = 0x210;
	constexpr short int KAMELLA = 0x211;
	constexpr short int VOLCANO_RING = 0x47;
	
	constexpr short int KAMEK_MAGIC_PARTICLE_ID = 0x001E;

	constexpr Fix12i WAND_LENGTH = 0x55000_f;
	
	constexpr short MOST_HORZ_SHOT_ANGLES[] = {-0x1000, 0x0000};

	constexpr unsigned SPRITE_ID = 0x2e; //Should be 2e.
	
	// unsigned aLoadFileInst = 0x00000000;
	Magikoopa::SharedRes* ptrToRes = nullptr;
	
	// unsigned onLoadFuncPtr = (unsigned)&Magikoopa::SharedRes::OnLoadFile;
	
	Particle::MainInfo particleInfo
	{
		0x00010104, //flags
		0x00001000_f, //rate, fix20.12
		0x00000000_f, //startHorzDist, fix23.9
		Vector3_16f{0x0000_fs, 0x1000_fs, 0x0000_fs}, //dir
		Color5Bit(0xff, 0xff, 0xff), //color
		0x00000800_f, //horzSpeed, fix20.12 (fix23.9???)
		0x00000000_f, //vertSpeed, fix20.12 (fix23.9???)
		0x00002800_f, //scale, fix20.12
		0x1000_fs, //horzScale, fix4.12
		0x0000,
		0x0079,
		0x0261,
		0x0000, //frames
		0x002d, //lifetime
		0x75, //scaleRand
		0x9d, //lifetimeRand
		0xce, //speedRand
		0x00,
		0x01, //spawnPeriod
		0x1f, //alpha
		0x24, //speedFalloff
		SPRITE_ID, //spriteID
		0x03,
		0._fs, //velStretchFactor
		0x00 //texMirrorFlags
	};
	
	Particle::MainInfo bossParticleInfo
	{
		0x00010104, //flags
		0x00001000_f, //rate, fix20.12
		0x00000000_f, //startHorzDist, fix23.9
		Vector3_16f{0x0000_fs, 0x1000_fs, 0x0000_fs}, //dir
		Color5Bit(0xff, 0xff, 0xff), //color
		0x00001000_f, //horzSpeed, fix20.12 (fix23.9???)
		0x00000000_f, //vertSpeed, fix20.12 (fix23.9???)
		0x00005000_f, //scale, fix20.12
		0x1000_fs, //horzScale, fix4.12
		0x0000,
		0x0079,
		0x0261,
		0x0000, //frames
		0x002d, //lifetime
		0xff, //scaleRand
		0x9d, //lifetimeRand
		0xce, //speedRand
		0x00,
		0x01, //spawnPeriod
		0x1f, //alpha
		0x24, //speedFalloff
		SPRITE_ID, //spriteID
		0x03,
		0._fs, //velStretchFactor
		0x00 //texMirrorFlags
	};
	
	Particle::ScaleTransition particleScaleTrans
	{
		0x1308_fs, //scaleStart, fix4.12
		0x1000_fs, //scaleMiddle, fix4.12
		0x06a9_fs, //scaleEnd, fix4.12
		0x00, //scaleTrans1End
		0x5b, //scaleTrans2Start;
		0x0004,
		0x057b
	};
	
	Particle::Glitter particleGlitter
	{
		0x0002,
		0x0000,
		0x1000_f, //scale 1, fix4.12
		0x0002, //lifetime
		0x00,
		0x40, //scale 2
		Color5Bit(0xff, 0xff, 0xff),
		0x01, //rate
		0x04,
		0x0c, //period
		0x1c, //spriteID;
		0x00000005, //texMirrorFlags;
	};
	
	Particle::SysDef particleSysDefs[]
	{
		Particle::SysDef
		{
			&particleInfo,
			&particleScaleTrans,
			nullptr,
			nullptr,
			nullptr,
			&particleGlitter,
			nullptr,
			0
		},
		
		Particle::SysDef
		{
			&bossParticleInfo,
			&particleScaleTrans,
			nullptr,
			nullptr,
			nullptr,
			&particleGlitter,
			nullptr,
			0
		}
	};
}

SharedFilePtr Magikoopa::modelFiles[2];
SharedFilePtr Magikoopa::animFiles[7];
	
SpawnInfo Magikoopa::spawnData =
{
	[] -> ActorBase* { return new Magikoopa; },
	0x00c6,
	0x0018,
	0x10000006,
	0x00032000_f,
	0x00046000_f,
	0x01000000_f,
	0x01000000_f,
};

SpawnInfo Magikoopa::bossSpawnData =
{
	[] -> ActorBase* { return new Magikoopa; },
	0x00c6,
	0x0018,
	0x10000006,
	0x000c8000_f,
	0x00118000_f,
	BOSS_STOP_BATTLE_RADIUS,
	BOSS_STOP_BATTLE_RADIUS,
};

[[gnu::target("thumb")]]
void Magikoopa::Resources::Add(SharedFilePtr& sf)
{
	for(int i = 0; i < Magikoopa::Resources::NUM_PER_CHUNK; ++i)
	{
		if((unsigned)&sf == files[i])
			return;
		else if(!files[i])
		{
			files[i] = (unsigned)&sf | 1;
			//++files[i]->numRefs; (Can't do this here or the Model::SetFile function thinks the resource was already loaded and doesn't offset the offsets)
			return;
		}
	}
	
	next = new Resources; //TEST THIS
	if(next)
		next->Add(sf);
	else
		Crash();
}

[[gnu::target("thumb")]]
void Magikoopa::Resources::ProcessAdditions()
{
	for(int i = 0; i < Magikoopa::Resources::NUM_PER_CHUNK; ++i) if(files[i] & 1)
	{
		SharedFilePtr& file = *(SharedFilePtr*)(files[i] & ~1);
		if(file.numRefs > 0)
		{
			files[i] &= ~1;
			++file.numRefs;
		}
		else //The resources may have been deallocated because the object failed to spawn. Don't reload them!
			files[i] = 0;
	}
	if(next)
		next->ProcessAdditions();
}

Magikoopa::Resources::~Resources()
{
	for(int i = 0; i < Magikoopa::Resources::NUM_PER_CHUNK; ++i) if (files[i])
		((SharedFilePtr*)files[i])->Release();
	if(next)
		delete next;
}

/*static*/ void Magikoopa::SharedRes::OnLoadFile(SharedFilePtr& file)
{
	ptrToRes->res.Add(file);
}
void Magikoopa::SharedRes::TrackRes()
{
	/*ptrToRes = this;
	aLoadFileInst = *(unsigned*)0x02017bfc;
	*(unsigned*)0x02017bfc = (onLoadFuncPtr - 0x02017bfc) / 4 - 2 & 0x00ffffff | 0xeb000000;*/
}
void Magikoopa::SharedRes::StopTracking()
{
	/* *(unsigned*)0x02017bfc = aLoadFileInst;*/
}

//START MAGIKOOPA SHOT
namespace
{
	constexpr Fix12i SHOT_SPEEDS[] = {0x20000_f, 0x20000_f};
	constexpr uint16_t SHOT_MAX_OFFSCREEN_TIME = 600;
	
	constexpr Fix12i SHOT_RADIUS = 0x30000_f;
}

SpawnInfo Magikoopa::Shot::spawnData =
{
	[] -> ActorBase* { return new Shot; },
	0x016c,
	0x0018,
	0x00000006,
	0x00032000_f,
	0x00046000_f,
	0x01000000_f,
	0x01000000_f,
};

void Magikoopa::Shot::SetMagikoopa(Magikoopa& magik)
{
	res = magik.res;
	shotState = magik.shotState;
	numFireToSpawn = 4 - magik.health;
	++res->numRefs;
	
	speed = Vector3{(Fix12i)direction.x * SHOT_SPEEDS[res->isBoss],
					(Fix12i)direction.y * SHOT_SPEEDS[res->isBoss],
					(Fix12i)direction.z * SHOT_SPEEDS[res->isBoss]};
}

[[gnu::target("thumb")]]
int Magikoopa::Shot::InitResources()
{
	if (LEVEL_ID == 40) drawDistAsr3 <<= 1;

	resourceRefCount = 0;
	
	direction = Vector3_16f{Fix12s(ang.x, as_raw), Fix12s(ang.y, as_raw), Fix12s(ang.z, as_raw)};
	ang.x = ang.z = 0;
	ang.y = Atan2(direction.x, direction.z);
	
	cylClsn.Init(this, SHOT_RADIUS, SHOT_RADIUS * 2, 0x00200000, 0x00000000);
	wmClsn.Init(this, SHOT_RADIUS, SHOT_RADIUS, nullptr, nullptr);
	wmClsn.StartDetectingWater();
	
	stateTimer = 0;
	
	return 1;
}

int Magikoopa::Shot::CleanupResources()
{
	if(res->numRefs == 1)
		delete res;
	else
		--res->numRefs;
	return 1;
}

extern "C" void IgniteBobOmb(Actor& bobOmb);
asm("IgniteBobOmb = 0x0214ad14");

int Magikoopa::Shot::Behavior()
{
	UpdatePosWithOnlySpeed(nullptr);
	shapesID = Particle::System::New(shapesID, (unsigned)&particleSysDefs[res->isBoss], pos.x, pos.y + SHOT_RADIUS, pos.z, &direction, nullptr);
	UpdateWMClsn(wmClsn, 0);
	
	Actor* actor = Actor::FindWithID(cylClsn.otherObjID);
	
	bool isBoss = res->isBoss;
	
	bool onGround = wmClsn.IsOnGround();
	bool onWall = wmClsn.IsOnWall();
	bool onActor = actor != nullptr && actor->actorID != KAMEK && actor->actorID != KAMELLA;
	
	ClsnResult& clsnRes = /*onGround ?*/ wmClsn.sphere.floorResult /*: wmClsn.sphere.wallResult*/; // boss cannot spawn object on walls
	
	// Magikoopa: check clsn with floor, wall or other object (same as before)
	// Kamella: check clsn with Volcano Ring or player
	if ((!isBoss && (onGround || onWall || onActor)) ||
		(isBoss && ((onGround && clsnRes.obj && clsnRes.obj->actorID == VOLCANO_RING) || (onActor && actor->actorID == 0xbf))))
	{
		res->TrackRes();
		Actor* newActor;

		if (actor && actor->actorID == 0xbf)
		{
			const Vector3 source =
			{
				actor->pos.x - direction.x,
				actor->pos.y - direction.y,
				actor->pos.z - direction.z
			};

			static_cast<Player*>(actor)->Hurt(source, 2, 10._f, 1, 0, 1);

			newActor = nullptr;
		}
		else if (!res->isBoss && res->spawnActorID == 0xddd)
		{
			const Vector3_16 fireAngle = {0x1000, ang.y, 0x0000};

			newActor = Actor::Spawn(0x00d7, 0x0003, pos, &fireAngle, areaID, -1);
		}
		else
		{
			newActor = Actor::Spawn(res->spawnActorID, res->spawnActorParams, pos, &ang, areaID, -1);

			if (newActor && newActor->actorID == 0xce) // if it's a bob-omb
				IgniteBobOmb(*newActor);
		}
		
		res->StopTracking();
		res->res.ProcessAdditions();
		if(newActor && (int)param1 >= 0)
			res->shotUniqueIDs[param1] = newActor->uniqueID;
		PoofDustAt(Vector3{pos.x, pos.y + SHOT_RADIUS, pos.z});
		MarkForDestruction();
	}
	else if (isBoss && (onGround || onWall || onActor))
	{
		res->StopTracking();
		res->res.ProcessAdditions();
		PoofDustAt(Vector3{pos.x, pos.y + SHOT_RADIUS, pos.z});
		MarkForDestruction();
	}
	
	if((flags & Actor::OFF_SCREEN))
	{
		++stateTimer;
		if(stateTimer >= SHOT_MAX_OFFSCREEN_TIME)
			MarkForDestruction();
	}
	else
		stateTimer = 0;
	
	cylClsn.Clear();
	cylClsn.Update();
	MakeVanishLuigiWork(cylClsn);
	return 1;
}

int Magikoopa::Shot::Render()
{
	return 1;
}

void Magikoopa::Shot::OnPendingDestroy() {}

//END MAGIKOOPA SHOT

void Magikoopa::UpdateModelTransform()
{
	Matrix4x3& modelMat = rigMdl.mat4x3;
	
	modelMat = Matrix4x3::RotationY(ang.y) * Matrix4x3::Scale(SCALES[res->isBoss], SCALES[res->isBoss], SCALES[res->isBoss]);
	modelMat.c3 = pos >> 3;
	
	if(!(flags & 0x40000))
		DropShadowRadHeight(shadow, modelMat, RADIUSES[0], 0x320000_f / SCALES[res->isBoss], 0xf);
}

[[gnu::target("thumb")]]
int Magikoopa::InitResources()
{
	if (LEVEL_ID == 40) drawDistAsr3 <<= 1;

	if(!(res = new SharedRes)) //the use of 1 equal sign was intentional
		return 0;
		
	res->isBoss = actorID == KAMELLA ? 1 : 0;
	shotState = 0;
	
	BMD_File& modelF = modelFiles[res->isBoss].LoadBMD();
	for(int i = 0; i < NUM_ANIMS; ++i)
		animFiles[i].LoadBCA();
	
	if(!rigMdl.SetFile(modelF, 1, -1))
		return 0;
	
	if(!shadow.InitCylinder())
		return 0;
	
	rigMdl.SetAnim(*animFiles[res->isBoss ? WAIT : APPEAR].BCA(), Animation::NO_LOOP, 1._f, 0);
		
	cylClsn.Init(this, RADIUSES[res->isBoss], HEIGHTS[res->isBoss], 0x00200000, 0x00a6efe0);
	wmClsn.Init(this, RADIUSES[res->isBoss], HEIGHTS[res->isBoss] >> 1, nullptr, nullptr);
	wmClsn.StartDetectingWater();
	
	scale = Vector3{0x1000_f, 0x1000_f, 0x1000_f};
	
	UpdateModelTransform();
	
	coinType = Enemy::CN_YELLOW;
	numCoinsMinus1 = NUM__COINS_MAGIKOOPA - 1;
	
	vertAccel = VERT_ACCEL;
	termVel = TERM_VEL;
	
	eventToTrigger = param1 >> 8 & 0xff;
	starID = ang.x >> 12 & 0xf;
	pathPtr.FromID(param1 & 0xff);
	numPathPts = pathPtr.NumNodes();
	currPathPt = numPathPts; //hax
	
	res->spawnActorID = ang.x & 0xfff;
	res->spawnActorParams = ang.z;
	//ang.x = ang.z = 0;
	originalPos = pos;
	
	res->TrackRes();
	Actor* newActor;
	//avoid the glitch where particles mess up if all the whatevers are killed before the Magikoopa spawns a whatever.
	//Fire uses only particles, so it doesn't count.
	if (res->isBoss)
	{
		if((newActor = Actor::Spawn(res->spawnActorID, res->spawnActorParams, pos, nullptr, areaID, -1)))
			newActor->MarkForDestruction();
		if((newActor = Actor::Spawn(0x00d7, 0x0003, pos, nullptr, areaID, -1)))
			newActor->MarkForDestruction();
	}
	else if (res->spawnActorID == 0xddd)
	{
		if((newActor = Actor::Spawn(0x00d7, 0x0003, pos, nullptr, areaID, -1)))
			newActor->MarkForDestruction();
	}
	else
	{
		if((newActor = Actor::Spawn(res->spawnActorID, res->spawnActorParams, pos,  nullptr, 0, -1)))
			newActor->MarkForDestruction();
	}
	//cout << res->spawnActorID  << '\n';
	//cout << res->spawnActorParams  << '\n';
	
	res->StopTracking();
	res->res.ProcessAdditions();
	
	state = 4;
	
	health = 3;
	battleStarted = false;
	invincible = true;
	
	return 1;
}

[[gnu::target("thumb")]]
int Magikoopa::CleanupResources()
{
	for(int i = 0; i < NUM_ANIMS; ++i)
		animFiles[i].Release();
	modelFiles[res->isBoss].Release();
	
	if(res->numRefs == 1)
		delete res;
	else
		--res->numRefs;
	
	return 1;
}

[[gnu::target("thumb")]]
void Magikoopa::KillMagikoopa()
{
	if (!res->isBoss && res->spawnActorID == 0xddd)
	{
		Vector3 actorSpawnPos = Vector3{ 3200._f, 550._f, 4900._f };
		Actor::Spawn(0x022b, 0xf10e, actorSpawnPos, nullptr, areaID, -1);
		Actor::Spawn(0x00b2, 0x0040 + STAR_ID, pos, nullptr, areaID, -1);
	}
	
	//The coins have already spawned.
	KillAndTrackInDeathTable();
}

void Magikoopa::HandleClsn()
{
	Actor* other = Actor::FindWithID(cylClsn.otherObjID);
	if(!other)
		return;
	
	unsigned hitFlags = cylClsn.hitFlags;
	
	if(res->isBoss)
	{
		if((hitFlags & CylinderClsn::HIT_BY_EGG) && !invincible)
		{
			--health;
			state = 5;
			stateTimer = 0;
			invincible = true;
			return;
		}
		
		if(other->actorID != 0x00bf)
			return;
	
		Player* player = (Player*)other;
		
		if((hitFlags & 0x400000) && state != 5)
			player->Hurt(pos, DAMAGE, PLAYER_KNOCKBACK, 1, 0, 1);
		
		return;
	}
	
	if(hitFlags & 0x000667c0)
	{
		if(other->actorID == 0x00bf)
			KillByInvincibleChar(KNOCKBACK_ROT, *(Player*)other);
		else
		{
			SpawnCoin();
			KillMagikoopa();
		}
		return;
	}
	else if(hitFlags & CylinderClsn::HIT_BY_SPIN_OR_GROUND_POUND)
	{
		defeatMethod = Enemy::DF_SQUASHED;
		KillByAttack(*other);
		scale.y = 0x0555_f;
		if (!res->isBoss && res->spawnActorID == 0xddd)
		{
			Vector3 actorSpawnPos = Vector3{ 3200._f, 550._f, 4900._f };
			Actor::Spawn(0x022b, 0xf10e, actorSpawnPos, nullptr, areaID, -1);
			Actor::Spawn(0x00b2, 0x0040 + STAR_ID, pos, nullptr, areaID, -1);
		}
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
		player->Bounce(PLAYER_BOUNCE_INIT_VEL);
		if (!res->isBoss && res->spawnActorID == 0xddd)
		{
			Vector3 actorSpawnPos = Vector3{ 3200._f, 550._f, 4900._f };
			Actor::Spawn(0x022b, 0xf10e, actorSpawnPos, nullptr, areaID, -1);
			Actor::Spawn(0x00b2, 0x0040 + STAR_ID, pos, nullptr, areaID, -1);
		}
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
	else if(hitFlags & 0x400000)
	{
		player->Hurt(pos, DAMAGE, PLAYER_KNOCKBACK, 1, 0, 1);
	}
	
}

Vector3 Magikoopa::GetWandTipPos()
{
	return rigMdl.mat4x3(rigMdl.data.transforms[5](Vector3::Temp(0_f, WAND_LENGTH >> 3, 0_f))) << 3;
}

void Magikoopa::State0_Appear()
{
	rigMdl.SetAnim(*animFiles[APPEAR].BCA(), Animation::NO_LOOP, 1._f, 0);
	
	if(!res->isBoss) //The boss should not be hit before talking.
		battleStarted = true;
	if(battleStarted)
		invincible = false;
	else if(res->isBoss)
		AttemptTalkStartIfNotStarted();
	
	if(stateTimer == 0)
	{
		Player* player = ClosestPlayer();
		if(player)
			ang.y = pos.HorzAngle(player->pos);
	}
	if(stateTimer >= APPEAR_TIME - 1)
	{
		state = (res->isBoss && !battleStarted) ? 6 : 1;
	}
}

void Magikoopa::State1_Wave()
{
	rigMdl.SetAnim(*animFiles[WAVE].BCA(), Animation::LOOP, 0x1000_f, 0);
	battleStarted = true;
	invincible = false;

	if (Player* player = ClosestPlayer())
		ApproachAngle(ang.y, pos.HorzAngle(player->pos), 5, 12_deg);

	if(stateTimer >= WAVE_TIME - 1)
	{
		state = 2;
	}
}

void Magikoopa::State2_Shoot()
{
	rigMdl.SetAnim(*animFiles[SHOOT].BCA(), Animation::NO_LOOP, 0x1000_f, 0);
	Player* player = ClosestPlayer();
	if(stateTimer == 2)
	{
		int shotID = -1;
		
		if(!res->isBoss || shotState != 0)
		{
			for(int i = 0; i < 3; ++i) if(!Actor::FindWithID(res->shotUniqueIDs[i]))
			{
				shotID = i;
				break;
			}
		}
		
		if(shotID >= 0 || (res->isBoss && shotState == 0))
		{	
			Vector3 shotPos = GetWandTipPos() - Vector3{0_f, SHOT_RADIUS, 0_f};
			
			// & 0xc000 because of that silly 0x8000 case

			short vertAng, horzAng;

			if (player)
			{
				horzAng = pos.HorzAngle(player->pos);

				if (AngleDiff(horzAng, ang.y) & 0xc000)
				{
					vertAng = -0x4000;
					horzAng = ang.y;
				}
				else
					vertAng = player->pos.VertAngle(shotPos);
			}
			else
			{
				vertAng = -0x1555;
				horzAng = ang.y;
			}

			if(vertAng < -0x4000)
				vertAng = -0x4000;
			if(vertAng > 0x2000)
				vertAng = 0x2000;

			Vector3_16 dir = {
				static_cast<short>((Sin(horzAng) * Cos(vertAng)).val),
				static_cast<short>((Sin(vertAng)).val),
				static_cast<short>((Cos(horzAng) * Cos(vertAng)).val)
			};

			Shot* shot = (Magikoopa::Shot*)Actor::Spawn(KAMEK_SHOT, shotID, shotPos, &dir, areaID, -1);
			if(shot)
			{
				shot->SetMagikoopa(*this);
				if(shotID >= 0)
					res->shotUniqueIDs[shotID] = shot->uniqueID;
				shotState ^= 1;
			}
		}
	}
	if(stateTimer >= SHOOT_TIME - 1)
	{
		state = 3;
	}
}

void Magikoopa::State3_Poof()
{
	rigMdl.SetAnim(*animFiles[POOF].BCA(), Animation::NO_LOOP, 0x1000_f, 0);
	if(stateTimer >= POOF_TIME - 1)
	{
		state = 4;
	}
}

void Magikoopa::State4_Teleport()
{
	if(stateTimer == 0)
	{
		if(battleStarted)
			DisappearPoofDustAt(Vector3{pos.x, pos.y + (HEIGHTS[res->isBoss] >> 1), pos.z});
		flags &= ~Actor::AIMABLE_BY_EGG;
	}
	
	if(stateTimer >= TELEPORT_TIME || !battleStarted)
	{
		stateTimer = TELEPORT_TIME; //prevent overflow
		
		Player* player = ClosestPlayer();
		if(!res->isBoss || battleStarted)
		{
			if((player && (pos.Dist(player->pos) <= TP_PLAYER_DIST_TOL || (!res->isBoss && pos.Dist(player->pos) > TP_PLAYER_DIST_MAX))))
				return;
		}
		else if(!player || originalPos.Dist(player->pos) > BOSS_START_BATTLE_RADIUS || player->currClsnState != 1)
			return;
		
		currPathPt = nextPathPt;
		PoofDustAt(Vector3{pos.x, pos.y + (HEIGHTS[res->isBoss] >> 1), pos.z});
		flags |= Actor::AIMABLE_BY_EGG;
		state = 0;
	}
}

void Magikoopa::State5_Hurt()
{
	rigMdl.SetAnim(*animFiles[HURT].BCA(), Animation::NO_LOOP, 0x1000_f, 0);
	if(health == 0)
		AttemptTalkStartIfNotStarted();
	if(stateTimer >= HURT_TIME)
		state = health == 0 ? 7 : 3;
}

void Magikoopa::State6_Wait()
{
	rigMdl.SetAnim(*animFiles[WAIT].BCA(), Animation::LOOP, 0x1000_f, 0);
	Talk();
	if(state == 6)
		AttemptTalkStartIfNotStarted();
}

void Magikoopa::State7_Defeat()
{
	rigMdl.SetAnim(*animFiles[DEFEAT].BCA(), Animation::NO_LOOP, 0x1000_f, 0);
	Talk();
	if(!shouldBeKilled)
		AttemptTalkStartIfNotStarted();
}

void Magikoopa::AttemptTalkStartIfNotStarted()
{
	Player* player = ClosestPlayer();
	if(player->StartTalk(*this, true))
	{
		Message::PrepareTalk();
		listener = player;
	}
}

void Magikoopa::Talk()
{
	if(!listener)
		return;
	
	int talkState = listener->GetTalkState();
	switch(talkState)
	{
		case Player::TK_NOT:
			Message::EndTalk();
			listener = nullptr;
			if(state == 7)
			{
				KillMagikoopa();
				Sound::StopLoadedMusic_Layer3();
				if (starID < 8)
					Actor::Spawn(0x00b2, (0x0040 + starID), Vector3{pos.x, pos.y + 250._f, pos.z}, nullptr, 0, -1);
				else if (starID < 13)
					Actor::Spawn(0x011a, (0x0000 + starID - 8), Vector3{pos.x, pos.y + 250._f, pos.z}, nullptr, 0, -1);
			}
			else
			{
				state = 1;
				Sound::LoadAndSetMusic_Layer3(MU_BOSS);
			}
			break;
			
		case Player::TK_START:
		{
			const Vector3 lookAt = {pos.x, pos.y + TALK_HEIGHT, pos.z};

			listener->ShowMessage(
				*this,
				state == 7 ? defeatMsgIDs[listener->param1] : waitMsgIDs[listener->param1],
				&lookAt, 0, 0
			);
			break;
		}
		default:
			return;
	}
}

int Magikoopa::Behavior()
{
	if (!Particle::Manager::LoadTex(KAMEK_MAGIC_PARTICLE_ID, SPRITE_ID)) //loads it the first time this is called
		return 0;
	
	if(UpdateYoshiEat(wmClsn)) //will set isBeingSpit to false if the magikoopa runs into ground
	{
		cylClsn.Clear();
		
		rigMdl.SetAnim(*animFiles[POOF].BCA(), Animation::NO_LOOP, 0x1000_f, 0);
		rigMdl.currFrame = 0_f;
		speed.y = 1_f;
	
		if(isBeingSpit && spitTimer == 0)
		{
			horzSpeed.ApproachLinear(0_f, SPIT_DECEL);
			cylClsn.Update();
		}
		if(horzSpeed == 0_f || !isBeingSpit)
		{
			horzSpeed = 0_f;
			isBeingSpit = false;
			state = 3;
			stateTimer = 0;
		}
		UpdateModelTransform();
		return 1;
	}
	
	if(defeatMethod != Enemy::DF_NOT)
	{
		int res = UpdateKillByInvincibleChar(wmClsn, rigMdl, 3);
		if(res == 2) //finished kill
		{
			KillMagikoopa();
		}
		else if(res == 0) //not yet
		{
			UpdateDeath(wmClsn);
			UpdateModelTransform();
		}
		return 1;
	}
	
	int prevState = state;
	
	//Dun, dun, dun!
	//----------------------------//
	(this->*stateFuncs[state])(); // dat syntax tho
	//----------------------------//
	
	++stateTimer;
	if(state != prevState)
		stateTimer = 0;
	
	Player* player = ClosestPlayer();
	if(res->isBoss && battleStarted && (!player || originalPos.Dist(player->pos) >= BOSS_STOP_BATTLE_RADIUS) && health != 0)
	{
		state = 4;
		stateTimer = 0;
		battleStarted = false;
		invincible = true;
		health = 3;
		shotState = 0;
		currPathPt = numPathPts;
		Sound::StopLoadedMusic_Layer3();
	}
	if(state == 4)
	{
		if(stateTimer >= TELEPORT_TIME)
		{
			if(currPathPt < numPathPts)
			{
				nextPathPt = (unsigned)RandomInt() % (numPathPts - 1);
				if(nextPathPt >= currPathPt)
					++nextPathPt;
			}
			else
				nextPathPt = 0;
			pathPtr.GetNode(pos, nextPathPt);
		}
		return 1;
	}
	
	UpdatePos(nullptr);
	UpdateModelTransform();
	
	Vector3 wandTip = GetWandTipPos();
	shapesID = Particle::System::New(shapesID, (unsigned)&particleSysDefs[res->isBoss], wandTip.x, wandTip.y, wandTip.z, nullptr, nullptr);
	
	//UpdateWMClsn(&wmClsn, 2);
	HandleClsn(); //must be done before clearing collision, of course
	
	cylClsn.Clear();
	if(defeatMethod == Enemy::DF_NOT)
		cylClsn.Update();
	
	MakeVanishLuigiWork(cylClsn);
	
	rigMdl.Advance();
	
	return 1;
}

int Magikoopa::Render()
{
	if(state == 4 || (flags & Actor::IN_YOSHI_MOUTH))
		return 1;
		
	rigMdl.Render(&scale);
	return 1;
}

void Magikoopa::OnPendingDestroy()
{
	if(eventToTrigger < 0x20)
		Event::SetBit(eventToTrigger);
}

unsigned Magikoopa::OnYoshiTryEat()
{
	return state == 4 || res->isBoss ? Actor::YE_DONT_EAT : Actor::YE_KEEP_AND_CAN_MAKE_EGG;
}

void Magikoopa::OnTurnIntoEgg(Player& player)
{
	if(player.IsCollectingCap())
		GivePlayerCoins(player, NUM__COINS_MAGIKOOPA, 0);
	else
		player.RegisterEggCoinCount(NUM__COINS_MAGIKOOPA, false, false);
	
	KillMagikoopa();
}

Fix12i Magikoopa::OnAimedAtWithEgg()
{
	return HEIGHTS[res->isBoss] >> 1;
}
