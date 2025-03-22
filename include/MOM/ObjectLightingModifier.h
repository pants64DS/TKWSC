#ifndef OBJECTLIGHTINGMODIFIER_INCLUDED
#define OBJECTLIGHTINGMODIFIER_INCLUDED

#include "SM64DS_PI.h"

struct ObjectLightingModifier : public Actor
{	
	void UpdateModelTransform();

	virtual int InitResources() override;
	virtual int CleanupResources() override;

	static SpawnInfo spawnData;
};

#endif