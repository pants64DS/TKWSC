#include "SM64DS_PI.h"

void repl_02126c8c_ov_50(Actor& painting, void(&stateFunc)(Actor&))
{
	stateFunc(painting);

	if (LEVEL_ID == 0 && !(SAVE_DATA.stars[17] & 0x80)) // if the big star hasn't been collected
		painting.MarkForDestruction();
}
