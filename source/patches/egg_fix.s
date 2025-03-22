@ Despawn eggs when they fly 65536 fxu away from the player
nsub_020ee138_ov_02:
	ldr     r1, =PLAYER_ARR
	ldr     r1, [r1]
	cmp     r1, #0
	popeq   {r4, r15}
	add     r0, r1, #0x5c
	add     r1, r4, #0x5c
	bl      Vec3_Dist
	cmp     r0, #0x10000000
	movls   r0, #1
	popls   {r4, r15}
	ldrb    r1, [r4, #0x428]
	tst     r1, #0x80
	movne   r0, r4
	addne   r1, r4, #0x400
	addne   r1, r1, #0x27
	blne    _ZN5Actor11UntrackStarERa
	mov     r0, r4
	bl      _ZN9ActorBase18MarkForDestructionEv
	mov     r0, #1
	pop     {r4, r15}
