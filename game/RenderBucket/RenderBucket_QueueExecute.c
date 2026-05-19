#include <common.h>

struct RenderBucketEntry
{
	struct Instance *inst;
	struct Instance *instPlayerBase;
};

typedef struct
{
	u_char x;
	u_char y;
	u_char z;
} RenderBucketCompVertex;

typedef struct
{
	u_char x;
	u_char y;
	u_char z;
	u_char w;
} RenderBucketVertex;

struct RenderBucketDrawContext
{
	struct Instance *inst;
	struct InstDrawPerPlayer *idpp;
	struct PushBuffer *pb;
	struct PrimMem *primMem;
	struct ModelHeader *mh;
	struct ModelFrame *mf;
	struct ModelAnim *anim;
	char *vertData;
	RenderBucketVertex tempCoords[4];
	int tempColor[4];
	RenderBucketVertex stack[256];
	int bitIndex;
	int x_alu;
	int y_alu;
	int z_alu;
	int stripLength;
	int vertexIndex;
};

static int RenderBucket_GetSignedBits(unsigned int *vertData, int *bitIndex, int bits)
{
	int const b = *bitIndex >> 5;
	int const e = 32 - bits;
	int const s = e - (*bitIndex & 31);
	int const ret = s < 0 ? (vertData[b] << -s) | (vertData[b + 1] >> (s & 31)) : vertData[b] >> s;

	*bitIndex += bits;
	return (ret << e) >> e;
}

static struct ModelAnim *RenderBucket_GetAnim(struct Instance *inst, struct ModelHeader *mh)
{
	if (mh->ptrAnimations == 0)
		return 0;

	if (mh->numAnimations == 0)
		return 0;

	if (inst->animIndex >= mh->numAnimations)
		return 0;

	return mh->ptrAnimations[inst->animIndex];
}

static struct ModelFrame *RenderBucket_GetFrame(struct Instance *inst, struct ModelHeader *mh, struct ModelAnim **animOut)
{
	struct ModelAnim *anim;
	int frameIndex;
	char *firstFrame;

	*animOut = 0;

	if (mh->ptrFrameData != 0)
		return mh->ptrFrameData;

	anim = RenderBucket_GetAnim(inst, mh);
	if (anim == 0)
		return 0;

	if (anim->numFrames == 0)
		return 0;

	frameIndex = inst->animFrame;
	if (frameIndex < 0)
		frameIndex = 0;
	frameIndex %= anim->numFrames;

	firstFrame = (char *)MODELANIM_GETFRAME(anim);
	*animOut = anim;
	return (struct ModelFrame *)(firstFrame + (anim->frameSize * frameIndex));
}

static struct RenderBucketEntry *RenderBucket_QueueDraw(struct Instance *inst, struct RenderBucketEntry *rbi, int playerIndex, unsigned int lodMask,
                                                        int gameMode1)
{
	struct ModelHeader *mh;
	struct ModelAnim *anim;
	struct ModelFrame *frame;
	struct InstDrawPerPlayer *idpp;
	struct Instance *instPlayerBase;

	// NOTE(aalhendi): PSX-backfeed blocker: retail QueueDraw consumes implicit
	// scratchpad/register state from QueueLev/QueueNonLev. This native helper
	// spells that state out as C parameters until the full ASM audit is done.
	(void)gameMode1;

	if (inst == 0)
		return rbi;

	if (inst->model == 0)
		return rbi;

	if (inst->model->headers == 0)
		return rbi;

	if ((inst->flags & lodMask) == 0)
		return rbi;

	instPlayerBase = (struct Instance *)((char *)inst + (playerIndex * sizeof(struct InstDrawPerPlayer)));
	idpp = INST_GETIDPP(instPlayerBase);

	if (idpp->pushBuffer == 0)
		return rbi;

	mh = &inst->model->headers[0];
	frame = RenderBucket_GetFrame(inst, mh, &anim);
	if (frame == 0)
		return rbi;

	idpp->instFlags = inst->flags | 0x40;
	idpp->unkbc = inst->alphaScale;
	idpp->ptrCurrFrame = frame;
	idpp->ptrNextFrame = 0;
	idpp->ptrCommandList = mh->ptrCommandList;
	idpp->ptrTexLayout = mh->ptrTexLayout;
	idpp->ptrColorLayout = (unsigned int)mh->ptrColors;
	idpp->ptrDeltaArray = (anim != 0) ? (int)anim->ptrDeltaArray : 0;
	idpp->lodIndex = 0;
	idpp->mh = mh;
	idpp->unkE4 = 0;
	idpp->unkE8 = 0;
	idpp->unkEC = 0;
	idpp->unkF0 = 0;

	rbi->inst = inst;
	rbi->instPlayerBase = instPlayerBase;
	return rbi + 1;
}

