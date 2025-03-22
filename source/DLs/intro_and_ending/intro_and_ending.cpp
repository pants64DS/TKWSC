#include <algorithm>
#include "SM64DS_PI.h"
#include "extended_ks.h"
#include "MOM_IDs.h"

using ColoredCoin = Actor::Alias<Actor, MOM_IDs::COLORED_COIN>;
using Peach       = Actor::Alias<Actor, MOM_IDs::PEACH_NPC>;
using Toad        = Actor::Alias<Actor, MOM_IDs::COLORED_TOAD_NPC>;
using Berry       = Actor::Alias<Actor, MOM_IDs::BERRY>;
using SilverStar  = Actor::Alias<Actor, 0xb3>;
using Door        = Actor::Alias<Actor, 0x161>;
using Flame       = Actor::Alias<Actor, 0x13c>;

asm("AdvancePlayerModelAnim = 0x020bedd4");

extern "C" void AdvancePlayerModelAnim(Player& player);

struct NoAnimChange_InitCleanup {};
struct NoAnimChange_Main {};

template<>
bool Player::CustomStateFunc<NoAnimChange_InitCleanup>()
{
	return true;
}

constinit Fix12i playerAnimEndFrame = -1;

template<>
bool Player::CustomStateFunc<NoAnimChange_Main>()
{
	if (playerAnimEndFrame >= 0._f)
	{
		if (bodyModels[param1 & 3]->currFrame >= playerAnimEndFrame)
			return true;
	}

	AdvancePlayerModelAnim(*this);

	return true;
}

constinit Player::State ST_NO_ANIM_CHANGE =
{
	&Player::CustomStateFunc<NoAnimChange_InitCleanup>,
	&Player::CustomStateFunc<NoAnimChange_Main>,
	&Player::CustomStateFunc<NoAnimChange_InitCleanup>,
};

asm("KS_PLAYER_FUNC_3 = 0x021107ac");
extern unsigned KS_PLAYER_FUNC_3;

constexpr Vector3_16 marioPos = { -6553, -616, 7823 };
constexpr Vector3_16 startTarget = { 220, -290, 0 };
constexpr Vector3_16 startPos = { 220, 350, 1799 };
constexpr Vector3_16 point1 = { 220,  700, 3359 };
constexpr Vector3_16 point2 = { 220,  400, 2259 };
constexpr Vector3_16 point3 = { 270, -400, 5559 };
constexpr Vector3_16 point4 = { 270, -400, 4099 };
constexpr Vector3_16 point5 = { -110, -400, 6966 };
constexpr Vector3_16 point6 = { -1499, -400, 7904 };
constexpr Vector3_16 point7 = { -3353,  -61, 7840 };
constexpr Vector3_16 point8 = { -6111,  244, 7616 };
constexpr Vector3_16 point9 = { -4759, -342, 10925 };
constexpr Vector3_16 point10 = { -6994, -115, 7813 };
constexpr short approachFactor = 70;

constinit auto intro =
	NewScript().
	ChangeMusic(90) (1).
	SetPlayerPosAndAngleY(marioPos, 0x4000) (2).
	ActivatePlayer() (2).
	SetCamTargetAndPos (startTarget, startPos) (1).
	AdjustCamPosDec    (point1, approachFactor) (2,   59).
	AdjustCamTargetDec (point2, approachFactor) (2,   59).
	AdjustCamPosDec    (point3, approachFactor) (60,  119).
	AdjustCamTargetDec (point4, approachFactor) (60,  119).
	AdjustCamPosDec    (point5, approachFactor) (120, 179).
	AdjustCamTargetDec (point3, approachFactor) (120, 179).
	AdjustCamPosDec    (point6, approachFactor) (180, 239).
	AdjustCamTargetDec (point5, approachFactor) (180, 239).
	AdjustCamPosDec    (point7, approachFactor) (240, 299).
	AdjustCamTargetDec (point6, approachFactor) (240, 299).
	AdjustCamPosDec    (point8, approachFactor) (300, 359).
	AdjustCamTargetDec (point9, approachFactor + 20) (300, 359).
	AdjustCamPosDec    (point10, 199) (360, 419).
	AdjustCamTargetDec (marioPos, 199) (360, 419).
	End();

constexpr Fix12i groundY = -639._f;
constexpr Vector3 spruceTop = {232, 540, 2267};
constexpr Vector3_16 peachPos = {266, static_cast<int>(groundY), 3400};

