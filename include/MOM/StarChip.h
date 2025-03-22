#ifndef STAR_CHIP_INCLUDED
#define STAR_CHIP_INCLUDED

#include "SM64DS_PI.h"

struct StarChip : public Platform
{	
	void UpdateModelTransform();
	int& GetChipCounter();

	static StarChip* Spawn();
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	void HandleClsn();
	void Kill();
	virtual unsigned OnYoshiTryEat() override;
	virtual void OnTurnIntoEgg(Player& player) override;

	uint8_t eventID;
	uint8_t colorID;
	bool killable = true;

	MovingCylinderClsn cylClsn;
	ShadowModel shadow;
	Matrix4x3 shadowMat;

	static SharedFilePtr modelFile;

	static SpawnInfo spawnData;
};

#endif
