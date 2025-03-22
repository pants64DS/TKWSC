#ifndef FINAL_BOSS_DL_INCLUDED
#define FINAL_BOSS_DL_INCLUDED

#include "SM64DS_PI.h"
#include "MOM_IDs.h"

class Hanger : public Actor
{
protected:
	Model ropeModel;
	MovingCylinderClsnWithPos ropeClsn;

	const Fix12i ropeStartLength = static_cast<Fix12i>(param1);
	Fix12i floorY = Fix12i::min;
	bool recovering = false;
	bool loaded = true;

	Hanger() { ropeClsn.height = ropeStartLength; }

	static constexpr Fix12i ropeThickness = 20._f;
	static constexpr Fix12i ropeClsnRadius = 0x35'555_f;
	static constexpr Fix12i recoverySpeed = 100._f;

public:
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	virtual Fix12i GetMaxLength();

	static constexpr auto staticActorID = 563;

	static SharedFilePtr ropeModelFile;
	static SpawnInfo spawnData;
};

class BombHanger final : public Hanger
{
	unsigned bombUniqueID = -1;
	bool detonatedByPlayer = false;

	static constexpr Fix12i bombVertOffset = 165._f;

public:
	virtual int InitResources() override;
	virtual int Behavior() override;
	virtual Fix12i GetMaxLength() override;

	static constexpr auto staticActorID = MOM_IDs::END_OF_MOM_OBJ_IDS;
	static_assert(staticActorID == 558);

	unsigned GetBombUniqueID() const { return bombUniqueID; }

	void Recover(bool touchedByPlayer)
	{
		detonatedByPlayer = touchedByPlayer;
		recovering = true;
	}

	[[gnu::always_inline]]
	auto GetBombPos() const
	{
		return Vector3::Proxy([this]<bool resMayAlias>[[gnu::always_inline]](Vector3& res)
		{
			res = pos;
			res.y -= ropeClsn.height + bombVertOffset;
		});
	}
	
	static SpawnInfo spawnData;
};

class Anchor final : public Hanger // angle: 65.7026d
{
	Model anchorModel;
	MovingMeshCollider anchorClsn;
	Matrix4x3 clsnTransform;
	bool oneUpSpawned = false;

public:
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	virtual Fix12i GetMaxLength() override;
	void UpdateHeight();

	static constexpr auto staticActorID = BombHanger::staticActorID + 1;
	static_assert(staticActorID == 559);

	static SharedFilePtr modelFile;
	static SharedFilePtr clsnFile;
	static SpawnInfo spawnData;
};

#endif