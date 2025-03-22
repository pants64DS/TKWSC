#include "NPC.h"

namespace
{
	enum Animations
	{
		WAIT,
		TALK,
		
		NUM_ANIMS
	};
	
	enum NPCs
	{
		YOSHI_NPC,
		COLORED_TOAD_NPC,
		PEACH_NPC,
		PENGUIN_NPC,
		BABY_PENGUIN_NPC,
		RABBIT_NPC,
		
		NUM_NPCS
	};
	
	enum NPC_IDs
	{
		YOSHI_NPC_ID = 0x218,
		COLORED_TOAD_NPC_ID = 0x220,
		PEACH_NPC_ID = 0x221,
		PENGUIN_NPC_ID = 0x22a,
		BABY_PENGUIN_NPC_ID = 0x22b,
		RABBIT_NPC_ID = 0x22d
	};
	
	enum ToadColor
	{
		RED,
		YELLOW,
		GREEN,
		BLUE,
		PURPLE,
		
		NUM_TOAD_COLORS
	};
	
	enum RabbitColor
	{
		R_YELLOW,
		R_RED,
		R_GREEN,
		R_ORANGE,
		R_CYAN,
		R_WHITE,
		
		NUM_RABBIT_COLORS
	};
	
	using StateFuncPtr = void(NPC::*)();
	
	const StateFuncPtr stateFuncs[]
	{
		&NPC::State0_Wait,
		&NPC::State1_Talk
	};
	
	constexpr Fix12i RADIUS = 0x78000_f;
	constexpr Fix12i RADIUS_PENGUIN = 200._f;
	constexpr Fix12i RADIUS_BABY_PENGUIN = 67._f;
	constexpr Fix12i HEIGHT = 0x96000_f;
	constexpr Fix12i HEIGHT_PENGUIN = 300._f;
	constexpr Fix12i HEIGHT_BABY_PENGUIN = 100._f;
	constexpr Fix12i TALK_RADIUS = 0xf0000_f;
	constexpr Fix12i TALK_HEIGHT[6] = {0x70000_f, 0x70000_f, 0x70000_f, 0x100000_f, 0x70000_f, 0x70000_f};
	constexpr short TURN_SPEED = 0x0800;
};

SharedFilePtr NPC::modelFiles[6];
SharedFilePtr NPC::animFiles[8];
SharedFilePtr NPC::texSeqFiles[2];

SpawnInfo NPC::spawnData =
{
	[] -> ActorBase* { return new NPC; },
	0x003b,
	0x00aa,
	0x00000003,
	0x00032000_f,
	0x00046000_f,
	0x01000000_f,
	0x01000000_f
};

void NPC::UpdateModelTransform()
{
	if (npcID != BABY_PENGUIN_NPC)
	{
		rigMdl.mat4x3 = Matrix4x3::RotationY(ang.y);
		rigMdl.mat4x3.c3 = pos >> 3;
		
		if (npcID == YOSHI_NPC)
			DropShadowRadHeight(shadow, rigMdl.mat4x3, 0x62980_f, 0x370000_f, 0xf); //radius and height are (C) Yoshi the Player.
		else if (npcID == COLORED_TOAD_NPC || npcID == RABBIT_NPC)
			DropShadowRadHeight(shadow, rigMdl.mat4x3, 80._f, 0x370000_f, 0xf);
		else if (npcID == PEACH_NPC)
			DropShadowRadHeight(shadow, rigMdl.mat4x3, 0x62980_f, 0x370000_f, 0xf);
		else
			DropShadowRadHeight(shadow, rigMdl.mat4x3, 320._f, 0x370000_f, 0xf);
	}
	else
	{
		babyModel.mat4x3 = Matrix4x3::RotationY(ang.y);
		babyModel.mat4x3.c3 = pos >> 3;
		
		DropShadowRadHeight(shadow, babyModel.mat4x3, 80._f, 0x370000_f, 0xf);
	}
}

