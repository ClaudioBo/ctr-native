#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80031734-0x80031744.
void LIST_Clear(struct LinkedList *L)
{
	L->first = 0;
	L->last = 0;
	L->count = 0;
}
