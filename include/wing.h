#ifndef WING_INCLUDED
#define WING_INCLUDED

#include "SM64DS_PI.h"
#include <span>

class Wing
{
	static SharedFilePtr modelFile;

	const Vector3& pos;
	short rowTimer = RandomInt() & 0x3f;
	CommonModel model;

public:
	explicit Wing(const Vector3& pos) : pos(pos) {}

	enum UpdateFlags
	{
		flutter     = 1 << 0,
		mirrorSound = 1 << 1,
		noWingSound = 1 << 2
	};

	void Update(unsigned flags);
	void Update(unsigned flags, const Vector3& soundOffset);
	void Render(const Vector3* offsetsBegin, const Vector3* offsetsEnd, const Vector3& scale);

	template<Fix12i scale>
	void Render(const Vector3* offsetsBegin, const Vector3* offsetsEnd)
	{
		static constexpr Vector3 scaleVec = {scale, scale, scale};

		Render(offsetsBegin, offsetsEnd, scaleVec);
	}

	void SetFile()
	{
		model.SetFile(modelFile.LoadBMD(), 1, -1);
	}

	static void ReleaseFile()
	{
		modelFile.Release();
	}
};

#endif