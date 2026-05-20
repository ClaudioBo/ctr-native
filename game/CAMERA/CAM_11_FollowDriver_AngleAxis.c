#include <common.h>

void DECOMP_CAM_FollowDriver_AngleAxis(struct CameraDC *cDC, struct Driver *d, int scratchpad, short *pushBufferPos, short *pushBufferRot)
{
	// TODO(aalhendi): Port the terrain axis-angle camera path. This fallback
	// keeps nonzero camera modes functional until that path is audited.
	(void)cDC;
	DECOMP_CAM_LookAtPosition(scratchpad, &d->posCurr.x, pushBufferPos, pushBufferRot);
}
