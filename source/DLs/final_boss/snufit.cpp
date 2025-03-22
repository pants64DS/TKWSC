#include "snufit.h"

constinit SharedFilePtr Snufit::modelFile       = 0x02b4;
constinit SharedFilePtr Snufit::attackAnimFile  = 0x02b5;
constinit SharedFilePtr Snufit::bulletModelFile = 0x02b6;
constinit SharedFilePtr Snufit::waitAnimFile    = 0x02b7;

constinit Snufit::State Snufit::states[4] =
{
	{&Snufit::State0Func0, &Snufit::State0Func1},
	{&Snufit::State1Func0, &Snufit::State1Func1},
	{&Snufit::State2Func0, &Snufit::State2Func1},
	{&Snufit::State3Func0, &Snufit::State3Func1}
};

constinit SpawnInfo Snufit::spawnData =
{
	[]() -> ActorBase* { return new Snufit; },
	0xec,    // behavPriority
	0x5b,    // renderPriority
	NO_BEHAVIOR_IF_OFF_SCREEN | NO_RENDER_IF_OFF_SCREEN | AIMABLE_BY_EGG,
	100._f,  // rangeOffsetY
	200._f,  // range
	3072._f, // drawDist
	3072._f  // unkc0
};

int Snufit::CleanupResources()
{
	bulletModelFile.Release();
	modelFile.Release();
	waitAnimFile.Release();
	attackAnimFile.Release();

	return 1;
}

Fix12i Snufit::OnAimedAtWithEgg()
{
	return 0._f;
}

void Snufit::OnTurnIntoEgg(Player& player)
{
	GivePlayerCoins(player, (numCoinsMinus1 + 1) & 0xff, 0);

	KillAndTrackInDeathTable();
}

unsigned Snufit::OnYoshiTryEat()
{
	return 4;
}

[[gnu::naked]]
int Snufit::Render()
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
	add     r0,r0,#0x300
	ldr     r2,[r0]
	mov     r1,#0x0
	ldr     r2,[r2,#0x14]
	blx     r2
	mov     r0,#0x1
	add     r13,r13,#0x4
	pop     {r15}
)"); }

