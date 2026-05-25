#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003197c-0x800319e8.
void LIST_Init(struct LinkedList *L, struct Item *item, int itemSize, int numItems)
{
	while (numItems > 0)
	{
		LIST_AddBack(L, item);

		numItems--;
		item = (struct Item *)((int)item + itemSize);
	}
}
