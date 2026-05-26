#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80043c04-0x80043c10.
struct RectMenu *QueueLoadTrack_GetMenuPtr()
{
	return &data.menuQueueLoadTrack;
}
