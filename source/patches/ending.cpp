#include <cstdlib>
#include "SM64DS_PI.h"
#include "extended_ks.h"
#include "final_boss_dl.h"
#include "flight_cam_ctrl.h"

asm(R"(
Div64:
	push    {r4}
	ldr     r4, =#0x04000280
	mov     r12, #2
	strh    r12,[r4]
	str     r0,[r4, #0x10]
	str     r1,[r4, #0x14]
	str     r2,[r4, #0x18]
	str     r3,[r4, #0x1c]
wait1:
	ldrh    r0,[r4]
	ands    r0, r0, #0x8000
	bne     wait1
	ldr     r0,[r4, #0x20]
	ldr     r1,[r4, #0x24]
	pop     {r4}
	bx      r14
)");

extern "C" uint64_t Div64(uint64_t nom, uint64_t den);

consteval uint64_t fac(uint64_t n)
{
	return n ? n * fac(n - 1) : 1;
}

static Fix12i cos(short a)
{
	int b = a;
	if (b < 0) b = -b;
	
	bool flipped;

	if (b > 0x4000)
	{
		b = 0x8000 - b;
		flipped = true;
	}
	else flipped = false;

	const uint64_t x = (b * 0x3243F6A8885A3 + (1ull << 31)) >> 32; // Q31
	constexpr int64_t h = 1ull << 31;

	const uint64_t y2  = x  * x;         // Q62
	const uint64_t x2  = (y2 + h) >> 32; // Q30
	const uint64_t y4  = x2 * x2;        // Q60
	const uint64_t x4  = (y4 + h) >> 32; // Q28
	const uint64_t y6  = x2 * x4;        // Q58
	const uint64_t x6  = (y6 + h) >> 32; // Q26
	const uint64_t y8  = x4 * x4;        // Q56
	const uint64_t x8  = (y8 + h) >> 32; // Q24
	const uint64_t y10 = x4 * x6;        // Q54
	const uint64_t y12 = x6 * x6;        // Q52
	const uint64_t y14 = x6 * x8;        // Q50
	const uint64_t y16 = x8 * x8;        // Q48

	const uint64_t resQ48 = (uint64_t(1) << 48)
		- (((Div64(y2  >> 1, fac( 2)) << 1) + (1ull << (23 + 14))) >> 14)
		+ (((Div64(y4  >> 1, fac( 4)) << 1) + (1ull << (23 + 12))) >> 12)
		- (((Div64(y6  >> 1, fac( 6)) << 1) + (1ull << (23 + 10))) >> 10)
		+ (((Div64(y8  >> 1, fac( 8)) << 1) + (1ull << (23 +  8))) >> 8)
		- (((Div64(y10 >> 1, fac(10)) << 1) + (1ull << (23 +  6))) >> 6)
		+ (((Div64(y12 >> 1, fac(12)) << 1) + (1ull << (23 +  4))) >> 4)
		- (((Div64(y14 >> 1, fac(14)) << 1) + (1ull << (23 +  2))) >> 2)
		+  ((Div64(y16 >> 1, fac(16)) << 1));

	const Fix12i resQ12 ((resQ48 + (1llu << 35)) >> 36, as_raw);

	return flipped ? -resQ12 : resQ12;
}

static Fix12i sin(short a)
{
	return cos(90_deg - a);
}

static void ApproachCylindrical(Vector3& v, const Vector3& pivot,
	Fix12i targetHorzDist, Fix12i targetVertDist, short targetAngleY,
	int invApproachFactor, int maxAngleDelta = 180_deg)
{
	short angle = pivot.HorzAngle(v);
	Fix12i horzDist = pivot.HorzDist(v);
	const Fix12i approachFactor = 1._f / invApproachFactor;

	ApproachAngle(angle, targetAngleY, invApproachFactor, maxAngleDelta);

	horzDist = Lerp(horzDist, targetHorzDist, approachFactor);

	v.x = pivot.x + sin(angle) * horzDist;
	v.z = pivot.z + cos(angle) * horzDist;
	v.y = Lerp(v.y, pivot.y + targetVertDist, approachFactor);
}

asm(R"(
nsub_021312b4_ov_59:
	mov     r0, r5
	mov     r1, r4
	bl      PrepareStarSpawnCamera
	add     r12, r5, #0x100
	b       0x021312b8
)");

constinit Vector3 starStartPos = {};
constinit Vector3 prevLookAt = {};

// Runs before the star starts moving
extern "C" void PrepareStarSpawnCamera(const Actor& star, Camera& cam)
{
	starStartPos = star.pos;
	prevLookAt = cam.lookAt;

	cam.lookAt = Lerp(cam.lookAt, star.pos, 0.05_f);

	const short minAbsTargetAngle = 72_deg;
	short targetAngle = Atan2(star.pos.x, star.pos.z);

	if (std::abs(targetAngle) < minAbsTargetAngle)
	{
		if (cam.pos.z > 0._f)
		{
			if (cam.pos.x > 0._f)
				targetAngle = minAbsTargetAngle;
			else
				targetAngle = -minAbsTargetAngle;
		}
		else
		{
			if (star.pos.x > 0._f)
				targetAngle = minAbsTargetAngle;
			else
				targetAngle = -minAbsTargetAngle;
		}
	}

	ApproachCylindrical(
		cam.pos,
		star.pos,
		star.pos.HorzLen() * 1.15_f,
		star.pos.y,
		targetAngle,
		20, 3_deg
	);
}

extern constexpr Vector3 starTargetPos = {0._f, 500._f, 2000._f};
constinit uint8_t starSpawnCounter = 0;

asm(R"(
repl_02131330_ov_59: @ Skips an str that sets horzDist
	ldr    r1, =starTargetPos
	b      0x02131338

nsub_02131448_ov_59:
	str    r1, [r5, #0x60]
	ldr    r1, =starSpawnCounter
	ldrb   r3, [r1]
	cmp    r3, #30 @ The number of frames to wait before returning to normal camera
	addlo  r3, #1
	strlob r3, [r1]
	blo    0x021314ec
	mov    r3, #0
	strb   r3, [r1]
	b      0x0213144c

repl_021313c0_ov_59:
	ldrb   r1, [r5, #0x443]
	b      AdjustStarSpawnCamera
)");

// Runs while the star moves and 30 frames after
extern "C" void AdjustStarSpawnCamera(Vector3& starPos, uint8_t numBounces)
{
	const Fix12i horzDist = starPos.HorzDist(starTargetPos);
	const Fix12i horzDelta = starStartPos.HorzDist(starTargetPos) / 80;

	if (horzDist <= horzDelta)
	{
		starPos.x = starTargetPos.x;
		starPos.z = starTargetPos.z;
	}
	else
	{
		const Fix12i factor = horzDelta / horzDist;

		starPos.x = Lerp(starPos.x, starTargetPos.x, factor);
		starPos.z = Lerp(starPos.z, starTargetPos.z, factor);
	}

	const Fix12i targetPosX = CAMERA->pos.x < 0._f ? -1700._f : 1700._f;
	const Fix12i targetPosZ = std::min(starStartPos.z + 1000._f, 1200._f);

	if (starStartPos.z < targetPosZ)
	{
		CAMERA->pos.x = Lerp(CAMERA->pos.x, targetPosX, 0.015_f);
		CAMERA->pos.z = Lerp(CAMERA->pos.z, targetPosZ, 0.015_f);
	}

	if (numBounces == 0)
		CAMERA->pos.y = Lerp(CAMERA->pos.y, starPos.y, 0.04_f);
	else
		CAMERA->pos.y = Lerp(CAMERA->pos.y, starTargetPos.y, 0.04_f);

	CAMERA->lookAt = Lerp(prevLookAt, starPos, 0.2_f);
	prevLookAt = CAMERA->lookAt;

	static constexpr Fix12i r = 800._f;
	static constexpr unsigned rLsl12 = r.val;
	const unsigned zLsl12 = std::abs(starPos.z.val);

	if (zLsl12 <= rLsl12)
	{
		const Fix12i minAbsX =
		{
			Sqrt(
				static_cast<uint64_t>(rLsl12) * rLsl12 -
				static_cast<uint64_t>(zLsl12) * zLsl12
			),
			as_raw
		};

		if (starPos.x > 0._f)
		{
			if (starPos.x < minAbsX)
				starPos.x.ApproachLinear(minAbsX, horzDelta);
		}
		else if (-minAbsX < starPos.x)
			starPos.x.ApproachLinear(-minAbsX, horzDelta);
	}
}

asm(R"(
nsub_020c3c24_ov_02 = 0x020c3c28 @ Avoid setting player.pos.z to zero
nsub_020c3c30_ov_02 = 0x020c3c34 @ Avoid setting player.pos.x to zero
nsub_020c3bb0_ov_02 = 0x020c3bb8 @ Avoid setting player.pos.x to zero
nsub_020c3bbc_ov_02 = 0x020c3bc0 @ Avoid setting player.pos.z to player.yPosOnPole

@ Set player.pos.y to 0xc1'000_f instead of 0x15e'000_f
nsub_020c3cc4_ov_02:
	ldr    r0, =0xc1000
	b      0x020c3cc8

@ Skips instructions that decrease player.pos.z by 0x21'000_f
nsub_020c3b78_ov_02:
	mov    r1, r5
	bl     UpdateEndingAnim
	b      0x020c3b88

IncrementEndingState = 0x020c3cf0
UpdateEndingParticles = 0x020c37a4

nsub_020fd2d4_ov_02:
	ldr    r1, =LEVEL_ID
	ldrb   r1, [r1]
	cmp    r1, #40
	moveq  r1, #4
	movne  r1, #5
	cmp    r0, r1
	movlo  r0, r1
	b      0x020fd2dc
)");

constinit auto endingPart0 =
	NewScript().
	ActivatePlayer()    (0).
	ChangeMusic(24)     (0).
	Call([] { CURRENT_GAMEMODE = 2; }) (690).
	ChangeLevel(1, 0, 1)(690).
	End();

void repl_020c3c80_ov_02()
{
	// Make the current character use the right animation at the start of the ending:
	Player::ANIM_PTRS[PLAYER_ARR[0]->param1 & 3 | 0x300] = Player::ANIM_PTRS[0x300];

	// ANIM_PTRS is in overlay 2, which will be unloaded after the ending sequence
	// so the changed pointer doesn't need to be restored.

	endingPart0.Run();
}

[[gnu::target("thumb")]]
void FlightCamCtrl::UpdateEnding(Camera& cam, const Player& player)
{
	if (!endingPart0.IsRunning())
		return;

	const unsigned frame = KS_FRAME_COUNTER;

	if (frame < 10)
		return;
	else if (frame < 92)
	{
		ApproachCylindrical(cam.pos, player.pos, 670._f, 300._f, -0x6d1c, 20);

		const Vector3 targetLookAt = {player.pos.x, 310._f, player.pos.z};
		cam.lookAt = Lerp(cam.lookAt, targetLookAt, 0.05_f);
	}
	else if (frame < 152)
	{
		const Vector3 targetPos = {-1750._f, player.pos.y, 900._f};

		cam.lookAt.x = player.pos.x;
		cam.lookAt.y += 1.25_f;
		cam.lookAt.z = player.pos.z;

		cam.pos = Lerp(cam.pos, targetPos, 0.02_f);
	}
	else
	{
		static constinit Vector3 camTargetPos0;
		static constinit Fix12i interp = 0._f;

		if (frame == 152)
		{
			camTargetPos0 = cam.pos;
			interp = 0._f;
		}

		camTargetPos0.x += player.speed.x >> 2;
		camTargetPos0.y += player.speed.y >> 1;
		camTargetPos0.z += player.speed.z >> 2;

		const Fix12i yInterp = (frame - 152) * 0x0'004_f + 0.02_f;

		camTargetPos0.x = Lerp(camTargetPos0.x, player.pos.x,  0.03_f);
		camTargetPos0.y = Lerp(camTargetPos0.y, player.pos.y, yInterp);
		camTargetPos0.z = Lerp(camTargetPos0.z, player.pos.z,  0.03_f);

		if (frame < 204)
			cam.pos = camTargetPos0;
		else
		{
			Vector3 camTargetPos1 = {player.pos.x, 0._f, player.pos.z};
			camTargetPos1.Normalize();
			camTargetPos1 *= 800._f;
			camTargetPos1 += (player.pos - player.prevPos) << 1;
			camTargetPos1 += player.pos;

			const unsigned camDipStart = 270;
			const unsigned camDipEnd = 470;

			if (camDipStart < frame && frame < camDipEnd)
				camTargetPos1.y -= 1._f - cos(((frame - camDipStart) << 16) / (camDipEnd - camDipStart)) << 8;

			const Fix12i lookAtFactor = cam.pos.Dist(player.pos) >> 11;
			const Vector3 camTargetLookAt = Lerp(player.pos, player.prevPos, lookAtFactor);

			const Fix12i interpInterp = Fix12i
			{
				std::min(std::abs(int(frame) - 444), 0x20),
				as_raw
			};

			if (frame < 444)
			{
				interp = Lerp(interp, 1.25_f, interpInterp);

				cam.lookAt = Lerp(cam.lookAt, camTargetLookAt, interp >> 1);
			}
			else
			{
				interp = Lerp(interp, 0._f, interpInterp);

				cam.lookAt = Lerp(cam.lookAt, camTargetLookAt, 0x0'8a6_f);
			}

			cam.pos = Lerp(camTargetPos0, camTargetPos1, interp);
		}

		cam.lookAt = Lerp(cam.lookAt, player.pos, 0.2_f);
		cam.lookAt.y += player.speed.y >> 1;
	}
}

void UpdateDebug();

// At the beginning of Camera::Render
void hook_0200da0c(Camera& cam)
{
	flightCamCtrl.Update(cam);

	// UpdateDebug();
}

extern "C" void IncrementEndingState(Player& player);
extern "C" void UpdateEndingParticles(Player& player, const Vector3& offset);

extern "C" Player& UpdateEndingAnim(Player& player, int animFrame)
{
	player.motionAng.y = -150._deg;
	ApproachAngle(player.ang.y, player.motionAng.y, 1, 5._deg);

	if (animFrame <= 110)
		player.horzSpeed = 33._f;
	else if (119 <= animFrame && animFrame <= 142)
		player.horzSpeed = 33._f;
	else if (animFrame >= 150)
	{
		player.horzSpeed = 40._f;
		player.speed.y = 108._f;
		player.vertAccel = -2._f;
		player.SetAnim(0x4a, 0x40000000, 1._f, 0);
		Sound::PlayCharVoice(player.realChar, 4, player.camSpacePos);

		IncrementEndingState(player);
	}
	else
		player.horzSpeed = 6._f;

	return player;
}

// Called from the ending state main function after the first animation
void repl_020c3d5c_ov_02(Player& player)
{
	if (KS_FRAME_COUNTER >= 200)
	{
		player.vertAccel = 0._f;
		player.horzSpeed = 0;

		static constinit int pivotAngle;
		static constinit Vector3 pivot = {};

		if (KS_FRAME_COUNTER == 200)
		{
			static constexpr Fix12i pivotDist = 2200._f;

			pivot.x = player.pos.x - cos(player.ang.y) * pivotDist;
			pivot.z = player.pos.z + sin(player.ang.y) * pivotDist;

			pivotAngle = pivot.HorzAngle(player.pos);
		}

		Fix12i newHorzDist = player.pos.HorzDist(pivot);
		newHorzDist += pivot.y;
		const Fix12i prevPosZ = player.pos.z;

		const int deltaAngle = 1_deg + std::min<int>(KS_FRAME_COUNTER - 200, 0.735_deg);

		if (ApproachLinear(pivotAngle, -630_deg, deltaAngle))
			player.pos.z += 86._f;
		else
		{
			ApproachCylindrical(
				player.pos,
				pivot,
				newHorzDist,
				player.pos.y - pivot.y,
				pivotAngle,
				1
			);

			player.motionAng.y = player.ang.y = pivotAngle - 90_deg;
			pivot.y.ApproachLinear(10._f, 0.01_f);

			const Fix12i deltaZ = player.pos.z - prevPosZ;

			if (deltaZ > 0._f)
				pivot.z.ApproachLinear(0._f, std::min(deltaZ * 0x0'530_f, -pivot.z >> 4));
		}
	}

	player.UpdateAnim();

	const Vector3 offset = {0._f, KS_FRAME_COUNTER < 152 ? 96._f : 0._f, 0._f};

	UpdateEndingParticles(player, offset);

	if (PARTICLE_SYS_TRACKER)
	{
		const Vector3 offset = (player.pos - player.prevPos) * 0x0'140_f;

		for (auto* sysData : PARTICLE_SYS_TRACKER->contents.usedSystems)
		{
			if (sysData && sysData->system)
			{
				for (auto& particle : sysData->system->particleList)
					particle.posAsr3 += offset;
			}
		}
	}
}

// Don't set game mode to 0 when the next script isn't set at the end of the cutscene
asm("nsub_0200e894 = 0x0200e8a4");

// Make the big star count
void nsub_02131df0_ov_59()
{
	SAVE_DATA.stars[17] |= 0x80;
}

/*
asm("entranceID = 0x0209f268");
asm("levelOvStart = 0x0214eaa0");

extern int8_t entranceID;
extern char levelOvStart[];

// Use final boss sound settings in the ending cutscene in the hub
void repl_0202df3c(int ovID) // replaces a call to LoadObjBankOverlay
{
	LoadObjBankOverlay(ovID);

	if (ovID == 104 && entranceID == 15)
	{
		levelOvStart[0x7c] = 34;
		levelOvStart[0x7d] = 40;
		levelOvStart[0x7e] = 67;
	}
}
*/
