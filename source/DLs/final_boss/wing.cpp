#include <ranges>
#include "wing.h"

constinit SharedFilePtr Wing::modelFile = 0x052c;

void Wing::Update(unsigned flags)
{
	if (flags & flutter)
		ApproachLinear(rowTimer, 28 + (FRAME_COUNTER & 4), 1);
	else
		++rowTimer &= 0x3f;
}

void Wing::Update(unsigned flags, const Vector3& soundOffset)
{
	Update(flags);

	if (NEXT_ACTOR_UPDATE_FLAGS & (Actor::UPDATE_DURING_STAR_SPAWNING | Actor::UPDATE_DURING_CUTSCENES))
		return;

	if (rowTimer == 0)
	{
		Sound::PlayArchive3(0x75, VIEW_MATRIX_ASR_3(pos + soundOffset >> 3));

		if (flags & mirrorSound)
			Sound::PlayArchive3(0x75, VIEW_MATRIX_ASR_3(pos - soundOffset >> 3));
	}
	else if (rowTimer == (flags & flutter ? 32 : 60) && (flags & noWingSound) == 0)
	{
		Sound::Play(3, 0x139, VIEW_MATRIX_ASR_3(pos + soundOffset >> 3));

		if (flags & mirrorSound)
			Sound::Play(3, 0x139, VIEW_MATRIX_ASR_3(pos - soundOffset >> 3));
	}
}

void Wing::Render(const Vector3* offsetsBegin, const Vector3* offsetsEnd, const Vector3& scale)
{
	const unsigned sineIndex = rowTimer << 7;

	const short angleX = SINE_TABLE[sineIndex + 1].val + 1 >> 1;
	const short angleY = (SINE_TABLE[sineIndex].val << 1) + 90_deg;

	const Vector3 posAsr3 = pos >> 3;

	for (const Vector3& wingOffset : std::ranges::subrange(offsetsBegin, offsetsEnd))
	{
		const Vector3 wingOffsetAsr3 = wingOffset >> 3;

		model.mat4x3 = Matrix4x3::RotationY(angleY);
		model.mat4x3.c3 = posAsr3 + wingOffsetAsr3;
		model.mat4x3.RotateX(angleX);

		model.Render(scale);

		model.mat4x3.c0.x = -model.mat4x3.c0.x;
		model.mat4x3.c1.x = -model.mat4x3.c1.x;
		model.mat4x3.c2.x = -model.mat4x3.c2.x;

		model.mat4x3.c3.x = posAsr3.x - wingOffsetAsr3.x;
		model.mat4x3.c3.y = posAsr3.y + wingOffsetAsr3.y;
		model.mat4x3.c3.z = posAsr3.z + wingOffsetAsr3.z;

		model.Render(scale);
	}
}
