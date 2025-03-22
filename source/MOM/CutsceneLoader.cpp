#include "CutsceneLoader.h"

SpawnInfo CutsceneLoader::spawnData =
{
	[] -> ActorBase* { return new CutsceneLoader; },
	0x0000,
	0x0100,
	0x00000002,
	0x00064000_f,
	0x000c8000_f,
	0x01000000_f,
	0x01000000_f
};

/*
CutsceneLoader parameters:

Parameter 1: FFFF
Parameter 2: CXEE

F: Overlay 0 ID of the script file
C: Condition - see the enum in CutsceneLoader.h
E: Event ID
*/

[[gnu::target("thumb")]]
int CutsceneLoader::InitResources()
{
	if (LEVEL_ID == 39 && LAST_ENTRANCE_ID != 0)
		return 1;

	const bool condition = ang.x & 0xff00;

	if (condition == always ||
		condition == noStarsInCurrentLevel &&
		SAVE_DATA.stars[SUBLEVEL_LEVEL_TABLE[LEVEL_ID]] == 0
	)
	{
		if (!RUNNING_KUPPA_SCRIPT)
		{
			file = LoadFile(param1);

			RunKuppaScript(file);
		}
	}

	return 1;
}

int CutsceneLoader::CleanupResources()
{
	delete[] file;

	return 1;
}

bool CutsceneLoader::BeforeBehavior()
{
	if (RUNNING_KUPPA_SCRIPT != file)
		MarkForDestruction();

	return false;
}
