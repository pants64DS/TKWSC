#include "swooper.h"
#include "background.h"

constinit SharedFilePtr Swooper::modelFile     = 0x11f;
constinit SharedFilePtr Swooper::flyAnimFile   = 0x120;
constinit SharedFilePtr Swooper::waitAnimFile  = 0x121;
constinit SharedFilePtr Swooper::waitModelFile = 0x122;

constinit Swooper::State Swooper::states[4] =
{
	{&Swooper::State0Func0, &Swooper::State0Func1},
	{&Swooper::State1Func0, &Swooper::State1Func1},
	{&Swooper::State2Func0, &Swooper::State2Func1},
	{&Swooper::State3Func0, &Swooper::State3Func1}
};

constinit SpawnInfo Swooper::spawnData
{
	[]() -> ActorBase* { return new Swooper; },
	0xed,    // behavPriority
	0x5c,    // renderPriority
	NO_BEHAVIOR_IF_OFF_SCREEN | NO_RENDER_IF_OFF_SCREEN,
	100._f,  // rangeOffsetY
	200._f,  // range
	4096._f, // drawDist
	4096._f  // unkc0
};

Fix12i Swooper::OnAimedAtWithEgg()
{
	return 0._f;
}

void Swooper::OnTurnIntoEgg(Player& player)
{
	GivePlayerCoins(player, (numCoinsMinus1 + 1) & 0xff, 0);

	KillAndTrackInDeathTable();
}

unsigned Swooper::OnYoshiTryEat()
{
	return 4;
}

int Swooper::CleanupResources()
{
	modelFile.Release();
	waitModelFile.Release();
	flyAnimFile.Release();
	waitAnimFile.Release();

	return 1;
}

extern "C" void PlaySwooperSound(unsigned soundID, const Vector3& camSpacePos)
{
	if (CAMERA && (CAMERA->flags & Camera::UNDERWATER) == 0)
		Sound::PlayArchive3(soundID, camSpacePos);
}

