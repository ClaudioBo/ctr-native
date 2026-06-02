#ifndef PSYX_SPUAL_H
#define PSYX_SPUAL_H

#include "psx/types.h"
#include "psx/libspu.h"
#include "PsyX/PsyX_config.h"
#include "PsyX/common/pgxp_defs.h"

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

extern int PsyX_SPUAL_InitSound();
extern void PsyX_SPUAL_ShutdownSound();

// Private
extern int PsyX_SPUAL_Alloc(int size);
extern int PsyX_SPUAL_AllocWithStartAddr(u_int addr, int size);
extern int PsyX_SPUAL_InitAlloc(int num, char* top);
extern void PsyX_SPUAL_Free(u_int addr);
extern u_int PsyX_SPUAL_Write(u_char* addr, u_int size);
extern u_int PsyX_SPUAL_Read(u_char* addr, u_int size);
extern u_int PsyX_SPUAL_SetTransferStartAddr(u_int addr);

extern void PsyX_SPUAL_GetVoiceVolume(int vNum, short* volL, short* volR);
extern void PsyX_SPUAL_GetVoicePitch(int vNum, u_short* pitch);
extern void PsyX_SPUAL_SetVoiceAttr(SpuVoiceAttr* psxAttrib);
extern void PsyX_SPUAL_SetKey(int on_off, u_int voice_bit);

extern int PsyX_SPUAL_GetKeyStatus(u_int voice_bit);
extern void PsyX_SPUAL_GetAllKeysStatus(char* status);

extern int PsyX_SPUAL_SetMute(int on_off);
extern int PsyX_SPUAL_SetReverb(int on_off);
extern int PsyX_SPUAL_GetReverbState();
extern int PsyX_SPUAL_SetReverbModeParam(SpuReverbAttr* attr);
extern void PsyX_SPUAL_GetReverbModeParam(SpuReverbAttr* attr);
extern int PsyX_SPUAL_SetReverbDepth(SpuReverbAttr* attr);
extern int PsyX_SPUAL_SetReverbModeType(int mode);
extern void PsyX_SPUAL_SetReverbModeDepth(short left, short right);
extern int PsyX_SPUAL_ReserveReverbWorkArea(int on_off);
extern int PsyX_SPUAL_IsReverbWorkAreaReserved(void);
extern int PsyX_SPUAL_ClearReverbWorkArea(int mode);
extern u_int PsyX_SPUAL_SetReverbVoice(int on_off, u_int voice_bit);
extern u_int PsyX_SPUAL_GetReverbVoice();
extern void PsyX_SPUAL_SetCommonAttr(SpuCommonAttr* attr);
extern void PsyX_SPUAL_GetCommonAttr(SpuCommonAttr* attr);
extern void PsyX_SPUAL_SetCommonMasterVolume(short left, short right);
extern void PsyX_SPUAL_SetCommonCDMix(int enabled);
extern void PsyX_SPUAL_SetCommonCDVolume(short left, short right);
extern void PsyX_SPUAL_SetCommonCDReverb(int enabled);

extern int PsyX_SPUAL_PlayXATrack(int categoryID, int xaID, int volumeLeft, int volumeRight);
extern int PsyX_SPUAL_GetXATrackLength(int categoryID, int xaID);
extern int PsyX_SPUAL_IsXAPlaying(void);
extern void PsyX_SPUAL_StopXA(void);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif // PSYX_SPUAL_H
