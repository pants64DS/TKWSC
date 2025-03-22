#include "StarChip.h"
#include "LaunchStar.h"
#include "MOM_IDs.h"

constexpr Fix12i RADIUS = 0x64000_f;
constexpr Fix12i HEIGHT = 0x64000_f;
constexpr short int ROT_SPEED = 0x300;
constexpr short int ACTOR_ID = 0x191;
constexpr short int OBJECT_ID = 0x191;

constinit SharedFilePtr StarChip::modelFile;

constinit SpawnInfo StarChip::spawnData =
{
	[]() -> ActorBase* { return new StarChip; },
	0x0030,
	0x0100,
	0x00000002,
	0x00064'000_f,
	0x000c8'000_f,
	Fix12i::max,
	Fix12i::max
};

void StarChip::UpdateModelTransform()
{
	model.mat4x3 = Matrix4x3::RotationY(ang.y);
	model.mat4x3.c3 = pos >> 3;
	DropShadowRadHeight(shadow, model.mat4x3, 0x50000_f, 0x137000_f, 0xc);
}

int& StarChip::GetChipCounter()
{
	static constinit std::array<int, 3> counters;

	if (LEVEL_ID == 40)
		return counters[eventID - 1];
	else
		return counters[0];
}

int StarChip::InitResources()
{
	model.SetFile(modelFile.LoadBMD(), 1, -1);
	shadow.InitCylinder();
	
	UpdateModelTransform();
	UpdateClsnPosAndRot();
	shadowMat = model.mat4x3;
	shadowMat.c3.y -= 20._f >> 3;
	
	cylClsn.Init(this, RADIUS, HEIGHT, 0x00100002, 0x8000);
	
	eventID = param1 & 0xff;
	GetChipCounter() = 0;

	colorID = (1 <= eventID && eventID <= 3) ? eventID + 1 : 0;
	const LaunchStar::Color& color = LaunchStar::colors[colorID];

	model.data.materials[0].difAmb = color.difAmb;
	model.data.materials[0].speEmi = color.speEmi;

	// enable light 1 and disable light 0 in the final boss level
	if (LEVEL_ID == 40) (model.data.materials[0].polygonAttr &= ~1) |= 2;
	
	return 1;
}

int StarChip::CleanupResources()
{
	modelFile.Release();
	return 1;
}

int StarChip::Behavior()
{
	ang.y += ROT_SPEED;
	UpdateModelTransform();
	HandleClsn();
	cylClsn.Clear();
	cylClsn.Update();
	return 1;
}

int StarChip::Render()
{
	if (!(flags & Actor::IN_YOSHI_MOUTH))
		model.Render(nullptr);

	return 1;
}

void StarChip::HandleClsn() {
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

void StarChip::OnTurnIntoEgg(Player& player)
{
	Kill();
	MarkForDestruction();
}

unsigned StarChip::OnYoshiTryEat()
{
	return Actor::YE_SWALLOW;
}

void StarChip::Kill()
{
	int& chipCounter = GetChipCounter();
	chipCounter++;
	
	Particle::System::NewSimple(0xD2, pos.x, pos.y + 0x28000_f, pos.z);
	pos.y += 180._f;
	SpawnNumber(pos, chipCounter | std::max<unsigned>(colorID, 1) << 8, false, 0);
	Sound::Play(4, 3, camSpacePos);
	
	if (chipCounter >= 5)
	{
		Event::SetBit(eventID);
		chipCounter = 0;
	}
	
	KillAndTrackInDeathTable();
	UntrackInDeathTable();
}