void *RenderBucket_QueueLevInstances(struct CameraDC *cDC, u_long *otMem, void *rbi, char *lod, char numPlyr, int gameMode1)
{
	struct RenderBucketEntry *entry = (struct RenderBucketEntry *)rbi;
	unsigned int lodMask = (unsigned int)(unsigned char)(unsigned int)lod;
	int count = (int)(unsigned char)numPlyr;

	// NOTE(aalhendi): PSX-backfeed blocker: native C context replaces the
	// retail scratchpad register-save frame at 0x1f800000.
	(void)otMem;

	for (int player = count - 1; player >= 0; player--)
	{
		struct Instance **visInstSrc = cDC[player].visInstSrc;

		if (visInstSrc == 0)
			continue;

		for (; *visInstSrc != 0; visInstSrc++)
		{
			entry = RenderBucket_QueueDraw(*visInstSrc, entry, player, lodMask, gameMode1);
		}
	}

	return entry;
}

void *RenderBucket_QueueNonLevInstances(struct Item *item, u_long *otMem, void *rbi, char *lod, char numPlyr, int gameMode1)
{
	struct RenderBucketEntry *entry = (struct RenderBucketEntry *)rbi;
	unsigned int lodMask = (unsigned int)(unsigned char)(unsigned int)lod;
	int count = (int)(unsigned char)numPlyr;

	// NOTE(aalhendi): PSX-backfeed blocker: native C context replaces the
	// retail scratchpad register-save frame at 0x1f800000.
	(void)otMem;

	for (int player = count - 1; player >= 0; player--)
	{
		for (struct Item *curr = item; curr != 0; curr = curr->next)
		{
			entry = RenderBucket_QueueDraw((struct Instance *)curr, entry, player, lodMask, gameMode1);
		}
	}

	return entry;
}

void RenderBucket_UncompressAnimationFrame(struct RenderBucketDrawContext *ctx, u_short stackIndex)
{
	if ((ctx->anim != 0) && (ctx->anim->ptrDeltaArray != 0))
	{
		u_int temporal = ctx->anim->ptrDeltaArray[ctx->vertexIndex];
		u_char xBits = (temporal >> 6) & 7;
		u_char yBits = (temporal >> 3) & 7;
		u_char zBits = temporal & 7;
		u_char bx = (temporal >> 0x19) << 1;
		u_char by = (temporal << 7) >> 0x18;
		u_char bz = (temporal << 0xf) >> 0x18;

		// NOTE(aalhendi): PSX-backfeed blocker: retail consumes command state
		// from registers and scratchpad. This native form keeps the same delta
		// decode semantics with explicit C context.
		if (xBits == 7)
			ctx->x_alu = 0;
		if (yBits == 7)
			ctx->y_alu = 0;
		if (zBits == 7)
			ctx->z_alu = 0;

		ctx->x_alu += RenderBucket_GetSignedBits((unsigned int *)ctx->vertData, &ctx->bitIndex, xBits + 1) + bx;
		ctx->y_alu += RenderBucket_GetSignedBits((unsigned int *)ctx->vertData, &ctx->bitIndex, yBits + 1) + by;
		ctx->z_alu += RenderBucket_GetSignedBits((unsigned int *)ctx->vertData, &ctx->bitIndex, zBits + 1) + bz;

		ctx->stack[stackIndex].x = ctx->x_alu;
		ctx->stack[stackIndex].y = ctx->z_alu;
		ctx->stack[stackIndex].z = ctx->y_alu;
	}
	else
	{
		RenderBucketCompVertex *ptrVerts = (RenderBucketCompVertex *)ctx->vertData;

		ctx->stack[stackIndex].x = ptrVerts[ctx->vertexIndex].x;
		ctx->stack[stackIndex].y = ptrVerts[ctx->vertexIndex].y;
		ctx->stack[stackIndex].z = ptrVerts[ctx->vertexIndex].z;
	}
}

