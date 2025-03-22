#include "IceCoinCounter.h"

SpawnInfo IceCoinCounter::spawnData =
{
	[] -> ActorBase* { return new IceCoinCounter; },
	0x0000,
	0x0100,
	0x00000002,
	0x00064000_f,
	0x000c8000_f,
	0x01000000_f,
	0x01000000_f
};

static OamAttr ICE_COIN =
{
	/* Y_COORDINATE */ 248,
	/* X_COORDINATE */ 504 | OamAttr::OBJ_SIZE_1 /* 16x16 */,
	/* TILE_NUMBER */ 152 | /* PRIORITY */ (0 << 10) | /* PALETTE_NUMBER */ (0 << 12),
	OamAttr::LAST,
};

extern uint8_t GAME_PAUSED; // 0 = not paused, 1 = paused, 2 = unpausing
asm("GAME_PAUSED = 0x0209f2c4");

bool IceCoinCounter::BeforeBehavior()
{
	return GAME_PAUSED == 0 && !Event::GetBit(29);
}

[[gnu::alias("_ZN14IceCoinCounter14BeforeBehaviorEv")]]
bool IceCoinCounter::BeforeRender();

int IceCoinCounter::Behavior()
{
	if (numIceCoins < 5) return 1;

	if (starSpawnDelayCounter > 0) --starSpawnDelayCounter;

	if (starSpawnDelayCounter == 0)
	{
		Actor* starMarker = nullptr;
		while ((starMarker = FindWithActorID(0xb4, starMarker)) != nullptr)
		{
			if (starMarker->param1 == (param1 | 0x20))
			{
				Actor::Spawn(0xb2, param1 | 0x40, starMarker->pos, nullptr, areaID, -1);
				break;
			}
		}

		starSpawnDelayCounter = -1;
	}

	return 1;
}

constexpr uint16_t ICE_COIN_PALETTE[0x10] = { 0x17FF, 0x0000, 0x1D94, 0x1E19, 0x269C, 0x2ADE, 0x2F7F, 0x47FF, 0x7FFF, 0x7D61, 0x79A1, 0x7E01, 0x7E61, 0x7EC1, 0x7F65, 0x7FEC };

int IceCoinCounter::Render()
{
	int x = 16;
	
	for (unsigned i = 0; i < numIceCoins; i++)
	{
		OAM::Render(false, &ICE_COIN, x, 10, -1, 1, 0);
		x += 11;
	}
	
	GX::LoadOBJPltt(&ICE_COIN_PALETTE, 0x00, 0x20);
	return 1;
}
