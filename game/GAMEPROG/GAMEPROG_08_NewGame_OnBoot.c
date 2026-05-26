#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026e48-0x80026e80.
void GAMEPROG_NewGame_OnBoot()
{
	GAMEPROG_NewProfile_OutsideAdv(&sdata->gameProgress);
	GAMEPROG_NewProfile_InsideAdv(&sdata->advProgress);
	GAMEPROG_GetPtrHighScoreTrack();
}
