#include "TreasureChest.h"

namespace
{
	struct State
	{
		using FuncPtr = void(TreasureChest::*)();
		
		FuncPtr init;
		FuncPtr main;
	};
	
	const State states[]
	{
		State{ &TreasureChest::State0_Init, &TreasureChest::State0_Main }, //closed
		State{ &TreasureChest::State1_Init, &TreasureChest::State1_Main }, //opening
		State{ &TreasureChest::State2_Init, &TreasureChest::State2_Main }, //opened
	};
	
	enum Animations
	{
		OPEN,
		
		NUM_ANIMS
	};
	
	const short TREASURE_CHEST_ID = 0x226;
}

SharedFilePtr TreasureChest::modelFile;
SharedFilePtr TreasureChest::animFiles[NUM_ANIMS];

SpawnInfo TreasureChest::spawnData =
{
	[] -> ActorBase* { return new TreasureChest; },
	0x000d,
	0x00a0,
	0x00000003,
	0x00064000_f,
	0x000c8000_f,
	0x01000000_f,
	0x00000000_f
};

void TreasureChest::UpdateModelTransform()
{
	rigMdl.mat4x3 = Matrix4x3::RotationY(ang.y);
	rigMdl.mat4x3.c3 = pos >> 3;
}

[[gnu::target("thumb")]]
int TreasureChest::InitResources()
{
	RED_NUMBER_MODEL_PTR.LoadBMD();
	BUBBLE_MODEL_PTR.LoadBMD();
	BMD_File& modelF = modelFile.LoadBMD();
	rigMdl.SetFile(modelF, 1, -1);
	
	for (int i = 0; i < NUM_ANIMS; ++i)
		animFiles[i].LoadBCA();

	rigMdl.SetAnim(*animFiles[OPEN].BCA(), Animation::NO_LOOP, 0x1000_f, 0);
	
	cylClsn.Init(this, 0x96000_f, 0x96000_f, 0x00200004, 0x00000000);
	
	UpdateModelTransform();
	
	order = param1 & 0xff;
	starID = param1 >> 8 & 0xff;
	if(starID < 8)
		trackedStarID = TrackStar(starID, 2);
	
	return 1;
}

[[gnu::target("thumb")]]
int TreasureChest::CleanupResources()
{
	for (int i = 0; i < NUM_ANIMS; ++i)
		animFiles[i].Release();

	modelFile.Release();
	BUBBLE_MODEL_PTR.Release();
	RED_NUMBER_MODEL_PTR.Release();

	return 1;
}

void TreasureChest::ChangeState(unsigned newState)
{
	state = newState;
	(this->*states[state].init)();
}

void TreasureChest::CallState()
{
	(this->*states[state].main)();
}

void TreasureChest::State0_Init()
{
	rigMdl.currFrame = 0_f;
}

[[gnu::target("thumb")]]
void TreasureChest::State0_Main()
{
	//this->r10
	CountDownToZero(cooldown);

	if(cooldown == 0x58)
		Sound::PlayArchive2_2D(0x0e);
	
	if(cylClsn.otherObjID == 0 || cooldown != 0)
		return;
	
	Actor* actor = Actor::FindWithID(cylClsn.otherObjID);
	if(!actor || actor->actorID != 0x00bf || AngleDiff(pos.HorzAngle(actor->pos), ang.y) >= 0x4000) //is the player opening it from the wrong side?
		return;
	Player& player = *(Player*)actor;
	
	int numOpenChests = 0; //r9
	int count = 0; //r8
	TreasureChest* currChest = nullptr; //r7
	while((currChest = (TreasureChest*)FindWithActorID(TREASURE_CHEST_ID, currChest)))
	{
		++count;
		if(currChest == this)
			continue;
		
		if(currChest->state == 1 || currChest->state == 2)
			++numOpenChests;
	}
	
	if(order == numOpenChests + 1) //670 else
	{
		if(order == count)
		{
			SpawnSoundObj(0);
			spawnStar = true;
		}
		else
			Sound::PlayArchive2_2D(0x26);
		
		Sound::PlayArchive3(player.isUnderwater ? 0x22 : 0x20, camSpacePos);
		ChangeState(1);
	}
	else
	{
		cooldown = 0x5a;
		player.Shock(!player.isMetalWario);
		
		currChest = nullptr;
		while((currChest = (TreasureChest*)FindWithActorID(TREASURE_CHEST_ID, currChest)) != this)
			currChest->ChangeState(0);
	}
}

void TreasureChest::State1_Init()
{
	if(spawnStar) cooldown = 0x2d;

	flags &= ~Actor::NO_BEHAVIOR_IF_OFF_SCREEN;
}

[[gnu::target("thumb")]]
void TreasureChest::State1_Main()
{
	rigMdl.Advance();

	if ((int)rigMdl.currFrame == 0x14) //468 after
	{
		Player& player = *ClosestPlayer();
		Vector3 spawnPos {pos.x, pos.y + 0xc8000_f, pos.z};
		if(!spawnStar)
			SpawnNumber(spawnPos, order, false, 0, 0);
		
		if(player.isUnderwater)
		{
			Actor* bubble = Actor::Spawn(0x0123, 0x0000, spawnPos, nullptr, areaID, -1);
			bubble->speed = Vector3{0_f, 0x800_f, 0_f};
		}
	}
	
	if (rigMdl.Finished())
		ChangeState(2);
}

void TreasureChest::State2_Init()
{
	if(!spawnStar)
		flags |= Actor::NO_BEHAVIOR_IF_OFF_SCREEN;
}

[[gnu::target("thumb")]]
void TreasureChest::State2_Main()
{
	if(!spawnStar || cooldown == 0 || CountDownToZero(cooldown) != 0)
		return;
	
	if(starID < 8) //360 else
		UntrackAndSpawnStar(trackedStarID, starID, Vector3{pos.x, pos.y + 0xc8000_f, pos.z}, 4);
	if(starID >= 0x10 && starID < 0x30)
		Event::SetBit(starID - 0x10);
	
	flags |= Actor::NO_BEHAVIOR_IF_OFF_SCREEN;
}

int TreasureChest::Behavior()
{
	CallState();
	cylClsn.Clear();
	cylClsn.Update();
	return 1;
}

int TreasureChest::Render()
{
	rigMdl.Render();
	return 1;
}
