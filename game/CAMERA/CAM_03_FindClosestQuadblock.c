#include <common.h>

void DECOMP_CAM_FindClosestQuadblock(short *scratchpad, struct CameraDC *cDC, struct Driver *d, short *pos)
{
	struct QuadBlock *quad = d->underDriver;
	if (quad == NULL)
		quad = d->lastValid;

	// TODO(aalhendi): Port the retail BSP search. These fields mirror the
	// scratchpad slots consumed by the audited camera follow path.
	scratchpad[0x1f] = quad != NULL;
	*(int *)(scratchpad + 0x40) = (int)quad;
	scratchpad[0x1e] = d->quadBlockHeight >> 8;

	cDC->ptrQuadBlock = quad;
	if (quad != NULL)
	{
		struct GameTracker *gGT = sdata->gGT;
		gGT->unk1cac[1] = ((int)quad - (int)gGT->level1->ptr_mesh_info->ptrQuadBlockArray) * -0x1642c859 >> 2;
	}
}
