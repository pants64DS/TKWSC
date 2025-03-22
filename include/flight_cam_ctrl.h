#ifndef FlIGHT_CAM_CTRL_INCLUDED
#define FlIGHT_CAM_CTRL_INCLUDED

#include "SM64DS_PI.h"
#include "LaunchStar.h"
#include "MOM_Interface.h"
#include <optional>
#include <algorithm>

class FlightCamCtrl
{
	Fix12i camPosInterp = 0._f;
	Fix12i lookAtInterp = 0._f;
	Fix12i interpInterp = 0._f;
	Fix12i introTalkInterp = -1._f;
	Vector3 camLaunchPos {};

	struct HoleCamState
	{
		Vector3 pos;
		Vector3 lookAt;

		void Update(Camera& cam)
		{
			static constexpr Fix12i holeCenterX = -1751.02_f;
			static constexpr Fix12i holeCenterZ = -3303.95_f;
			static constexpr Fix12i approachFactor = 0.075_f;

			pos.x = Lerp(pos.x, holeCenterX, approachFactor);
			pos.y = Lerp(pos.y,      400._f, approachFactor);
			pos.z = Lerp(pos.z, holeCenterZ, approachFactor);

			cam.pos = pos;

			lookAt.x = Lerp(lookAt.x, holeCenterX, approachFactor);
			lookAt.y = Lerp(lookAt.y,     -400._f, approachFactor);
			lookAt.z = Lerp(lookAt.z, holeCenterZ, approachFactor);

			cam.lookAt = lookAt;
		}
	};

	std::optional<HoleCamState> holeCamState;

	void OnLaunchStarLaunchAnim(Camera& cam, const Player& player, short targetAngle, bool adjustDist = false)
	{
		camPosInterp = lookAtInterp = interpInterp = 0._f;
		camLaunchPos = {};

		short angle = cam.angY;
		ApproachAngle(angle, targetAngle, 7, 180_deg, 5_deg);

		cam.pos.RotateAround(player.pos, Matrix4x3{Matrix4x3::RotationY(angle - cam.angY)}.Linear());

		if (adjustDist)
		{
			Vector3 delta = cam.pos - player.pos;
			Fix12i dist = delta.Len();
			delta /= dist;
			dist.ApproachLinear(1500._f, 70._f);
			delta *= dist;

			cam.pos = player.pos + delta;
		}
	}

