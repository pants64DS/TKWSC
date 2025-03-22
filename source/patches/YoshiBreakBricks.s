@ Make Yoshi be able to break brick blocks (credits to dy for the address and overlay)

@ When ground pounding
nsub_020B3838_ov_02:
	cmp r2, #0x4
	b 0x020B383C

@ When kicked / sweep kicked
nsub_020B3718_ov_02:
	cmp r2, #0x4
	b 0x020B371C
