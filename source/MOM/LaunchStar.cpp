#include "LaunchStar.h"
#include "MOM_Interface.h"
#include <algorithm>
#include <memory>

constinit std::array<LaunchStar::Color, 5> LaunchStar::colors =
{{
	{ // Ice
		Command30 {
			.diffuse = Color5Bit(0x00, 0x98, 0xff),
			.ambient = Color5Bit(0x00, 0x48, 0xE0)
		},
		Command31 {
			.specular = Color5Bit(0xF8, 0xF8, 0xF8),
			.emission = Color5Bit(0x00, 0x00, 0x00)
		}
	},
	{ // Orange
		Command30 {
			.diffuse = Color5Bit(0xF8, 0xA8, 0x00),
			.ambient = Color5Bit(0xF8, 0x68, 0x00)
		},
		Command31 {
			.specular = Color5Bit(0xF8, 0xF8, 0x00),
			.emission = Color5Bit(0x00, 0x10, 0x00)
		}
	},
	{ // Pink
		Command30 {
			.diffuse = Color5Bit(0x00, 0x00, 0xF8),
			.ambient = Color5Bit(0xF8, 0x50, 0x40)
		},
		Command31 {
			.specular = Color5Bit(0xF8, 0xF8, 0xF8),
			.emission = Color5Bit(0x18, 0x00, 0x20)
		}
	},
	{ // Green
		Command30 {
			.diffuse = Color5Bit(0xF8, 0xF8, 0x00),
			.ambient = Color5Bit(0x00, 0x00, 0x20)
		},
		Command31 {
			.specular = Color5Bit(0xF8, 0xF8, 0xC0),
			.emission = Color5Bit(0x00, 0xB8, 0x00)
		}
	},
	{ // Blue
		Command30 {
			.diffuse = Color5Bit(0x60, 0x20, 0xF8),
			.ambient = Color5Bit(0x00, 0x00, 0x90)
		},
		Command31 {
			.specular = Color5Bit(0x30, 0x70, 0xF8),
			.emission = Color5Bit(0x00, 0x00, 0x00)
		}
	}
}};

enum Animations
{
	WAIT,
	LAUNCH,
	
	NUM_ANIMS
};

constinit SharedFilePtr LaunchStar::modelFile;
constinit SharedFilePtr LaunchStar::animFiles[2];
constinit BezierPathIter LaunchStar::bzIt = {};
constinit LaunchStar* launchingLS = nullptr;
constinit Vector3 LaunchStar::lsInitPos;
constinit Vector3 LaunchStar::lsPos;
constinit Vector3_16 LaunchStar::lsDiffAng;
constinit Vector3_16 LaunchStar::lsInitAng;
constinit unsigned LaunchStar::particleID = 0;
constexpr Fix12i clsnSize = 160._f;

constinit SpawnInfo LaunchStar::spawnData =
{
	[]() -> ActorBase* { return new LaunchStar; },
	0x0168,
	0x00aa,
	Actor::NO_BEHAVIOR_IF_OFF_SCREEN | Actor::NO_RENDER_IF_OFF_SCREEN | Actor::UPDATE_DURING_STAR_SPAWNING | Actor::UPDATE_DURING_CUTSCENES,
	0_f,
	0x600'000_f,
	Fix12i::max,
	Fix12i::max
};

struct LS_Init {};
struct LS_Behavior {};
struct LS_Cleanup {};

template<>
bool Player::CustomStateFunc<LS_Init>()
{
	if (launchingLS)
	{
		SetLaunchStarPtr(*this, launchingLS);

		LaunchStar::lsPos = launchingLS->pos;
		LaunchStar::lsPos.y += clsnSize;
		LaunchStar::lsDiffAng.x = launchingLS->ang.x - ang.x;
		LaunchStar::lsDiffAng.y = launchingLS->ang.y - ang.y;
		LaunchStar::lsDiffAng.z = launchingLS->ang.z - ang.z;
		LaunchStar::bzIt.metric = Vec3_Dist;

		Sound::PlayArchive3(0x61, camSpacePos);
	}
	else
	{
		cylClsn.flags1 |= CylinderClsn::DISABLED;

		LaunchStar::bzIt.pathPtr.FromID(8);
		LaunchStar::bzIt.currSplineX3 = 0;
		LaunchStar::bzIt.tinyStep = 0x0'010_fs;
		LaunchStar::bzIt.step = horzSpeed;
		LaunchStar::bzIt.currTime = 0_f;
		LaunchStar::bzIt.pos = LaunchStar::bzIt.pathPtr.GetNode(0);

		LaunchStar::bzIt.metric = [](const Vector3& v0, const Vector3& v1)
		{
			return Abs(v0.z - v1.z);
		};

		LaunchStar::particleID = 0;
		unk763 = 3; // launchState

		// Sound::PlayArchive0(0x0b9, camSpacePos);
	}

	horzSpeed = vertAccel = 0_f;
	LaunchStar::lsInitPos = pos;
	LaunchStar::lsInitAng = ang;

	SetAnim(0x5f, Animation::NO_LOOP, 1._f, 0);

	return true;
}

