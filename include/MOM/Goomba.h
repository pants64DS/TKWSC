#ifndef GOOMBA_INCLUDED
#define GOOMBA_INCLUDED

#include "SM64DS_PI.h"

// vtable at 0x02130948, size is 0x478, actor ids are 0xc8, 0xc9 and 0xca
struct Goomba : public CapEnemy
{
	enum Type
	{
		SMALL = 0,
		NORMAL = 1,
		BIG = 2
	};

	enum Animations
	{
		WALK,
		ROLLING,
		STRETCH,
		UNBALANCE,
		RECOVER,
		WAIT,
		RUN,

		NUM_ANIMS,
	};

	enum Colors
	{
		RED,
		GREEN,
		YELLOW,
		BLUE,
		ORANGE,
		PURPLE,
		
		BROWN, // anything else is BROWN
	};

	MovingCylinderClsn cylClsn;
	WithMeshClsn wmClsn;
	ModelAnim modelAnim;
	ShadowModel shadow;
	MaterialChanger materialChg;
	Vector3 noCliffPos;
	Vector3 originalPos;
	Vector3 backupPos;
	u32 state;
	u32 goombossID;
	Fix12i distToPlayer; // (capped at 0x001a8000, and actually 0x061a8000 when it hits the cap)
	Fix12i targetSpeed;
	Fix12i maxDist;
	u32 recoverFlags; // (flags the Goomba would've used if it could recover from being spit out, always 0)
	u16 movementTimer;
	u16 regurgTimer;
	u16 changeDirTimer; // (only used by Goomboss Goombas, would've also been used by regular Goombas if they could recover)
	u16 stuckInSpotTimer;
	u16 noChargeTimer;
	s16 targetDir;
	s16 targetDir2;
	u32 type;
	u8 spawnStar; // (0: no star, 1: silver star, 2: VS mode star)
	s8 starID; // (it is signed)
	u8 starID_VS;
	u8 regurgBounceCount;
	u8 flags468;
	u32 hitFlags;
	u8 goombaID; // (only used by Goomboss Goombas, set by Goomboss)
	u8 followGoombossTimer; // (when the timer hits 0, the Goomba will stop going after / looking for the player and will try to go back to following Goomboss)
	TextureSequence texSeq;
	u8 color; // 0 = jump, 1 = jump 'n' spin, 2 = do double damage, 3 = speed + shock, 4 = fire, 5 = instakill
	u8 extraDamage;
	u8 extraSpeed;

	void State0(); // walking, running...
	void State0_NormalGoomba();
	void State1(); // just hit player
	void State2(); // jumping
	void State3(); // bouncing after being spit out
	void State4();
	void State5();
	void State6();
	void UpdateMaxDist();
	void TrackVsStarIfNecessary();
	void Kill();
	void SpawnStarIfNecessary();
	bool UpdateIfDying();
	void RenderRegurgGoombaHelpless(Player* player);
	void KillIfTouchedBadSurface();
	void UpdateModelTransform();
	bool UpdateIfEaten();
	void PlayMovingSound();
	void GetHurtOrHurtPlayer();
	void KillIfIntoxicated();
	void Jump();
	void UpdateTargetDirAndDist(Fix12i theMaxDist);

	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	virtual void OnPendingDestroy() override;
	virtual u32 OnYoshiTryEat() override;
	virtual void OnTurnIntoEgg(Player& player) override;
	virtual Fix12i OnAimedAtWithEgg() override;

	static SharedFilePtr modelFile;
	static SharedFilePtr texSeqFile;
	static SharedFilePtr animFiles[NUM_ANIMS];

	static SpawnInfo spawnData;
	static SpawnInfo spawnDataSmall;
	static SpawnInfo spawnDataBig;
};

#endif
