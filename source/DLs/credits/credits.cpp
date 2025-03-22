#include "SM64DS_PI.h"
#include "extended_ks.h"

asm(R"(
VanillaMinimapBehavior = 0x020fa690
VanillaMinimapRender   = 0x020f9e98
minimapVtable          = 0x0210c1c0

VanillaHUDBehavior     = 0x020fd7a4
VanillaHUDRender       = 0x020fd5e0
hudVtable              = 0x0210c2c8

vanillaCamBehavior     = _ZN6Camera8BehaviorEv
camVtable              = 0x02086F84
)");

extern "C" int VanillaMinimapBehavior(Minimap&);
extern "C" int VanillaMinimapRender(Minimap&);
extern "C" int (*minimapVtable[])(Minimap&);

extern "C" int VanillaHUDBehavior(HUD&);
extern "C" int VanillaHUDRender(HUD&);
extern "C" int (*hudVtable[])(HUD&);

extern "C" int vanillaCamBehavior(Camera& cam);
extern "C" int (*camVtable[])(Camera&);

constexpr auto nop = [](auto&) { return 1; };

constinit auto credits =
	NewScript().
	ChangeMusic(73) (1).
	SetCamTargetAndPos(-11518, 23473, 91, -11518, 23473, 4891) (1).
	FadeToWhite() (100).
	SetCamTargetAndPos(-16998, 11302, 68, -16998, 11302, 3968) (130).
	FadeFromWhite() (130).
	FadeToWhite() (300).
	SetCamTargetAndPos(-7298, 11302, 68, -7298, 11302, 3968) (330).
	FadeFromWhite() (330).
	FadeToWhite() (500).
	SetCamTargetAndPos(1965, 23183, 121, 4365, 24383, 3721) (530).
	FadeFromWhite() (530).
	AdjustCamPosDec(1965, 23183, 5521, 40, 40, 40) (530, 800).
	FadeToWhite() (800).
	SetCamTargetAndPos(2902, 11302, 68, 502, 12502, 3968) (830).
	FadeFromWhite() (830).
	AdjustCamPosDec(3502, 11302, 5468, 40, 40, 40) (830, 1100).
	FadeToWhite() (1100).
	SetCamTargetAndPos(13802, 11302, 68, 15902, 10102, 3968) (1130).
	FadeFromWhite() (1130).
	AdjustCamPosDec(11902, 12202, 6368, 40, 40, 40) (1130, 1400).
	FadeToWhite() (1400).
	SetCamTargetAndPos(26102, 11302, 68, 26102, 8602, 1868) (1430).
	FadeFromWhite() (1430).
	AdjustCamPosDec(26102, 11302, 7868, 40, 40, 40) (1430, 1700).
	FadeToWhite() (1700).
	SetCamTargetAndPos(-22132, 2487, 109, -17032, 2487, 1009) (1730).
	FadeFromWhite() (1730).
	AdjustCamPosDec(-22132, 2487, 7309, 40, 40, 40) (1730, 2000).
	FadeToWhite() (2000).
	SetCamTargetAndPos(-9429, 2536, 82, -9429, 1336, 1282) (2030).
	FadeFromWhite() (2030).
	AdjustCamPosDec(-9429, 2836, 5482, 40, 40, 40) (2030, 2300).
	FadeToWhite() (2300).
	SetCamTargetAndPos(4120, 2569, 137, 6820, 4069, 3737) (2330).
	FadeFromWhite() (2330).
	AdjustCamPosDec(4120, 2569, 5537, 40, 40, 40) (2330, 2600).
	FadeToWhite() (2600).
	SetCamTargetAndPos(18820, 2569, 137, 13420, 2569, 737) (2630).
	FadeFromWhite() (2630).
	AdjustCamPosDec(18820, 2569, 8000, 40, 40, 40) (2630, 2900).
	FadeToWhite() (2900).
	SetCamTargetAndPos(-22263, -5699, 120, -22263, -3299, 1619) (2930).
	FadeFromWhite() (2930).
	AdjustCamPosDec(-22263, -5699, 8220, 40, 40, 40) (2930, 3200).
	FadeToWhite() (3200).
	SetCamTargetAndPos(-8063, -5698, 119, -10463, -6898, 5819) (3230).
	FadeFromWhite() (3230).
	AdjustCamPosDec(-8063, -5698, 8219, 40, 40, 40) (3230, 3500).
	FadeToWhite() (3500).
	SetCamTargetAndPos(4652, -5963, 34, 7352, -4763, 3633) (3530).
	FadeFromWhite() (3530).
	AdjustCamPosDec(4652, -5963, 5734, 40, 40, 40) (3530, 3800).
	FadeToWhite() (3800).
	SetCamTargetAndPos(15065, 23283, 121, 15065, 23283, 1921) (3830).
	FadeFromWhite() (3830).
	AdjustCamPosDec(15065, 23283, 7121, 40, 40, 40) (3830, 4100).
	FadeToWhite() (4100).
	SetCamTargetAndPos(16552, -6062, 33, 14152, -4862, 3933) (4130).
	FadeFromWhite() (4130).
	AdjustCamPosDec(16552, -6062, 5433, 40, 40, 40) (4130, 4400).
	FadeToWhite() (4400).
	SetCamTargetAndPos(28952, -6062, 33, 29852, -6962, 2733) (4430).
	FadeFromWhite() (4430).
	AdjustCamPosDec(28952, -6062, 5433, 40, 40, 40) (4430, 4700).
	FadeToWhite() (4700).
	SetCamTargetAndPos(-22963, -15798, 719, -22963, -12798, 5219) (4730).
	FadeFromWhite() (4730).
	AdjustCamPosDec(-22963, -15798, 9719, 40, 40, 40) (4730, 5100).
	FadeToWhite() (5300).
	SetCamTargetAndPos(-7304, -15730, 124, -4904, -16930, 4024) (5330).
	FadeFromWhite() (5330).
	AdjustCamPosDec(-7304, -15730, 7924, 40, 40, 40) (5330, 5700).
	FadeToWhite() (5800).
	SetCamTargetAndPos(7543, -15754, 79, 5443, -16654, 3979) (5830).
	FadeFromWhite() (5830).
	AdjustCamPosDec(8143, -15354, 7579, 40, 40, 40) (5830, 6200).
	FadeToWhite() (6200).
	SetCamTargetAndPos(28790, 23109, 54, 30290, 24309, 3054) (6230).
	FadeFromWhite() (6230).
	AdjustCamPosDec(28190, 22809, 6954, 40, 40, 40) (6230, 6500).
	FadeToWhite() (6500).
	SetCamTargetAndPos(22558, -16039, 12, 19958, -14839, 3312) (6530).
	FadeFromWhite() (6530).
	AdjustCamPosDec(22558, -16039, 8112, 40, 40, 40) (6530, 6800).
	Call([] { camVtable[6] = nop; }) (7070).
	End();

