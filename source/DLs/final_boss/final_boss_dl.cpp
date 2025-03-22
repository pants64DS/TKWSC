#include "final_boss_dl.h"
#include "airship.h"
#include "background.h"
#include "snufit.h"
#include "swooper.h"
#include "Lists.h"
#include "Actors/Bowser.h"
#include "Actors/SpikeBomb.h"

constinit SharedFilePtr Hanger::ropeModelFile     = 0x5bd; // "data/stage/ex_w_map/ex_w_map_00_isc.bmd";
constinit SharedFilePtr Airship::modelFile        = 0x5be; // "data/stage/ex_w_map/ex_w_map_all.bmd";
constinit SharedFilePtr Airship::clsnFile         = 0x5bc; // "data/stage/ex_w_map/ex_w_map.kcl";
constinit SharedFilePtr Airship::shutterModelFile = 0x5af; // "data/stage/ex_luigi/ex_luigi_all.bmd";
constinit SharedFilePtr Airship::shutterClsnFile  = 0x5ad; // "data/stage/ex_luigi/ex_luigi.kcl";
constinit SharedFilePtr Anchor::modelFile         = 0x5aa; // "data/stage/ex_l_map/ex_l_map_all.bmd";
constinit SharedFilePtr Anchor::clsnFile          = 0x5a4; // "data/stage/ex_l_map/ex_l_map.kcl";
constinit SharedFilePtr Background::modelFile     = 0x5a5; // "data/stage/ex_l_map/ex_l_map_00_isc.bmd

asm(R"(
.global _ZN13MOM_Interface8instanceE
_ZN13MOM_Interface8instanceE = 0x023c4000
)");

template<class T> constinit uint16_t ogActorID;
template<class T> constinit SpawnInfo* ogSpawnDataAddr;

template<class T, bool saveOriginal = false>
void RegisterActor()
{
	if constexpr (saveOriginal)
	{
		ogActorID<T> = OBJ_TO_ACTOR_ID_TABLE[T::staticActorID];
		ogSpawnDataAddr<T> = ACTOR_SPAWN_TABLE[T::staticActorID];
	}

	OBJ_TO_ACTOR_ID_TABLE[T::staticActorID] = T::staticActorID;
	ACTOR_SPAWN_TABLE[T::staticActorID] = &T::spawnData;
}

template<class T>
void UnregisterActor()
{
	OBJ_TO_ACTOR_ID_TABLE[T::staticActorID] = ogActorID<T>;
	ACTOR_SPAWN_TABLE[T::staticActorID] = ogSpawnDataAddr<T>;
}

extern "C" bool BowserBeforeBehavior(Bowser& bowser);
extern "C" bool BowserBeforeRender(Bowser& bowser);

extern bool(*bowserBeforeBehaviorAddr)(Bowser&);
extern bool(*bowserBeforeRenderAddr)(Bowser&);

asm("bowserBeforeBehaviorAddr = 0x0211a6d4");
asm("bowserBeforeRenderAddr   = 0x0211a6e0");

asm("onYoshiTryEatBowserTailAddr = 0x0211a67c");
extern "C" unsigned(*onYoshiTryEatBowserTailAddr)(BowserTail&);

extern "C" unsigned snufitBulletModelFileAddress1;
extern "C" unsigned snufitBulletModelFileAddress2;

asm("snufitBulletModelFileAddress1 = 0x020fee10");
asm("snufitBulletModelFileAddress2 = 0x020fefc0");

asm(R"(
.global VanillaMinimapBehavior
VanillaMinimapBehavior = 0x020fa690
VanillaMinimapRender   = 0x020f9e98
minimapVtable          = 0x0210c1c0
)");

int CustomMinimapBehavior(Minimap&);

extern "C" int VanillaMinimapBehavior(Minimap&);
extern "C" int VanillaMinimapRender(Minimap&);
extern "C" int (*minimapVtable[])(Minimap&);

void init()
{
	RegisterActor<Hanger>();
	RegisterActor<BombHanger>();
	RegisterActor<Anchor>();
	RegisterActor<Airship>();
	RegisterActor<Background>();
	RegisterActor<Snufit, true>();
	RegisterActor<Swooper, true>();

	snufitBulletModelFileAddress1 = (unsigned)&Snufit::bulletModelFile;
	snufitBulletModelFileAddress2 = (unsigned)&Snufit::bulletModelFile;

	minimapVtable[6] = &CustomMinimapBehavior;
	minimapVtable[9] = +[](Minimap&) { return 1; };

	bowserBeforeBehaviorAddr = &BowserBeforeBehavior;
	bowserBeforeRenderAddr = &BowserBeforeRender;

	onYoshiTryEatBowserTailAddr = +[](BowserTail& tail) -> unsigned
	{
		tail.cylClsn.vulnerableFlags |= 1 << 15;

		return Actor::YE_KEEP_IN_MOUTH;
	};
}

void cleanup()
{
	UnregisterActor<Snufit>();
	UnregisterActor<Swooper>();

	snufitBulletModelFileAddress1 = 0x0211d610;
	snufitBulletModelFileAddress2 = 0x0211d610;

	minimapVtable[6] = &VanillaMinimapBehavior;
	minimapVtable[9] = &VanillaMinimapRender;
}
