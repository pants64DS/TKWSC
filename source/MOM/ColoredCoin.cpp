#include "ColoredCoin.h"
#include "IceCoinCounter.h"
#include "MOM_IDs.h"

// Overlaps with HEALTH_ARR
extern u8 PLAYER_HEALTH;
asm("PLAYER_HEALTH = 0x02092145");

namespace
{
	constexpr Fix12i RADIUS = 0x64000_f;
	constexpr Fix12i HEIGHT = 0x64000_f;
	constexpr short int ROT_SPEED = 0xC00;
	constexpr short int ACTOR_ID = 0x191;
	constexpr short int OBJECT_ID = 0x191;
}

SharedFilePtr ColoredCoin::modelFile;
int ColoredCoin::trackCounter;

SpawnInfo ColoredCoin::spawnData =
{
	[] -> ActorBase* { return new ColoredCoin; },
	0x0030,
	0x0100,
	0x00000002,
	0x00064000_f,
	0x000c8000_f,
	0x01000000_f,
	0x01000000_f
};

void ColoredCoin::UpdateModelTransform()
{
	model.mat4x3 = Matrix4x3::RotationY(ang.y);
	shadowMat.c3 = model.mat4x3.c3 = pos >> 3;

	if (!(deathCoin && deathStarted) && !(flags & Actor::IN_YOSHI_MOUTH))
		DropShadowRadHeight(shadow, shadowMat, RADIUS, 0x200000_f, 0xf);
}


[[gnu::target("thumb")]]
int ColoredCoin::InitResources()
{
	value = ang.x & 0xff;
	health = ang.x >> 8 & 0xf;
	hurt = (ang.x >> 12 & 0xf) == 1;
	fake = value == 0;
	deathCoin = ang.z != 0;
	
	deathFrames = ang.z & 0x0fff;
	starID = ang.z & 0xf000;
	frameCounter = 0;
	
	BMD_File& modelF = modelFile.LoadBMD();
	model.SetFile(modelF, 1, -1);
	shadow.InitCylinder();
	
	model.data.materials[0].difAmb = (param1 & 0x7fff) << 16 | 0x8000;
	
	UpdateModelTransform();
	shadowMat = model.mat4x3;
	shadowMat.c3.y -= 0x1'500_f;
	
	cylClsn.Init(this, RADIUS, HEIGHT, 0x00100002, 0x8000);
	
	return 1;
}

int ColoredCoin::CleanupResources()
{
	modelFile.Release();
	return 1;
}

int ColoredCoin::Behavior()
{
	if (iceCoinStatus == UNKNOWN)
	{
		if (LEVEL_ID == 6 || LEVEL_ID == 14)
		{
			Actor* secret = nullptr;
			while (true)
			{
				secret = FindWithActorID(0x149, secret);
				if (!secret)
				{
					iceCoinStatus = NORMAL;
					break;
				}
				else if (secret->pos == pos)
				{
					iceCoinStatus = ICE_COIN;
					secret->MarkForDestruction();
					break;
				}
			}
		}
		else
			iceCoinStatus = NORMAL;
	}

	ang.y += ROT_SPEED;
	UpdateModelTransform();
	HandleClsn();
	cylClsn.Clear();

	if (PLAYER_ARR[0] && pos.Dist(PLAYER_ARR[0]->pos) < 800._f)
		cylClsn.Update();
	
	if (deathStarted)
	{
		frameCounter++;
		
		if (PLAYER_ARR[0]->pos == spawnedStar->pos)
		{
			deathStarted = false;
			KillAndTrackInDeathTable();
		}
		
		else if (frameCounter == deathFrames)
		{
			//Make sure to kill the player properly
			if (PLAYER_HEALTH == 1)
			{
				Player* cPlayer = ClosestPlayer();
				cPlayer->Hurt(pos, 1, 0xc000_f, 1, 0, 1);
				
				if (cPlayer->GetHealth() == 0)
				{
					deathStarted = false;
					KillAndTrackInDeathTable();
				}
			}
			else if (PLAYER_HEALTH <= 8 && PLAYER_HEALTH > 1)
			{
				PLAYER_HEALTH -= 1;
				frameCounter = 0;
			}
		}
	}
	
	return 1;
}

int ColoredCoin::Render()
{
	if ((!deathStarted || !deathCoin) && !(flags & Actor::IN_YOSHI_MOUTH))
		model.Render(nullptr);
	return 1;
}

void ColoredCoin::HandleClsn()
{
	if (!deathStarted || !deathCoin)
	{
		Actor* other = Actor::FindWithID(cylClsn.otherObjID);
		if(!other)
			return;
		if(other->actorID != 0x00bf)
			return;
		unsigned hitFlags = cylClsn.hitFlags;
		if ((hitFlags & 0x8000) == 0 && killable) {
			Kill();
		} else {
			killable = false;
		}
	}
}

void ColoredCoin::OnTurnIntoEgg(Player& player)
{
	Kill();
	if (!deathCoin)
		MarkForDestruction();
}

unsigned ColoredCoin::OnYoshiTryEat()
{
	if (deathStarted && deathCoin)
		return Actor::YE_DONT_EAT;
	return Actor::YE_SWALLOW;
}

[[gnu::target("thumb")]]
void ColoredCoin::Kill()
{
	if (!deathCoin)
	{
		trackCounter++;
		KillAndTrackInDeathTable();
		
		if (fake)
		{
			Particle::System::NewSimple(0x0B, pos.x, pos.y + 0x28000_f, pos.z);
			Sound::Play(4, 4, camSpacePos);
		}
		else
		{
			Particle::System::NewSimple(0xD2, pos.x, pos.y + 0x28000_f, pos.z);
			Sound::PlayArchive3(17, camSpacePos);
		}
		
		if (hurt)
			ClosestPlayer()->Hurt(pos, health, 0xc000_f, 1, 0, 1);
		else
			PLAYER_HEALTH += (PLAYER_HEALTH + health > 8 ? 8 - PLAYER_HEALTH : health);

		if (iceCoinStatus == ICE_COIN)
		{
			Actor* counter = nullptr;
			while ((counter = Next(counter)) != 0)
			{
				if (counter->actorID == MOM_IDs::ICE_COIN_COUNTER || counter->actorID == MOM_IDs::FFL_SPECIFICS)
				{
					const unsigned num = ++static_cast<IceCoinCounter*>(counter)->numIceCoins;
					const Vector3 numberPos = {pos.x, pos.y + 180._f, pos.z};

					SpawnNumber(numberPos, num | 0x100, false, 0, nullptr);
					Sound::UnkPlaySoundFunc(37);

					break;
				}
			}
		}

		if (trackCounter == 5)
			Event::SetBit(6);
		
		GiveCoins(0, value);
	}
	else
	{
		PLAYER_HEALTH -= 1;
		deathStarted = true;
		
		Actor* starMarker = nullptr;
		spawnedStar = nullptr;
		
		while (starMarker = FindWithActorID(0x00b4, starMarker))
		{
			if (starMarker->param1 == 0x20u + starID && spawnedStar == nullptr)
				spawnedStar = Actor::Spawn(0x00b2, 0x0040 + starID, starMarker->pos, nullptr, areaID, -1);
		}
		
		Particle::System::NewSimple(0xD2, pos.x, pos.y + 0x28000_f, pos.z);
		Sound::PlayArchive3(17, camSpacePos);
	}
}
