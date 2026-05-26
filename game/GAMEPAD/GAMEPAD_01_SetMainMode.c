#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80025208-0x800252a0.
void GAMEPAD_SetMainMode(void)
{
	PadSetMainMode(0, 0, 0);
	PadSetMainMode(1, 0, 0);
	PadSetMainMode(2, 0, 0);
	PadSetMainMode(3, 0, 0);
	PadSetMainMode(0x10, 0, 0);
	PadSetMainMode(0x11, 0, 0);
	PadSetMainMode(0x12, 0, 0);
	PadSetMainMode(0x13, 0, 0);
}
