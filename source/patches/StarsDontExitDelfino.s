nsub_020e9678_ov_02:
    beq 0x020e967c @ don't exit if course ID is 29 (hub)
    cmp r0, #0x16
    beq 0x020e967c @ don't exit if course ID is 22 (Delfino Plaza)
    b 0x020e968c   @ otherwise, exit level