static const Vector3& GetStarSpawnOffset(const Player& player)
{
	static constexpr Vector3 wariosOffset = { 0._f,   0._f, -80._f};
	static constexpr Vector3 normalOffset = {70._f,  40._f, -70._f};

	if (player.param1 == 2)
		return wariosOffset;
	else
		return normalOffset;
}

static const Vector3& GetStarThrowOffset(const Player& player)
{
	static constexpr Vector3 wariosOffset = { 0._f, 150._f,  70._f};
	static constexpr Vector3 normalOffset = {70._f, 130._f,  70._f};

	if (player.param1 == 2)
		return wariosOffset;
	else
		return normalOffset;
}

static Fix12i MakeInterpolator(unsigned startFrame, unsigned endFrame)
{
	const Fix12i nom = {KS_FRAME_COUNTER - startFrame, as_raw};
	const Fix12i den = {endFrame - startFrame, as_raw};

	return std::clamp(nom / den, 0._f, 1._f);
}

static void SendToHell(Actor& actor)
{
	actor.pos.y -= 16000._f;
}

static void BringBackFromHell(Actor& actor)
{
	actor.pos.y += 16000._f;
}

template<class T>
static void HideActorsBehindSpruce()
{
	for (auto& actor : Actor::Iterate<T>())
	{
		if (actor.pos.z < spruceTop.z - 40._f)
			SendToHell(actor);
		else
			actor.drawDistAsr3 = Fix12i::max >> 3;
	}
}

template<class T>
static void ShowActorsBehindSpruce()
{
	for (auto& actor : Actor::Iterate<T>())
	{
		actor.flags = 0;

		if (actor.pos.z < spruceTop.z - 40._f)
			BringBackFromHell(actor);
	}
}

static void Celebrate(Player& player)
{
	player.ChangeState(ST_NO_ANIM_CHANGE);
	player.SetAnim(0x19, Animation::NO_LOOP, 1._f, 0);
}

constexpr unsigned fallEnd        = 204;
constexpr unsigned runStart       = 230;
constinit unsigned runEnd         = 0x10000;
constexpr unsigned longJumpStart  = runStart + 29;
constexpr unsigned camInterpStart = longJumpStart;
constexpr unsigned camInterpEnd   = longJumpStart + 80;
constexpr unsigned firstJumpCut   = longJumpStart + 70;
constexpr unsigned secondJumpCut  = firstJumpCut + 92;
constexpr unsigned thirdJumpCut   = secondJumpCut + 104;
constexpr unsigned windupStart    = thirdJumpCut + 30;
constexpr unsigned starSpawnStart = windupStart + 16;
constexpr unsigned starSpawnEnd   = windupStart + 32;
constexpr unsigned fourthJumpCut  = starSpawnEnd + 35;
constexpr unsigned throwStart     = fourthJumpCut + 25;
constexpr unsigned throwEnd       = throwStart + 120;
constexpr unsigned bounceStart    = throwEnd + 1;
constexpr unsigned bounceEnd      = bounceStart + 40;
constexpr unsigned fifthJumpCut   = bounceEnd + 62;

consteval unsigned GetCelebrationStart(unsigned ord)
{
	return fifthJumpCut + 55 + ord * 5;
}

constexpr unsigned zoomOutStart = GetCelebrationStart(3) + 24;
constexpr unsigned zoomOutEnd   = zoomOutStart + 210;

constinit Vector3 savedCamPos;
constinit Vector3 savedLookAt;
constinit std::array<Player*, 3> otherPlayers = {};
constinit Actor* star;
constinit Fix12i ringCenterZ;

asm("KS_SPAWNED_PLAYER_UNIQUE_IDS = 0x0209b284");
extern unsigned KS_SPAWNED_PLAYER_UNIQUE_IDS[4];

asm("UpdateEndingParticles = 0x020c37a4");
extern "C" void UpdateEndingParticles(Player& player, const Vector3& offset);

extern "C" void UpdateBigStar(Actor& cutsceneObject);
asm("UpdateBigStar = 0x020f72bc");

