#include "SM64DS_PI.h"
#include "MOM_Interface.h"
#include "extended_ks.h"
#include "flight_cam_ctrl.h"
#include "Actors/Bowser.h"
#include "Actors/SpikeBomb.h"

constinit auto intro =
	NewScript().
	PlayWindSound() ( 55, 264).
	PlayWindSound() (310, 330).
	Nop()(0x7fff).
	End();

static void SetWindCLPS(unsigned clpsID)
{
	(*LEVEL_OVERLAY.clps)[clpsID].behaviorID = 11;
	(*LEVEL_OVERLAY.clps)[clpsID].windID = 7;
}

static void ClearWindCLPS(unsigned clpsID)
{
	(*LEVEL_OVERLAY.clps)[clpsID].behaviorID = 0;
	(*LEVEL_OVERLAY.clps)[clpsID].windID = 255;
}

extern "C" void SetLevelEntranceState(Player& player, unsigned entranceMode);
asm("SetLevelEntranceState = 0x020c7dd0");

void repl_020e586c_ov_02(Player& player, unsigned entranceMode)
{
	if (LEVEL_ID == 40)
	{
		constexpr Fix12i introFlightSpeed = 180._f;

		player.unk764 = 0; // lsPtr
		player.horzSpeed = introFlightSpeed;
		player.flags |= Actor::UPDATE_DURING_STAR_SPAWNING | Actor::UPDATE_DURING_CUTSCENES;
		player.ChangeState(MOM_Interface::instance.launchStarState);

		ClearWindCLPS(9);
		ClearWindCLPS(10);

		intro.Run();
	}
	else
		SetLevelEntranceState(player, entranceMode);
}

constinit Bowser* introBowserPtr = nullptr;

// Delay Bowser's start dialogue
asm(R"(
nsub_021154e8_ov_3c:
	ldr    r1, [r0, #0x3a0]
	cmp    r1, #0
	bxeq   lr
	ldr    r1, [r1, #0x370]
	ldr    r2, =_ZN6Player7ST_WALKE
	cmp    r1, r2
	bxne   lr
	ldr    r1, =introBowserPtr
	str    r0, [r1]
	push   {r4, r14}
	b      0x021154ec
)");

extern constexpr Vector3 playerSpawnPos = {0._f, 10125._f, -512000._f};

asm(R"(
@ Player position
nsub_020fe7a0_ov_02:
	ldr    r2, =LEVEL_ID
	ldrb   r2, [r2]
	cmp    r2, #40
	addne  r2, r13, #0xc
	ldreq  r2, =playerSpawnPos
	b      0x020fe7a4
)");


void FlightCamCtrl::UpdateIntro(Camera& cam, Player& player)
{
	if (introTalkInterp == -1._f)
	{
		if (intro.IsRunning())
			introTalkInterp = 0._f;
		else
			return;
	}

	const bool talking = player.currState == &Player::ST_TALK;

	if (introTalkInterp == 1._f && !talking)
	{
		introTalkInterp = -1._f;
		introBowserPtr = nullptr;

		return;
	}

	Vector3 pos1 = {-5000._f, 9600._f - (player.pos.y >> 5), 10000._f};

	if (player.pos.z > -256._f && player.pos.x > -8600._f)
	{
		static constexpr Fix12i deckY = 0x94'700_f;
		static constexpr Fix12i transitionStart = 4096._f; // If this value stays, divisions by it can be optimized

		const Fix12i yDiff = transitionStart - player.pos.y + deckY;

		if (yDiff > 0._f)
		{
			const Fix12i cubic = yDiff / transitionStart * yDiff / transitionStart * yDiff;

			pos1.y -= cubic * (12000._f / transitionStart);
			pos1.x += cubic * (13000._f / transitionStart);
		}

		if (player.currState != &MOM_Interface::instance.launchStarState)
		{
			static constexpr Fix12i diveStartZ = 0x4e3'f58_f;

			pos1.z -= diveStartZ - player.pos.z << 3;
		}

		cam.pos = pos1 + (player.pos - pos1) * 1.08_f;

		if (talking)
		{
			if (intro.IsRunning())
			{
				Sound::LoadAndSetMusic_Layer1(67);

				// Reduce spike bomb draw distance to make them work with cannons
				for (SpikeBomb& spikeBomb : Actor::Iterate<SpikeBomb>())
					spikeBomb.drawDistAsr3 = Fix12i::max >> 3;

				using Egg = Actor::Alias<Actor, 9>;

				for (Actor& egg : Actor::Iterate<Egg>())
					egg.flags &= ~Actor::UPDATE_DURING_CUTSCENES;

				SetWindCLPS(9);
				SetWindCLPS(10);

				EndKuppaScript();
			}

			introTalkInterp.ApproachLinear(1._f, 0.01_f);

			const Fix12i t = SmoothStep(SmoothStep(introTalkInterp));
			const Vector3& bowserPos = introBowserPtr->pos;

			cam.pos = Lerp(cam.pos, bowserPos, t >> 1);

			cam.lookAt.x = Lerp(player.pos.x, bowserPos.x, t);
			cam.lookAt.y = Lerp(player.pos.y, bowserPos.y + 140._f, t);
			cam.lookAt.z = Lerp(player.pos.z, bowserPos.z, t);
		}
		else
		{
			cam.lookAt = player.pos;

			if (cam.lookAt.y < deckY)
				cam.lookAt.y = deckY;
		}
	}
	else
	{
		pos1 += (player.pos - pos1) * 1.08_f;

		static constexpr Fix12i transitionStart = -120000._f;
		static constexpr Fix12i transitionEnd = 11500._f;
		static constexpr Fix12i transitionLength = transitionEnd - transitionStart;
		static constexpr Fix12i vertOffs = 10125._f;

		const Vector3 pos0 =
		{
			player.pos.x * 0x0'fe8_f,
			(player.pos.y - vertOffs) * 0x0'fe8_f + vertOffs,
			player.pos.z - 1024._f
		};

		const Vector3 lookAt0 = {pos0.x, pos0.y, player.pos.z};

		Fix12i t = std::clamp((player.pos.z - transitionStart) / transitionLength, 0._f, 1._f);
		t = t * t * t * t;

		AssureUnaliased(cam.pos)    = Lerp(pos0, pos1, t);
		AssureUnaliased(cam.lookAt) = Lerp(lookAt0, player.pos, t);
	}
}

// Stop the player from screaming during the intro
bool repl_020e5234_ov_02(const Player& player, Player::State* spinState)
{
	return intro.IsRunning() || player.currState == spinState;
}

// Change the position of Bowser's textbox
asm(R"(
nsub_0201f5b4:
	ldr    r0, =LEVEL_ID
	ldrb   r0, [r0]
	cmp    r0, #40
	moveq  r0, #162
	movne  r0, #99
	b      0x0201f5b8

nsub_0201f5e8:
	rsbeq  r5, r5, #163
	rsbne  r5, r5, #100
	b      0x0201f5ec
)");
