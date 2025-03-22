#include "SM64DS_PI.h"
#include "final_boss/bowser_and_spike_bomb.h"
#include "final_boss/airship.h"
#include "LaunchStar.h"
#include "Cutscene.h"

inline void DefeatBowser()
{
	Bowser* bowser = Actor::Find<Bowser>();
	SpikeBomb* bomb = Actor::Find<SpikeBomb>();

	if (bowser && bomb)
	{
		bowser->health = 1;
		bowser->pos = bomb->pos;
	}
}

template<class A>
inline void KillAll()
{
	for (auto& actor : Actor::Iterate<A>())
		actor.Destroy();
}

template<class A>
inline void WarpToActor(unsigned n)
{
	for (unsigned i = 0; const auto& actor : Actor::Iterate<A>())
	{
		if (i++ == n)
		{
			PLAYER_ARR[0]->pos = actor.pos;
			CAMERA->lookAt = PLAYER_ARR[0]->pos;
			CAMERA->pos = PLAYER_ARR[0]->pos;
			CAMERA->pos.x += 1000._f;
			break;
		}
	}
}

inline int GetSoundHeapMemoryLeft()
{
	if (!Memory::soundHeapAllocatorPtrPtr || !*Memory::soundHeapAllocatorPtrPtr)
		return -1;

	const SolidHeapAllocator& allocator = **Memory::soundHeapAllocatorPtrPtr;

	return static_cast<char*>(allocator.freeBlockEnd) - static_cast<char*>(allocator.freeBlockBegin);
}

struct HeapRecord
{
	int8_t levelID = -1;
	int8_t starID  = -1;

	unsigned mainHeapMemLeft  = ~0;
	unsigned actorHeapMemLeft = ~0;
	unsigned soundHeapMemLeft = ~0;

	static HeapRecord worstCaseForMainHeap;
	static HeapRecord worstCaseForActorHeap;
	static HeapRecord worstCaseForSoundHeap;

	bool UpdateWorstCases() const
	{
		bool changed = false;

		if (worstCaseForMainHeap.mainHeapMemLeft > mainHeapMemLeft)
		{
			worstCaseForMainHeap = *this;
			changed = true;
		}

		if (worstCaseForActorHeap.actorHeapMemLeft > actorHeapMemLeft)
		{
			worstCaseForActorHeap = *this;
			changed = true;
		}

		if (worstCaseForSoundHeap.soundHeapMemLeft > soundHeapMemLeft)
		{
			worstCaseForSoundHeap = *this;
			changed = true;
		}

		return changed;
	}

	bool Record()
	{
		levelID = LEVEL_ID;
		starID = STAR_ID;

		mainHeapMemLeft  = Memory::rootHeapPtr->VMemoryLeft();
		actorHeapMemLeft = Memory::gameHeapPtr->VMemoryLeft();
		soundHeapMemLeft = GetSoundHeapMemoryLeft();

		return UpdateWorstCases();
	}

	void Print()
	{
		cout << "Heap record for level " << static_cast<int>(levelID);
		cout << ", star " << static_cast<int>(starID);
		cout << ":\n\tMain heap memory left:  " << mainHeapMemLeft;
		cout << "\n\tActor heap memory left: " << actorHeapMemLeft;
		cout << "\n\tSound heap memory left: " << soundHeapMemLeft;
		cout << '\n';
	}
};

constinit inline HeapRecord HeapRecord::worstCaseForMainHeap;
constinit inline HeapRecord HeapRecord::worstCaseForActorHeap;
constinit inline HeapRecord HeapRecord::worstCaseForSoundHeap;

extern "C" void LoadLevel(int8_t levelID, int8_t entranceID, int8_t starID, int8_t mode);

constexpr auto levelIDs = std::to_array<int8_t>
({
	0, 1, 6, 7, 10, 11, 14, 15, 16, 19, 20, 21, 22, 24, 26, 27, 28, 39, 40, 50
});

