#ifndef TREASURE_CHEST_INCLUDED
#define TREASURE_CHEST_INCLUDED

#include "SM64DS_PI.h"

//because Koopas and Treasure Chests are incompatible
struct TreasureChest : public Actor
{
	ModelAnim rigMdl; //0x0d4
	MovingCylinderClsn cylClsn; //0x138

	unsigned state;
	uint16_t cooldown;
	uint8_t order;
	bool spawnStar;
	uint8_t starID;
	int8_t trackedStarID;

	void UpdateModelTransform();
	void ChangeState(unsigned state);
	void CallState();

	void State0_Init();
	void State0_Main();
	void State1_Init();
	void State1_Main();
	void State2_Init();
	void State2_Main();

	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;

	static SharedFilePtr modelFile;
	static SharedFilePtr animFiles[1];

	static SpawnInfo spawnData;
};

#endif