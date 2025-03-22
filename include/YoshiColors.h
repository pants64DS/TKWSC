#ifndef YOSHI_COLORS_INCLUDED
#define YOSHI_COLORS_INCLUDED

asm("characterSpeed           = 0x020ff170");
asm("characterCarrySpeed      = 0x020ff1C0");
asm("characterHeavyCarrySpeed = 0x020ff1D0");
asm("characterSpeedSwim       = 0x020ff158");
asm("characterJumpHeight      = 0x020ff140");
extern "C" unsigned short characterSpeed[4];
extern "C" Fix12i         characterCarrySpeed[4];
extern "C" Fix12i         characterHeavyCarrySpeed[4];
extern "C" unsigned short characterSpeedSwim[4];
extern "C" unsigned short characterJumpHeight[4];

//             yoshiStat                  Green         Red           Blue          Yellow
constexpr unsigned short yoshiSpeed[4]           = {      0x0ccc,       0x0e80,       0x0b60,       0x0ccc};
constexpr Fix12i         yoshiCarrySpeed[4]      = {0x00011000_f, 0x00012000_f, 0x0000b000_f, 0x0001a000_f};
constexpr Fix12i         yoshiHeavyCarrySpeed[4] = {0x00004000_f, 0x00005000_f, 0x00001a00_f, 0x00015000_f};
constexpr unsigned short yoshiSwimSpeed[4]       = {      0x0e66,       0x0c00,       0x1900,       0x0e66};
constexpr unsigned short yoshiJumpHeight[4]      = {      0x0e66,       0x0c80,       0x10a0,       0x0e66};

asm("yoshiOamPaletteTop = 0x050003ac");
asm("yoshiOamPaletteBottom = 0x050007ac");
extern "C" uint8_t yoshiOamPaletteTop[16];
extern "C" uint8_t yoshiOamPaletteBottom[16];

constexpr uint8_t yoshiOamPalettes[4][16] = 
{
	{ 0x25, 0x15, 0x87, 0x19, 0xe7, 0x1d, 0x49, 0x26, 0xaa, 0x2a, 0x0c, 0x33, 0x6f, 0x3f, 0x93, 0x4f },
	{ 0x55, 0x0c, 0x78, 0x10, 0x9a, 0x14, 0xbd, 0x18, 0x3d, 0x29, 0xde, 0x39, 0x5f, 0x4e, 0xff, 0x5e },
	{ 0xa4, 0x3c, 0xe5, 0x48, 0x47, 0x55, 0xa9, 0x5d, 0x0b, 0x66, 0x4c, 0x6e, 0xae, 0x76, 0x10, 0x7f },
	{ 0x52, 0x01, 0xb5, 0x01, 0x37, 0x02, 0x9a, 0x02, 0xfb, 0x02, 0x5c, 0x03, 0xbd, 0x03, 0xfe, 0x03 },
};

asm("GX_LoadOBJPltt = 0x020567a0");
asm("GXS_LoadOBJPltt = 0x02056738");
extern "C" void GX_LoadOBJPltt(const void* src, unsigned offset, unsigned size);
extern "C" void GXS_LoadOBJPltt(const void* src, unsigned offset, unsigned size);

inline unsigned defaultYoshiPalette;
inline bool initYoshi;
inline uint8_t currentYoshiColor;

inline void SetYoshiPaletteOffset(Player& player)
{
	player.unk61c = defaultYoshiPalette + currentYoshiColor * 2;
}

#endif