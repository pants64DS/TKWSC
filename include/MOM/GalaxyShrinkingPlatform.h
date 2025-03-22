#ifndef GALAXYSHRINKINGPLATFORM_INCLUDED
#define GALAXYSHRINKINGPLATFORM_INCLUDED

#include "SM64DS_PI.h"

struct GalaxyShrinkingPlatform : public Platform
{	
	void UpdateModelTransform();

	static void OnFloorAfterClsn(MeshColliderBase& clsn, Actor* clsnActor, Actor* otherActor);
	void Shrink();
	void UnShrink();
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;

	Vector3 skl = {1._f, 1._f, 1._f};
	bool shrinkActivated;
	//bool playSound;
	Model frameModel;

	static SharedFilePtr modelFile;
	static SharedFilePtr clsnFile;
	static SharedFilePtr frameModelFile;

	static SpawnInfo spawnData;
};

#endif