asm(R"(
StartSceneFade = 0x0202e348
ShowMessage1   = 0x0201f32c
ShowMessage2   = 0x0201eb94
textBoxVisible = 0x0209D660
selectedOption = 0x0209d684
)");

extern "C" void StartSceneFade(unsigned actorID, unsigned param, uint16_t fadeColor);
extern "C" void ShowMessage1(short msgID);
extern "C" void ShowMessage2(short msgID);

extern uint8_t textBoxVisible;
extern uint8_t selectedOption;

enum class State
{
	start,
	asking,
	saving,
	changingLevel,
}
constinit state = State::start;

int CustomMinimapBehavior(Minimap& minimap)
{
	if (state == State::start && !RUNNING_KUPPA_SCRIPT)
	{
		ShowMessage1(0x189);
		state = State::asking;
	}
	else if (state == State::asking && !textBoxVisible)
	{
		if (selectedOption == 1)
		{
			ShowMessage2(0x295);
			state = State::saving;
		}
		else
		{
			StartSceneFade(1, 0, 0);
			state = State::changingLevel;
		}
	}
	else if (state == State::saving && !textBoxVisible)
	{
		StartSceneFade(1, 0, 0);
		state = State::changingLevel;
	}

	return 1;
}

void init()
{
	minimapVtable[6] = &CustomMinimapBehavior;
	minimapVtable[9] = nop;
	hudVtable[6] = nop;
	hudVtable[9] = nop;

	credits.Run();
}

void cleanup()
{
	minimapVtable[6] = &VanillaMinimapBehavior;
	minimapVtable[9] = &VanillaMinimapRender;

	hudVtable[6] = &VanillaHUDBehavior;
	hudVtable[9] = &VanillaHUDRender;

	camVtable[6] = &vanillaCamBehavior;
}