	void OnPinkLaunchStarFlight()
	{
		if (camPosInterp < 0x0'a00_f)
			lookAtInterp = std::max(1._f - Abs(0x0'292_f - camPosInterp) * 0x6'399_f, 0._f);
		else
			lookAtInterp.ApproachLinear(1._f, 0x0'020_f);

		camPosInterp.ApproachLinear(1._f, 0x0'00e_f);
	}

	void OnGreenLaunchStarFlight(Camera& cam, const Player& player, const LaunchStar& launchStar)
	{
		const unsigned nodeID = launchStar.rBzIt.currSplineX3;

		if (nodeID < launchStar.pathPtr->numNodes - 4u)
		{
			if (player.pos.Dist(launchStar.pos) >= 5000._f)
				launchStar.rBzIt.step.ApproachLinear(launchStar.launchSpeed << 4, 1.75_f);
		}
		else
			launchStar.rBzIt.step.ApproachLinear(launchStar.launchSpeed, 10._f);

		if (nodeID == 0)
		{
			const Fix12i dist = player.pos.Dist(launchStar.pos);

			if (camLaunchPos == Vector3 {} || dist < 1000._f)
			{
				const Vector3 targetPos = {launchStar.pos.x + 200._f, launchStar.pos.y, launchStar.pos.z};

				cam.pos = Lerp(cam.pos, targetPos, 0.1_f);
				camLaunchPos = cam.pos;

				return;
			}
			else if (dist < 5000._f)
			{
				const Vector3 targetPos = {player.pos.x, player.pos.y - 200._f, player.pos.z + 100._f};
				cam.pos = Lerp(camLaunchPos, targetPos, 0.7_f);

				return;
			}
		}

		if (nodeID < launchStar.pathPtr->numNodes - 4u || launchStar.pathPtr->numNodes > 16)
			camPosInterp.ApproachLinear(1._f, 0.01_f);
		else
			camPosInterp.ApproachLinear(0._f, 0.0175_f);

		lookAtInterp.ApproachLinear(1._f, 0.01_f);
	}

	void OnBlueLaunchStarFlight(Camera& cam, const Player& player, const LaunchStar& launchStar)
	{
		const unsigned nodeID = launchStar.rBzIt.currSplineX3;

		if (nodeID > 9)
		{
			camPosInterp.ApproachLinear(0._f, 0x0'060_f);

			const Vector3 delta = cam.pos - player.pos;
			const Fix12i dist = delta.Len();
			cam.pos = player.pos + (dist - 64._f) / dist * delta;
		}
		else if (launchStar.pathPtr->numNodes < 16 && nodeID == 6)
		{
			camPosInterp.ApproachLinear(0._f, 0x0'060_f);
			cam.pos.y += (1._f - camPosInterp) * 200._f;
		}
		else
		{
			camPosInterp.ApproachLinear(1._f, interpInterp);
			interpInterp.ApproachLinear(0.01_f, 0x0'002_f);
		}

		if (nodeID >= (launchStar.pathPtr->numNodes == 16 ? 9 : 6))
			launchStar.rBzIt.step.ApproachLinear(launchStar.launchSpeed, 6._f);
		else
			launchStar.rBzIt.step.ApproachLinear(launchStar.launchSpeed * 2.5_f, 3._f);

		if (nodeID == 0 || nodeID > 6)
			lookAtInterp.ApproachLinear(0.4_f, 0.009_f);
		else
			lookAtInterp.ApproachLinear(0.0_f, 0.009_f);
	}

	void UpdateIntro(Camera& cam, Player& player);
	static void UpdateEnding(Camera& cam, const Player& player);

public:
	void Update(Camera& cam)
	{
		if (GAME_PAUSED || !cam.owner || cam.owner->actorID != 0xbf)
			return;

		Player& player = static_cast<Player&>(*cam.owner);
		LaunchStar* lsPtr = GetLaunchStarPtr(player);

		if (LEVEL_ID == 39 && player.currState == &MOM_Interface::instance.launchStarState)
		{
			if (lsPtr && player.unk763 <= 1) // unk763 = launchState
			{
				const short angle = cam.pos.x > lsPtr->pos.x ? 120_deg : -120_deg;

				OnLaunchStarLaunchAnim(cam, player, angle);

				cam.lookAt.y.ApproachLinear(player.pos.y, 10._f);
			}
			else
			{
				cam.lookAt.y = Lerp(cam.lookAt.y, player.pos.y, lookAtInterp);

				lookAtInterp.ApproachLinear(1._f, 0.01_f);

				Stage& stage = static_cast<Stage&>(*ROOT_ACTOR_BASE);
				stage.areas[0].texSRT->currFrame = 35._f;

				static constexpr Vector3 startColor = {100, 100, 255};
				static constexpr Vector3 endColor   = { 44,   0, 160};

				const Fix12i fogInterp = std::clamp((lookAtInterp - 0.5_f) * 2.1_f, 0._f, 1._f);
				const Vector3 colorVec = Lerp(startColor, endColor, fogInterp);

				stage.fog[0].color = Color5Bit(
					static_cast<int>(colorVec.x),
					static_cast<int>(colorVec.y),
					static_cast<int>(colorVec.z)
				);

				stage.fog[0].offset = std::clamp((lookAtInterp - 0.08_f).val * 5 >> 2, 0, 0x400);
			}
		}

		if (LEVEL_ID != 40)
		{
			holeCamState.reset();
			return;
		}

		if (holeCamState)
			holeCamState->Update(cam);

		else if (player.currState == &Player::ST_DEAD_HIT &&
			player.pos.y < 100._f &&
			player.floorBehavID == CLPS::BH_WIND_GUST
		)
			holeCamState.emplace(cam.pos, cam.lookAt);

		UpdateIntro(cam, player);
		UpdateEnding(cam, player);

		if (!lsPtr || player.currState != &MOM_Interface::instance.launchStarState)
			return;

		const LaunchStar& launchStar = *lsPtr;
		const unsigned pathID = launchStar.param1 & 0xff;

		if (player.unk763 <= 1) // unk763 = launchState
		{
			OnLaunchStarLaunchAnim(cam, player,
				launchStar.ang.y + (pathID == 3 ? 45_deg : 120_deg),
				pathID == 3
			);

			return;
		}

		switch (pathID)
		{
		case 1:
			OnPinkLaunchStarFlight();
			break;
		case 2:
			OnGreenLaunchStarFlight(cam, player, launchStar);
			if (cam.pos.x > 4096._f) player.ChangeArea(0);
			break;
		case 3:
			OnBlueLaunchStarFlight(cam, player, launchStar);
			break;
		}

		cam.pos    = Lerp(cam.pos,    player.pos * 1.1_f, camPosInterp);
		cam.lookAt = Lerp(cam.lookAt, player.pos, lookAtInterp);
	}
}
inline constinit flightCamCtrl;

#endif