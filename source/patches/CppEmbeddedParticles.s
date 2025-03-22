@ the code below is taken from an old sm256 demo using no$gba (credits to HayashiSTL)

@ stop the game from reading past the bounds of the particle array
@ and make it read the address of the custom particle if the particle id > 0x02000000
nsub_02021D40:
	cmp r6, #0x02000000
	ldrge r4, [r6]
	ldrlt r4, [r0, r6, lsl #0x5]
	b 0x02021D44
	
nsub_02049E44:
	cmp r7, #0x02000000
	movge r1, r7
	addlt r1, r1, r7, lsl #0x5
	b 0x02049E48
	
nsub_02049E90 = 0x02049E98
	
nsub_02021CF0:
	cmp r2, #0x02000000
	ldrlt r2, [r3, r2, lsl #0x5]
	strlth r12, [r2, #0x28]
	b 0x02021CF8
