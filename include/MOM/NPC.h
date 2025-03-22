#ifndef YOSHI_NPC_INCLUDED
#define YOSHI_NPC_INCLUDED

#include "SM64DS_PI.h"

struct NPC : public Actor
{
	ModelAnim rigMdl;
	Model babyModel;
	TextureSequence texSeq;
	MovingCylinderClsn cylClsn;
	ShadowModel shadow;
	uint8_t state;
	uint8_t eventID;
	uint8_t starID;
	int counter;
	bool starSpawned;
	bool shouldTalk;
	bool isWhiteRabbit;
	Player* listener;
	uint16_t messages[2];
	unsigned npcID;
	
	void UpdateModelTransform();
	
	void State0_Wait();
	void State1_Talk();
	
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	
	static SharedFilePtr modelFiles[6];
	static SharedFilePtr animFiles[8];
	static SharedFilePtr texSeqFiles[2];
	
	static SpawnInfo spawnData;
};

#endif