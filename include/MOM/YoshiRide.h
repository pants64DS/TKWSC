#ifndef YOSHI_RIDE_INCLUDED
#define YOSHI_RIDE_INCLUDED

#include "SM64DS_PI.h"

struct YoshiRide : public Actor
{
	ModelAnim rigMdl;
	Model* headMdl; //pointer because Yoshi's head has animation, but the other heads don't.
	MovingCylinderClsn cylClsn;
	WithMeshClsn wmClsn;
	ShadowModel shadow;
	Matrix4x3 shadowMat;
	Vector3 origPos;
	uint8_t state;
	uint8_t riderChar;
	uint8_t cooldown;
	bool riding;
	uint16_t runTimer;
	
	void UpdateModelTransform();
	void ChangeState(uint8_t newState);
	void StartRide(int charID);
	void EndRide(bool changeChar);
	
	void StWait_Init();
	void StWait_Main();
	
	void StRide_Init();
	void StRide_Main();
	
	void StRun_Init();
	void StRun_Main();
	
	static YoshiRide* Spawn();
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	
	static SpawnInfo spawnData;
	static SharedFilePtr ridingAnim;
};

#endif