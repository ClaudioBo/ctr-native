#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80031788-0x800317cc.
void LIST_AddBack(struct LinkedList *L, struct Item *I)
{
	struct Item *oldLast;

	if (I == 0)
		return;

	I->next = 0;

	oldLast = L->last;
	I->prev = oldLast;

	if (oldLast != 0)
	{
		L->last->next = I;
	}
	else
	{
		L->first = I;
	}

	L->last = I;
	L->count = L->count + 1;
}
