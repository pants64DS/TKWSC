@ Make the debug screen appear when the game crashes
@ without having to enter the button combination
nsub_02013fa0:
	bhi     0x020140f8
	cmp     r12, #5
	bls     0x0201404c
	b       0x02014060

nsub_02014054:
	mov     r1, #6
	b       0x0201405c

@ Replace the build time with a custom message
nsub_02014274:
	adr     r1, custom_debug_msg
	b       0x02014280

custom_debug_msg: @ max 32 characters
	.asciz "DO NOT RESET!! Show to pants64:"
