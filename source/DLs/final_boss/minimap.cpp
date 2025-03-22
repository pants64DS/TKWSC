#include "SM64DS_PI.h"

extern "C" int VanillaMinimapBehavior(Minimap&);

// The way the SDK stores affine maps is needlessly confusing and redundant
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
	const int res = VanillaMinimapBehavior(minimap);

	static constexpr Fix12i invHeight = 0x0'c00_f;

	static constexpr Fix12i minInvWidth = 0x0'060_f;
	static constexpr Fix12i maxInvWidth = 0x0'160_f;

	static constexpr Fix12i avgInvWidth = (minInvWidth + maxInvWidth) / 2;
	static constexpr Fix12i invWidthMag = maxInvWidth - avgInvWidth;

	static constinit short angle = 0;
	static constinit short invWidthAngle = -90_deg;

	const Fix12i invWidth = avgInvWidth + invWidthMag * Sin(invWidthAngle);

	const Fix12i cos = Cos(angle);
	const Fix12i sin = Sin(angle);

	if (!GAME_PAUSED)
	{
		static constexpr short angularVel = -0x130;
		static constexpr short invWidthAngularVel = -angularVel * 2 / 3;

		angle += angularVel;
		invWidthAngle += invWidthAngularVel;
	}

	minimapTransform.linear.c0 = { cos * invWidth, sin * invHeight};
	minimapTransform.linear.c1 = {-sin * invWidth, cos * invHeight};

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

	return res;
}
