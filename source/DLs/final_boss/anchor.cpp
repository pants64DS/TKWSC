#include "final_boss_dl.h"

SpawnInfo Anchor::spawnData =
{
	+[]() -> ActorBase* { return new Anchor; },
	0x100, // behavPriority
	0x100, // renderPriority
	0,     // flags
	0_f,   // rangeOffsetY
	-1._f, // range
	0_f,   // drawDist
	0_f    // unkc0
};

using AnchorCLPS = StaticCLPS_Block<{ .tractionID = CLPS::TR_SLIPPERY }>;

int Anchor::InitResources()
{
	Hanger::InitResources();

	anchorModel.SetFile(modelFile.LoadBMD(), 1, -1);
	anchorModel.mat4x3 = Matrix4x3::RotationY(ang.y);

	anchorClsn.SetFile(&clsnFile.LoadKCL(),
		clsnTransform, 1._f, ang.y, AnchorCLPS::instance<>
	);
	clsnTransform = Matrix4x3::RotationY(ang.y);

	UpdateHeight();

	return 1;
}

int Anchor::CleanupResources()
{
	Hanger::CleanupResources();

	modelFile.Release();
	clsnFile.Release();
	anchorClsn.Disable();
	
	return 1;
}

int Anchor::Behavior()
{
	Hanger::Behavior();

	UpdateHeight();

	return 1;
}

int Anchor::Render()
{
	Hanger::Render();

	anchorModel.Render();

	return 1;
}

Fix12i Anchor::GetMaxLength()
{
	return 3600._f;
}

void Anchor::UpdateHeight()
{
	clsnTransform.c3 = pos;
	clsnTransform.c3.y -= ropeClsn.height;
	anchorModel.mat4x3.c3 = clsnTransform.c3 >> 3;

	if (const Player* player = ClosestPlayer())
	{
		Vector3 rangeCenter = clsnTransform.c3;
		rangeCenter.y -= 900._f;

		if (player->pos.Dist(rangeCenter) <= 1200._f)
		{
			anchorClsn.Transform(clsnTransform, ang.y);

			if (!anchorClsn.IsEnabled()) anchorClsn.Enable();
		}
		else if (anchorClsn.IsEnabled()) anchorClsn.Disable();
	}
}
