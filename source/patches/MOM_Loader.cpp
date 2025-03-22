#include "SM64DS_PI.h"

asm(R"(
.global _ZN13MOM_Interface8instanceE
_ZN13MOM_Interface8instanceE = 0x023c4000
)");

bool repl_0201a7c8(int ovID)
{
	const bool res = LoadOverlay(ovID);

	static constinit bool loaded = false;

	if (ovID == 2 && !loaded)
	{
		FS_LoadOverlay(false, 155);
		loaded = true;
	}

	return res;
}
