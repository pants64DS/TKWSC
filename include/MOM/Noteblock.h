#ifndef NOTEBLOCK_INCLUDED
#define NOTEBLOCK_INCLUDED

#include "SM64DS_PI.h"

struct Noteblock : public Platform
{	
	void UpdateModelTransform();

	static void OnFloorAfterClsn(MeshColliderBase& clsn, Actor* clsnActor, Actor* otherActor);
	virtual void Jiggle();
	virtual void Launch();
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual bool BeforeBehavior() override;
	virtual int Behavior() override;
	virtual int Render() override;
	virtual ~Noteblock();

	int stage;
	Vector3 oldPos;
	bool isLaunching;
	bool boost;
	
	ShadowModel shadow;
	Matrix4x3 shadowMat;

	static SpawnInfo spawnData;

	static SharedFilePtr modelFile;
	static SharedFilePtr clsnFile;
};

#endif
