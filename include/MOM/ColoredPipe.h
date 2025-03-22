#ifndef COLOREDPIPE_INCLUDED
#define COLOREDPIPE_INCLUDED

#include "SM64DS_PI.h"

struct ColoredPipe : public Platform
{	
	void UpdateModelTransform();
	
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	
	static SharedFilePtr modelFile;
	static SharedFilePtr clsnFile;

	static SpawnInfo spawnData;
};

#endif
