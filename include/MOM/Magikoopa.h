#ifndef MAGIKOOPA_INCLUDED
#define MAGIKOOPA_INCLUDED

#include "SM64DS_PI.h"

struct Magikoopa : public Enemy
{
	//This keeps track of all the resources what the Magikoopa spawns needs.
	//Without this, particle glitches can happen.
	struct Resources
	{
		static const int NUM_PER_CHUNK = 16;
		unsigned files[NUM_PER_CHUNK] = {0};
		Resources* next = nullptr;
		
		void Add(SharedFilePtr& sf);
		void ProcessAdditions();
		~Resources();
	};
	
	static const int SHOT_LIMIT = 3;
	struct SharedRes
	{
		unsigned numRefs = 1;
		unsigned spawnActorID;
		unsigned spawnActorParams;
		uint8_t isBoss;
		unsigned shotUniqueIDs[SHOT_LIMIT] = {0xffffffff, 0xffffffff, 0xffffffff};
		Resources res;
		
		static void OnLoadFile(SharedFilePtr& file);
		void TrackRes();
		void StopTracking();
	};
	
	struct Shot : public Enemy
	{
		MovingCylinderClsn cylClsn;
		WithMeshClsn wmClsn;
		
		Vector3_16f direction;
		uint8_t shotState;
		uint8_t numFireToSpawn;
		uint8_t* resourceRefCount;
		SharedRes* res;
		unsigned shapesID;
		
		void SetMagikoopa(Magikoopa& magik);
		
		virtual int InitResources() override;
		virtual int CleanupResources() override;
		virtual int Behavior() override;
		virtual int Render() override;
		virtual void OnPendingDestroy() override;

		static SpawnInfo spawnData;
	};
	
	MovingCylinderClsn cylClsn;
	WithMeshClsn wmClsn;
	ModelAnim rigMdl;
	ShadowModel shadow;
	
	uint8_t state;
	uint8_t numPathPts;
	uint8_t currPathPt;
	uint8_t nextPathPt;
	PathPtr pathPtr;
	unsigned shapesID;
	uint8_t eventToTrigger;
	uint8_t starID;
	uint8_t resourceRefCount;
	uint8_t shotState;
	uint8_t health;
	bool invincible;
	bool battleStarted;
	SharedRes* res;
	Player* listener;
	Fix12i horzDecel;
	Vector3 originalPos;
	
	void UpdateModelTransform();
	void KillMagikoopa();
	void HandleClsn();
	Vector3 GetWandTipPos();

	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	virtual void OnPendingDestroy() override;
	virtual unsigned OnYoshiTryEat() override;
	virtual void OnTurnIntoEgg(Player& player) override;
	virtual Fix12i OnAimedAtWithEgg() override; //returns egg height
	
	void AttemptTalkStartIfNotStarted();
	void Talk();
	
	void State0_Appear();
	void State1_Wave();
	void State2_Shoot();
	void State3_Poof();
	void State4_Teleport();
	void State5_Hurt();
	void State6_Wait();
	void State7_Defeat();

	static SharedFilePtr modelFiles[2];
	static SharedFilePtr animFiles[7];
	
	static SpawnInfo spawnData;
	static SpawnInfo bossSpawnData;

};
#endif