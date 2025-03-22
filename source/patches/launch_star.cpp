#include "SM64DS_PI.h"
#include "MOM_interface.h"
#include "MOM_IDs.h"
#include "LaunchStar.h"

asm(R"(
nsub_020e30a0_ov_02: @ at the beginning of Player::ChangeState
	push   {r0, r1, lr}
	bl     ShouldPlayerEnterState
	cmp    r0, #0
	pop    {r0, r1, lr}
	moveq  r0, #1
	bxeq   lr
	push   {r4, r5, lr}
	b      0x020e30a4
)");

extern "C" bool ShouldPlayerEnterState(Player& player, const Player::State* newState)
{
	static constinit bool recursiveCall = false;

	if (!recursiveCall &&
		INPUT_ARR[player.playerID].buttonsPressed & Input::A &&
		player.cylClsn.hitFlags & 1 << 20)
	{
		Actor* actor = Actor::FindWithID(player.cylClsn.otherObjID);

		if (actor && actor->actorID == MOM_IDs::LAUNCH_STAR)
		{
			recursiveCall = true;
			MOM_Interface::instance.launchFromLaunchStar(player, static_cast<LaunchStar&>(*actor));
			recursiveCall = false;

			return false;
		}
	}

	return true;
}

asm(R"(
@ Don't change the player's state when warping in the launch star state
repl_020b0c54_ov_02:
	ldr    r12, [r0, #0x370]
	ldr    r3, =_ZN13MOM_Interface8instanceE
	ldr    r3, [r3]
	cmp    r3, r12
	movne  r3, #0
	bne    0x020ca1b8
	bx     lr

@ Don't change the camera's state and don't play the sound effect
repl_020b0d44_ov_02:
	ldr    r12, [r5, #0x370]
	ldr    r2, =_ZN13MOM_Interface8instanceE
	ldr    r2, [r2]
	cmp    r2, r12
	bne    0x0200d184
	b      0x020b0d50

@ Prevent the player's position from being changed by the warp
nsub_020b0adc_ov_02:
	ldr    r12, [r5, #0x370]
	ldr    r14, =_ZN13MOM_Interface8instanceE
	ldr    r14, [r14]
	cmp    r14, r12
	ldrne  r12, =MATRIX_SCRATCH_PAPER
	bne    0x020b0ae0
	b      0x020b0b20

@ Set the fade color to black
nsub_020b0b6c_ov_02:
	ldr    r12, [r5, #0x370]
	ldr    r3, =_ZN13MOM_Interface8instanceE
	ldr    r3, [r3]
	cmp    r3, r12
	moveq  r1, #0
	strh   r1, [r0, #0xc]
	b      0x020b0d58
)");

// The hooks above assume this
static_assert(offsetof(MOM_Interface, launchStarState) == 0);
