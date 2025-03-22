#ifndef INVISIBLEWALL_INCLUDED
#define INVISIBLEWALL_INCLUDED

#include "SM64DS_PI.h"

struct InvisibleWall : public Platform
{	
	void UpdateModelTransform();

	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	
	static SharedFilePtr clsnFile;
	static SpawnInfo spawnData;
};

#endif
