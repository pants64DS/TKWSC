#include "background.h"

template<std::bidirectional_iterator Iter>
void InsertionSort(Iter begin, Iter end, auto&& cmp)
{
	if (begin == end) return;

	Iter firstUnsorted = begin;

	while (++firstUnsorted != end)
	{
		Iter insertPos = firstUnsorted;

		while (insertPos > begin && cmp(*insertPos, *std::prev(insertPos)))
		{
			std::swap(*insertPos, *std::prev(insertPos));
			--insertPos;
		}
	}
}

template<std::ranges::bidirectional_range Range, class Cmp>
void InsertionSort(Range&& range, Cmp&& cmp)
{
	InsertionSort(std::ranges::begin(range), std::ranges::end(range), std::forward<Cmp>(cmp));
}

constexpr Fix12i length = 140000._f;
constexpr Fix12i radius =  5000._f;
constexpr Fix12i vertOffset = 500._f;
constexpr Vector3 scale = {radius, radius, length};

constexpr unsigned numClouds = std::tuple_size<decltype(Background::cloudPositions)>();
constexpr Fix12i spawnDist = 0x10000'000_f;
constexpr Fix12i interval = 2 * spawnDist / numClouds;

constinit SharedFilePtr Background::cloudModelFile = 0x02fc; // data/normal_obj/obj_kumo/obj_kumo.bmd
constinit bool Background::renderInInterior = false;

SpawnInfo Background::spawnData =
{
	+[]() -> ActorBase* { return new Background; },
	0x0100, // behavPriority
	0x0100, // renderPriority
	UPDATE_DURING_DIALOGUE | UPDATE_DURING_CUTSCENES | UPDATE_DURING_STAR_SPAWNING,
	0._f,   // rangeOffsetY
	-1._f,  // range
	0._f,   // drawDist
	0._f    // unkc0
};

static void RandomizeXY(Vector3& v)
{
	while (true)
	{
		v.x.val = RandomInt() >> 7;
		v.y.val = RandomInt() >> 7;

		const int distSquared = int(v.x) * int(v.x) + int(v.y) * int(v.y);

		static constexpr int maxDistSquared = 0x1000 * 0x1000;
		static constexpr int minDistSquared = 0x600 * 0x600;

		if (distSquared <= maxDistSquared && distSquared >= minDistSquared)
			break;
	}

	v.y += vertOffset;
}

int Background::InitResources()
{
	wingCtrl.SetFile();
	model.SetFile(modelFile.LoadBMD(), true, 0);
	model.mat4x3 = Matrix4x3::Translation(0._f, vertOffset, 0._f);

	model.data.materials[0].SetTransformMode(Material::TEXCOORD_SOURCE);
	model.data.materials[1].SetTransformMode(Material::TEXCOORD_SOURCE);
	model.data.materials[1].SetAlpha(9);

	cloudModel.SetFile(cloudModelFile.LoadBMD(), false);
	*reinterpret_cast<uint16_t*>(cloudModelFile.filePtr + 0xb0) = Color5Bit(0x70, 0x40, 0xf8);

	for (Fix12i zPos = spawnDist; Vector3& cloudPos : cloudPositions)
	{
		RandomizeXY(cloudPos);

		cloudPos.z = zPos;
		zPos -= interval;
	}

	return 1;
}

int Background::CleanupResources()
{
	wingCtrl.ReleaseFile();
	modelFile.Release();
	cloudModelFile.Release();

	return 1;
}

bool Background::BeforeBehavior()
{
	areaID = AREA_ID;

	Actor::BeforeBehavior();

	return true;
}

bool Background::BeforeRender()
{
	Actor::BeforeRender();

	return true;
}

constexpr int zStart = 2800;
constexpr int zEnd = -3200;
constexpr int z3rd = (zStart - zEnd) / 3;

constexpr std::array<Vector3, 4> wingOffsets =
{{
	{2450._f,  -920._f, zStart - z3rd * 0},
	{2770._f, -1010._f, zStart - z3rd * 1},
	{2885._f, -1030._f, zStart - z3rd * 2},
	{2780._f,  -960._f, zStart - z3rd * 3}
}};

constexpr unsigned numAreas = 5;
constexpr Fix12i hatchX = 2730._f;

constinit bool gamePrevPaused = false;
constinit bool renderInInteriorBeforePause = false;

constinit std::array<bool, numAreas> areasShowingBeforePause;

