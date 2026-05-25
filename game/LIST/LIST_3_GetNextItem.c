#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800317cc-0x800317d8.
void *LIST_GetNextItem(struct Item *I)
{
	return I->next;
}
