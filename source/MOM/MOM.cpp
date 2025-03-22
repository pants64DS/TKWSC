#include "SM64DS_PI.h"
#include "GalaxyShrinkingPlatform.h"
#include "InvisibleWall.h"
#include "Berry.h"
#include "YoshiRide.h"
#include "ObjectLightingModifier.h"
#include "Goomba.h"
#include "Noteblock.h"
#include "ShyGuy.h"
#include "LaunchStar.h"
#include "Magikoopa.h"
#include "NPC.h"
#include "ColoredPipe.h"
#include "CharacterBlock.h"
#include "TreeShadow.h"
#include "SaveBlock.h"
#include "StarChip.h"
#include "ColoredCoin.h"
#include "BlueIceBlock.h"
#include "CutsceneLoader.h"
#include "TreasureChest.h"
#include "FFL_Specifics.h"
#include "MOM_IDs.h"

using namespace MOM_IDs;

//Modify the object and actor tables.
static void Register(u16 actorID, SpawnInfo* spawnData)
{
	OBJ_TO_ACTOR_ID_TABLE[actorID] = actorID;
	ACTOR_SPAWN_TABLE[actorID] = spawnData;
}

//Initialize the objects.
void init()
{

	//Shrinking platforms.
	Register(GALAXY_SHRINKING_PLATFORM, &GalaxyShrinkingPlatform::spawnData);
	GalaxyShrinkingPlatform::modelFile.FromOv0ID(GALAXY_SHRINKING_PLATFORM_MODEL_ID);
	GalaxyShrinkingPlatform::clsnFile .FromOv0ID(GALAXY_SHRINKING_PLATFORM_COLLISION_ID);
	GalaxyShrinkingPlatform::frameModelFile.FromOv0ID(GALAXY_SHRINKING_PLATFORM_FRAME_MODEL_ID);

	//Invisible walls.
	Register(INVISIBLE_WALL, &InvisibleWall::spawnData);
	InvisibleWall::clsnFile.FromOv0ID(INVISIBLE_WALL_COLLISION_ID);

	//Berries.
	Register(BERRY, &Berry::spawnData);
	Berry::modelFile.FromOv0ID(BERRY_MODEL_ID);
	Berry::stemFile.FromOv0ID(BERRY_STEM_MODEL_ID);

	//Rideable yoshis.
	Register(YOSHI_RIDE, &YoshiRide::spawnData);
	YoshiRide::ridingAnim.FromOv0ID(YOSHI_RIDE_ANIM_ID);

	//Object lighting modifier.
	Register(OBJECT_LIGHTING_MODIFIER, &ObjectLightingModifier::spawnData);

	//Colored goombas.
	Register(COLORED_GOOMBA_SMALL, &Goomba::spawnDataSmall);
	Register(COLORED_GOOMBA,       &Goomba::spawnData);
	Register(COLORED_GOOMBA_LARGE, &Goomba::spawnDataBig);
	Goomba::modelFile .FromOv0ID(COLORED_GOOMBA_MODEL_ID);
	Goomba::texSeqFile.FromOv0ID(COLORED_GOOMBA_TEXSEQ_ID);
	Goomba::animFiles[Goomba::WALK     ].FromOv0ID(0x38d);
	Goomba::animFiles[Goomba::ROLLING  ].FromOv0ID(0x388);
	Goomba::animFiles[Goomba::STRETCH  ].FromOv0ID(0x38a);
	Goomba::animFiles[Goomba::UNBALANCE].FromOv0ID(0x38b);
	Goomba::animFiles[Goomba::RECOVER  ].FromOv0ID(0x387);
	Goomba::animFiles[Goomba::WAIT     ].FromOv0ID(0x38c);
	Goomba::animFiles[Goomba::RUN      ].FromOv0ID(0x389);

	//Noteblocks.
	Register(NOTEBLOCK, &Noteblock::spawnData);
	Noteblock::modelFile.FromOv0ID(NOTEBLOCK_MODEL_ID);
	Noteblock::clsnFile.FromOv0ID(NOTEBLOCK_COLLISION_ID);

	//Shy guys.
	Register(SHY_GUY, &ShyGuy::spawnData);
	ShyGuy::modelFile.FromOv0ID(SHY_GUY_MODEL_ID);
	ShyGuy::animFiles[0].FromOv0ID(SHY_GUY_WAIT_ANIM_ID);
	ShyGuy::animFiles[1].FromOv0ID(SHY_GUY_WALK_ANIM_ID);
	ShyGuy::animFiles[2].FromOv0ID(SHY_GUY_RUN_ANIM_ID);
	ShyGuy::animFiles[3].FromOv0ID(SHY_GUY_FREEZE_ANIM_ID);

	Register(LaunchStar::staticActorID, &LaunchStar::spawnData);
	LaunchStar::modelFile   .FromOv0ID(LAUNCH_STAR_MODEL_ID);
	LaunchStar::animFiles[0].FromOv0ID(LAUNCH_STAR_WAIT_ANIM_ID);
	LaunchStar::animFiles[1].FromOv0ID(LAUNCH_STAR_LAUNCH_ANIM_ID);

	//Kamek.
	Register(KAMEK_SHOT, &Magikoopa::Shot::spawnData);
	Register(KAMEK, &Magikoopa::spawnData);
	Register(KAMELLA, &Magikoopa::bossSpawnData);
	Magikoopa::modelFiles[0].FromOv0ID(KAMEK_MODEL_ID);
	Magikoopa::modelFiles[1].FromOv0ID(KAMELLA_MODEL_ID);
	Magikoopa::animFiles[0].FromOv0ID(KAMEK_APPEAR_ANIM_ID);
	Magikoopa::animFiles[1].FromOv0ID(KAMEK_WAVE_ANIM_ID);
	Magikoopa::animFiles[2].FromOv0ID(KAMEK_SHOOT_ANIM_ID);
	Magikoopa::animFiles[3].FromOv0ID(KAMEK_POOF_ANIM_ID);
	Magikoopa::animFiles[4].FromOv0ID(KAMEK_WAIT_ANIM_ID);
	Magikoopa::animFiles[5].FromOv0ID(KAMEK_HURT_ANIM_ID);
	Magikoopa::animFiles[6].FromOv0ID(KAMEK_DEFEAT_ANIM_ID);

	//Colored goombas 2.
	Register(COLORED_GOOMBA_2_SMALL, &Goomba::spawnDataSmall);
	Register(COLORED_GOOMBA_2,       &Goomba::spawnData);
	Register(COLORED_GOOMBA_2_LARGE, &Goomba::spawnDataBig);
	
	//NPCs.
	Register(YOSHI_NPC, &NPC::spawnData);
	Register(COLORED_TOAD_NPC, &NPC::spawnData);
	Register(PEACH_NPC, &NPC::spawnData);
	Register(PENGUIN_NPC, &NPC::spawnData);
	Register(BABY_PENGUIN_NPC, &NPC::spawnData);
	Register(RABBIT_NPC, &NPC::spawnData);
	NPC::modelFiles[0].FromOv0ID(YOSHI_NPC_MODEL_ID);
	NPC::modelFiles[1].FromOv0ID(COLORED_TOAD_NPC_MODEL_ID);
	NPC::modelFiles[2].FromOv0ID(PEACH_NPC_MODEL_ID);
	NPC::modelFiles[3].FromOv0ID(PENGUIN_MODEL_ID);
	NPC::modelFiles[4].FromOv0ID(BABY_PENGUIN_MODEL_ID);
	NPC::modelFiles[5].FromOv0ID(RABBIT_MODEL_ID);
	NPC::animFiles[0].FromOv0ID(YOSHI_NPC_ANIM_ID);
	NPC::animFiles[1].FromOv0ID(YOSHI_NPC_ANIM_ID);
	NPC::animFiles[2].FromOv0ID(COLORED_TOAD_NPC_ANIM_1_ID);
	NPC::animFiles[3].FromOv0ID(COLORED_TOAD_NPC_ANIM_2_ID);
	NPC::animFiles[4].FromOv0ID(PEACH_NPC_ANIM_1_ID);
	NPC::animFiles[5].FromOv0ID(PEACH_NPC_ANIM_2_ID);
	NPC::animFiles[6].FromOv0ID(PENGUIN_ANIM_ID);
	NPC::animFiles[7].FromOv0ID(RABBIT_ANIM_ID);
	NPC::texSeqFiles[0].FromOv0ID(COLORED_TOAD_NPC_TEXANIM_ID);
	NPC::texSeqFiles[1].FromOv0ID(RABBIT_TEXANIM_ID);

	//Colored Pipes.
	Register(COLORED_PIPE, &ColoredPipe::spawnData);
	ColoredPipe::modelFile.FromOv0ID(COLORED_PIPE_MODEL_ID);
	ColoredPipe::clsnFile.FromOv0ID(COLORED_PIPE_COLLISION_ID);

	//Character Blocks.
	Register(CHARACTER_BLOCK, &CharacterBlock::spawnData);
	CharacterBlock::modelFiles[0].FromOv0ID(CHARACTER_BLOCK_MODEL_ID);
	CharacterBlock::modelFiles[1].FromOv0ID(CHARACTER_BLOCK_TRANSPARENT_MODEL_ID);
	CharacterBlock::texSeqFiles[0].FromOv0ID(CHARACTER_BLOCK_TEXANIM_ID);
	CharacterBlock::texSeqFiles[1].FromOv0ID(CHARACTER_BLOCK_TRANSPARENT_TEXANIM_ID);
	CharacterBlock::texSeqFiles[2].FromOv0ID(CHARACTER_BLOCK_TRANSPARENT_MARK_TEXANIM_ID);
	CharacterBlock::clsnFile.FromOv0ID(CHARACTER_BLOCK_COLLSION_ID);
	CharacterBlock::animFile.FromOv0ID(CHARACTER_BLOCK_TRANSPARENT_ANIMATION_ID);

	//Tree Shadows.
	Register(TREE_SHADOW, &TreeShadow::spawnData);
	
	//Save Blocks.
	Register(SAVE_BLOCK, &SaveBlock::spawnData);
	SaveBlock::modelFile.FromOv0ID(SAVE_BLOCK_MODEL_ID);
	SaveBlock::texSeqFile.FromOv0ID(SAVE_BLOCK_ANIM_ID);
	SaveBlock::clsnFile.FromOv0ID(SAVE_BLOCK_COLLISION_ID);

	Register(STAR_CHIP, &StarChip::spawnData);
	StarChip::modelFile.FromOv0ID(STAR_CHIP_MODEL_ID);

	//Colored Coins.
	Register(COLORED_COIN, &ColoredCoin::spawnData);
	ColoredCoin::modelFile.FromOv0ID(COLORED_COIN_MODEL_ID);
	
	//Blue Ice Blocks.
	Register(BLUE_ICE_BLOCK, &BlueIceBlock::spawnData);
	BlueIceBlock::modelFile.FromOv0ID(BLUE_ICE_BLOCK_MODEL_ID);
	BlueIceBlock::clsnFile.FromOv0ID(BLUE_ICE_BLOCK_COLLISION_ID);
	
	//Cutscene Loader.
	Register(CUTSCENE_LOADER, &CutsceneLoader::spawnData);
	
	//Treasure chest
	Register(TREASURE_CHEST, &TreasureChest::spawnData);
	TreasureChest::modelFile.FromOv0ID(TREASURE_CHEST_MODEL_ID);
	TreasureChest::animFiles[0].FromOv0ID(TREASURE_CHEST_ANIM_ID);
	
	//Frostbite Manager
	Register(FFL_SPECIFICS, &FFL_Specifics::spawnData);

	Register(ICE_COIN_COUNTER, &IceCoinCounter::spawnData);
}