int Background::Behavior()
{
	if (GAME_PAUSED)
	{
		if (!gamePrevPaused)
		{
			for (unsigned i = 0; i < numAreas; ++i)
				areasShowingBeforePause[i] = AREAS[i].showing;

			renderInInteriorBeforePause = renderInInterior;
			renderInInterior = true;
			gamePrevPaused = true;
		}

		AREAS[0].showing = true;
		AREAS[1].showing = false;
		AREAS[2].showing = false;
		AREAS[3].showing = true;
		AREAS[4].showing = true;

		return 1;
	}
	else if (gamePrevPaused)
	{
		for (unsigned i = 0; i < numAreas; ++i)
			AREAS[i].showing = areasShowingBeforePause[i];

		renderInInterior = renderInInteriorBeforePause;
		gamePrevPaused = false;
	}

	Player& player = *PLAYER_ARR[0];
	const Camera& cam = *CAMERA;

	if (cam.flags & Camera::UNDERWATER)
	{
		wingCtrl.Update(0);
	}
	else if (areaID == 0 || !renderInInterior && cam.pos.z > -3900._f)
	{
		const Vector3 soundOffset =
		{
			areaID == 0 ? 2720._f : 7000._f,
			-980._f,
			std::clamp(cam.pos.z, wingOffsets.back().z, wingOffsets.front().z)
		};

		unsigned wingFlags = Wing::mirrorSound;

		if (areaID != 0)
			wingFlags |= Wing::noWingSound;

		wingCtrl.Update(wingFlags, soundOffset);
	}
	else
	{
		static constexpr Vector3 soundOffset = {3600._f, -1100._f, -2400._f};

		wingCtrl.Update(0, soundOffset);
	}

	auto& scroll0 = model.data.materials[0].texTransY;
	auto& scroll1 = model.data.materials[1].texTransY;

	scroll0 -= 0.02_f;
	scroll1 += 0.013_f;

	scroll0.val &= 0xfff;
	scroll1.val &= 0xfff;

	for (Vector3& cloudPos : cloudPositions)
	{
		cloudPos.z -= 128._f;

		if (cloudPos.z < -spawnDist)
		{
			cloudPos.z = spawnDist;

			RandomizeXY(cloudPos);
		}
	}

	if (areaID == 1)
		renderInInterior = false;

	if (areaID == 0)
	{
		if (  hatchX < player.pos.x &&
			-1450._f < player.pos.y && player.pos.y < -200._f &&
			-3400._f < player.pos.z && player.pos.z < -1000._f)
		{
			player.ChangeArea(2);
			renderInInterior = true;
			areaID = 2;
		}
	}

	if (areaID == 2)
	{
		if (player.pos.z < -3800._f)
		{
			const bool currSide = player.pos.x > -300._f;
			const bool prevSide = player.prevPos.x > -300._f;

			if (currSide && !prevSide) renderInInterior = true;
			if (prevSide && !currSide) renderInInterior = false;
		}

		if (renderInInterior && player.pos.y < -1450._f && !player.isUnderwater)
		{
			player.ChangeArea(0);
			areaID = 0;
		}
		else
			AREAS[0].showing = CAMERA->pos.x > hatchX;
	}

	AREAS[3].showing = areaID == 0 && cam.pos.x > hatchX;

	AREAS[4].showing = areaID == 0 &&
		(cam.pos.z < 11000._f || cam.pos.y < 4000._f) &&
		(cam.pos.z < -4000._f || cam.pos.y < 100._f || Abs(cam.pos.x) > 3700._f);

	return 1;
}

int Background::Render()
{
	if (areaID != 0 && !renderInInterior)
	{
		const bool launchStarSpawing =
			NEXT_ACTOR_UPDATE_FLAGS & Actor::UPDATE_DURING_STAR_SPAWNING;

		if (!launchStarSpawing)
			return 1;
	}

	wingCtrl.Render<3._f>(wingOffsets.begin(), wingOffsets.end());
	model.Render(::scale);

	cloudModel.mat4x3.Linear() = VIEW_MATRIX_ASR_3.Linear();

	InsertionSort(cloudPositions, [](const Vector3& v0, const Vector3& v1)
	{
		const auto& zAxis = INV_VIEW_MATRIX_ASR_3.c2;
		const auto& pos   = INV_VIEW_MATRIX_ASR_3.c3;

		return zAxis.Dot(v0 - pos) < zAxis.Dot(v1 - pos);
	});

	for (unsigned polygonID = 1; const Vector3& cloudPos : cloudPositions)
	{
		const int dist = std::abs(cloudPos.z.val);
		static constexpr int fullAlphaDist = spawnDist.val - 0x2000000;
		static constexpr int maxAlpha = 14;

		if (dist > fullAlphaDist)
		{
			const int alpha = std::min(33 - ((dist - fullAlphaDist) >> 20), maxAlpha);

			cloudModel.data.materials[0].SetAlpha(alpha);
		}
		else
			cloudModel.data.materials[0].SetAlpha(maxAlpha);

		cloudModel.data.materials[0].SetPolygonID(polygonID++);

		cloudModel.mat4x3.c3 = VIEW_MATRIX_ASR_3(cloudPos);

		static constinit Vector3 scale = {1._f, 8._f, 1._f};
		cloudModel.data.Render(&cloudModel.mat4x3, &scale);
	}

	return 1;
}
