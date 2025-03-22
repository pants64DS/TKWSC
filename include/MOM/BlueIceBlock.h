#ifndef BLUE_ICE_BLOCK_INCLUDED
#define BLUE_ICE_BLOCK_INCLUDED

#include "SM64DS_PI.h"

struct BlueIceBlock : public Platform
{
	void UpdateModelTransform();

	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	
	Model model;
	
	static SpawnInfo spawnData;

	static SharedFilePtr modelFile;
	static SharedFilePtr clsnFile;
};

#endif
