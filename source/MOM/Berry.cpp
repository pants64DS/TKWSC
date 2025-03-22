#include "Berry.h"

namespace
{	
	constexpr Fix12i RADIUS = 0x32000_f;
	constexpr Fix12i COIN_SPEED = 0xb000_f;
	
	const uint8_t coinPrizeTypes[] = {0, 0, 0, 0,
									  0, 0, 0, 2,
									  0, 2, 2, 2};

};

SharedFilePtr Berry::modelFile;
SharedFilePtr Berry::stemFile;

unsigned Berry::berryCount;
unsigned Berry::berryMaxCount;
unsigned Berry::killCounter;

SpawnInfo Berry::spawnData =
{
	[] -> ActorBase* { return new Berry; },
	0x016d,
	0x00aa,
	0x00000003,
	0x00032000_f,
	0x00060000_f,
	0x01000000_f,
	0x01000000_f
};

void Berry::UpdateModelTransform()
{
	model.mat4x3.c3 = pos >> 3;
	
	DropShadowRadHeight(shadow, model.mat4x3, RADIUS * 2, 0x32000_f, 0xf);
}

[[gnu::target("thumb")]]
void Berry::Kill()
{
	++berryCount;
	Actor::Spawn(0x014a, berryCount, Vector3{pos.x, pos.y + 0xa0000_f, pos.z}, nullptr, areaID, -1);
	berryCount &= 7;
	if(berryCount == 0)
	{
		++berryMaxCount;
		switch(berryMaxCount)
		{
			case 1:
			case 2:
			case 3:
				for(int i = 0; i < 4; ++i)
				{
					Actor* coin = Actor::Spawn(0x120 + coinPrizeTypes[4 * (berryMaxCount - 1) + i], 0xf2, pos, nullptr, areaID, -1);
					if(coin)
					{
						coin->motionAng = Vector3_16{0,  short(i * 0x4000), 0};
						coin->horzSpeed = COIN_SPEED;
					}
				}
				break;
			default:
				Actor::Spawn(0x114, 0, Vector3{pos.x, pos.y + 0x5000_f, pos.z}, nullptr, areaID, -1);
				break;
		}
	}
	pos = origPos;
	killed = true;
	
	if (trackKills)
		killCounter++;
	
	if (killCounter == 30)
	{
		Vector3 starSpawnPos = ClosestPlayer()->pos;
		starSpawnPos.y += 300._f;
		Actor::Spawn(0x00b4, 0x20 + STAR_ID, starSpawnPos, nullptr, areaID, -1);
		Actor::Spawn(0x00b2, 0x40 + STAR_ID, starSpawnPos, nullptr, areaID, -1);
	}
}

[[gnu::target("thumb")]]
int Berry::InitResources()
{
	BMD_File& modelF = modelFile.LoadBMD();
	model.SetFile(modelF, 1, -1);

	BMD_File& stemModelF = stemFile.LoadBMD();
	stem.SetFile(stemModelF, 1, -1);

	model.data.materials[0].difAmb = (param1 & 0x7fff) << 16 | 0x8000;
	trackKills = (ang.x & 0xffff) == 0xdddd;
	
	// COIN_YELLOW_POLY4_MODEL_PTR.LoadBMD();
	// COIN_BLUE_POLY32_MODEL_PTR.LoadBMD();
	// COIN_BLUE_POLY4_MODEL_PTR.LoadBMD();
	LoadBlueCoinModel();
	
	cylClsn.Init(this, RADIUS, RADIUS * 2, 0x00200000, 0x00008000);
	shadow.InitCylinder();
	
	UpdateModelTransform();
	origPos = pos;
	
	return 1;
}

[[gnu::target("thumb")]]
int Berry::CleanupResources()
{
	stemFile.Release();
	modelFile.Release();
	killCounter = 0;
	berryCount = 0;
	berryMaxCount = 0;
	UnloadBlueCoinModel();
	return 1;
}

int Berry::Behavior()
{
	if(killed)
		return 1;
	
	if(!groundFound) //not called during InitResources because not all the platforms may have initialized their resources yet
	{
		groundFound = true; 
		SphereClsn sphere;
		sphere.SetObjAndSphere(Vector3{pos.x, pos.y + RADIUS, pos.z}, RADIUS * 3 / 2, nullptr);
		
		Vector3 stemPos, stemNormal;
		if(sphere.DetectClsn())
		{
			Vector3 newSpherePos = sphere.pos + sphere.pushback;
			stemPos = newSpherePos - sphere.result.surfaceInfo.normal * sphere.radius;
			stemNormal = sphere.result.surfaceInfo.normal;
		}
		else
		{
			stemPos = pos;
			stemNormal = Vector3{0_f, 0x1000_f, 0_f};
		}
		
		Vector3 zeros {0_f, 0_f, 0_f};
		stem.mat4x3 = Matrix4x3::RotationZXY(zeros.VertAngle(stemNormal) + 0x4000, zeros.HorzAngle(stemNormal), 0) * Matrix4x3::RotationY(ang.y);
		stem.mat4x3.c3 = stemPos >> 3;
	}
	
	cylClsn.Clear();
	cylClsn.Update();	
	UpdateModelTransform();
	
	return 1;
}

int Berry::Render()
{
	stem.Render(nullptr);
	if(!killed && !(flags & Actor::IN_YOSHI_MOUTH))
		model.Render(nullptr);
	return 1;
}

unsigned Berry::OnYoshiTryEat()
{
	return Actor::YE_SWALLOW;
}

void Berry::OnTurnIntoEgg(Player& player)
{
	player.Heal(0x100);
	Kill();
}
