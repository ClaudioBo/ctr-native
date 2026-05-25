#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800317e4-0x8003186c.
struct Item *LIST_RemoveMember(struct LinkedList *L, struct Item *I)
{
	if (I == 0)
		return 0;

	if (L->first != 0)
	{
		if (I->prev != 0)
		{
			I->prev->next = I->next;
		}
		else
		{
			L->first = I->next;
		}

		if (I->next != 0)
		{
			I->next->prev = I->prev;
		}
		else
		{
			L->last = I->prev;
		}

		L->count = L->count - 1;
	}

	I->next = 0;
	I->prev = 0;

	return I;
}