int RenderBucket_DrawInstPrim_Normal(struct RenderBucketDrawContext *ctx, u_int command)
{
	short posWorld1[4];
	short posWorld2[4];
	short posWorld3[4];
	u_short flags = (command >> 24) & 0xff;
	u_short texIndex = command & 0x1ff;
	int boolPassCull;
	int otZ;

	// NOTE(aalhendi): PSX-backfeed blocker: retail DrawInstPrim_Normal is a
	// register-entry leaf. Native uses explicit context until the register and
	// scratchpad protocol is audited symbol-by-symbol.
	posWorld1[0] = ctx->mf->pos[0] + ctx->tempCoords[1].x;
	posWorld1[1] = ctx->mf->pos[1] + ctx->tempCoords[1].z;
	posWorld1[2] = ctx->mf->pos[2] + ctx->tempCoords[1].y;
	posWorld1[3] = 0;
	gte_ldv0(&posWorld1[0]);

	posWorld2[0] = ctx->mf->pos[0] + ctx->tempCoords[2].x;
	posWorld2[1] = ctx->mf->pos[1] + ctx->tempCoords[2].z;
	posWorld2[2] = ctx->mf->pos[2] + ctx->tempCoords[2].y;
	posWorld2[3] = 0;
	gte_ldv1(&posWorld2[0]);

	posWorld3[0] = ctx->mf->pos[0] + ctx->tempCoords[3].x;
	posWorld3[1] = ctx->mf->pos[1] + ctx->tempCoords[3].z;
	posWorld3[2] = ctx->mf->pos[2] + ctx->tempCoords[3].y;
	posWorld3[3] = 0;
	gte_ldv2(&posWorld3[0]);

	gte_rtpt();

	boolPassCull = ((flags & 0x10) == 0);
	if (!boolPassCull)
	{
		int opZ;

		gte_nclip();
		gte_stopz(&opZ);
		boolPassCull = (opZ >= 0);

		if ((flags & 0x20) != 0)
			boolPassCull = !boolPassCull;

		if ((ctx->inst->flags & REVERSE_CULL_DIRECTION) != 0)
			boolPassCull = !boolPassCull;
	}

	if (!boolPassCull)
		return 0;

	gte_avsz3();
	gte_stotz(&otZ);

	if (otZ <= 32)
		return 0;

	otZ -= 32;
	if (otZ >= 4080)
		return 0;

	if ((char *)ctx->primMem->curr + sizeof(POLY_GT3) >= (char *)ctx->primMem->endMin100)
		return -1;

	if (texIndex == 0)
	{
		POLY_G3 *p = ctx->primMem->curr;

		*(int *)&p->r0 = ctx->tempColor[1];
		*(int *)&p->r1 = ctx->tempColor[2];
		*(int *)&p->r2 = ctx->tempColor[3];
		setPolyG3(p);
		gte_stsxy3(&p->x0, &p->x1, &p->x2);
		AddPrim((u_long *)ctx->pb->ptrOT + (otZ >> 2), p);
		ctx->primMem->curr = p + 1;
	}
	else
	{
		struct TextureLayout *tex;
		POLY_GT3 *p;

		if (ctx->mh->ptrTexLayout == 0)
			return 0;

		tex = ctx->mh->ptrTexLayout[texIndex - 1];
		if (tex == 0)
			return 0;

		p = ctx->primMem->curr;
		*(int *)&p->r0 = ctx->tempColor[1];
		*(int *)&p->r1 = ctx->tempColor[2];
		*(int *)&p->r2 = ctx->tempColor[3];
		*(int *)&p->u0 = *(int *)&tex->u0;
		*(int *)&p->u1 = *(int *)&tex->u1;
		*(short *)&p->u2 = *(short *)&tex->u2;
		setPolyGT3(p);
		gte_stsxy3(&p->x0, &p->x1, &p->x2);
		AddPrim((u_long *)ctx->pb->ptrOT + (otZ >> 2), p);
		ctx->primMem->curr = p + 1;
	}

	return 0;
}

void RenderBucket_DrawFunc_Normal(struct RenderBucketDrawContext *ctx)
{
	u_int *pCmd;

	// NOTE(aalhendi): Native C command-list traversal for the normal
	// RenderBucket draw function. It has the named retail boundary, but the
	// exact branch/register choreography remains a pending ASM audit.
	pCmd = (u_int *)ctx->mh->ptrCommandList;
	pCmd++;

	for (; *pCmd != 0xffffffff; pCmd++, ctx->stripLength++)
	{
		u_int command = *pCmd;
		u_short flags = (command >> 24) & 0xff;
		u_short stackIndex = (command >> 16) & 0xff;
		u_short colorIndex = (command >> 9) & 0x7f;

		if ((flags & 4) == 0)
		{
			RenderBucket_UncompressAnimationFrame(ctx, stackIndex);
			ctx->vertexIndex++;
		}

		ctx->tempCoords[0] = ctx->tempCoords[1];
		ctx->tempCoords[1] = ctx->tempCoords[2];
		ctx->tempCoords[2] = ctx->tempCoords[3];
		ctx->tempCoords[3] = ctx->stack[stackIndex];

		ctx->tempColor[0] = ctx->tempColor[1];
		ctx->tempColor[1] = ctx->tempColor[2];
		ctx->tempColor[2] = ctx->tempColor[3];
		ctx->tempColor[3] = ctx->mh->ptrColors[colorIndex];

		if ((flags & 0x40) != 0)
		{
			ctx->tempCoords[1] = ctx->tempCoords[0];
			ctx->tempColor[1] = ctx->tempColor[0];
		}

		if ((flags & 0x80) != 0)
			ctx->stripLength = 0;

		if (ctx->stripLength < 2)
			continue;

		if (RenderBucket_DrawInstPrim_Normal(ctx, command) < 0)
			return;
	}
}

