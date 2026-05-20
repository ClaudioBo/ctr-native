#include <common.h>

void DECOMP_CAM_StartLine_FlyIn(struct FlyInData *flyInData, short maxFrames, int frame, short *desiredPos, short *desiredRot)
{
	struct Level *lev = sdata->gGT->level1;
	int frameIndex = (frame << 0x10) >> 4;
	int frameRatio = frameIndex / maxFrames;
	int countEnd = flyInData->frameCount1;
	short count = flyInData->frameCount2;
	SVECTOR local_78;
	SVECTOR local_70;
	SVECTOR rot;
	MATRIX matrix;
	VECTOR transformed;
	long flags[2];
	short *pathEnd;
	short *pathStart;
	int pathRatioEnd;

	if (count < countEnd)
		count = countEnd;

	short pathIndex = (short)(count * frameRatio >> 0xc);
	if (pathIndex < countEnd - 1)
	{
		pathEnd = (short *)(flyInData->ptrEnd + pathIndex * 6);
		pathRatioEnd = frameRatio;
	}
	else
	{
		pathEnd = (short *)(flyInData->ptrEnd + countEnd * 6 - 0xc);
		pathRatioEnd = 0;
	}

	if (pathIndex < flyInData->frameCount2 - 1)
	{
		pathStart = (short *)(flyInData->ptrStart + pathIndex * 6);
	}
	else
	{
		pathStart = (short *)(flyInData->ptrStart + flyInData->frameCount2 * 6 - 0xc);
		frameRatio = 0;
	}

	int ratio = count * pathRatioEnd & 0xfff;
	local_78.vx = pathEnd[0] + (short)(((pathEnd[3] - pathEnd[0]) * ratio) >> 0xc);
	local_78.vy = pathEnd[1] + (short)(((pathEnd[4] - pathEnd[1]) * ratio) >> 0xc);
	local_78.vz = pathEnd[2] + (short)(((pathEnd[5] - pathEnd[2]) * ratio) >> 0xc);

	ratio = count * frameRatio & 0xfff;
	local_70.vx = pathStart[0] + (short)(((pathStart[3] - pathStart[0]) * ratio) >> 0xc);
	local_70.vy = pathStart[1] + (short)(((pathStart[4] - pathStart[1]) * ratio) >> 0xc) - 0x60;
	local_70.vz = pathStart[2] + (short)(((pathStart[5] - pathStart[2]) * ratio) >> 0xc);

	rot.vx = lev->DriverSpawn[0].rot[0];
	rot.vy = lev->DriverSpawn[0].rot[1] + 0x400;
	rot.vz = lev->DriverSpawn[0].rot[2];

	ConvertRotToMatrix(&matrix, (short *)&rot);

	DECOMP_CAM_StartLine_FlyIn_FixY(&lev->DriverSpawn[1].pos[0]);
	DECOMP_CAM_StartLine_FlyIn_FixY(&lev->DriverSpawn[2].pos[0]);
	DECOMP_CAM_StartLine_FlyIn_FixY(&lev->DriverSpawn[5].pos[0]);

	matrix.t[0] = lev->DriverSpawn[1].pos[0] + (lev->DriverSpawn[2].pos[0] - lev->DriverSpawn[1].pos[0]) / 2;
	matrix.t[1] = lev->DriverSpawn[1].pos[1] + (lev->DriverSpawn[2].pos[1] - lev->DriverSpawn[1].pos[1]) / 2 + 0x40;
	matrix.t[2] = lev->DriverSpawn[1].pos[2] + (lev->DriverSpawn[2].pos[2] - lev->DriverSpawn[1].pos[2]) / 2;

	SetRotMatrix(&matrix);
	SetTransMatrix(&matrix);

	RotTrans(&local_78, &transformed, flags);
	desiredPos[0] = (short)transformed.vx;
	desiredPos[1] = (short)transformed.vy;
	desiredPos[2] = (short)transformed.vz;

	RotTrans(&local_70, &transformed, flags);

	short deltaX = desiredPos[0] - (short)transformed.vx;
	short deltaY = desiredPos[1] - (short)transformed.vy;
	short deltaZ = desiredPos[2] - (short)transformed.vz;

	desiredRot[1] = (short)ratan2(deltaX, deltaZ);
	desiredRot[0] = 0x800 - (short)ratan2(deltaY, SquareRoot0(deltaX * deltaX + deltaZ * deltaZ));
	desiredRot[2] = 0;
}
