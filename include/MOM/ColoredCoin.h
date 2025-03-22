#ifndef COLORED_COIN_INCLUDED
#define COLORED_COIN_INCLUDED

#include "SM64DS_PI.h"

struct ColoredCoin : public Actor
{	
	void UpdateModelTransform();

	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	void HandleClsn();
	void Kill();
	virtual unsigned OnYoshiTryEat() override;
	virtual void OnTurnIntoEgg(Player& player) override;
	
	static SharedFilePtr modelFile;
	static int trackCounter;

	Model model;
	MovingCylinderClsn cylClsn;
	ShadowModel shadow;
	Matrix4x3 shadowMat;
	Actor* spawnedStar;
	bool killable : 1 = true;
	bool fake : 1;
	bool hurt : 1;
	bool deathCoin : 1;
	bool deathStarted : 1;
	enum : char { UNKNOWN, NORMAL, ICE_COIN } iceCoinStatus : 2 = UNKNOWN;
	uint8_t starID;
	int deathFrames;
	int frameCounter;
	int health;
	int value;
	
	static SpawnInfo spawnData;
};

#endif
