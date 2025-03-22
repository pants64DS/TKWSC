#ifndef SNUFIT_INCLUDED
#define SNUFIT_INCLUDED

#include "SM64DS_PI.h"

struct Snufit : public Enemy
{
	static constexpr uint16_t staticActorID = 0xec;

	MovingCylinderClsn cylClsn;
	WithMeshClsn wmClsn;
	ModelAnim model;
	ShadowModel shadow;
	Matrix4x3 shadowMatrix;
	unsigned unk3bc;
	unsigned unk3c0;
	unsigned unk3c4;
	unsigned unk3c8;
	unsigned unk3cc;
	unsigned unk3d0;
	unsigned unk3d4;
	unsigned unk3d8;
	unsigned unk3dc;
	unsigned unk3e0;

	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	virtual Fix12i OnAimedAtWithEgg() override;
	virtual void OnTurnIntoEgg(Player& player) override;
	virtual unsigned OnYoshiTryEat() override;

	void State0Func0();
	void State0Func1();
	void State1Func0();
	void State1Func1();
	void State2Func0();
	void State2Func1();
	void State3Func0();
	void State3Func1();

	struct State
	{
		void (Snufit::*func0)();
		void (Snufit::*func1)();
	}
	static states[4];

	static SpawnInfo spawnData;

	static SharedFilePtr modelFile;
	static SharedFilePtr attackAnimFile;
	static SharedFilePtr bulletModelFile;
	static SharedFilePtr waitAnimFile;
};

static_assert(sizeof(Snufit) == 0x3e4);

#endif