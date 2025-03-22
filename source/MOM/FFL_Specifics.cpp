#include "FFL_Specifics.h"
#include "Berry.h"

namespace
{
	constexpr u32 ICE_COIN_ACTOR_ID = 0x21f;
	constexpr u32 BERRY_ACTOR_ID = 0x203;
	
	constexpr u32 SNOW_POKEY_STAR_ID = 1;
	constexpr u32 BULLY_STAR_ID = 3;
	constexpr u32 ICE_COIN_STAR_ID = 6;
	constexpr u32 BERRY_STAR_ID = 7;
	
	constexpr s32 COUNTER_ICON_X_START = 10;
	constexpr s32 COUNTER_ICON_Y = 170;
	constexpr s32 COUNTER_ICON_NUMBER_SIZE = 9;
	constexpr s32 COUNTER_ICON_SLASH_SIZE = 10;
}

SpawnInfo FFL_Specifics::spawnData =
{
	[] -> ActorBase* { return new FFL_Specifics; },
	0x0000,
	0x0100,
	0x00000002,
	0x00064000_f,
	0x000c8000_f,
	0x01000000_f,
	0x01000000_f
};

inline OamAttr SLASH[] =
{
	{
	/* Y_COORDINATE */ 0 | /* OBJ_SHAPE */ OamAttr::VERTICAL,
	/* X_COORDINATE */ 0 | OamAttr::OBJ_SIZE_0 /* 8x16 */,
	/* TILE_NUMBER */ 210 | /* PRIORITY */ (0 << 10) | /* PALETTE_NUMBER */ (1 << 12),
	},
	{
	/* Y_COORDINATE */ 0 | /* OBJ_SHAPE */ OamAttr::SQUARE,
	/* X_COORDINATE */ 8 | OamAttr::OBJ_SIZE_0 /* 8x8 */,
	/* TILE_NUMBER */ 178 | /* PRIORITY */ (0 << 10) | /* PALETTE_NUMBER */ (1 << 12),
	OamAttr::LAST,
	},
};

int FFL_Specifics::InitResources()
{
	searchObjectActorID = 0;
	
	if (STAR_ID == SNOW_POKEY_STAR_ID)
		searchObjectActorID = POKEY_ACTOR_ID;
	else if (STAR_ID == BULLY_STAR_ID)
		searchObjectActorID = BULLY_ACTOR_ID;
	else if (STAR_ID == BERRY_STAR_ID)
		searchObjectActorID = BERRY_ACTOR_ID;
	
	return 1;
}

[[gnu::target("thumb")]]
int FFL_Specifics::Behavior()
{
	IceCoinCounter::Behavior();

	if (searchObjectActorID == 0)
		return 1;
	
	if (!counted)
	{
		totalObjectCount = GetObjectCount();
		counted = true;
	}
	
	remaining = GetObjectCount();
	
	if (remaining == 0)
	{
		Vector3 starSpawnPos = ClosestPlayer()->pos;
		starSpawnPos.y += 250._f;
		
		if (STAR_ID == SNOW_POKEY_STAR_ID)
		{
			Event::SetBit(1);
			Actor::Spawn(0x00b2, 0x0040 + STAR_ID, starSpawnPos, nullptr, areaID, -1);
		}
		else if (STAR_ID == BULLY_STAR_ID)
		{
			Event::SetBit(3);
			Actor::Spawn(0x00b2, 0x0040 + STAR_ID, starSpawnPos, nullptr, areaID, -1);
		}
		// Berry handles star spawning for BERRY_STAR_ID
		// ColoredCoin handles star spawning for ICE_COIN_STAR_ID
		
		searchObjectActorID = 0;
	}
	
	return 1;
}

int FFL_Specifics::GetObjectCount()
{
	Actor* actor = FindWithActorID(searchObjectActorID, FIRST_ACTOR_LIST_NODE->actor);
	int amount = 0;
	
	while (actor != nullptr)
	{
		if (searchObjectActorID == POKEY_ACTOR_ID || searchObjectActorID == BULLY_ACTOR_ID)
		{
			if (actor->pos.y >= -250._f && actor->pos.y <= 10000._f)
				amount++;
		}
		else if (searchObjectActorID == BERRY_ACTOR_ID)
		{
			Berry* berry = static_cast<Berry*>(actor);
			if (!berry->killed)
				amount++;
		}
		
		actor = FindWithActorID(searchObjectActorID, actor);
	}
	
	return amount;
}

int FFL_Specifics::Render()
{
	if (STAR_ID == ICE_COIN_STAR_ID)
		return IceCoinCounter::Render();

	if (totalObjectCount == 0)
		return 1;

	s32 x = COUNTER_ICON_X_START;
	
	RenderNumber(totalObjectCount - remaining, x, COUNTER_ICON_Y);
	OAM::Render(false, &SLASH[0], x, COUNTER_ICON_Y, -1, -1, 1._f, 1._f, 0, -1);
	x += COUNTER_ICON_SLASH_SIZE;
	RenderNumber(totalObjectCount, x, COUNTER_ICON_Y);

	return 1;
}

void FFL_Specifics::RenderNumber(uint8_t number, int& x, int y)
{
	bool digitRendered = false;
	
	for (s32 i = 0; i < 2; i++)
	{
		s32 digit = number / POWERS_OF_TEN[i+1];
		number %= POWERS_OF_TEN[i+1];
		
		if (digit != 0 || digitRendered || i == 1)
		{
			OAM::Render(false, OAM::NUMBERS[digit], x, y, -1, -1, 1._f, 1._f, 0, -1);
			
			digitRendered = true;
			x += COUNTER_ICON_NUMBER_SIZE;
		}
	}
}
