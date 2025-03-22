#include "SM64DS_PI.h"
#include "MOM_Interface.h"

using namespace Particle;

// Enable loading particle textures outside of the SPA file
asm(R"(
nsub_0204a408:
	ldrb   r2, [r10, #0x26]
	b      0x0204a40c

nsub_0204a420:
	ldrb   r0, [r10, #0x27]
	b      0x0204a424

nsub_0204a490:
	ldrb   r2, [r10, #0x27]
	b      0x0204a494

nsub_0204a0f0:
	ldrb   r0, [r10, #0x27]
	b      0x0204a0f4

nsub_0204a154:
	ldrb   r0, [r10, #0x27]
	b      0x0204a158

nsub_0204a03c:
	ldrb   r0, [r10, #0x27]
	b      0x0204a040

nsub_0204a0b4:
	ldrb   r0, [r10, #0x27]
	b      0x0204a0b8
)");

// This used to be called Particle::Manager::UnloadNewTexs
int nsub_0202cbb8() // at the end of Stage::CleanupResources
{
	TexDef* texDefArr = PARTICLE_SYS_TRACKER->manager->texDefArr;
	int numTexDefs = PARTICLE_SYS_TRACKER->manager->numTextures;

	for(int i = PARTICLE_SYS_TRACKER->manager->numBuiltInTexs; i < numTexDefs; ++i) if (texDefArr[i].texture)
		delete texDefArr[i].texture;

	MOM_Interface::instance.camRotationDisabled = false;

	return 1;
}
