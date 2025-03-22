#include "SM64DS_PI.h"
#include "YoshiColors.h"

asm(R"(
@ hook on SetRealCharacter
nsub_020be1e8_ov_02:
	cmp 	r1, #0x3
	push	{r0-r4}
	bleq	SetYoshiColorStats
	pop		{r0-r4}
	mov		r5, r0				@ original instruction
	b		0x020be1ec
)");

extern "C" void SetYoshiColorStats(Player& player, unsigned character)
{
	currentYoshiColor = (SAVE_DATA.cannonsUnlocked >> 8) & 0x3;
	
	characterSpeed[3] = yoshiSpeed[currentYoshiColor];
	characterCarrySpeed[3] = yoshiCarrySpeed[currentYoshiColor];
	characterHeavyCarrySpeed[3] = yoshiHeavyCarrySpeed[currentYoshiColor];
	characterSpeedSwim[3] = yoshiSwimSpeed[currentYoshiColor];
	characterJumpHeight[3] = yoshiJumpHeight[currentYoshiColor];

	SetYoshiPaletteOffset(player);
	
	GX_LoadOBJPltt(&yoshiOamPalettes[currentYoshiColor][0], 0x1ac, 16);
	GXS_LoadOBJPltt(&yoshiOamPalettes[currentYoshiColor][0], 0x1ac, 16);
	
	return;
}

void hook_0202cc10()
{
	initYoshi = false;
}

// hook at the start of Player::Render
void hook_020e3a08_ov_02(Player& player)
{
	if (!initYoshi)
	{
		defaultYoshiPalette = player.bodyModels[3]->data.materials[0].paletteInfo;
		
		SetYoshiColorStats(player, 3);
		
		initYoshi = true;
	}
}

extern "C" bool IsYoshiUnlocked(unsigned yoshiColorID)
{
	constexpr uint8_t starReq[4] = { 0, 12, 35, 60 };
	return NumStars() >= starReq[yoshiColorID];
}

asm(R"(
@ remove the check that only updates the body model palette when in VS mode
nsub_020e3ae4_ov_02 = 0x020e3b00

@ remove the check that only updates the head model palette when in VS mode
nsub_020e3c84_ov_02 = 0x020e3ca0

@ fix an obscure bug that updates Mario's body model with Yoshi's palette
nsub_020e3b28:
	ldrb r0, [r6, #0x6db]
	cmp r0, #0x3
	ldr r0, [r6, #0x61c]
	beq 0x020e3b2c
	b 0x020e3b44

@ the same thing as above, but for Mario's head model
nsub_020e3ccc:
	ldrb r0, [r6, #0x6db]
	cmp r0, #0x3
	ldr r0, [r6, #0x61c]
	beq 0x020e3cd0
	b 0x020e3ce8

@ make it so mother penguin doesn't ignore Yellow Yoshi
nsub_02112268_ov_12:
	ldrne r2, [r1, #0x358]
	bne 0x0211226c
	ldr r0, =0x0209cab1
	ldrb r0, [r0]
	cmp r0, #0x3
	ldreq r2, [r1, #0x358]
	b 0x0211226c

@ make selecting yoshi color icons on star select screen turn you into the correct yoshi
nsub_020af1b0_ov_03:
	ldr r1, =0x0209cab1
	add r2, r0, #0x1
	strb r2, [r1]
	and r2, r0, #0xff
	b 0x020af1c4

@ when touched
nsub_020ae4d4_ov_03:
	ldr r2, =0x0209cab1
	add r0, r5, #0x1
	strb r0, [r2]
	and r3, r5, #0xff
	b 0x020ae4e8

@ make selecting green yoshi head turn you into green yoshi
nsub_020af1e8_ov_03:
	ldr r0, =0x0209cae1
	ldrb r0, [r0]
	cmp r0, #0x3
	ldreq r1, =0x0209cab1
	moveq r0, #0x0
	streqb r0, [r1]
	mov r0, #0x2
	b 0x020af1ec

@ when touched
nsub_020ae414_ov_03:
	cmp r0, #0x3
	ldreq r1, =0x0209cab1
	moveq r2, #0x0
	streqb r2, [r1]
	add r0, r0, #0x3c
	b 0x020ae418

@ make the yoshi color icons only appear when collected
repl_020b024c_ov_03 = IsYoshiUnlocked
repl_020adf88_ov_03 = IsYoshiUnlocked
repl_020ae464_ov_03 = IsYoshiUnlocked
repl_020b0314_ov_03:
repl_020aed84_ov_03:
	add r0, r0, #0x1
	b IsYoshiUnlocked
)");

/*
these are the new oam attributes for the yoshi color icons in overlay 1 (these were the oam attributes the star select screen cap icons before)
Blue Yoshi: 0x020abc34
0x40f0
0x81f0
0x4040
0xffff

Yellow Yoshi: 0x020abc3c
0x40f0
0x81f0
0x5040
0xffff

Red Yoshi: 0x020abc44
0x40f0
0x81f0
0x3040
0xffff
*/