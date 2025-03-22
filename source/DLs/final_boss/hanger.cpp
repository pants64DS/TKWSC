#include "final_boss_dl.h"

SpawnInfo Hanger::spawnData =
{
	+[]() -> ActorBase* { return new Hanger; },
	0x100, // behavPriority
	0x100, // renderPriority
	0,     // flags
	0_f,   // rangeOffsetY
	-1._f, // range
	0_f,   // drawDist
	0_f    // unkc0
};

int Hanger::InitResources()
{
	char* file = ropeModelFile.Load();

	static constexpr uint16_t bmdOffsets[] =
	{
		0x000,
		0x194,
		0x32C,
		0x4C4
	};

	if (ropeModelFile.numRefs == 1)
	{
		for (unsigned offset : bmdOffsets)
			reinterpret_cast<BMD_File*>(file + offset)->InitPointers();
	}

	ropeModel.SetFile(*reinterpret_cast<BMD_File*>(file + bmdOffsets[ang.x & 3]), 1, -1);

	ropeModel.mat4x3.c0.x = ropeThickness >> 3;
	ropeModel.mat4x3.c1.y = -ropeClsn.height >> 3;
	ropeModel.mat4x3.c2.z = -ropeThickness >> 3;
	ropeModel.mat4x3.c3 = pos >> 3;

	if (areaID == 2)
		ropeModel.data.materials[0].polygonAttr |= Material::RENDER_POLYGON_BACK;

	ropeClsn.MovingCylinderClsn::Init(this, ropeClsnRadius, ropeStartLength, 0x80000c, 0);
	ropeClsn.pos = Vector3::Temp(pos.x, pos.y - ropeStartLength, pos.z);

	RaycastGround ray;
	ray.SetObjAndPos(pos, this);
	ray.DetectClsn();
	floorY = ray.clsnPosY;

	return 1;
}

int Hanger::CleanupResources()
{
	ropeModelFile.Release();
	
	return 1;
}

int Hanger::Behavior()
{
	if (recovering)
		recovering = !ropeClsn.height.ApproachLinear(ropeStartLength, recoverySpeed);

	Actor* clsnActor = Actor::FindWithID(ropeClsn.otherObjID);

	if (clsnActor && clsnActor->actorID == 0xbf)
	{
		Player& player = static_cast<Player&>(*clsnActor);

		if (player.currState == &Player::ST_CLIMB)
		{
			if (recovering)
				player.ChangeState(Player::ST_FALL);
			else if (loaded)
			{
				const Fix12i deltaY = player.pos.y - player.prevPos.y;

				if (deltaY < 0._f)
				{
					const Fix12i newLength = ropeClsn.height - deltaY;
					const Fix12i maxLength = GetMaxLength();

					if (maxLength > newLength)
						ropeClsn.height = newLength;
					else
						ropeClsn.height = maxLength;
				}
			}
		}
	}

	ropeModel.mat4x3.c1.y = -ropeClsn.height >> 3;
	ropeClsn.pos.y = pos.y - ropeClsn.height;
	ropeClsn.Clear();
	ropeClsn.Update();

	return 1;
}

int Hanger::Render()
{
	ropeModel.Render();

	return 1;
}

Fix12i Hanger::GetMaxLength()
{
	return ropeStartLength;
}