inline void GatherHeapRecords()
{
	static constinit uint8_t levelID_ID = 0;
	static constinit HeapRecord currRecord;

	std::ranges::fill(SAVE_DATA.stars, 0xff);

	if (PLAYER_ARR[0] && PLAYER_ARR[0]->currState == &Player::ST_WAIT ||
		LEVEL_ID == 50 && KS_FRAME_COUNTER == 60)
	{
		if (currRecord.Record())
		{
			cout << "(worst case for the main heap) ";
			HeapRecord::worstCaseForMainHeap.Print();

			cout << "(worst case for the actor heap) ";
			HeapRecord::worstCaseForActorHeap.Print();

			cout << "(worst case for the sound heap) ";
			HeapRecord::worstCaseForSoundHeap.Print();
		}

		if (levelIDs[levelID_ID] == LEVEL_ID)
		{
			++levelID_ID;

			if (levelID_ID >= levelIDs.size())
				levelID_ID = 0;

			LoadLevel(levelIDs[levelID_ID], 0, 0, 0);
		}
	}
}

asm("LISTRAM_COUNT = 0x04000604");
asm("VTXRAM_COUNT = 0x04000606");

extern volatile uint16_t LISTRAM_COUNT;
extern volatile uint16_t VTXRAM_COUNT;

struct
{
	uint16_t CURR_LISTRAM_COUNT = 0;
	uint16_t MAX_LISTRAM_COUNT = 0;
	uint16_t CURR_VTXRAM_COUNT = 0;
	uint16_t MAX_VTXRAM_COUNT = 0;

	void ResetMaxCounts()
	{
		MAX_LISTRAM_COUNT = 0;
		MAX_VTXRAM_COUNT = 0;
	}

	void Update()
	{
		CURR_LISTRAM_COUNT = LISTRAM_COUNT;
		CURR_VTXRAM_COUNT = VTXRAM_COUNT;

		if (MAX_LISTRAM_COUNT < CURR_LISTRAM_COUNT)
			MAX_LISTRAM_COUNT = CURR_LISTRAM_COUNT;

		if (MAX_VTXRAM_COUNT < CURR_VTXRAM_COUNT)
			MAX_VTXRAM_COUNT = CURR_VTXRAM_COUNT;
	}

	void Display()
	{
		ShowDecimalInt(CURR_LISTRAM_COUNT, 10, 10, false, CURR_LISTRAM_COUNT >= 2048);
		ShowDecimalInt(MAX_LISTRAM_COUNT,  10, 30, false, MAX_LISTRAM_COUNT  >= 2048);
	}
}
counters;

// Right after SwapBuffers
void nsub_020190f4()
{
	counters.Update();
}

inline void ShowHeapInfo()
{
	const unsigned rootHeapKilobytesLeft           = Memory::rootHeapPtr->VMemoryLeft()         >> 10;
	const unsigned rootHeapMaxAllocatableKilobytes = Memory::rootHeapPtr->VMaxAllocatableSize() >> 10;
	const unsigned gameHeapKilobytesLeft           = Memory::gameHeapPtr->VMemoryLeft()         >> 10;
	const unsigned gameHeapMaxAllocatableKilobytes = Memory::gameHeapPtr->VMaxAllocatableSize() >> 10;

	ShowDecimalInt(rootHeapKilobytesLeft,           10,  30, true);
	ShowDecimalInt(rootHeapMaxAllocatableKilobytes, 10,  50, true);
	ShowDecimalInt(gameHeapKilobytesLeft,           10,  90, true);
	ShowDecimalInt(gameHeapMaxAllocatableKilobytes, 10, 110, true);

	const int soundHeapKilobytesLeft = GetSoundHeapMemoryLeft() >> 10;

	ShowDecimalInt(soundHeapKilobytesLeft, 200, 30, true);
}

void UpdateDebug()
{
	if (INPUT_ARR[0].buttonsPressed & Input::R)
	{
		cout << int(LEVEL_ID) << ", " << int(STAR_ID) << ": ";
		cout << Memory::rootHeapPtr->VMemoryLeft() << '\n';
	}

	GatherHeapRecords();

	// if (INPUT_ARR[0].buttonsPressed & Input::R) DefeatBowser();

	// if (LEVEL_ID == 39) EndKuppaScript();

	// if (PLAYER_ARR[0]->currState == &Player::ST_GROUND_POUND)
	// {
		// if (INPUT_ARR[0].buttonsPressed & Input::L) WarpToActor<LaunchStar>(0);
		// if (INPUT_ARR[0].buttonsPressed & Input::A) WarpToActor<Airship>(2);
		// if (INPUT_ARR[0].buttonsPressed & Input::B) WarpToActor<Airship>(1);
		// if (INPUT_ARR[0].buttonsPressed & Input::Y) WarpToActor<Airship>(0);
	// }

	counters.Display();
	ShowHeapInfo();
}
