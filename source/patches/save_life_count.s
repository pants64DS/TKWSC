@ Initialize NUM_LIVES based on SAVE_DATA.unk43 (0 is mapped to 4, other values are decremented)
nsub_0202ad08:
	ldr     r1, =SAVE_DATA + 0x43
	ldrb    r1, [r1]
	cmp     r1, #0
	moveq   r1, #4
	subne   r1, #1
	b       0x0202ad0c

@ When saving the file, store NUM_LIVES + 1 at 0x43
nsub_02013b9c:
	ldr     r0, =NUM_LIVES
	ldrsb   r0, [r0]
	add     r0, #1
	ldr     r1, =SAVE_DATA + 0x43
	strb    r0, [r1]
	push    {r14}
	b       0x02013ba0

@ Set NUM_LIVES to 4 when exiting the game over screen
nsub_020b0b38_ov_03:
	mov     r1, #4
	ldr     r2, =NUM_LIVES
	strb    r1, [r2]
	bx      lr