/*void Vector3::Lerp(const Vector3& a, const Vector3& b, Fix12i t)
{
    *this = b;
    *this -= a;
    *this *= t;
    *this += a;
}*/

template<>
bool Player::CustomStateFunc<LS_Behavior>()
{
	u8& lsState0Timer = unk762;
	u8& launchState = unk763;

	Fix12s t = Fix12s(lsState0Timer) >> 2;
	speed.y = 0_f;
	LaunchStar* lsPtr = GetLaunchStarPtr(*this);

	if (launchState == 1 || launchState == 2 && lsPtr)
	{
		lsPtr->rigMdl.data.UpdateBones(lsPtr->rigMdl.file, (int)lsPtr->rigMdl.currFrame);

		pos = lsPtr->rigMdl.mat4x3(lsPtr->rigMdl.data.bones[1].pos) << 3;
	}

	switch(launchState)
	{
	case 0:
		pos = Lerp(LaunchStar::lsInitPos, LaunchStar::lsPos, t);

		ang.x = (Fix12s{LaunchStar::lsDiffAng.x, as_raw} * t).val + LaunchStar::lsInitAng.x;
		ang.y = (Fix12s{LaunchStar::lsDiffAng.y, as_raw} * t).val + LaunchStar::lsInitAng.y;
		ang.z = (Fix12s{LaunchStar::lsDiffAng.z, as_raw} * t).val + LaunchStar::lsInitAng.z;

		++lsState0Timer;
		if (lsState0Timer > 4 && lsPtr)
		{
			lsPtr->rigMdl.SetAnim(*reinterpret_cast<BCA_File*>(LaunchStar::animFiles[LAUNCH].filePtr), Animation::NO_LOOP, 1._f, 0x00000000);
			launchState = 1;
		}
		break;
	case 1:
		if (lsPtr && lsPtr->rigMdl.currFrame >= 0x14000_f)
		{
			lsPtr->rigMdl.speed = lsPtr->launchSpeed / 8;
			Sound::PlayArchive3(0x14f, camSpacePos);
			Sound::PlayArchive3(0x0c5, camSpacePos);
			Sound::PlayArchive0(0x0b9, camSpacePos);
			launchState = 2;
		}
		break;
	case 2:
		if (lsPtr && lsPtr->rigMdl.currFrame >= 0x2f000_f)
		{
			lsPtr->rigMdl.currFrame = 0x2f000_f;
			lsPtr->rigMdl.speed = 1._f;
			pos = LaunchStar::lsPos;

			LaunchStar::bzIt.pathPtr = lsPtr->pathPtr;
			LaunchStar::bzIt.currSplineX3 = 0;
			LaunchStar::bzIt.tinyStep = 0x0010_fs;
			LaunchStar::bzIt.step = lsPtr->launchSpeed;
			LaunchStar::bzIt.currTime = 0_f;
			LaunchStar::bzIt.pos = LaunchStar::lsPos;

			Vector3_16f zeros {0_fs, 0_fs, 0_fs};
			LaunchStar::particleID = Particle::System::New(LaunchStar::particleID, 0x114, pos.x, pos.y, pos.z, &zeros, nullptr);
			launchState = 3;
		}
		break;
	case 3:
		{
			bool finished = !LaunchStar::bzIt.Advance();
			pos = LaunchStar::bzIt.pos;

			if (!lsPtr)
			{
				pos.z <<= 4;

				if (LaunchStar::bzIt.currSplineX3 >= 24)
					LaunchStar::bzIt.metric = Vec3_Dist;
			}

			ang.y = motionAng.y = prevPos.HorzAngle(pos);
			ang.x = 0x4000 - pos.VertAngle(prevPos);

			if (eggPtrArr && eggPtrArr[0])
			{
				if (pos.z < 0._f || pos.y > 500._f)
					eggPtrArr[0]->pos = pos;

				eggPtrArr[0]->flags |= Actor::UPDATE_DURING_CUTSCENES;
			}

			Vector3_16f zeros {0_fs, 0_fs, 0_fs};
			LaunchStar::particleID = Particle::System::New(LaunchStar::particleID, 0x114, pos.x, pos.y, pos.z, &zeros, nullptr);

			if (LEVEL_ID == 40 && PARTICLE_SYS_TRACKER)
			{
				const auto* sysData = PARTICLE_SYS_TRACKER->contents.FindData(LaunchStar::particleID);

				if (sysData && sysData->system)
				{
					const Vector3 diff = pos - prevPos;

					if (finished)
					{
						sysData->system->lifetime = 10;

						for (auto& particle : sysData->system->particleList)
							if (particle.lifetime > 10)
								particle.lifetime = 10;
					}
					else if (pos.z < -340000._f)
					{
						for (auto& particle : sysData->system->particleList)
							particle.posAsr3 += diff * 0.98_f >> 3;
					}
					else
					{
						sysData->system->lifetime = 200;

						static constexpr Fix12i relSpeed = 40._f;
						const Vector3 offset = diff - diff.Normalized() * relSpeed;

						for (auto& particle : sysData->system->particleList)
						{
							static constexpr int frames = 30;

							if (particle.age < frames)
								particle.posAsr3 += (1._f - Sqr(SmoothStep(Fix12i{particle.age} / frames))) * offset >> 3;
						}
					}
				}
			}

			if (finished)
			{
				const short ang = pos.VertAngle(LaunchStar::bzIt.pathPtr[LaunchStar::bzIt.currSplineX3 + 2]);

				const bool shouldDive = LEVEL_ID == 40 && pos.y < 7000._f;

				if (shouldDive)
				{
					ChangeState(ST_DIVE);

					auto& model = *bodyModels[currHatChar];
					Fix12i numFrames = Fix12i{model.numFramesAndFlags.val & 0x0fffffff, as_raw};
					model.currFrame = numFrames - 1._f;
				}

				const int shift = lsPtr ? 0 : 2;

				horzSpeed = (Cos(ang) * LaunchStar::bzIt.step) << shift;
				speed.y   = (Sin(ang) * LaunchStar::bzIt.step) << shift;
				
				extern char* RUNNING_KUPPA_SCRIPT;
				if (LEVEL_ID == 40 && RUNNING_KUPPA_SCRIPT)
					horzSpeed += 20._f;

				if (!shouldDive)
					ChangeState(ST_FALL);
			}
			break;
		}
	}

	prevPos = pos;

	return true;
}

