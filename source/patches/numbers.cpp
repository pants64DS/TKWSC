#include "SM64DS_PI.h"

extern unsigned unk020a4bd8;
extern unsigned unk020a4bcc;

asm("unk020a4bd8 = 0x020a4bd8");
asm("unk020a4bcc = 0x020a4bcc");

constexpr unsigned paletteDataSize = 0x220;
constinit std::array<std::array<unsigned, 5>, 5> vramOffsets = {};

static void LoadExtraPalettes(BMD_File& file, unsigned colorID)
{
	for (unsigned i = 0; i < 5; ++i)
	{
		auto& palette = file.palettes[i + 1];

		if (unk020a4bd8 < unk020a4bcc + palette.size)
			Crash();

		uint16_t* data = (uint16_t*)(palette.data + colorID * paletteDataSize);

		if (palette.size < 9)
		{
			GX::LoadTexPltt(data, unk020a4bcc, palette.size);
			vramOffsets[colorID][i] = unk020a4bcc;
			unk020a4bcc += palette.size + 7 & 0xfff8;
		}
		else
		{
			unk020a4bd8 -= palette.size + 0xf & 0xfff0;
			GX::LoadTexPltt(data, unk020a4bd8, palette.size);
			vramOffsets[colorID][i] = unk020a4bd8;
		}
	}
}

BMD_File& repl_0202d1d8(SharedFilePtr& sfp)
{
	if (sfp.fileID != 0x8009)
		return sfp.LoadBMD();

	BMD_File& file = *reinterpret_cast<BMD_File*>(sfp.Load());

	if (sfp.numRefs == 1)
	{
		file.InitPointers();
		file.AddToArrayAndLoadTexAndPal();

		for (unsigned i = 0; i < 5; ++i)
			vramOffsets[0][i] = file.palettes[i + 1].vramOffset;

		GX::BeginLoadTexPltt();

		switch (LEVEL_ID)
		{
		case 6:
		case 14:
		case 15:
		case 16:
		case 27:
			LoadExtraPalettes(file, 1);
			break;
		case 40:
			LoadExtraPalettes(file, 2);
			LoadExtraPalettes(file, 3);
			LoadExtraPalettes(file, 4);
		}

		GX::EndLoadTexPltt();
		file.ShrinkAllocation();
	}

	return file;
}

asm(R"(
repl_020f0a2c_ov_02:
	mov    r0, r4
	b      UpdateNumberPalette
)");

extern "C" TextureSequence& UpdateNumberPalette(Number& number)
{
	if ((char*)number.model.data.modelFile != RED_NUMBER_MODEL_PTR.filePtr)
		return number.textureSequence;

	auto& file = *number.model.data.modelFile;
	const unsigned colorID = std::min((unsigned)number.param1 >> 8, 4u);

	for (unsigned i = 0; i < 5; ++i)
		file.palettes[i + 1].vramOffset = vramOffsets[colorID][i];

	return number.textureSequence;
}

// Make invisible secrets spawn numbers higher
Number* repl_020f04c4_ov_02(Actor& secret, const Vector3& pos, unsigned number, bool isSilver, uint16_t unk14c, Actor* unkActor)
{
	const Vector3 newPos = {pos.x, pos.y + 180._f, pos.z};

	return secret.SpawnNumber(newPos, number, isSilver, unk14c, unkActor);
}