constinit auto ending =
	NewScript().

	// Init
	SetEntranceMode(0) (0).
	ActivatePlayer<Mario>() (0).
	ActivatePlayer<Luigi>() (0).
	ActivatePlayer<Wario>() (0).
	ActivatePlayer<Yoshi>() (0).
	SetPlayerPosAndAngleY({270, 18000, 10000}, -180_deg) (0).
	Call([](Player& player)
	{
		for (unsigned i = 0; unsigned uniqueID : KS_SPAWNED_PLAYER_UNIQUE_IDS)
		{
			if (Player* otherPlayer = static_cast<Player*>(Actor::FindWithID(uniqueID)))
			{
				otherPlayer->flags &= ~(Actor::NO_BEHAVIOR_IF_OFF_SCREEN | Actor::NO_RENDER_IF_OFF_SCREEN);

				// Make the Yoshi green
				if ((otherPlayer->param1 & 3) == 3)
					otherPlayer->unk61c = otherPlayer->bodyModels[3]->data.materials[0].paletteInfo;

				otherPlayers[i++] = otherPlayer;
			}

			if (i == 3) break;
		}

		otherPlayers[0]->pos = {peachPos.x + 200._f, groundY, peachPos.z - 200._f};
		otherPlayers[1]->pos = {-3472, -582, 10531}; // will change on the first jump cut
		otherPlayers[2]->pos = {peachPos.x + 400._f, groundY, peachPos.z - 100._f};

		for (auto& peach : Actor::Iterate<Peach>())
		{
			if (peach.param1 == 0)
			{
				peach.pos = peachPos;
				peach.flags |= Actor::UPDATE_DURING_CUTSCENES;
				peach.flags &= ~(Actor::NO_BEHAVIOR_IF_OFF_SCREEN | Actor::NO_RENDER_IF_OFF_SCREEN);
				peach.drawDistAsr3 = Fix12i::max >> 3;
			}
			else
				peach.MarkForDestruction();
		}

		player.ChangeState(Player::ST_FALL);
		player.featherCapTimeRemaining = ~0;
		AMBIENT_SOUND_EFFECTS_DISABLED = true;

		for (auto& toad : Actor::Iterate<Toad>())
			SendToHell(toad);

		HideActorsBehindSpruce<ColoredCoin>();
		HideActorsBehindSpruce<Berry>();

		for (auto& silverStar : Actor::Iterate<SilverStar>())
			silverStar.MarkForDestruction();

		for (auto& flame : Actor::Iterate<Flame>())
			flame.drawDistAsr3 = Fix12i::max >> 3;
	})
	(0).

	Call([]
	{
		for (auto& door : Actor::Iterate<Door>())
			SendToHell(door);
	})
	(longJumpStart + 45).

	// Disable fall damage
	Call([](Player& player) { player.jumpPeakHeight = player.pos.y; }) (0, -1).

	// Update falling
	EnableAmbientSoundEffects() (30).
	GivePlayerWingsAndDrop() (0, fallEnd).
	Call([](Player& player)
	{
		if (!player.isInAir) return;

		player.pos.y -= 50._f;
		player.pos.z -= 5._f;

		CAMERA->lookAt = player.pos;
		CAMERA->lookAt.y += std::max(KS_FRAME_COUNTER - 60, 0u) * 0.7_f;

		const short angle = KS_FRAME_COUNTER * 195 - 10_deg;
		const Fix12i yOffset = {KS_FRAME_COUNTER * 52, as_raw};

		CAMERA->pos = player.pos + Vector3::Temp(Cos(angle), yOffset - 2.5_f, Sin(angle)) * 1000._f;

		UpdateEndingParticles(player, {});
	})
	(0, fallEnd).

	// Run and long jump towards the tree
	SendPlayerInput(peachPos, 0.6_f) (runStart, runStart + 4).
	Call([](Player& player)
	{
		if (runEnd != 0x10000) return;

		if (player.pos.z > 3980._f)
		{
			short params[4] = {peachPos.x, peachPos.y, peachPos.z, 0x1000};

			(player.*KS_PLAYER_FUNCTIONS[1])(reinterpret_cast<char*>(&params), runStart + 5, -1);
		}
		else
			runEnd = KS_FRAME_COUNTER;
	})
	(runStart + 5, -1).

	Call([](Player& player)
	{
		Camera& cam = *CAMERA;

		cam.lookAt.x = cam.ownerPos.x;
		cam.lookAt.y += (player.pos.y - player.prevPos.y) * 0.35_f;
		cam.lookAt.z = cam.ownerPos.z;
	})
	(runStart, firstJumpCut - 1).

	PlayerHoldButtons(Input::R) (longJumpStart - 1).
	PlayerHoldButtons(Input::B) (longJumpStart).

	// Handle animations
	Call([](Player& player)
	{
		CURRENT_GAMEMODE = 2;
		char state = 5;

		if (KS_FRAME_COUNTER > runEnd + 25)
			(player.*KS_PLAYER_FUNCTIONS[10])(&state, longJumpStart, -1);

		for (Player* otherPlayer : otherPlayers)
			(otherPlayer->*KS_PLAYER_FUNCTIONS[10])(&state, longJumpStart, -1);
	})
	(longJumpStart, fifthJumpCut).

	// Make the player get smoothly through the invisible wall
	Call([](Player& player) {player.isIntangibleToMesh = true;  }) (longJumpStart, longJumpStart + 29).
	Call([](Player& player) {player.isIntangibleToMesh = false; }) (longJumpStart + 30).

	Call([](Camera& cam) { savedCamPos = cam.pos; }) (camInterpStart).

	Call([](Camera& cam)
	{
		const Fix12i t = MakeInterpolator(camInterpStart, camInterpEnd);

		AssureUnaliased(cam.pos) = Lerp(savedCamPos,
			cam.ownerPos + Vector3::Temp(-100._f, 300._f, 700._f),
			SmoothStep(t)
		);

		const Fix12i maxLookAtY = cam.ownerPos.y + 300._f;

		if (cam.lookAt.y > maxLookAtY)
			cam.lookAt.y = Lerp(cam.lookAt.y, maxLookAtY, 0.2_f);
	})
	(camInterpStart, firstJumpCut - 1).

	Call([]
	{
		ShowActorsBehindSpruce<ColoredCoin>();
		ShowActorsBehindSpruce<Berry>();

		for (auto& door : Actor::Iterate<Door>())
			BringBackFromHell(door);

		otherPlayers[1]->pos = {peachPos.x - 200._f, groundY, peachPos.z - 200._f};
		AMBIENT_SOUND_EFFECTS_DISABLED = true;
	})
	(firstJumpCut).

	Call([](Player& player)
	{
		CAMERA->pos = {1073, -200, 2400};
		CAMERA->lookAt = player.pos;
		CAMERA->lookAt.y += 300._f;
		CAMERA->lookAt.x -= 100._f;
	})
	(firstJumpCut, secondJumpCut - 1).

	Call([](Camera& cam)
	{
		cam.lookAt.x = peachPos.x;
		cam.lookAt.y = groundY + 100._f;
		cam.lookAt.z = peachPos.z + (cam.ownerPos.z - peachPos.z) * 0.2_f;

		static constinit short angle = -75_deg;

		cam.pos.y = groundY + 300._f;
		cam.pos.x = cam.lookAt.x + Sin(angle) * 1200._f;
		cam.pos.z = cam.lookAt.z + Cos(angle) * 650._f;

		angle += 0.5_deg;
	})
	(secondJumpCut, thirdJumpCut - 1).

	Call([](Camera& cam)
	{
		cam.lookAt.x = cam.ownerPos.x;
		cam.lookAt.y = cam.ownerPos.y + 140._f;
		cam.lookAt.z = cam.ownerPos.z + 3._f;

		cam.pos.x = cam.ownerPos.x + 400._f;
		cam.pos.y = cam.ownerPos.y + 180._f;
		cam.pos.z = cam.ownerPos.z - 200._f;
	})
	(thirdJumpCut, fourthJumpCut - 1).

	// Star sound effects
	PlayerPlaySoundSSAR3(0x61) (windupStart + 10). // launch star
	PlayerPlaySoundSSAR3(0x57) (windupStart + 10). // big star spawn
	PlayerPlaySoundSSAR3(0x57) (throwStart).       // big star spawn
	PlayerPlaySoundSSAR3(0x73) (throwStart).       // big star fly
	PlayerPlaySoundSSAR3(0x74) (bounceStart).      // big star bounce

	Call([](Player& player)
	{
		CURRENT_GAMEMODE = 0;
		player.ChangeState(ST_NO_ANIM_CHANGE);
		player.SetAnim(0x30, Animation::NO_LOOP, -1._f, 30);
	})
	(windupStart).

	Call([](Player& player)
	{
		star = Actor::Spawn(0x0160, 0x18, player.pos + GetStarSpawnOffset(player));
		star->ang.y = -90_deg;
	})
	(starSpawnStart).

	Call([](Player& player)
	{
		const short angle = star->ang.y;
		UpdateBigStar(*star);

		const Fix12i t = MakeInterpolator(starSpawnStart, starSpawnEnd);

		star->pos = player.pos + Lerp(GetStarSpawnOffset(player), GetStarThrowOffset(player), t * t);
		star->ang.y = angle + (0x1000 - t.val) * 5;

		star->scale = {t, t, t};
	})
	(starSpawnStart, fourthJumpCut - 1).

	Call([](Player& player)
	{
		const Fix12i t = MakeInterpolator(throwStart, throwEnd);

		const Vector3 p0 = player.pos + GetStarThrowOffset(player);
		const Vector3& p2 = spruceTop;

		const Vector3 p1 = {
			p0.x + p2.x >> 1,
			2500._f,
			p0.z + p2.z >> 1,
		};

		star->pos = Lerp(Lerp(p0, p1, t), Lerp(p1, p2, t), t);

		const short angle = star->ang.y;
		UpdateBigStar(*star);
		if (t == 0._f) star->ang.y = angle;

		const Fix12i scale = Lerp(1._f, 3._f, t);
		star->scale = {scale, scale, scale};

		CAMERA->lookAt = star->pos + (1._f - t) * Vector3::Temp(0._f, 150._f, -500._f);
		CAMERA->pos.y = Lerp(player.pos.y + GetStarThrowOffset(player).y, star->pos.y, t);
	})
	(fourthJumpCut, throwEnd).

	Call([](Player& player)
	{
		Sound::PlayCharVoice(player.param1, player.param1 == 1 ? 2 : 10, player.camSpacePos);
		player.SetAnim(0x30, Animation::NO_LOOP, 1._f, 0);
	})
	(throwStart).

	Call([](Camera& cam)
	{
		static constinit short angle = -85_deg;

		cam.pos.x = star->pos.x + Sin(angle) * 1200._f;
		cam.pos.z = star->pos.z + Cos(angle) * 800._f;

		if (KS_FRAME_COUNTER > throwStart)
			ApproachAngle(angle, 90_deg, 100, 1_deg);
	})
	(fourthJumpCut, fifthJumpCut - 1).

	Call([](Camera& cam)
	{
		const Fix12i t = MakeInterpolator(bounceStart, bounceEnd);
		const Fix12i y1 = spruceTop.y + 400._f;

		star->pos.y = Lerp(Lerp(spruceTop.y, y1, t), Lerp(y1, spruceTop.y, t), t);
		cam.lookAt.y = Lerp(spruceTop.y, star->pos.y, 0.666_f);
	})
	(bounceStart, bounceEnd).

	Call([](Player& player)
	{
		CURRENT_GAMEMODE = 0;
		playerAnimEndFrame = 32;

		const Fix12i ringRadius  = player.pos.z - peachPos.z >> 1;
		ringCenterZ              = player.pos.z + peachPos.z >> 1;

		std::array<Player*, 4> players = {
			otherPlayers[0],
			otherPlayers[1],
			&player,
			otherPlayers[2]
		};

		for (unsigned i = 0; i < players.size(); ++i)
		{
			const short angle = 180_deg - 72_deg * (i + 1);

			players[i]->pos.x = peachPos.x  + Sin(angle) * ringRadius;
			players[i]->pos.z = ringCenterZ + Cos(angle) * ringRadius;
			players[i]->ang.y = angle - 180_deg;
		}

		HideActorsBehindSpruce<ColoredCoin>();
		HideActorsBehindSpruce<Berry>();

		for (auto& door : Actor::Iterate<Door>())
			SendToHell(door);

		for (auto& flame : Actor::Iterate<Flame>())
			SendToHell(flame);
	})
	(fifthJumpCut - 1).

	Call([](Camera& cam)
	{
		const Fix12i t = SmoothStep(MakeInterpolator(fifthJumpCut, fifthJumpCut + 70));

		cam.lookAt = savedLookAt = Lerp(
			Vector3::Temp(spruceTop.x, spruceTop.y - 80, spruceTop.z),
			Vector3::Temp(peachPos.x - 7, groundY + 75, ringCenterZ),
			t
		);

		cam.pos = savedCamPos = Lerp(
			Vector3::Temp(-300, groundY + 400, 3900),
			Vector3::Temp( 535, groundY + 189, 4050),
			t * 1.085_f
		);
	})
	(fifthJumpCut, zoomOutStart - 1).

	Call([](Camera& cam)
	{
		cam.pos = Lerp(savedCamPos,
			Vector3::Temp(1500, 1000, 5000),
			SmoothStep(MakeInterpolator(zoomOutStart, zoomOutEnd))
		);

		const Fix12i t = SmoothStep(Sqr(MakeInterpolator(zoomOutStart + 20, zoomOutEnd)));

		cam.lookAt.x = cam.pos.x + Lerp(savedLookAt.x - savedCamPos.x, -800, t);
		cam.lookAt.y = cam.pos.y + Lerp(savedLookAt.y - savedCamPos.y, 2450, t);
	})
	(zoomOutStart, zoomOutEnd).

	Call([] { Celebrate(*otherPlayers[0]); })       (GetCelebrationStart(0), -1).
	Call([] { Celebrate(*otherPlayers[1]); })       (GetCelebrationStart(1), -1).
	Call([](Player& player) { Celebrate(player); }) (GetCelebrationStart(2), -1).
	Call([] { Celebrate(*otherPlayers[2]); })       (GetCelebrationStart(3), -1).

	Call([](Player& player)
	{
		const unsigned charID = player.param1 & 3;

		Sound::PlayCharVoice(charID, charID == 3 ? 16 : 5, player.camSpacePos);
	})
	(fifthJumpCut + 91).

	Call([] { UpdateBigStar(*star); }) (bounceStart, -1).

	Call([] { CURRENT_GAMEMODE = 0; }) (zoomOutEnd - 40).
	ChangeLevel (50, 0, 0, 0) (zoomOutEnd - 40).
	End();

