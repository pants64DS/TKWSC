@ Make Mario's Door Yoshi's.
repl_02144588_ov_64:

	@ If player enters their own door, go to same player as door logic.
	moveq  r1, #0x0
	beq    samePlayerAsDoor
	
	@ If player does not enter their own door, but enter's Yoshi's door, return Yoshi.
	cmp    r2, #0x0
	beq    returnYoshi
	bxne   lr
	
	@ Player entered their own door. If it is Yoshi's door (Was Mario's), return Yoshi.
samePlayerAsDoor:
	cmp    r2, #0x0
	bxne   lr
	mov    r1, #0x3
	bx     lr
	
	@ Return Yoshi, but if Yoshi enters his own door, return Mario.
returnYoshi:
	cmp r0, #0x3
	moveq r1, #0x0
	movne r1, #0x3
	bx lr