static int RenderBucket_PrepareDrawContext(struct RenderBucketDrawContext *ctx, struct Instance *inst, struct Instance *instPlayerBase, struct PrimMem *primMem)
{
	struct InstDrawPerPlayer *idpp;
	struct PushBuffer *pb;
	struct ModelHeader *mh;
	struct ModelFrame *mf;
	struct ModelAnim *anim;
	int scale[3];
	MATRIX mvp;
	VECTOR pos;

	if (inst == 0)
		return 0;

	if (inst->model == 0)
		return 0;

	idpp = INST_GETIDPP(instPlayerBase);
	pb = idpp->pushBuffer;
	if (pb == 0)
		return 0;

	if ((idpp->instFlags & 0x40) == 0)
		return 0;

	if ((idpp->instFlags & 0x80) != 0)
		return 0;

	mh = idpp->mh;
	if (mh == 0)
		return 0;

	if ((mh->ptrCommandList == 0) || (mh->ptrColors == 0))
		return 0;

	mf = idpp->ptrCurrFrame;
	if (mf == 0)
		return 0;

	anim = RenderBucket_GetAnim(inst, mh);

	scale[0] = (mh->scale[0] * inst->scale[0]) >> 12;
	scale[1] = (mh->scale[1] * inst->scale[1]) >> 12;
	scale[2] = (mh->scale[2] * inst->scale[2]) >> 12;

#define RB_MVP(row, col, index) ((pb->matrix_ViewProj.m[row][index] * ((inst->matrix.m[index][col] * scale[col]) >> 8)) >> 0x10)

	mvp.m[0][0] = RB_MVP(0, 0, 0) + RB_MVP(0, 0, 1) + RB_MVP(0, 0, 2);
	mvp.m[0][1] = RB_MVP(0, 1, 0) + RB_MVP(0, 1, 1) + RB_MVP(0, 1, 2);
	mvp.m[0][2] = RB_MVP(0, 2, 0) + RB_MVP(0, 2, 1) + RB_MVP(0, 2, 2);
	mvp.m[1][0] = RB_MVP(1, 0, 0) + RB_MVP(1, 0, 1) + RB_MVP(1, 0, 2);
	mvp.m[1][1] = RB_MVP(1, 1, 0) + RB_MVP(1, 1, 1) + RB_MVP(1, 1, 2);
	mvp.m[1][2] = RB_MVP(1, 2, 0) + RB_MVP(1, 2, 1) + RB_MVP(1, 2, 2);
	mvp.m[2][0] = RB_MVP(2, 0, 0) + RB_MVP(2, 0, 1) + RB_MVP(2, 0, 2);
	mvp.m[2][1] = RB_MVP(2, 1, 0) + RB_MVP(2, 1, 1) + RB_MVP(2, 1, 2);
	mvp.m[2][2] = RB_MVP(2, 2, 0) + RB_MVP(2, 2, 1) + RB_MVP(2, 2, 2);

#undef RB_MVP

	pos.vx = inst->matrix.t[0] - pb->matrix_Camera.t[0];
	pos.vy = inst->matrix.t[1] - pb->matrix_Camera.t[1];
	pos.vz = inst->matrix.t[2] - pb->matrix_Camera.t[2];

	ApplyMatrixLV(&pb->matrix_ViewProj, &pos, &mvp.t[0]);
	gte_SetRotMatrix(&mvp);
	gte_SetTransMatrix(&mvp);
	gte_SetGeomOffset(pb->rect.w >> 1, pb->rect.h >> 1);
	gte_SetGeomScreen(pb->distanceToScreen_PREV);

	idpp->mvp = mvp;

	if ((inst->flags & 0x400) != 0)
		return 0;

	ctx->inst = inst;
	ctx->idpp = idpp;
	ctx->pb = pb;
	ctx->primMem = primMem;
	ctx->mh = mh;
	ctx->mf = mf;
	ctx->anim = anim;
	ctx->vertData = MODELFRAME_GETVERT(mf);
	return 1;
}

void RenderBucket_Execute(void *param_1, struct PrimMem *param_2)
{
	struct RenderBucketEntry *entry = (struct RenderBucketEntry *)param_1;

	// NOTE(aalhendi): Native C implementation of the RenderBucket execution
	// contract. Full scratchpad/register ASM audit is still pending.
	for (; entry->inst != 0; entry++)
	{
		struct RenderBucketDrawContext ctx = {0};

		if (RenderBucket_PrepareDrawContext(&ctx, entry->inst, entry->instPlayerBase, param_2) == 0)
			continue;

		RenderBucket_DrawFunc_Normal(&ctx);
	}
}
