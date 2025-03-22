#include "SM64DS_PI.h"
#include "MOM_IDs.h"

asm(R"(
nsub_020b0cc4_ov_02:
    bne  0x020b0cc8
    mov  r0, r6
    bl   FindCustomPipe
    movs r7, r0
    beq  0x020b0cd4
    b    0x020b0cc8
)");

extern "C" Actor* FindCustomPipe(const Actor& exit)
{
    Actor* customPipe = nullptr;
    while ((customPipe = Actor::FindWithActorID(MOM_IDs::COLORED_PIPE, customPipe)) && customPipe->pos.Dist(exit.pos) >= 300._f);
    return customPipe;
}
