#ifndef LINUX
#include <mint/falcon.h>
#include <mint/osbind.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "libatari800.h"

#ifndef LINUX
#define ATARI_WIDTH 320
#define ATARI_HEIGHT 192
static UWORD* atari_screen;

static void debug_screen()
{
	const UWORD *screen = (UWORD *)(libatari800_get_main_memory_ptr() + 0xE100);
	UWORD* p = atari_screen;
	for (int j = 0; j < ATARI_HEIGHT; j++) {
		for (int i = 0; i < ATARI_WIDTH / 16; i++) {
			*p = *screen++;
			p+= 4;
		}
	}
}
#endif


int main(int argc, char **argv) {
	input_template_t input;

	/* force the 400/800 OS to get the Memo Pad */
	char *test_args[] = {
		"tony.xex",
		NULL,
	};
	libatari800_init(-1, test_args);

	libatari800_clear_input_array(&input);

	printf("emulation: fps=%f\n", libatari800_get_fps());

#ifndef LINUX
	atari_screen = (UWORD *)Mxalloc(ATARI_WIDTH * ATARI_HEIGHT * 4 / 8, MX_STRAM);

	VsetMode(PAL | TV | COL40 | BPS4);	// 320x200

	extern int Colours_table[256];
#define Colours_GetR(x) ((UBYTE) (Colours_table[x] >> 16))
#define Colours_GetG(x) ((UBYTE) (Colours_table[x] >> 8))
#define Colours_GetB(x) ((UBYTE) Colours_table[x])
	_RGB background = { .red = Colours_GetR(0x60), .green = Colours_GetG(0x60), .blue = Colours_GetB(0x60) };
	VsetRGB(0, 1, &background);
	_RGB foreground = { .red = Colours_GetR(0x6e), .green = Colours_GetG(0x6e), .blue = Colours_GetB(0x6e) };
	VsetRGB(1, 1, &foreground);

	VsetScreen(SCR_NOCHANGE, atari_screen, SCR_NOCHANGE, SCR_NOCHANGE);
#endif

	for (;;) {
		clock_t beg = clock();
#if 0
		emulator_state_t state;
		cpu_state_t *cpu;
		pc_state_t *pc;

		libatari800_get_current_state(&state);
		cpu = (cpu_state_t *)&state.state[state.tags.cpu];  /* order: A,SR,SP,X,Y */
		pc = (pc_state_t *)&state.state[state.tags.pc];
		printf("frame %d: A=%02x X=%02x Y=%02x SP=%02x SR=%02x PC=%04x\n", libatari800_get_frame_number(), cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S, pc->PC);
#endif
		libatari800_next_frame(&input);
#ifndef LINUX
		debug_screen();
#endif
		clock_t end = clock();

		int vbls = (end - beg) * 50 / (int)CLOCKS_PER_SEC;
		printf("frame took: %d VBLs (%d FPS)\n", vbls, 50 / vbls);
	}

	libatari800_exit();
}
