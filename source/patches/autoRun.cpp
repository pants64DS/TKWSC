#include "Sound.h"

asm(R"(
nsub_02029784:
	beq    0x020297b0 @ skip if only start and/or select is pressed
	bl     UpdateAutoRunState
	cmp    r0, #0
	beq    0x020297b0
	b      0x02029788
)");

bool autoRunEnabled = false;

extern volatile const uint16_t hardwareButtons;
asm("hardwareButtons = 0x04000130");

extern "C" unsigned UpdateAutoRunState(unsigned buttons)
{
	// The button flags here are slightly different from Input.h
	constexpr unsigned r = 0x100;
	constexpr unsigned l = 0x200;
	constexpr unsigned y = 0x800;

	if (buttons & y) // if Y is pressed
	{
		if (const auto heldButtons = ~hardwareButtons; heldButtons & l && heldButtons & r)
		{
			Sound::UnkPlaySoundFunc(autoRunEnabled ? 1 : 38);

			autoRunEnabled = !autoRunEnabled;
		}
		else
			Sound::UnkPlaySoundFunc(8);
	}
	else if (buttons & (r | l)) // if L or R is pressed
		Sound::UnkPlaySoundFunc(8);

	// return non-zero if buttons other than L, R or Y are pressed
	return buttons & ~(r | l | y);
}

asm(R"(
@ Swap Y pressed / Y not pressed when auto-running is enabled
nsub_0202c78c:
	ldr    r0, =autoRunEnabled
	ldrb   r0, [r0]
	cmp    r0, #0
	ldrneh r0, [r9, #4]
	eornes r0, r0, #0x800
	strneh r0, [r9, #4]
	ldrb   r0, [r9, #0x16]
	b      0x0202c790

@ Disable run-up when auto-running is enabled
nsub_020d3044_ov_02:
	beq    0x020d3134
	ldr    r1, =autoRunEnabled
	ldrb   r1, [r1]
	cmp    r1, #0
	beq    0x020d3048
	b      0x020d3134

@ Fix swimming when auto-running is enabled
repl_020cda18_ov_02:
	ldr    r1, =autoRunEnabled
	ldrb   r1, [r1]
	cmp    r1, #0
	mov    r1, #2
	orreq  r1, #0x800
	bx     lr
)");