void SetNPCID(NPC& npc)
{
	if (npc.actorID == YOSHI_NPC_ID)
		npc.npcID = YOSHI_NPC;
	else if (npc.actorID == COLORED_TOAD_NPC_ID)
		npc.npcID = COLORED_TOAD_NPC;
	else if (npc.actorID == PEACH_NPC_ID)
		npc.npcID = PEACH_NPC;
	else if (npc.actorID == PENGUIN_NPC_ID)
		npc.npcID = PENGUIN_NPC;
	else if (npc.actorID == BABY_PENGUIN_NPC_ID)
		npc.npcID = BABY_PENGUIN_NPC;
	else if (npc.actorID == RABBIT_NPC_ID)
		npc.npcID = RABBIT_NPC;
	else
	{
		cout << __LINE__ << '\n';
		cout << "Actor is not a NPC." << '\n';
		Crash();
	}
}

[[gnu::target("thumb")]]
int NPC::InitResources()
{
	SetNPCID(*this);
	
	//The player should load his stuff first, so the SharedFilePtr's should be there before now.
	BMD_File& modelFile = modelFiles[npcID].LoadBMD();
	
	if (npcID != PENGUIN_NPC && npcID != BABY_PENGUIN_NPC && npcID != RABBIT_NPC)
	{
		rigMdl.SetFile(modelFile, 1, -1);
		
		for (int i = 0; i < NUM_ANIMS; ++i)
			animFiles[i + 2 * npcID].LoadBCA();
		
		rigMdl.SetAnim(*animFiles[WAIT + 2 * npcID].BCA(), Animation::LOOP, 0x1000_f, 0);
	}
	else if (npcID == RABBIT_NPC)
	{
		rigMdl.SetFile(modelFile, 1, -1);
		rigMdl.SetAnim(animFiles[7].LoadBCA(), Animation::LOOP, 0x1000_f, 0);
	}
	else if (npcID != BABY_PENGUIN_NPC)
	{
		rigMdl.SetFile(modelFile, 1, -1);
		rigMdl.SetAnim(animFiles[6].LoadBCA(), Animation::LOOP, 0x1000_f, 0);
	}
	else
	{
		babyModel.SetFile(modelFile, 1, -1);
	}
	
	if (npcID == PENGUIN_NPC)
		cylClsn.Init(this, RADIUS_PENGUIN, HEIGHT_PENGUIN, 0x04200004, 0x00000000);
	else if (npcID == BABY_PENGUIN_NPC)
		cylClsn.Init(this, RADIUS_BABY_PENGUIN, HEIGHT_BABY_PENGUIN, 0x04200004, 0x00000000);
	else
		cylClsn.Init(this, RADIUS, HEIGHT, 0x04200004, 0x00000000);
	shadow.InitCylinder();
	RaycastGround raycaster;
	raycaster.SetObjAndPos(Vector3{pos.x, pos.y + 0x14000_f, pos.z}, this);
	if(raycaster.DetectClsn())
		pos.y = raycaster.clsnPosY;
	
	UpdateModelTransform();
	
	state = 0;
	counter = 0;
	listener = nullptr;
	starSpawned = false;
	
	messages[0] = param1 & 0xfff;
	messages[1] = ang.x  & 0xfff;
	starID = (param1 >> 12 & 0xf);
	eventID = (ang.x >> 12 & 0xf);
	isWhiteRabbit = (npcID == RABBIT_NPC && ang.z == 5);
	
	//Only change lighting color on Yoshi NPCs
	if (npcID == YOSHI_NPC)
	{
		unsigned r = ang.z >>  0 & 0x1f,
				 g = ang.z >>  5 & 0x1f,
				 b = ang.z >> 10 & 0x1f;
		rigMdl.data.materials[1].difAmb =
			rigMdl.data.materials[2].difAmb = (uint16_t)ang.z | 0x8000 | r >> 1 << 16 
																	   | g >> 1 << 21
																	   | b >> 1 << 26;
	} //Only set texture sequence on Colored Toads
	else if (npcID == COLORED_TOAD_NPC || npcID == RABBIT_NPC)
	{
		BTP_File& texSeqFile = texSeqFiles[npcID == RABBIT_NPC ? 1 : 0].LoadBTP();
		modelFile.PrepareAnim(texSeqFile);
		texSeq.SetFile(texSeqFile, Animation::NO_LOOP, 0x10000_f, (ang.z > 5 ? 0 : ang.z));
	}
														 
	shouldTalk = (npcID == YOSHI_NPC ? ang.z & 0x8000 : true);
	ang.x = ang.z = 0;
	
	return 1;
}

