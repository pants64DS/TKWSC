#include "Save.h"

// hook to the end of HUD::Render because it's called every frame in a stage and who'd ever want to hook to that
asm(R"( nsub_020fd77c_ov_02 = CheckAntiCheat )");

extern "C" void ExtremeCrash()
{
	while (true) *(char*)((unsigned(RandomInt()) >> 10) + 0x02000000) = RandomInt();
}

extern "C" int CheckAntiCheat()
{
	// 10-14: unused main levels, 15-16: unused bowser levels, 18-28: unused side levels
	for (int i = 10; i < 29; i++)
	{
		if (i == 17 || i == 22) i++; // skip Bowser Chase and Delfino Plaza
		if (SAVE_DATA.stars[i] != 0) ExtremeCrash();
	}
	
	// only the bits in 0x4a can be set without cheats
	if ((SAVE_DATA.starDoorState & ~0x4a) != 0) ExtremeCrash();
	
	return 1; // return 1 because we're actually returning the value for HUD::Render
}
