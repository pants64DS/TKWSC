#ifndef SWOOPER_INCLUDED
#define SWOOPER_INCLUDED

#include "SM64DS_PI.h"

struct Swooper : public Enemy
{
	static constexpr uint16_t staticActorID = 0xed;

	MovingCylinderClsn cylClsn;
	WithMeshClsn wmClsn;
	ModelAnim model1;
	ModelAnim model2;
	ShadowModel shadow;
	unsigned unk3f0;
	unsigned unk3f4;
	unsigned unk3f8;
	unsigned unk3fc;
	unsigned unk400;
	unsigned unk404;
	unsigned unk408;
	unsigned unk40c;
	unsigned unk410;
	unsigned unk414;
	unsigned unk418;
	unsigned unk41c;
	unsigned unk420;
	unsigned unk424;
	unsigned unk428;
	unsigned unk42c;
	unsigned unk430;
	unsigned unk434;
	unsigned unk438;
	unsigned unk43c;

	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual bool BeforeBehavior() override;
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
		void (Swooper::*func0)();
		void (Swooper::*func1)();
	}
	static states[4];

	static SpawnInfo spawnData;

	static SharedFilePtr modelFile;
	static SharedFilePtr flyAnimFile;
	static SharedFilePtr waitAnimFile;
	static SharedFilePtr waitModelFile;
};

static_assert(sizeof(Swooper) == 0x440);

#endif