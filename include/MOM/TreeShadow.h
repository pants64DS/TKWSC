#ifndef TREE_SHADOW_INCLUDED
#define TREE_SHADOW_INCLUDED

#include "SM64DS_PI.h"

struct TreeShadow : public Actor
{
	ShadowModel shadow;
	Matrix4x3 shadowMat;
	unsigned opacity;
	
	void DropShadow();
	
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	
	static SpawnInfo spawnData;
};

#endif
