#ifndef BERRY_INCLUDED
#define BERRY_INCLUDED

#include "SM64DS_PI.h"

struct Berry : public Actor
{
	Model model;
	Model stem;
	MovingCylinderClsn cylClsn;
	ShadowModel shadow;
	Vector3 origPos;
	bool groundFound;
	bool killed;
	bool trackKills;
	
	void UpdateModelTransform();
	void Kill();
	
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	virtual unsigned OnYoshiTryEat() override;
	virtual void OnTurnIntoEgg(Player& player) override;
	
	static SpawnInfo spawnData;

	static SharedFilePtr modelFile;
	static SharedFilePtr stemFile;

	static unsigned berryCount;
	static unsigned berryMaxCount;
	static unsigned killCounter;

};

#endif