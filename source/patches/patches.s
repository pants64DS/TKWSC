
@ Allow loading overlays outside of the expected address range
nsub_02018b3c = 0x02018b44

@ Change starting character
repl_02013ddc:
	mov   r0, #0
	bx    lr

@ Start with all characters unlocked
nsub_02013df4:
	orr   r1, r1, #0x8f
	mov   r3, #0x38
	orr   r3, r3, #0x1c00
	strh  r3,[r4, #4]
	b     0x02013df8

@ Change level loaded on exit course to level ID 0 and the entrance ID to 14
nsub_02029378:
	mov   r0, #0
	mov   r1, #14
	b     0x0202937c

@ Enable getting stuck in ground in levels 6, 7 and 15, and in area 1 in level 40
nsub_020e2ecc_ov_02:
	beq   0x020e2ed0
	cmp   r4, #6
	beq   0x020e2ed0
	cmp   r4, #7
	beq   0x020e2ed0
	cmp   r4, #15
	beq   0x020e2ed0
	cmp   r4, #40
	bne   0x020e2ff8
	ldrb  r0,[r5, #0xcc]
	cmp   r0, #1
	beq   0x020e2ed0
	b     0x020e2ff8

@ Enable normal snow particles in levels 6, 16, 21 and 28
nsub_0200e0e8:
	beq   0x0200e0f4
	cmp   r3, #6
	beq   0x0200e0f4
	cmp   r3, #16
	beq   0x0200e0f4
	cmp   r3, #21
	beq   0x0200e0f4
	cmp   r3, #28
	beq   0x0200e0f4
	b     0x0200e0ec

@ Enable Chief Chilly blizzard particles in levels 14, 26, 37 and 38
nsub_0200e138:
	cmp   r3, #14
	beq   0x0200e13c
	cmp   r3, #26
	beq   0x0200e13c
	cmp   r3, #37
	beq   0x0200e13c
	cmp   r3, #38
	beq   0x0200e13c
	cmp   r3, #49
	b     0x0200e13c

@ Enable freezing water in levels 6 and 40
nsub_020cec98_ov_02:
	beq   0x020cec9c
	cmp   r0, #6
	beq   0x020cec9c
	cmp   r0, #40
	beq   0x020cec9c
	b     0x020ceca0

@ Make freezing water do less damage in level 40
nsub_020ced04_ov_02:
	beq   0x020ced08
	ldr   r14, =LEVEL_ID
	ldrb  r14,[r14]
	cmp   r14, #40
	movne r4, #0x50
	moveq r4, #0x78
	b     0x020ced08

@ Remove lava burn fire effects in levels 6, 14, 16 and 26
nsub_020d5088_ov_02:
	beq   0x020d5164
	cmp   r0, #6
	beq   0x020d5164
	cmp   r0, #14
	beq   0x020d5164
	cmp   r0, #16
	beq   0x020d5164
	cmp   r0, #26
	beq   0x020d5164
	b     0x020d508c

@ Prevent up/downwarps
nsub_020c06e8_ov_02:
	strlo r2,[r0, #0x60]
	movlo r0, #1
	movhs r0, #0
	bx    lr

@ Dive infinitely (with A button)
repl_020dce18_ov_02:
	ldr   r0, =PLAYER_ARR
	ldr   r0,[r0]
	ldr   r0,[r0, #0x370]    @ currState
	ldr   r1, =#0x021105bc   @ ST_DIVE
	cmp   r1, r0
	ldrne r1, =#0x021101fc   @ ST_SLIDE_KICK_RECOVER
	bxne  lr
	ldr   r0, =#0x0209f49e   @ INPUT_1_FRAME
	ldrh  r0,[r0]
	ands  r0, r0, #1         @ A button
	ldreq r1, =#0x021101fc   @ ST_SLIDE_KICK_RECOVER
	bx    lr

@ Make the player continue long jumping after landing a long jump on a goomba
repl_0212a290_ov_54:
	push  {r2, r3}
	ldr   r2,[r0, #0x370]
	ldr   r3, =0x0211055c
	cmp   r2, r3
	pop   {r2, r3}
	bne   0x020d932c
	str   r1,[r0, #0xa8]
	bx    lr

@ Make the player wall slide after long jumping into a wall
nsub_020c2098_ov_02:
	bne   0x020c20f8
	b     0x020c209c

@ Increase acceleration by 1 fxu/frame^2
nsub_020d40e0:
	add   r2, r4, #0x1000
	b     0x020d40e4

@ Disable the instant 180 turn after receiving 4 fall damage
nsub_020d9814_ov_02 = 0x020d9828


@ Limit actor draw distances in level 28
nsub_020110d8:
	ldr   r2, =LEVEL_ID
	ldrb  r2,[r2]
	cmp   r2, #28
	ldr   r2,[r4,#0xbc]
	bne   0x020110dc
	cmp   r2, #0x200000
	movhi r2, #0x200000
	b     0x020110dc

@ Prevent the player from changing angle after exiting ST_HURT
nsub_020d9668_ov_02:
	ldrsh r1,[r2, #-0x22]
	strh  r1,[r2, #-0x1c]
	bx    lr

@ Make the player unable to wall jump on walls with traction 5 in level 15
nsub_020c201c_ov_02:
	mov   r4, r1
	ldr   r3, =LEVEL_ID
	ldrb  r3,[r3]
	cmp   r3, #15
	bne   0x020c2020
	ldr   r3,[r5,#0x440]
	and   r3, r3, #0x7000
	cmp   r3, #0x5000
	moveq r2, #1
	b     0x020c2020

@ Ignore the firmware user settings and only use English
nsub_02059ebc:
	mov    r2, #1
	b      0x02059ec0

@ Remove hardcoded invisible cannon hatch on mount bobomb
nsub_020bc9fc_ov_02 = 0x020bca38

@ makes Koopa the Quick say his THI dialogue in Koopa Mall
repl_0211a82c_ov_3E:
repl_02119da0_ov_3E:
	cmp r0, #0x16 @ level id of Koopa Mall
	bx r14

@ ice surfing particles for FFL and SSL
repl_0214cad8_ov_66:
	push { r0 }
	ldr r0, =LEVEL_ID
	ldr r0, [r0]
	cmp r0, #0xe  @ Frozen Frostbite Land
	beq ice_surfing_particles
	cmp r0, #0x10 @ Shifting Snow Land
	bne exit_0214cad8

ice_surfing_particles:
	mov r1, #0xe2

exit_0214cad8:
	pop { r0 }
	b _ZN8Particle6System3NewEjj5Fix12IiES2_S2_PK11Vector3_16fPNS_8CallbackE

@ make it possible for a 100 coin star to spawn in hub levels and impossible for them to spawn in main courses with a death coin star
nsub_020b1920_ov_02:
	cmp r0, #6
	beq 0x020b19c8 @ dont spawn (The Lost Valley)
	cmp r0, #8
	beq 0x020b19c8 @ dont spawn (Frozen Frostbite Land)
	cmp r0, #0x1d
	beq 0x020b1930 @ spawn in hub
	cmp r0, #0xf   @ original instruction
	b 0x020b1924

nsub_020ebbe4_ov_02 = 0x020ebc04

@ show the coin counter in the outside hub and hide it in final boss levels and courses with a death coin star
repl_020fc918_ov_02:
	cmp    r0, #39
	beq    0x020fc920 @ dont render (final level)
	cmp    r0, #40
	beq    0x020fc920 @ dont render (final boss)
	cmp    r0, #1
	bne    check_death_coin_levels
	ldr    r1, =STAR_ID
	ldrb   r1, [r1]
	cmp    r1, #6 @ purple coin mission
	beq    0x020fc92c @ render

check_death_coin_levels:
	bl     SublevelToLevel
	cmp    r0, #6
	beq    0x020fc920 @ dont render (The Lost Valley)
	cmp    r0, #8
	beq    0x020fc920 @ dont render (Frozen Frostbite Land)
	cmp    r0, #22
	beq    0x020fc920 @ dont render (Delfino Plaza)
	b      0x020fc91c

@ Allow all characters to open the star door for the first time
nsub_02145cb4_ov_64:
	mov    r1, #0
	b      0x02145cb8

@ Change tree particles to snow in all levels
repl_020cb550_ov_02:
	mov    r1, #0xba
	b      _ZN8Particle6System3NewEjj5Fix12IiES2_S2_PK11Vector3_16fPNS_8CallbackE

@ Limit collision range of ice blocks to 3072 fxu
@ but activate the collision even when it's "off-screen"
repl_02127e30_ov_51:
	mov    r1, #0xc00000
	mov    r2, #0
	b      _ZN8Platform13IsClsnInRangeE5Fix12IiES1_

@ Make the player invisible when waiting to be kicked out of a painting
nsub_020c70c4_ov_02:
	movne   r1, #1
	addne   r0, #0x6a0
	strneh  r1, [r0]
	bxne    lr
	b       0x020c70c8

/*
@ Enable the debug map selector
nsub_0202aed8:
	mov    r0, #2
	b      0x0202aedc
*/