template<>
bool Player::CustomStateFunc<LS_Cleanup>()
{
	cylClsn.flags1 &= ~CylinderClsn::DISABLED;

	return nextState != &Player::ST_SWIM &&
	       nextState != &Player::ST_HURT &&
	       nextState != &Player::ST_HURT_WATER;
}

constinit Player::State LaunchStar::playerState =
{
	&Player::CustomStateFunc<LS_Init>,
	&Player::CustomStateFunc<LS_Behavior>,
	&Player::CustomStateFunc<LS_Cleanup>
};

void LaunchStar::Launch(Player& player, LaunchStar& launchStar)
{
	if (player.currState == &LaunchStar::playerState && GetLaunchStarPtr(player) == &launchStar)
		return;

	launchingLS = &launchStar;
	player.ChangeState(LaunchStar::playerState);
	launchingLS = nullptr;

	SetLaunchStarPtr(player, &launchStar);

	if (LEVEL_ID == 39)
		Sound::ChangeMusicVolume(0, 1.05_f);

	if (LEVEL_ID == 39 || LEVEL_ID == 40)
		MOM_Interface::instance.camRotationDisabled = true;
}

void LaunchStar::UpdateModelTransform()
{
	Model& model = rigMdl;
	model.mat4x3 = Matrix4x3::RotationZXY(ang);
	model.mat4x3.c3 = pos >> 3;
}

[[gnu::target("thumb")]]
int LaunchStar::InitResources()
{
	GXFIFO::SetLightColor(1, 0xff, 0xff, 0xff);

	rigMdl.SetFile(modelFile.LoadBMD(), 1, -1);

	const Color& color = colors[std::clamp((unsigned)ang.z, 0u, colors.size() - 1)];

	rigMdl.data.materials[0].difAmb = color.difAmb;
	rigMdl.data.materials[0].speEmi = color.speEmi;

	animFiles[LAUNCH].LoadBCA();
	BCA_File& waitAnim = animFiles[WAIT].LoadBCA();

	rigMdl.SetAnim(waitAnim, Animation::LOOP, 1._f, 0);
	
	cylClsn.Init(this, clsnSize, 2 * clsnSize, 0x00100002, 0x00008000);
	
	launchSpeed = Fix12i(ang.x);
	eventID = param1 >> 8 & 0xff;
	particleID = 0;
	flags--;
	
	pathPtr.FromID(param1 & 0xff);
	const Vector3 p0 = pathPtr[0];
	const Vector3 p1 = pathPtr[1];
	
	ang.x = 0x4000 - p1.VertAngle(p0);
	ang.y = p0.HorzAngle(p1);
	ang.z = 0;
	
	UpdateModelTransform();
	
	pos.y -= 160.0_f; // silly cylinder colliders that can't be offset
	
	return 1;
}

