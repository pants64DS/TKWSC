#ifndef LAUNCH_STAR_INCLUDED
#define LAUNCH_STAR_INCLUDED

#include "SM64DS_PI.h"
#include "MOM_IDs.h"

struct BezierPathIter
{
	PathPtr pathPtr;
	uint16_t currSplineX3;
	Fix12s tinyStep;
	Fix12i step;
	Fix12i currTime;
	Vector3 pos;
	Fix12i (*metric)(const Vector3& v0, const Vector3& v1) = Vec3_Dist;
	
	bool Advance();
};

struct LaunchStar : public Actor
{
	static constexpr uint16_t staticActorID = MOM_IDs::LAUNCH_STAR;

	ModelAnim rigMdl;
	MovingCylinderClsn cylClsn;
	
	Fix12i launchSpeed;
	PathPtr pathPtr;
	uint8_t eventID;
	uint8_t spawnTimer = 0xff;
	bool areaWasShowing = false;
	Vector3 savedCamPos;
	Vector3 savedCamTarget;
	Vector3 savedCamOffset;
	BezierPathIter& rBzIt = bzIt;
	
	void UpdateModelTransform();

	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual bool BeforeBehavior() override;
	virtual int Behavior() override;
	virtual int Render() override;

	static void Launch(Player& player, LaunchStar& launchStar);

	static SharedFilePtr modelFile;
	static SharedFilePtr animFiles[2];

	static BezierPathIter bzIt;
	static Vector3 lsInitPos;
	static Vector3 lsPos;
	static Vector3_16 lsDiffAng;
	static Vector3_16 lsInitAng;
	static unsigned particleID;

	static SpawnInfo spawnData;
	static Player::State playerState;

	struct Color { unsigned difAmb, speEmi; };
	static std::array<Color, 5> colors;
};

[[gnu::always_inline]]
inline LaunchStar* GetLaunchStarPtr(Player& player)
{
	return reinterpret_cast<LaunchStar*>(player.unk764);
}

[[gnu::always_inline]]
inline void SetLaunchStarPtr(Player& player, LaunchStar* lsPtr)
{
	player.unk764 = reinterpret_cast<u32>(lsPtr);
}

#endif