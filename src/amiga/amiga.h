#ifndef _ATARI_AMIGA_H_
#define _ATARI_AMIGA_H_

LONG DisplayYesNoWindow(void);

LONG InsertROM(LONG CartType);
LONG InsertDisk( LONG Drive );

VOID FreeDisplay(void);
LONG SetupDisplay(void);
VOID Iconify(void);

enum{
	MEN_PROJECT=1,
	MEN_PROJECT_ABOUT,
	MEN_PROJECT_LOADSTATE,
	MEN_PROJECT_SAVESTATE,
	MEN_PROJECT_LOADBIN,
	MEN_PROJECT_RECORDSOUND,
	MEN_PROJECT_SOUNDPATH,
	MEN_PROJECT_ICONIFY,
	MEN_PROJECT_HELP,
	MEN_PROJECT_QUIT,

	MEN_SYSTEM,
	MEN_SYSTEM_BOOT,
	MEN_SYSTEM_ID,
	MEN_SYSTEM_ID1,
	MEN_SYSTEM_ID2,
	MEN_SYSTEM_ID3,
	MEN_SYSTEM_ID4,
	MEN_SYSTEM_ID5,
	MEN_SYSTEM_ID6,
	MEN_SYSTEM_ID7,
	MEN_SYSTEM_ID8,
	MEN_SYSTEM_ED,
	MEN_SYSTEM_ED1,
	MEN_SYSTEM_ED2,
	MEN_SYSTEM_ED3,
	MEN_SYSTEM_ED4,
	MEN_SYSTEM_ED5,
	MEN_SYSTEM_ED6,
	MEN_SYSTEM_ED7,
	MEN_SYSTEM_ED8,
	MEN_SYSTEM_IC,
	MEN_SYSTEM_IC8K,
	MEN_SYSTEM_IC16K,
	MEN_SYSTEM_ICOSS,
	MEN_SYSTEM_REMOVEC,
	MEN_SYSTEM_PILL,
	MEN_SYSTEM_ATARI800A,
	MEN_SYSTEM_ATARI800B,
	MEN_SYSTEM_ATARI800XL,
	MEN_SYSTEM_ATARI130XL,
	MEN_SYSTEM_UI,

	MEN_CONSOLE,
	MEN_CONSOLE_RESET,
	MEN_CONSOLE_OPTION,
	MEN_CONSOLE_SELECT,
	MEN_CONSOLE_START,
	MEN_CONSOLE_HELP,
	MEN_CONSOLE_BREAK,
	MEN_CONSOLE_COLDSTART,

	MEN_SETTINGS,
	MEN_SETTINGS_FRAMERATE,
	MEN_SETTINGS_CUSTOMSCREEN,
	MEN_SETTINGS_SAVE,
};

#endif /* _ATARI_AMIGA_H_ */
