@ called in multiple states
nsub_020ddeb4_ov_02:
	bne    0x020ddedc         @ punch if not yoshi
	bl     check_yellow_yoshi
	beq    0x020ddedc         @ punch if yellow
	b      0x020ddeb8         @ tongue if not

@ in Player::St_SlopeJump_Main
nsub_020e0bbc_ov_02:
	bne    0x020e0bd0         @ dive if not yoshi
	bl     check_yellow_yoshi
	beq    0x020e0bd0         @ dive if yellow
	b      0x020e0bc0         @ tongue if not

@ called in multiple states
nsub_020e283c_ov_02:
	bne    0x020e2850         @ dive if not yoshi
	bl     check_yellow_yoshi
	beq    0x020e2850         @ dive if yellow
	b      0x020e2840         @ tongue if not

check_yellow_yoshi:
	ldr    r0, =currentYoshiColor
	ldrb   r0, [r0, #0x0]
	cmp    r0, #0x3
	bx     r14