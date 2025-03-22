#include "SM64DS_PI.h"
#include "MOM_Interface.h"
#include "final_boss_dl.h"
#include "airship.h"
#include "Actors/Bowser.h"
#include "Actors/BowserFlame.h"
#include "Actors/SpikeBomb.h"

// Load overlays 98 and 102 in Bowser fight levels
asm(R"(
nsub_0202e02c:
	mov    r0, #0x62
	blx    r10
	mov    r0, #0x66
	blx    r10
	ldr    r4, =0x0211a5ff
	mov    r0, #0x24
	strb   r0, [r4] @ change Bowser's flags
	mov    r0, r10
	pop    {r4-r11, r14}
	b      0x0209098c @ proceed to load DLs
)");

asm(R"(
repl_02118ef4_ov_3c:
	mov    r0, r5
	mov    r1, r6
	b      IsPlayerNotOnBowsersLevel
)");

extern "C" bool IsPlayerNotOnBowsersLevel(const Player& player, const Actor& bowser)
{
	return player.isInAir || Abs(player.pos.y - bowser.pos.y) > 60._f;
}

// Change the radius and vertical offset of Bowser's WMC
asm(R"(
nsub_0211627c_ov_3c:
	mov    r2, #0x108000 @ = 264._f
	b      0x02116280
)");

// Make spike bombs use their Actor::pos when checking collision with Bowser
// and skip the distance to origin check
asm(R"(
nsub_0211856c_ov_3c:
	add    r0, r5, #0x5c
	b      0x02118598
)");

