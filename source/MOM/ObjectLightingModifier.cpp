#include "ObjectLightingModifier.h"

SpawnInfo ObjectLightingModifier::spawnData =
{
	[] -> ActorBase* { return new ObjectLightingModifier; },
	0x0000,
	0x0100,
	0x00000002,
	0x00064000_f,
	0x000c8000_f,
	0x01000000_f,
	0x01000000_f
};

[[gnu::target("thumb")]]
int ObjectLightingModifier::InitResources()
{
	GXFIFO::SetLightColor((u8)((param1 & 0xFF00) >> 8), (u8)(param1 & 0xFF), (u8)((ang.x & 0xFF00) >> 8), (u8)(ang.x & 0xFF));

	return 1;
}

int ObjectLightingModifier::CleanupResources()
{
	GXFIFO::SetLightColor(0, 0xFF, 0xFF, 0xFF);
	return 1;
}