[[gnu::target("thumb")]]
int LaunchStar::CleanupResources()
{
	GXFIFO::SetLightColor(1, 0, 0, 0);

	animFiles[WAIT].Release();
	animFiles[LAUNCH].Release();
	modelFile.Release();

	if (LEVEL_ID == 40)
		Event::ClearBit(31);

	return 1;
}

constexpr unsigned spawnCamStart = 25;
constexpr unsigned spawnCamEnd = spawnCamStart + 90;

bool LaunchStar::BeforeBehavior()
{
	if (eventID < 0x20 && Event::GetBit(eventID))
	{
		Event::ClearBit(eventID);
		spawnTimer = 0;

		if (AREAS)
			areaWasShowing = AREAS[areaID].showing;
	}

	return Actor::BeforeBehavior() || spawnTimer != 0xff;
}

asm("msgSoundTargetVolume = 0x0209b470");
asm("bgmTargetVolume = 0x0208e42c");
extern char msgSoundTargetVolume;
extern char bgmTargetVolume;

int LaunchStar::Behavior()
{
	switch (LEVEL_ID)
	{
	case 14:
		GXPORT_LIGHT_VECTOR = 0x63D3686A;
		break;
	case 27:
		GXPORT_LIGHT_VECTOR = 0x63337FFD;
		break;
	case 15:
		GXPORT_LIGHT_VECTOR = 0x6421D8DD;
		break;
	case 16:
		GXPORT_LIGHT_VECTOR = 0x61827000;
		break;
	case 39:
	case 40:
		GXPORT_LIGHT_VECTOR = 0x696a5800;
		break;
	}

	if (spawnCamStart <= spawnTimer && spawnTimer < spawnCamEnd)
	{
		if (AREAS) AREAS[areaID].showing = true;

		if (spawnTimer == spawnCamStart)
		{
			eventID = 0xff;
			savedCamPos = CAMERA->pos;
			savedCamTarget = CAMERA->lookAt;
			savedCamOffset = CAMERA->lookAtOffset;
			CAMERA->flags |= Camera::BOSS_TALK;

			NEXT_ACTOR_UPDATE_FLAGS |= Actor::UPDATE_DURING_STAR_SPAWNING;
		}

		CAMERA->lookAt = pos;

		if (LEVEL_ID == 40)
		{
			static constexpr std::array<Vector3, 3> positions =
			{{
				{  348._f,  1893._f, 10890._f},
				{  863._f,  -675._f, -2878._f},
				{-1498._f, -2193._f, -5322._f},
			}};

			CAMERA->pos = positions[(param1 & 0xff) - 1];
			CAMERA->lookAt.y += clsnSize;
		}
		else
		{
			CAMERA->pos = pos;
			CAMERA->pos.y += 0x250'000_f;
			CAMERA->pos.x += Sin(ang.x) * 0x350'000_f;
			CAMERA->pos.z += Cos(ang.y) * 0x350'000_f;
		}

		// Play the 'secret found' sound effect
		if (spawnTimer == spawnCamStart + 14)
			Sound::PlaySub(MU_KEY_GET_2, 0x40, 0x7f, 107._f, false);
	}
	else if (spawnTimer == spawnCamEnd)
	{
		spawnTimer = 0xff;
		CAMERA->lookAt = savedCamTarget;
		CAMERA->pos = savedCamPos;
		CAMERA->lookAtOffset = savedCamOffset;
		CAMERA->flags &= ~Camera::BOSS_TALK;

		NEXT_ACTOR_UPDATE_FLAGS &= ~Actor::UPDATE_DURING_STAR_SPAWNING;
		msgSoundTargetVolume = 0;
		bgmTargetVolume = 0x7f;

		if (AREAS)
			AREAS[areaID].showing = areaWasShowing;
	}

	if (spawnTimer != 0xff) spawnTimer++;
	if (eventID < 0x20)	return 1;

	if (Player* player = PLAYER_ARR[0])
	{
		if (player->uniqueID == cylClsn.otherObjID &&
			cylClsn.hitFlags & CylinderClsn::HIT_BY_CHAR_BODY &&
			INPUT_ARR[0].buttonsPressed & Input::A &&
			player->currState != &Player::ST_FIRST_PERSON)
		{
			Launch(*player, *this);
		}
	}
	
	if (reinterpret_cast<char*>(rigMdl.file) == animFiles[LAUNCH].filePtr &&
		rigMdl.currFrame + 1_f >= Fix12i{rigMdl.numFramesAndFlags.val & 0x0fffffff, as_raw}
	)
		rigMdl.SetAnim(*reinterpret_cast<BCA_File*>(animFiles[WAIT].filePtr), Animation::LOOP, 1._f, 0);
	
	rigMdl.Advance();
	cylClsn.Clear();
	cylClsn.Update();

	return 1;
}

int LaunchStar::Render()
{
	if (eventID >= 0x20) rigMdl.Render();
	
	return 1;
}