[[gnu::naked]]
int Swooper::Render()
{ asm(R"(
	push    {r14}
	sub     r13,r13,#0x4
	ldr     r1,[r0,#0xB0]
	ands    r1,r1,#0x40000
	movne   r1,#0x1
	moveq   r1,#0x0
	cmp     r1,#0x0
	addne   r13,r13,#0x4
	movne   r0,#0x1
	popne   {r15}
	ldrb    r1,[r0,#0x43C]
	cmp     r1,#0x1
	bne     LAB_02117B40
	add     r0,r0,#0x300
	ldr     r2,[r0]
	mov     r1,#0x0
	ldr     r2,[r2,#0x14]
	blx     r2
	b       LAB_02117B54
LAB_02117B40:
	add     r0,r0,#0x364
	ldr     r2,[r0]
	mov     r1,#0x0
	ldr     r2,[r2,#0x14]
	blx     r2
LAB_02117B54:
	mov     r0,#0x1
	add     r13,r13,#0x4
	pop     {r15}
)"); }

bool Swooper::BeforeBehavior()
{
	return Actor::BeforeBehavior() && !Background::renderInInterior;
}

[[gnu::naked]]
int Swooper::Behavior()
{ asm(R"(
	push    {r4,r14}
	mov     r4,r0
	add     r1,r4,#0x144
	bl      _ZN5Enemy14UpdateYoshiEatER12WithMeshClsn
	cmp     r0,#0x0
	beq     LAB_02117BBC
	add     r0,r4,#0x110
	bl      _ZN12CylinderClsn5ClearEv
	ldrb    r0,[r4,#0x107]
	cmp     r0,#0x0
	beq     LAB_02117BA8
	add     r0,r4,#0x100
	ldrh    r0,[r0,#0x4]
	cmp     r0,#0x0
	bne     LAB_02117BA8
	add     r0,r4,#0x110
	bl      _ZN12CylinderClsn6UpdateEv
LAB_02117BA8:
	mov     r0,r4
	bl      FUNC_02117994
	mov     r0,#0x1
	pop     {r4,r15}
LAB_02117BBC:
	mov     r0,r4
	add     r1,r4,#0x144
	add     r2,r4,#0x300
	mov     r3,#0x3
	bl      _ZN5Enemy26UpdateKillByInvincibleCharER12WithMeshClsnR9ModelAnimj
	cmp     r0,#0x0
	movne   r0,#0x1
	popne   {r4,r15}
	ldr     r0,[r4,#0x10C]
	cmp     r0,#0x0
	beq     LAB_02117C0C
	mov     r0,r4
	add     r1,r4,#0x144
	bl      _ZN5Enemy11UpdateDeathER12WithMeshClsn
	mov     r0,r4
	bl      FUNC_02117994
	mov     r0,#0x1
	pop     {r4,r15}
LAB_02117C0C:
	add     r0,r4,#0x100
	bl      _Z15CountDownToZeroRt
	ldr     r1,[r4,#0x420]
	ldr     r0,[r1,#0x8]
	cmp     r0,#0x0
	beq     LAB_02117C48
	add     r3,r1,#0x8
	ldr     r1,[r3,#0x4]
	add     r0,r4,r1,asr #0x1
	ands    r1,r1,#0x1
	ldrne   r2,[r0]
	ldrne   r1,[r3]
	ldrne   r1,[r2,r1]
	ldreq   r1,[r3]
	blx     r1
LAB_02117C48:
	ldr     r1,[r4,#0x420]
	ldr     r0,=_ZN7Swooper6statesE + 0x00
	cmp     r1,r0
	beq     LAB_02117C64
	ldr     r0,=_ZN7Swooper6statesE + 0x10
	cmp     r1,r0
	bne     LAB_02117CAC
LAB_02117C64:
	add     r0,r4,#0x350
	mov     r1,#0x3
	bl      _ZNK9Animation12WillHitFrameEi
	cmp     r0,#0x0
	bne     LAB_02117CA0
	add     r0,r4,#0x350
	mov     r1,#0xF
	bl      _ZNK9Animation12WillHitFrameEi
	cmp     r0,#0x0
	bne     LAB_02117CA0
	add     r0,r4,#0x350
	mov     r1,#0x1B
	bl      _ZNK9Animation12WillHitFrameEi
	cmp     r0,#0x0
	beq     LAB_02117CAC
LAB_02117CA0:
	add     r1,r4,#0x74
	mov     r0,#0xE1
	bl      PlaySwooperSound
LAB_02117CAC:
	ldr     r1,[r4,#0xA8]
	ldr     r0,[r4,#0x9C]
	ldr     r3,[r4,#0xA0]
	add     r0,r1,r0
	cmp     r0,r3
	movge   r3,r0
	ldr     r2,[r4,#0xAC]
	mov     r0,r4
	str     r3,[r4,#0xA8]
	add     r1,r4,#0x110
	str     r2,[r4,#0xAC]
	bl      _ZN5Actor22UpdatePosWithOnlySpeedEP12CylinderClsn
	mov     r0,r4
	add     r1,r4,#0x144
	mov     r2,#0x0
	bl      _ZN5Enemy12UpdateWMClsnER12WithMeshClsnj
	ldrsh   r1,[r4,#0x92]
	mov     r0,r4
	strh    r1,[r4,#0x8C]
	ldrsh   r1,[r4,#0x94]
	strh    r1,[r4,#0x8E]
	ldrsh   r1,[r4,#0x96]
	strh    r1,[r4,#0x90]
	bl      FUNC_02117994
	ldrb    r0,[r4,#0x43C]
	cmp     r0,#0x1
	bne     LAB_02117D20
	mov     r0,r4
	bl      FUNC_0211704C
LAB_02117D20:
	add     r0,r4,#0x110
	bl      _ZN12CylinderClsn5ClearEv
	mov     r0,r4
	bl      _ZNK5Actor13ClosestPlayerEv
	cmp     r0,#0x0
	beq     LAB_02117D4C
	ldrb    r0,[r0,#0x6FB]
	cmp     r0,#0x0
	bne     LAB_02117D4C
	add     r0,r4,#0x110
	bl      _ZN12CylinderClsn6UpdateEv
LAB_02117D4C:
	ldrb    r0,[r4,#0x43C]
	cmp     r0,#0x1
	bne     LAB_02117D64
	add     r0,r4,#0x350
	bl      _ZN9Animation7AdvanceEv
	b       LAB_02117D6C
LAB_02117D64:
	add     r0,r4,#0x3B4
	bl      _ZN9Animation7AdvanceEv
LAB_02117D6C:
	mov     r0,#0x1
	pop     {r4,r15}
)"); }

[[gnu::naked]]
int Swooper::InitResources()
{ asm(R"(
	push    {r4,r14}
	sub     r13,r13,#0x8
	mov     r4,r0
	ldr     r0,=_ZN7Swooper9modelFileE
	bl      _ZN13SharedFilePtr7LoadBMDEv
	mov     r1,r0
	add     r0,r4,#0x300
	mov     r2,#0x1
	mvn     r3,#0x0
	bl      _ZN9ModelBase7SetFileER8BMD_Filebi
	ldr     r0,=_ZN7Swooper13waitModelFileE
	bl      _ZN13SharedFilePtr7LoadBMDEv
	mov     r1,r0
	add     r0,r4,#0x364
	mov     r2,#0x1
	mvn     r3,#0x0
	bl      _ZN9ModelBase7SetFileER8BMD_Filebi
	add     r0,r4,#0x3C8
	bl      _ZN11ShadowModel12InitCylinderEv
	ldr     r0,=_ZN7Swooper11flyAnimFileE
	bl      _ZN13SharedFilePtr7LoadBCAEv
	ldr     r0,=_ZN7Swooper12waitAnimFileE
	bl      _ZN13SharedFilePtr7LoadBCAEv
	mov     r0,#0xA000
	rsb     r0,r0,#0x0
	str     r0,[r4,#0xA0]
	mov     r0,#0x200000
	str     r0,[r13]
	ldr     r1,=#0x7EFF0
	add     r0,r4,#0x110
	str     r1,[r13,#0x4]
	mov     r1,r4
	mov     r2,#0x28000
	mov     r3,r2
	bl      _ZN18MovingCylinderClsn4InitEP5Actor5Fix12IiES3_jj
	ldrsh   r1,[r4,#0x94]
	mov     r0,#0x8000
	rsb     r0,r0,#0x0
	strh    r1,[r4,#0x8E]
	strh    r0,[r4,#0x92]
	ldrsh   r2,[r4,#0x92]
	mov     r1,#0x0
	mov     r0,#0x1000
	strh    r2,[r4,#0x8C]
	strb    r1,[r4,#0x43C]
	str     r0,[r4,#0x35C]
	str     r0,[r4,#0x3C0]
	str     r1,[r13]
	str     r1,[r13,#0x4]
	add     r0,r4,#0x144
	mov     r1,r4
	mov     r2,#0x28000
	mov     r3,#0x1E000
	bl      _ZN12WithMeshClsn4InitEP5Actor5Fix12IiES3_P10Vector3_16S5_
	ldr     r0,[r4,#0x5C]
	mov     r2,#0x1
	str     r0,[r4,#0x424]
	ldr     r1,[r4,#0x60]
	mov     r0,r4
	str     r1,[r4,#0x428]
	ldr     r3,[r4,#0x64]
	ldr     r1,=_ZN7Swooper6statesE + 0x20
	str     r3,[r4,#0x42C]
	strb    r2,[r4,#0x108]
	bl      FUNC_02117944
	mov     r0,#0x1
	add     r13,r13,#0x8
	pop     {r4,r15}
)"); }
