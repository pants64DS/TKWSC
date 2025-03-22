#include "final_boss_dl.h"
#include "Actors/Bowser.h"
#include "Actors/SpikeBomb.h"

SpawnInfo BombHanger::spawnData =
{
	+[]() -> ActorBase* { return new BombHanger; },
	0x100, // behavPriority
	0x100, // renderPriority
	0,     // flags
	0_f,   // rangeOffsetY
	-1._f, // range
	0_f,   // drawDist
	0_f    // unkc0
};

int BombHanger::InitResources()
{
	if (SpikeBomb* bomb = Actor::Spawn<SpikeBomb>(-1, GetBombPos(), nullptr, areaID, -1))
	{
		bombUniqueID = bomb->uniqueID;
		bomb->drawDistAsr3 = 400000._f;
		bomb->flags |= Actor::UPDATE_DURING_CUTSCENES;
	}

	Hanger::InitResources();

	// enable light 1 and disable light 0
	(ropeModel.data.materials[0].polygonAttr &= ~1) |= 2;

	return 1;
}

int BombHanger::Behavior()
{
	SpikeBomb* bomb = static_cast<SpikeBomb*>(Actor::FindWithID(bombUniqueID));

	if (bomb)
	{
		bomb->pos = GetBombPos();
		loaded = bomb->stateID == 0;

		if (!recovering && detonatedByPlayer && bomb->stateID == 3 && (bomb->flags & Actor::OFF_SCREEN))
			bomb->Recover();
	}
	else
		loaded = false;

	return Hanger::Behavior();
}

Fix12i BombHanger::GetMaxLength()
{
	return pos.y - floorY - 2 * bombVertOffset;
}
