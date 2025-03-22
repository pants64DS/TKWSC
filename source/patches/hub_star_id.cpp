#include "SM64DS_PI.h"

asm(R"(
nsub_02029d34:
	cmp    r0, #29
	ldrneb r1, [r1]
	bne    0x02029d38
	add    r13, r13, #4
	pop    {r14}
	b      SetHubStarID
)");

extern "C" void SetHubStarID(int actSelectorID, char& starID)
{
	if (CURRENT_GAMEMODE == 2)
	{
		starID = 5;
		return;
	}

	const int numStars = NumStars();

	if (numStars < 3)
		starID = 1;
	else if (numStars < 12)
		starID = 2;
	else if (numStars < 35)
		starID = 3;
	else if (numStars < 60)
		starID = 4;
	else if (numStars < 100)
		starID = 5;
	else if (numStars < 101)
		starID = 6;
	else
		starID = 7;
}