// Recover bomb hangers when their bombs explode
asm(R"(
nsub_021185c4_ov_3c:
	push   {r4, r14}
	mov    r1, #0
	bl     OnSpikeBombExplosion
	b      0x021185c8

nsub_021189c4_ov_3c:
	mov    r0, r4
	mov    r1, #1
	bl     OnSpikeBombExplosion
	ldr    r1, [r4, #0x5c]
	b      0x021189c8
)");

extern "C" SpikeBomb& OnSpikeBombExplosion(SpikeBomb& spikeBomb, bool touchedByPlayer)
{
	for (BombHanger& bombHanger : Actor::Iterate<BombHanger>())
	{
		if (bombHanger.GetBombUniqueID() == spikeBomb.uniqueID)
		{
			bombHanger.Recover(touchedByPlayer);
			break;
		}
	}

	return spikeBomb;
}

asm(R"(
nsub_02010da4:       @ Disable level boundaries
nsub_02118728_ov_3c: @ Stop spike bombs from recovering normally
	bx     lr
)");

// Make Bowser vulnerable to spike bombs without having to throw him
void repl_02112974_ov_3c(Bowser& bowser, void (&state)(Bowser&))
{
	state(bowser);

	if (bowser.CheckSpikeBombClsn())
	{
		if (--bowser.health < 1)
			bowser.subStateID = 4;
		else
			bowser.subStateID = 0xc;

		bowser.unk427 = 1;
	}
}

// Make Bowser roam in random directions when the player is either too high or below deck level
Player* repl_02115f7c_ov_3c(Bowser& bowser)
{
	Player* player = bowser.ClosestPlayer();

	if (player && bowser.wmClsn.IsOnGround())
	{
		if (player->pos.y > 1400._f || player->pos.y < 0._f || player->currState == &Player::ST_CLIMB)
			player = nullptr;
	}

	if (player) return player;

	static constinit int angle = 0;

	if (bowser.wmClsn.IsOnWall())
	{
		const Vector3& normal = bowser.wmClsn.sphere.wallResult.surfaceInfo.normal;

		angle = Atan2(normal.x, normal.z);
	}
	else if ((FRAME_COUNTER & 0xff) == 0)
		RandomIntInternal(&angle);

	bowser.horzDistToPlayer = Fix12i::max;
	bowser.horzAngleToPlayer = static_cast<short>(angle);

	return nullptr;
}

// Skip the instructions that normally set horzDistToPlayer
// and horzAngleToPlayer when they player can't be found
asm("nsub_02115f8c_ov_3c = 0x02115fa0");

// Reduce the spike bomb detonation radius to 416 fxu
asm(R"(
nsub_02118c44_ov_3c: @ skips a redundant call
	mov    r0, #0x1a0000
	b      0x02118c50
)");

// Make Bowser collide with the cage when the player spins him
asm(R"(
nsub_020da6f0_ov_02:
	push   {r0, r1, r14}
	mov    r0, r4
	bl     DetectBowserSpinClsn
	pop    {r0, r1, r14}
	b      0x020da6f4
)");

extern "C" void DetectBowserSpinClsn(Player& player, int nothing, short newAngleY)
{
	Bowser* bowser = Actor::Find<Bowser>();
	if (!bowser) return;

	const Fix12i l = player.pos.HorzDist(bowser->pos);

	const Vector3 p = {
		player.pos.x + l * Sin(newAngleY),
		bowser->pos.y,
		player.pos.z + l * Cos(newAngleY)
	};

	SphereClsn sphere;
	sphere.SetObjAndSphere(p, 200._f, nullptr);
	sphere.DetectClsn();

	if (sphere.resultFlags & SphereClsn::ON_WALL)
	{
		player.pos.x += sphere.pushback.x;
		player.pos.z += sphere.pushback.z;
	}

	player.ang.y = newAngleY;
}

// Don't enter ST_FIRST_PERSON when the player is on a moving airship
asm(R"(
nsub_020caa78_ov_02:
	mov    r0, r6
	bl     GetFlags2Offset
	cmp    r0, #0
	bne    0x020caa7c
	b      0x020caa90
)");

// returns 0 if the player is on an airship
extern "C" unsigned GetFlags2Offset(const Player& player)
{
	for (const Airship& airship : Actor::Iterate<Airship>())
	{
		if (airship.GetPlayerOnBoard() == &player && !airship.CannonUnlocked())
			return 0;
	}

	return 0x6ce;
}

// Prevent the player from climbing automatically when
// hanging next to a corner in a MovingMeshCollider
asm(R"(
nsub_020cfe50_ov_02:
	cmpne  r1, #0x230
	beq    0x020cfe80 @ the player should hold on to the ledge
	b      0x020cfe60 @ the player should climb up
)");

static_assert(Airship::staticActorID == 0x230);

asm(R"(
ContinueToClimbState:
	push   {r4-r7, r14}
	b      0x020cb5c0
)");

extern "C" bool ContinueToClimbState(Player&);

bool nsub_020cb5bc_ov_02(Player& player)
{
	static constexpr Fix12i maxYPosOnPole = 15._f;

	if (player.yPosOnPole > maxYPosOnPole)
		return ContinueToClimbState(player);

	Actor* clsnActor = Actor::FindWithID(player.cylClsn.otherObjID);

	if (clsnActor && clsnActor->actorID == Anchor::staticActorID)
	{
		player.yPosOnPole = maxYPosOnPole;

		if (INPUT_ARR[0].dirZ > 0._f)
			INPUT_ARR[0].dirZ = 0._f;
	}

	return ContinueToClimbState(player);
}

// Prevent spinies from despawning when their param1 is 0x6969
void repl_02125fcc_ov_4d(Actor& spiny)
{
	if (spiny.param1 != 0x6969)
		spiny.MarkForDestruction();
}

static constexpr auto crewActorIDs = std::to_array<uint16_t>
({
	0xce,  // Bob-omb
	0x120, // Coin
	0x122, // Blue Coin
	MOM_IDs::COLORED_GOOMBA,
	MOM_IDs::COLORED_GOOMBA_LARGE,
	MOM_IDs::KAMEK_SHOT,
	MOM_IDs::KAMEK
});

static_assert(std::ranges::is_sorted(crewActorIDs));

Actor* nsub_02010e70(Actor* actor)
{
	if (LEVEL_ID != 40 || !actor || actor->pos.y < 3000._f)
		return actor;

	if (!std::ranges::binary_search(crewActorIDs, actor->actorID))
		return actor;

	Fix12i minDist = Fix12i::max;
	Airship* closestAirShip = nullptr;

	for (Airship& airship : Actor::Iterate<Airship>())
	{
		const Fix12i newDist = actor->pos.HorzDist(airship.pos);

		if (newDist < minDist)
		{
			minDist = newDist;
			closestAirShip = &airship;
		}
	}

	if (closestAirShip)
		closestAirShip->AddCrewMember(*actor);

	return actor;
}

// Make seaweed 30% shorter in the last room inside the ship
void repl_020bc50c_ov_02(ModelAnim& weedModel, const Vector3* scale, void (&render)(ModelAnim&, const Vector3*))
{
	static constexpr Vector3 shorterScale = {1._f, 0.7_f, 1._f};

	if (LEVEL_ID == 40 && (weedModel.mat4x3.c3.x << 3) > 600._f)
		scale = &shorterScale;

	render(weedModel, scale);
}

// Disable camera rotation on each airship until it's cleared of enemies
asm(R"(
nsub_0200bb40:
	ldr    r4, =_ZN13MOM_Interface8instanceE + 4
	ldrb   r4, [r4]
	cmp    r4, #0
	orreq  r3, r3, #0x1000
	b      0x0200bb44

nsub_0200a23c:
	ldr    r14, =_ZN13MOM_Interface8instanceE + 4
	ldrb   r14, [r14]
	cmp    r14, #0
	orreq  r1, r1, #0x1000
	b      0x0200a240

nsub_0200debc:
	push   {r4, r5, r14}
	ldr    r4, =_ZN13MOM_Interface8instanceE + 4
	ldrb   r4, [r4]
	cmp    r4, #0
	beq    0x0200dec0
	ldr    r4, =INPUT_ARR
	ldr    r5, [r4, #4]
	bic    r5, #0x4000
	bic    r5, #0x40000000
	str    r5, [r4, #4]
	add    r4, r0, #0x100
	ldrh   r5, [r4, #0x7c]
	strh   r5, [r4, #0x9e]
	mov    r5, #0
	strh   r5, [r4, #0xa0]
	b      0x0200dec0
)");

// The hooks above assume this
static_assert(offsetof(MOM_Interface, camRotationDisabled) == 4);

// Compare the star count to 101 instead of 150 when deciding
// which message Bowser should say after being defeated
asm(R"(
nsub_02112f84_ov_3c:
	cmp    r0, #101
	b      0x02112f88
)");

static bool ShouldUseMeshCollision(const BowserFlame& flame)
{
	const Fix12i absPosX = Abs(flame.pos.x);

	return absPosX < 3130._f &&
	(
		flame.pos.z > 4300._f ||
		flame.pos.z < -3600._f ||
		flame.pos.z > 3500._f && absPosX > 1800._f ||
		flame.pos.y < 290._f &&
		(
			absPosX > 2020._f ||
			flame.pos.x < -1450._f && flame.pos.z < -3010._f
		)
	);
}

static void UpdateTreeCollision(BowserFlame& flame)
{
	static constexpr Fix12i treeRadius = 600._f;
	static constexpr int64_t minDistToOriginSquared = Sqr<int64_t>(treeRadius.val);

	const int64_t distToOriginSquared = Sqr<int64_t>(flame.pos.x.val) + Sqr<int64_t>(flame.pos.z.val);

	if (distToOriginSquared <= minDistToOriginSquared)
	{
		const Fix12i distToOrigin = {Sqrt(distToOriginSquared), as_raw};
		const Fix12i factor = treeRadius / distToOrigin;

		flame.pos.x *= factor;
		flame.pos.z *= factor;
	}
}

static void UpdateHardcodedCollision(BowserFlame& flame)
{
	static constexpr Fix12i floorY = 148._f;

	static constexpr unsigned affectedFlags =
		WithMeshClsn::ON_GROUND |
		WithMeshClsn::JUST_HIT_GROUND |
		WithMeshClsn::JUST_LEFT_GROUND;

	flame.wmClsn.flags &= ~affectedFlags;

	if (Abs(flame.pos.x) >= 3130._f) return;

	if (flame.pos.y <= floorY)
	{
		flame.pos.y = floorY;
		flame.wmClsn.flags |= WithMeshClsn::ON_GROUND;

		if (flame.prevPos.y > floorY)
			flame.wmClsn.flags |= WithMeshClsn::JUST_HIT_GROUND;
	}
	else if (flame.prevPos.y <= floorY)
		flame.wmClsn.flags |= WithMeshClsn::JUST_LEFT_GROUND;

	UpdateTreeCollision(flame);
}

static void CheckFlameClsnWithPlayer(BowserFlame& flame)
{
	if (PLAYER_ARR[0])
	{
		Player& player = *PLAYER_ARR[0];

		if ((flame.cylClsn.flags1 & CylinderClsn::DISABLED) == 0 &&
			flame.pos.y < player.cylClsn.pos.y + 150._f &&
			player.cylClsn.pos.y < flame.pos.y + 80._f &&
			flame.pos.HorzDist(player.cylClsn.pos) < 80._f
		)
			player.Burn();
	}
}

asm(R"(
nsub_02117728_ov_3c:
	mov    r0, r4
	pop    {r4, r14}
	b      CheckFlameClsn
)");

extern "C" int CheckFlameClsn(BowserFlame& flame)
{
	if (flame.vertAccel != 0._f)
	{
		if (ShouldUseMeshCollision(flame))
			flame.wmClsn.UpdateDiscreteNoLava();
		else
			UpdateHardcodedCollision(flame);

		if (flame.currState != 4 && flame.wmClsn.IsOnGround())
		{
			flame.speed.y = 0;
			flame.vertAccel = 0;
		}
	}

	CheckFlameClsnWithPlayer(flame);

	return 1;
}

asm(R"(
nsub_020deb28_ov_02:
	mvn    r0, #0x680000 @ the max height of the wind
	b      0x020deb2c

nsub_020dec8c_ov_02:
	ldr    r1, [r5, #0x370]
	ldr    r14, =0x02110274 @ ST_WIND_CARRY
	cmp    r1, r14
	beq    0x020deca0
	add    r1, r3, #0xc0000 @ the negative of the y-coordinate below which the wind activates
	mvns   r1, r1
	b      0x020dec90

IsFallingToPit = 0x020ca270
)");

extern "C" bool IsFallingToPit(const Player& player);

// Prevent the camera from teleporting when the player falls into the hole
bool repl_02008cdc(const Player& player)
{
	return IsFallingToPit(player) || player.floorBehavID == 19;
}

// Stop the camera from rotating when the player is floating on wind
int repl_0200a720(short& angle, short targetAngle, int invFactor, int maxDelta, int minDelta)
{
	if (PLAYER_ARR[0] && PLAYER_ARR[0]->currState == &Player::ST_WIND_CARRY)
		return 0;

	return ApproachAngle(angle, targetAngle, invFactor, maxDelta, minDelta);
}

static Fix12i MoveAwayFromZero(Fix12i val, Fix12i dist)
{
	if (val < 0._f)
		return val - dist;
	else
		return val + dist;
}


void repl_0200de68(Camera& cam)
{
	if (!GAME_PAUSED || LEVEL_ID != 40)
	{
		cam.View::Render();

		return;
	}

	static constexpr Fix12i horzDist = 19000._f;

	const short angle = cam.ownerPos.y < 0._f ? 60_deg : 45_deg;

	const Fix12i xDist = horzDist * Sin(angle);
	const Fix12i zDist = horzDist * Cos(angle);

	const Vector3 pos = {
		MoveAwayFromZero(cam.ownerPos.x, xDist),
		cam.ownerPos.y < 0._f ? -1000._f : cam.ownerPos.y + 3800._f,
		MoveAwayFromZero(cam.ownerPos.z, zDist)
	};

	const Vector3 lookAt = {
		0._f,
		3500._f,
		cam.ownerPos.z < 0._f ? 0._f : 3000._f
	};

	INV_VIEW_MATRIX_ASR_3.c2 = (pos - lookAt).NormalizedTwice();
	INV_VIEW_MATRIX_ASR_3.c0 = Vector3::Temp(INV_VIEW_MATRIX_ASR_3.c2.z, 0._f, -INV_VIEW_MATRIX_ASR_3.c2.x).NormalizedTwice();
	INV_VIEW_MATRIX_ASR_3.c1 = INV_VIEW_MATRIX_ASR_3.c2.Cross(INV_VIEW_MATRIX_ASR_3.c0).NormalizedTwice();
	INV_VIEW_MATRIX_ASR_3.c3 = pos >> 3;

	VIEW_MATRIX_ASR_3 = INV_VIEW_MATRIX_ASR_3.Inverse();
}

// Prevent both types of flames that Bowser spits from spawning caps
asm("nsub_020f89d4_ov_02 = 0x020f8ab4");
asm("nsub_0211733c_ov_3c = 0x0211741c");

// Allow Yoshi to grab Bowser's tail using his tongue
asm(R"(
nsub_02115dd0_ov_3c:
	ands    r1, r1, #0x9000
	b       0x02115dd4

nsub_020daca0_ov_02:
	ldrh    r1, [r4, #0xc]
	mov     r2, #0x8b        @ The actor ID of Bowser's tail divided by 2
	cmp     r1, r2, lsl #1
	popne   {r4, r5, r15}
	mov     r0, r5
	mov     r1, r4
	bl      0x020dacb4
	mov     r0, #1
	pop     {r4, r5, r15}
)");

/*
asm(R"(
@ Camera position
nsub_0200d000:
	ldr    r0, =LEVEL_ID
	ldrb   r0, [r0]
	cmp    r0, #40
	popne  {r4, r15}
	mov    r0, #0
	mov    r1, #0
	ldr    r2, =-0x7D000000
	add    r4, r4, #0x8c
	stmia  r4, {r0-r2}
	pop    {r4, r15}
)");
*/


/*
asm(R"(
@ change starting level
nsub_020CC380_ov_07:
    mov r0, #40
    b   0x020CC384
)");
*/