[[gnu::target("thumb")]]
int NPC::CleanupResources()
{
	for (int i = 0; i < NUM_NPCS; ++i)
		modelFiles[i].Release();
	
	for (int i = 0; i < (int(NUM_ANIMS) * int(NUM_NPCS)) - 3; ++i)
		animFiles[i].Release();
	
	texSeqFiles[npcID == RABBIT_NPC ? 1 : 0].Release();
	
	return 1;
}

void NPC::State0_Wait()
{
	if (npcID != PENGUIN_NPC && npcID != BABY_PENGUIN_NPC && npcID != RABBIT_NPC)
		rigMdl.SetAnim(*animFiles[WAIT + 2 * npcID].BCA(), Animation::LOOP, 0x1000_f, 0);
	if(!(cylClsn.hitFlags & CylinderClsn::HIT_BY_PLAYER))
		return;
	
	Actor* actor = Actor::FindWithID(cylClsn.otherObjID);
	if(!actor || actor->actorID != 0x00bf)
		return;
	
	Player& player = *(Player*)actor;
	if(player.StartTalk(*this, false))
	{
		Message::PrepareTalk();
		state = 1;
		listener = &player;
	}
}
void NPC::State1_Talk()
{
	if (npcID != PENGUIN_NPC && npcID != BABY_PENGUIN_NPC && npcID != RABBIT_NPC)
		rigMdl.SetAnim(*animFiles[TALK + 2 * npcID].BCA(), Animation::LOOP, 0x1000_f, 0);
	
	if(!ApproachLinear(ang.y, pos.HorzAngle(listener->pos), TURN_SPEED))
		return;
	
	int talkState = listener->GetTalkState();
	switch(talkState)
	{
		case Player::TK_NOT:
			Message::EndTalk();
			state = 0;
			if ((Event::GetBit(eventID) || eventID == 0xf) && starID < 8 && !starSpawned)
			{
				//Vector3 vec = pos;
				//vec.y += 150._f;
				Actor::Spawn(0x00b2, 0x0040 + starID, pos, nullptr, 0, -1);
				starSpawned = true;
			}
			break;
			
		case Player::TK_START:
		{
			const Vector3 lookAt = {pos.x, pos.y + TALK_HEIGHT[npcID], pos.z};

			listener->ShowMessage(
				*this,
				shouldTalk ? (Event::GetBit(eventID) && eventID != 0xf ? messages[1] : messages[0]) : 0x000C,
				&lookAt, 0, 0
			);

			if (npcID == YOSHI_NPC_ID)
				Sound::PlayCharVoice(Player::CH_YOSHI, 0x4, camSpacePos);
			break;
		}
		default:
			return;
	}
}

int NPC::Behavior()
{
	if (npcID != BABY_PENGUIN_NPC)
		rigMdl.Advance();
	
	(this->*stateFuncs[state])();
	
	cylClsn.Clear();
	cylClsn.Update();
	
	UpdateModelTransform();

	return 1;
}

int NPC::Render()
{
	if (npcID == COLORED_TOAD_NPC || npcID == RABBIT_NPC)
		texSeq.Update(rigMdl.data);
	
	if (npcID != BABY_PENGUIN_NPC)
		rigMdl.Render(nullptr);
	else
		babyModel.Render(nullptr);
	
	if (isWhiteRabbit)
	{
		counter++;
		if (counter == 30)
		{
			counter = 0;
			Particle::System::NewSimple(0x37, pos.x, pos.y, pos.z);
		}
	}
	
	return 1;
}
