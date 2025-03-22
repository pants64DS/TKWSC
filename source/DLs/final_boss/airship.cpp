#include "airship.h"
#include "MOM_Interface.h"

SpawnInfo Airship::spawnData =
{
	+[]() -> ActorBase* { return new Airship; },
	0x1000, // behavPriority
	0x1000, // renderPriority
	Actor::UPDATE_DURING_STAR_SPAWNING | Actor::UPDATE_DURING_CUTSCENES,
	0._f,   // rangeOffsetY
	-1._f,  // range
	0._f,   // drawDist
	0._f    // unkc0
};

using AirshipCLPS = StaticCLPS_Block<
	{ .textureID = CLPS::TX_WOOD, .camBehavID = CLPS::CA_EIGHT_DIRECTIONS },
	{ .camBehavID  = CLPS::CA_EIGHT_DIRECTIONS }
>;

void Airship::AfterClsn(MeshColliderBase& clsn, Actor* clsnActor, Actor* otherActor)
{
	if (otherActor && otherActor->actorID == 0xbf &&
		clsnActor && clsnActor->actorID == staticActorID)
	{
		Airship& airship = static_cast<Airship&>(*clsnActor);

		airship.player = static_cast<Player*>(otherActor);

		if (airship.interp > 0._f)
			MOM_Interface::instance.camRotationDisabled = false;
	}
}

int Airship::InitResources()
{
	model.SetFile(modelFile.LoadBMD(), 1, -1);
	model.mat4x3 = Matrix4x3::RotationY(ang.y);
	model.mat4x3.c3 = pos >> 3;

	clsn.SetFile(&clsnFile.LoadKCL(),
		clsnTransform, 1._f, ang.y, AirshipCLPS::instance<>
	);
	clsnTransform = Matrix4x3::RotationY(ang.y);
	clsnTransform.c3 = pos;
	clsn.afterClsnCallback = AfterClsn;
	clsn.Enable(this);

	wingCtrl.SetFile();

	shutterModel.SetFile(shutterModelFile.LoadBMD(), 1, -1);
	shutterModel.mat4x3 = Matrix4x3::RotationY(ang.y);
	shutterModel.mat4x3.c3 = model.mat4x3.c3;
	
	leftShutterClsnMat = Matrix4x3::RotationY(ang.y);
	leftShutterClsnMat.c3 = pos;
	leftShutterClsnMat.c3.y += shutterVertOffset;
	rightShutterClsnMat = leftShutterClsnMat;

	KCL_File& shutterKCL = shutterClsnFile.LoadKCL();

	leftShutterClsn.SetFile(&shutterKCL,
		leftShutterClsnMat, 1._f, ang.y, AirshipCLPS::instance<>
	);
	leftShutterClsn.afterClsnCallback = AfterClsn;
	leftShutterClsn.Enable(this);

	rightShutterClsn.SetFile(&shutterKCL,
		rightShutterClsnMat, 1._f, ang.y, AirshipCLPS::instance<>
	);
	rightShutterClsn.afterClsnCallback = AfterClsn;
	rightShutterClsn.Enable(this);

	auto& numPathNodes = PathPtr(param1 + 1)->numNodes;
	ogNumPathNodes = numPathNodes;
	const_cast<uint8_t&>(numPathNodes) -= (param1 == 2 ? 6 : 3);
	
	return 1;
}

int Airship::CleanupResources()
{
	clsn.Disable();
	leftShutterClsn.Disable();
	rightShutterClsn.Disable();
	modelFile.Release();
	clsnFile.Release();
	wingCtrl.ReleaseFile();
	shutterModelFile.Release();
	shutterClsnFile.Release();

	return 1;
}

asm("msgSoundTargetVolume = 0x0209b470");
asm("bgmTargetVolume = 0x0208e42c");
extern char msgSoundTargetVolume;
extern char bgmTargetVolume;

static bool IsCrewMember(const Actor& actor)
{
	switch (actor.actorID)
	{
	case 0xce: // Bob-omb
	case MOM_IDs::COLORED_GOOMBA:
	case MOM_IDs::COLORED_GOOMBA_LARGE:
	case MOM_IDs::KAMEK_SHOT:
	case MOM_IDs::KAMEK:
		return true;
	}

	return false;
}

static bool IsOnBoard(const Airship& airship, const Actor& actor)
{
	const auto [x, y, z] = Vector3(actor.pos - airship.pos);

	return y > -700._f &&
		-1000._f < x && x < 1000._f &&
		-700._f < z && z < 5300._f;
}

static constexpr std::array<Vector3, 3> wingOffsets =
{{
	{ 584._f, -920._f, 2616._f },
	{ 590._f, -896._f, 2024._f },
	{ 590._f, -872._f, 1408._f }
}};

static constexpr Vector3 soundOffset =
{
	(wingOffsets[0].x + wingOffsets[1].x + wingOffsets[2].x) / 3,
	(wingOffsets[0].y + wingOffsets[1].y + wingOffsets[2].y) / 3,
	(wingOffsets[0].z + wingOffsets[1].z + wingOffsets[2].z) / 3
};

static void EnableWind()
{
	(*LEVEL_OVERLAY.clps)[11].behaviorID = 11;
	(*LEVEL_OVERLAY.clps)[11].windID = 7;
}

