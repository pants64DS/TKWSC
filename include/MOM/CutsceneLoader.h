#ifndef CUTSCENELOADER_INCLUDED
#define CUTSCENELOADER_INCLUDED

#include "SM64DS_PI.h"

struct CutsceneLoader : public Actor
{
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual bool BeforeBehavior() override;

	enum Condition
	{
		noStarsInCurrentLevel = 0,
		always = 1
	};
	
	char* file = nullptr;

	static SpawnInfo spawnData;
};

#endif