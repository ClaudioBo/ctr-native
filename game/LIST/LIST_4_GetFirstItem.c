#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800317d8-0x800317e4.
void *LIST_GetFirstItem(struct LinkedList *L)
{
	return L->first;
}
