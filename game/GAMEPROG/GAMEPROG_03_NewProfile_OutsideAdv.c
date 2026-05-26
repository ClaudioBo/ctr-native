#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026bf0-0x80026c24.
void GAMEPROG_NewProfile_OutsideAdv(struct GameProgress *gameProg)
{
	// GameOptions is probably a struct "inside"
	// of GameProgress, still working on it

	// GameProgress and GameOptions
	memset(gameProg, 0, sizeof(struct GameProgress) + sizeof(struct GameOptions));

	GAMEPROG_ResetHighScores(gameProg);
}
