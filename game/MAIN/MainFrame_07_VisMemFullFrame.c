#include <common.h>

#ifdef CTR_NATIVE
static void MainFrame_CopyPackedVisList(int *dst, int *src, int bytesIfRaw)
{
	signed char *rle;
	unsigned char *out;

	if ((dst == NULL) || (src == NULL))
		return;

	if (((unsigned int)src & 1) == 0)
	{
		memcpy(dst, src, bytesIfRaw);
		return;
	}

	rle = (signed char *)((unsigned int)src & ~(unsigned int)3);
	out = (unsigned char *)dst;

	for (;;)
	{
		int count = *rle++;

		if (count == 0)
			return;

		if (count > 0)
		{
			while (count-- > 0)
				*out++ = (unsigned char)*rle++;
		}
		else
		{
			unsigned char value = (unsigned char)*rle++;
			count = 1 - count;
			while (count-- > 0)
				*out++ = value;
		}
	}
}

static struct QuadBlock *MainFrame_GetVisMemQuadBlock(struct CameraDC *camDC, struct Driver *driver)
{
	if ((camDC != NULL) && (camDC->ptrQuadBlock != NULL))
		return camDC->ptrQuadBlock;

	if (driver != NULL)
	{
		if (driver->underDriver != NULL)
			return driver->underDriver;

		if (driver->lastValid != NULL)
			return driver->lastValid;
	}

	return NULL;
}

// NOTE(aalhendi): CTR_NATIVE bridge, not ASM-verified. Retail refreshes VisMem
// from camera/driver PVS before 226 builds render lists; native mirrors that
// data flow so temporary level drawing no longer renders every leaf.
void MainFrame_VisMemFullFrame(struct GameTracker *gGT, struct Level *level)
{
	if ((gGT == NULL) || (level == NULL) || (gGT->visMem1 == NULL))
		return;

	if ((gGT->numPlyrCurrGame == 0) || (level->ptr_mesh_info == NULL))
		return;

	for (int i = 0; i < gGT->numPlyrCurrGame; i++)
	{
		struct CameraDC *camDC = &gGT->cameraDC[i];
		struct Driver *driver = gGT->drivers[i];
		struct QuadBlock *quad = MainFrame_GetVisMemQuadBlock(camDC, driver);
		struct PVS *pvs = quad != NULL ? quad->pvs : NULL;
		int *visLeafSrc = camDC->visLeafSrc;
		int *visFaceSrc = camDC->visFaceSrc;
		int *visExtraSrc;

		camDC->flags &= ~0x4000;

		if (pvs != NULL)
		{
			if (visLeafSrc == NULL)
				visLeafSrc = pvs->visLeafSrc;
			if (visFaceSrc == NULL)
				visFaceSrc = pvs->visFaceSrc;
			if (camDC->visInstSrc == NULL)
				camDC->visInstSrc = pvs->visInstSrc;
		}

		if (visLeafSrc != NULL)
		{
			gGT->visMem1->visLeafSrc[i] = visLeafSrc;
			MainFrame_CopyPackedVisList(gGT->visMem1->visLeafList[i], visLeafSrc, ((level->ptr_mesh_info->numBspNodes + 31) >> 5) * 4);
		}

		if (visFaceSrc != NULL)
		{
			gGT->visMem1->visFaceSrc[i] = visFaceSrc;
			MainFrame_CopyPackedVisList(gGT->visMem1->visFaceList[i], visFaceSrc, ((level->ptr_mesh_info->numQuadBlock + 31) >> 5) * 4);
		}

		if ((pvs != NULL) && (pvs->visExtraSrc != NULL))
		{
			if ((level->configFlags & 4) == 0)
				camDC->visOVertSrc = pvs->visExtraSrc;
			else
				camDC->visSCVertSrc = pvs->visExtraSrc;
		}

		visExtraSrc = ((level->configFlags & 4) == 0) ? camDC->visOVertSrc : camDC->visSCVertSrc;
		if (visExtraSrc != NULL)
		{
			if ((level->configFlags & 4) == 0)
			{
				gGT->visMem1->visOVertSrc[i] = visExtraSrc;
				MainFrame_CopyPackedVisList(gGT->visMem1->visOVertList[i], visExtraSrc, ((level->numWaterVertices + 31) >> 5) * 4);
			}
			else
			{
				gGT->visMem1->visSCVertSrc[i] = visExtraSrc;
				MainFrame_CopyPackedVisList(gGT->visMem1->visSCVertList[i], visExtraSrc, ((level->numSCVert + 31) >> 5) * 4);
			}
		}
	}
}
#endif
