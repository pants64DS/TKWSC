#ifndef AIRSHIP_INCLUDED
#define AIRSHIP_INCLUDED

#include "final_boss_dl.h"
#include "fixed_size_actor_list.h"
#include "wing.h"

class Airship : public Actor
{
	static constexpr int interpFrames = 300;
	static constexpr Fix12i interpStep = 1._f / interpFrames;

	Model model;
	MovingMeshCollider clsn;
	Matrix4x3 clsnTransform;
	Fix12s interp = 0._f;
	uint8_t ogNumPathNodes;
	Actor* cannon = nullptr;
	Player* player = nullptr;
	Vector3 destination = PathPtr(0).GetNode(param1);
	Vector3 startingPos = pos;
	Model shutterModel;
	MovingMeshCollider leftShutterClsn;
	MovingMeshCollider rightShutterClsn;
	Matrix4x3 leftShutterClsnMat;
	Matrix4x3 rightShutterClsnMat;
	Fix12i shutterHorzOffset = 100._f;
	Wing wingCtrl {pos};
	FixedSizeActorList<16> crew;

	static constexpr Fix12i shutterVertOffset = -25._f;
	static constexpr Fix12i shutterMaxHorzOffset = 250._f;

	static void AfterClsn(MeshColliderBase& clsn, Actor* clsnActor, Actor* otherActor);

public:
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	Player* GetPlayerOnBoard() const { return player; }
	bool CannonUnlocked() const { return cannon; }
	void AddCrewMember(Actor& actor) { crew.Insert(actor.uniqueID); }

	static constexpr auto staticActorID = Anchor::staticActorID + 1;
	static_assert(staticActorID == 560);

	static SharedFilePtr modelFile;
	static SharedFilePtr clsnFile;
	static SharedFilePtr shutterModelFile;
	static SharedFilePtr shutterClsnFile;

	static SpawnInfo spawnData;
};

#endif