[[gnu::naked]]
int Snufit::Behavior()
{ asm(R"(
_ZN6Snufit8BehaviorEv:
	push    {r4,r14}
	sub     r13,r13,#0x8
	mov     r4,r0
	add     r1,r4,#0x144
	bl      _ZN5Enemy14UpdateYoshiEatER12WithMeshClsn
	cmp     r0,#0x0
	beq     LAB_02116C08
	add     r0,r4,#0x110
	bl      _ZN12CylinderClsn5ClearEv
	ldrb    r0,[r4,#0x107]
	cmp     r0,#0x0
	beq     LAB_02116BCC
	add     r0,r4,#0x100
	ldrh    r0,[r0,#0x4]
	cmp     r0,#0x0
	bne     LAB_02116BCC
	add     r0,r4,#0x110
	bl      _ZN12CylinderClsn6UpdateEv
LAB_02116BCC:
	mov     r0,r4
	bl      FUNC_0211696C
	ldr     r0,[r4,#0x5C]
	ldr     r1,=_ZN6Snufit6statesE + 0x20
	str     r0,[r4,#0x3CC]
	ldr     r2,[r4,#0x60]
	mov     r0,r4
	str     r2,[r4,#0x3D0]
	ldr     r2,[r4,#0x64]
	str     r2,[r4,#0x3D4]
	bl      FUNC_0211691C
	add     r13,r13,#0x8
	mov     r0,#0x1
	pop     {r4,r15}
LAB_02116C08:
	mov     r0,r4
	add     r1,r4,#0x144
	add     r2,r4,#0x300
	mov     r3,#0x3
	bl      _ZN5Enemy26UpdateKillByInvincibleCharER12WithMeshClsnR9ModelAnimj
	cmp     r0,#0x0
	addne   r13,r13,#0x8
	movne   r0,#0x1
	popne   {r4,r15}
	ldr     r0,[r4,#0x10C]
	cmp     r0,#0x0
	beq     LAB_02116C80
	mov     r1,#0x4000
	mov     r12,#0x100
	add     r0,r4,#0x8C
	rsb     r1,r1,#0x0
	mov     r2,#0xA
	mov     r3,#0x200
	str     r12,[r13]
	bl      ApproachAngle
	mov     r0,r4
	add     r1,r4,#0x144
	bl      _ZN5Enemy11UpdateDeathER12WithMeshClsn
	mov     r0,r4
	bl      FUNC_0211696C
	add     r13,r13,#0x8
	mov     r0,#0x1
	pop     {r4,r15}
LAB_02116C80:
	add     r0,r4,#0x100
	bl      _Z15CountDownToZeroRt
	ldr     r1,[r4,#0x3BC]
	ldr     r0,[r1,#0x8]
	cmp     r0,#0x0
	beq     LAB_02116CBC
	add     r3,r1,#0x8
	ldr     r1,[r3,#0x4]
	add     r0,r4,r1,asr #0x1
	ands    r1,r1,#0x1
	ldrne   r2,[r0]
	ldrne   r1,[r3]
	ldrne   r1,[r2,r1]
	ldreq   r1,[r3]
	blx     r1
LAB_02116CBC:
	ldr     r1,[r4,#0xA8]
	ldr     r0,[r4,#0x9C]
	ldr     r2,[r4,#0xA0]
	add     r0,r1,r0
	cmp     r0,r2
	movge   r2,r0
	ldr     r1,[r4,#0xAC]
	ldr     r0,=_ZN6Snufit6statesE + 0x00
	str     r2,[r4,#0xA8]
	str     r1,[r4,#0xAC]
	ldr     r1,[r4,#0x3BC]
	cmp     r1,r0
	beq     LAB_02116D68
	add     r2,r4,#0x3D8
	ldr     r0,[r2]
	ldr     r1,=SINE_TABLE
	add     r0,r0,#0x200
	str     r0,[r2]
	ldr     r0,[r4,#0x3D8]
	mov     r2,#0x0
	mov     r0,r0,lsl #0x10
	mov     r0,r0,asr #0x10
	mov     r0,r0,lsl #0x10
	mov     r0,r0,lsr #0x10
	mov     r0,r0,asr #0x4
	mov     r0,r0,lsl #0x2
	ldrsh   r3,[r1,r0]
	mov     r0,#0x46000
	mov     r1,#0x800
	umull   r14,r12,r3,r0
	mla     r12,r3,r2,r12
	mov     r2,r3,asr #0x1F
	adds    r1,r14,r1
	mla     r12,r2,r0,r12
	adc     r0,r12,#0x0
	mov     r1,r1,lsr #0xC
	orr     r1,r1,r0,lsl #0x14
	ldr     r2,[r4,#0x3D0]
	add     r1,r1,#0xB4000
	add     r0,r4,#0x60
	add     r1,r2,r1
	mov     r2,#0x3000
	bl      _Z14ApproachLinearRiii
LAB_02116D68:
	mov     r0,r4
	add     r1,r4,#0x110
	bl      _ZN5Actor22UpdatePosWithOnlySpeedEP12CylinderClsn
	mov     r0,r4
	add     r1,r4,#0x144
	mov     r2,#0x0
	bl      _ZN5Enemy12UpdateWMClsnER12WithMeshClsnj
	mov     r0,r4
	bl      FUNC_0211696C
	ldr     r1,[r4,#0x3BC]
	ldr     r0,=_ZN6Snufit6statesE + 0x10
	cmp     r1,r0
	beq     LAB_02116DBC
	ldrsh   r1,[r4,#0x92]
	mov     r0,r4
	strh    r1,[r4,#0x8C]
	ldrsh   r1,[r4,#0x94]
	strh    r1,[r4,#0x8E]
	ldrsh   r1,[r4,#0x96]
	strh    r1,[r4,#0x90]
	bl      FUNC_02115FF0
LAB_02116DBC:
	add     r0,r4,#0x110
	bl      _ZN12CylinderClsn5ClearEv
	mov     r0,r4
	bl      _ZNK5Actor13ClosestPlayerEv
	cmp     r0,#0x0
	beq     LAB_02116DE8
	ldrb    r0,[r0,#0x6FB]
	cmp     r0,#0x0
	bne     LAB_02116DE8
	add     r0,r4,#0x110
	bl      _ZN12CylinderClsn6UpdateEv
LAB_02116DE8:
	add     r0,r4,#0x350
	bl      _ZN9Animation7AdvanceEv
	mov     r0,#0x1
	add     r13,r13,#0x8
	pop     {r4,r15}
)"); }

[[gnu::naked]]
int Snufit::InitResources()
{ asm(R"(
	push    {r4,r14}
	sub     r13,r13,#0x8
	mov     r4,r0
	ldr     r0,=_ZN6Snufit9modelFileE
	bl      _ZN13SharedFilePtr7LoadBMDEv
	mov     r1,r0
	add     r0,r4,#0x300
	mov     r2,#0x1
	mvn     r3,#0x0
	bl      _ZN9ModelBase7SetFileER8BMD_Filebi
	ldr     r0,=_ZN6Snufit15bulletModelFileE
	bl      _ZN13SharedFilePtr7LoadBMDEv
	add     r0,r4,#0x364
	bl      _ZN11ShadowModel12InitCylinderEv
	ldr     r0,=_ZN6Snufit12waitAnimFileE
	bl      _ZN13SharedFilePtr7LoadBCAEv
	ldr     r0,=_ZN6Snufit14attackAnimFileE
	bl      _ZN13SharedFilePtr7LoadBCAEv
	mov     r0,#0x1E000
	rsb     r0,r0,#0x0
	str     r0,[r4,#0xA0]
	mov     r0,#0x200000
	str     r0,[r13]
	ldr     r1,=#0x7EFF0
	add     r0,r4,#0x110
	str     r1,[r13,#0x4]
	mov     r1,r4
	mov     r2,#0x38000
	mov     r3,#0x7E000
	bl      _ZN18MovingCylinderClsn4InitEP5Actor5Fix12IiES3_jj
	ldrsh   r1,[r4,#0x94]
	mov     r3,#0x0
	add     r0,r4,#0x144
	strh    r1,[r4,#0x8E]
	str     r3,[r13]
	str     r3,[r13,#0x4]
	mov     r1,r4
	mov     r2,#0x46000
	bl      _ZN12WithMeshClsn4InitEP5Actor5Fix12IiES3_P10Vector3_16S5_
	mov     r0,#0x1
	strb    r0,[r4,#0x108]
	strb    r0,[r4,#0x10A]
	ldr     r0,[r4,#0x5C]
	mov     r2,#0x1000
	str     r0,[r4,#0x3CC]
	ldr     r1,[r4,#0x60]
	mov     r0,r4
	str     r1,[r4,#0x3D0]
	ldr     r3,[r4,#0x64]
	ldr     r1,=_ZN6Snufit6statesE + 0x20
	str     r3,[r4,#0x3D4]
	str     r2,[r4,#0x35C]
	bl      FUNC_0211691C
	mov     r0,#0x1
	add     r13,r13,#0x8
	pop     {r4,r15}
)"); }
