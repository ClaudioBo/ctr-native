#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80031744-0x80031788.
void LIST_AddFront(struct LinkedList *L, struct Item *I)
{
	struct Item *oldFirst;

	if (I == 0)
		return;

	I->prev = 0;

	oldFirst = L->first;
	I->next = oldFirst;

	if (oldFirst != 0)
	{
		L->first->prev = I;
	}
	else
	{
		L->last = I;
	}

	L->first = I;
	L->count = L->count + 1;
}
