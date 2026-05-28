#include <common.h>

static s32 CAM_FollowDriver_AngleAxis_MulLo(s32 a, s32 b)
{
	return (s32)(u32)((s64)a * (s64)b);
}

static s32 CAM_FollowDriver_AngleAxis_Lerp256(s32 current, s32 previous, s32 ratio)
{
	return (CAM_FollowDriver_AngleAxis_MulLo(0x100 - ratio, current) + CAM_FollowDriver_AngleAxis_MulLo(ratio, previous)) >> 8;
}

static void CAM_FollowDriver_AngleAxis_LoadGteMatrix(MATRIX *axisMatrix, struct Driver *d)
{
	gte_SetRotMatrix(axisMatrix);
	gte_SetTransVector(d->instSelf->matrix.t);
}

static void CAM_FollowDriver_AngleAxis_TransformOffset(const s16 *offset, VECTOR *out)
{
	gte_ldv0((SVECTOR *)offset);
	gte_rtv0tr();
	gte_stlvnl(out);
}

void CAM_FollowDriver_AngleAxis(struct CameraDC *cDC, struct Driver *d, int scratchpad, s16 *pushBufferPos, s16 *pushBufferRot)
{
	MATRIX *axisMatrix = (MATRIX *)(scratchpad + 0x220);
	VECTOR *eye = (VECTOR *)(scratchpad + 0x240);
	VECTOR lookAt;
	int ratio;
	int dx;
	int dy;
	int dz;
	int distanceXZ;

	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80019128-0x800194c8.
	if (cDC->cameraMode == 0xe)
		VehPhysForce_RotAxisAngle(axisMatrix, d->AxisAngle2_normalVec, d->angle);
	else
		VehPhysForce_RotAxisAngle(axisMatrix, d->AxisAngle2_normalVec, d->rotCurr.y);

	CAM_FollowDriver_AngleAxis_LoadGteMatrix(axisMatrix, d);
	CAM_FollowDriver_AngleAxis_TransformOffset(cDC->driverOffset_CamEyePos, eye);
	CAM_FollowDriver_AngleAxis_TransformOffset(cDC->driverOffset_CamLookAtPos, &lookAt);

	if ((cDC->flags & 8) != 0)
	{
		cDC->unkTriplet3[0] = lookAt.vx;
		cDC->unkTriplet3[1] = lookAt.vy;
		cDC->unkTriplet3[2] = lookAt.vz;
	}
	else
	{
		ratio = cDC->unk7A;

		eye->vx = CAM_FollowDriver_AngleAxis_Lerp256(eye->vx, pushBufferPos[0], ratio);
		eye->vy = CAM_FollowDriver_AngleAxis_Lerp256(eye->vy, pushBufferPos[1], ratio);
		eye->vz = CAM_FollowDriver_AngleAxis_Lerp256(eye->vz, pushBufferPos[2], ratio);

		cDC->unkTriplet3[0] = CAM_FollowDriver_AngleAxis_Lerp256(lookAt.vx, cDC->unkTriplet3[0], ratio);
		cDC->unkTriplet3[1] = CAM_FollowDriver_AngleAxis_Lerp256(lookAt.vy, cDC->unkTriplet3[1], ratio);
		cDC->unkTriplet3[2] = CAM_FollowDriver_AngleAxis_Lerp256(lookAt.vz, cDC->unkTriplet3[2], ratio);
	}

	dx = eye->vx - cDC->unkTriplet3[0];
	dy = eye->vy - cDC->unkTriplet3[1];
	dz = eye->vz - cDC->unkTriplet3[2];

	*(int *)(scratchpad + 0x24c) = dx;
	*(int *)(scratchpad + 0x250) = dy;
	*(int *)(scratchpad + 0x254) = dz;

	pushBufferRot[1] = (s16)ratan2(dx, dz);
	distanceXZ = SquareRoot0_stub(CAM_FollowDriver_AngleAxis_MulLo(dx, dx) + CAM_FollowDriver_AngleAxis_MulLo(dz, dz));
	pushBufferRot[0] = 0x800 - (s16)ratan2(dy, distanceXZ);
	pushBufferRot[2] = 0;

	pushBufferPos[0] = (s16)eye->vx;
	pushBufferPos[1] = (s16)eye->vy;
	pushBufferPos[2] = (s16)eye->vz;
}