static void DisableWind()
{
	(*LEVEL_OVERLAY.clps)[11].behaviorID = 4;
	(*LEVEL_OVERLAY.clps)[11].windID = 255;
}

int Airship::Behavior()
{
	model.mat4x3.c3 = pos >> 3;
	shutterModel.mat4x3.c3 = leftShutterClsnMat.c3 >> 3;

	const Vector3 oldPos = pos;
	pos = Lerp(startingPos, destination, interp);

	if (interp > 0._f && interp.ApproachLinear(1._f, interpStep) && !cannon)
	{
		const Vector3_16 cannonAngle = {
			0,
			static_cast<short>(ang.y - 180_deg),
			Atan2(-pos.x, -pos.z)
		};

		cannon = Actor::Spawn(207, 2, pos, &cannonAngle, areaID);

		msgSoundTargetVolume = 0;

		crew.ForEach([&](Actor& actor)
		{
			if (IsCrewMember(actor))
				actor.MarkForDestruction();
		});

		crew.Clear();
	}

	if (cannon && shutterHorzOffset.ApproachLinear(shutterMaxHorzOffset, 10._f))
	{
		leftShutterClsn .Disable();
		rightShutterClsn.Disable();
	}

	if (player)
	{
		const auto* const meshClsn = player->wmClsn.sphere.floorResult.meshClsn;

		if (meshClsn &&
			meshClsn != &clsn &&
			meshClsn != &leftShutterClsn &&
			meshClsn != &rightShutterClsn)
		{
			player = nullptr;
			DisableWind();

			crew.ForEach([&](Actor& actor)
			{
				switch (actor.actorID)
				{
				case MOM_IDs::COLORED_GOOMBA:
				case MOM_IDs::COLORED_GOOMBA_LARGE:
					actor.flags |= Actor::NO_BEHAVIOR_IF_OFF_SCREEN;
				}
			});

			if (CAMERA)
				CAMERA->eightDirDeltaAngle = CAMERA->eightDirAngleY = 0;

			MOM_Interface::instance.camRotationDisabled = false;
		}
	}

	if (player && interp == 0._f)
	{
		if (player->currState == &Player::ST_SPIN &&
			player->pos.y > 6600._f &&
			Abs(player->pos.x - pos.x) > 4000._f
		)
			EnableWind();
		else
			DisableWind();

		bool containsEnemies = false;

		crew.ForEach([&](Actor& actor)
		{
			if (IsCrewMember(actor) && IsOnBoard(*this, actor))
			{
				containsEnemies = true;
				actor.flags &= ~Actor::NO_BEHAVIOR_IF_OFF_SCREEN;
			}
		});

		if (!containsEnemies)
		{
			interp = interpStep;

			const_cast<uint8_t&>(PathPtr(param1 + 1)->numNodes) = ogNumPathNodes;

			DisableWind();
			Sound::PlaySub(MU_CORRECT_SOLUTION, 0x40, 0x7f, 107._f, false);
		}
	}
	else if (0._f < interp && interp < 1._f)
	{
		if (interp > 0.2_f)
			bgmTargetVolume = 0x7f;

		const Vector3 posOffset = pos - oldPos;

		// move yellow and blue coins with the ship
		crew.ForEach([&](Actor& actor)
		{
			if (actor.actorID == 0x120 || actor.actorID == 0x122)
				actor.pos += posOffset;
		});

		if (player)
		{
			player->pos                   += posOffset;
			player->wmClsn.sphere.pos     += posOffset;
			player->wmClsn.line.line.pos0 += posOffset;
			player->wmClsn.line.line.pos1 += posOffset;
			player->floorY                += posOffset.y;
			player->jumpPeakHeight        += posOffset.y;
		}
	}

	clsnTransform.c3 = pos;
	clsn.Transform(clsnTransform, ang.y);
	
	if (shutterHorzOffset < shutterMaxHorzOffset)
	{
		AssureUnaliased(leftShutterClsnMat .c3) = pos + shutterHorzOffset * clsnTransform.c0;
		AssureUnaliased(rightShutterClsnMat.c3) = pos - shutterHorzOffset * clsnTransform.c0;
		leftShutterClsnMat .c3.y += shutterVertOffset;
		rightShutterClsnMat.c3.y += shutterVertOffset;

		leftShutterClsn .Transform(leftShutterClsnMat,  ang.y);
		rightShutterClsn.Transform(rightShutterClsnMat, ang.y);
	}

	wingCtrl.Update(0._f < interp && interp < 1._f, soundOffset);

	return 1;
}

int Airship::Render()
{
	model.Render();
	wingCtrl.Render<1.5_f>(wingOffsets.begin(), wingOffsets.end());

	if (shutterHorzOffset < shutterMaxHorzOffset)
	{
		shutterModel.Render();
		shutterModel.mat4x3.c3.x = (model.mat4x3.c3.x << 1) - shutterModel.mat4x3.c3.x;
		shutterModel.mat4x3.c3.z = (model.mat4x3.c3.z << 1) - shutterModel.mat4x3.c3.z;
		shutterModel.Render();
	}

	return 1;
}