[[maybe_unused]]
static bool TestAndSetBits(u32& n, unsigned bits)
{
	return n & bits || (n |= bits, false);
}

asm(R"(
VanillaMinimapBehavior = 0x020fa690
VanillaMinimapRender   = 0x020f9e98
minimapVtable          = 0x0210c1c0

VanillaHUDBehavior     = 0x020fd7a4
VanillaHUDRender       = 0x020fd5e0
hudVtable              = 0x0210c2c8
)");

extern "C" int VanillaMinimapBehavior(Minimap&);
extern "C" int VanillaMinimapRender(Minimap&);
extern "C" int (*minimapVtable[])(Minimap&);

extern "C" int VanillaHUDBehavior(HUD&);
extern "C" int VanillaHUDRender(HUD&);
extern "C" int (*hudVtable[])(HUD&);

struct MinimapTransform
{
	Matrix2x2 linear;
	int centerX;
	int centerY;
	int otherPointX;
	int otherPointY;
}
extern minimapTransform;

asm("minimapTransform = 0x0209f3c8");

int CustomMinimapBehavior(Minimap& minimap)
{
	static constinit bool set = false;

	if (!set)
	{
		VanillaMinimapBehavior(minimap);
		set = true;
	}

	minimapTransform.linear.c0 = {0._f, -0.75_f};
	minimapTransform.linear.c1 = {0.75_f,  0._f};

	static constexpr Vector2_16 imgCenter = {64, 64};
	static constexpr Vector2_16 screenCenter = {128, 96};

	static constexpr Vector2_16 refPoint =
	{
		imgCenter.x - screenCenter.x,
		imgCenter.y - screenCenter.y
	};

	minimapTransform.centerX = imgCenter.x;
	minimapTransform.centerY = imgCenter.y;

	minimapTransform.otherPointX = refPoint.x;
	minimapTransform.otherPointY = refPoint.y;

	return 1;
}

void init()
{
	if (!TestAndSetBits(SAVE_DATA.flags2, 1 << 31))
		intro.Run();
	else if (CURRENT_GAMEMODE == 2)
	{
		CURRENT_GAMEMODE = 0;
		ending.Run();

		minimapVtable[6] = &CustomMinimapBehavior;

		auto nop = [](auto&) { return 1; };

		minimapVtable[9] = nop;
		hudVtable[6] = nop;
		hudVtable[9] = nop;
	}
	else if (SAVE_DATA.stars[17] & 0x80) // if the big star has been collected
	{
		minimapVtable[6] = +[](Minimap& minimap)
		{
			if (!star) star = Actor::Spawn(0x0160, 0x18, spruceTop);

			UpdateBigStar(*star);

			return VanillaMinimapBehavior(minimap);
		};
	}
}

void cleanup()
{
	minimapVtable[6] = &VanillaMinimapBehavior;
	minimapVtable[9] = &VanillaMinimapRender;

	hudVtable[6] = &VanillaHUDBehavior;
	hudVtable[9] = &VanillaHUDRender;
}
