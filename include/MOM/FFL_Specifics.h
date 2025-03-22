#ifndef FFL_SPECIFICS_INCLUDED
#define FFL_SPECIFICS_INCLUDED

#include "SM64DS_PI.h"
#include "IceCoinCounter.h"

struct FFL_Specifics : IceCoinCounter
{
	virtual int InitResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	
	int GetObjectCount();
	void RenderNumber(uint8_t number, int& x, int y);
	
	unsigned searchObjectActorID;
	bool counted;
	int totalObjectCount;
	int remaining;

	static SpawnInfo spawnData;
};

#endif