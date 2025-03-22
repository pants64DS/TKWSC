#ifndef BACKGROUND_INCLUDED
#define BACKGROUND_INCLUDED

#include "airship.h"

struct Background : public Actor
{
	Wing wingCtrl {pos};
	Model model;
	Model cloudModel;
	std::array<Vector3, 63> cloudPositions;

	static bool renderInInterior;

	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual bool BeforeBehavior() override;
	virtual int Behavior() override;
	virtual bool BeforeRender() override;
	virtual int Render() override;

	static constexpr auto staticActorID = Airship::staticActorID + 1;
	static_assert(staticActorID == 561);

	static SpawnInfo spawnData;
	static SharedFilePtr modelFile;
	static SharedFilePtr cloudModelFile;
};

#endif