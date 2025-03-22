#ifndef ICE_COIN_COUNTER_INCLUDED
#define ICE_COIN_COUNTER_INCLUDED

#include "SM64DS_PI.h"

struct IceCoinCounter : Actor
{
	uint16_t numIceCoins = 0;
	short starSpawnDelayCounter = 30;

	virtual bool BeforeBehavior() override;
	virtual bool BeforeRender() override;
	virtual int Behavior() override;
	virtual int Render() override;

	static SpawnInfo spawnData;
};

#endif
