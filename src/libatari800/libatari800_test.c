#ifndef LINUX
#include <mint/falcon.h>
#include <mint/osbind.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libatari800.h"

#ifndef LINUX
void asm_c2p1x1_8_rect(const UBYTE *pChunky, const UBYTE *pChunkyEnd, ULONG chunkyWidth, ULONG chunkyPitch, UBYTE *pScreen, ULONG screenPitch);

#define ATARI_WIDTH 320
#define ATARI_HEIGHT 192
static UBYTE* atari_screen;

static void debug_screen()
{
	const UBYTE *screen = libatari800_get_screen_ptr();
	const UBYTE *screen_start = screen + ((240 - ATARI_HEIGHT) / 2) * 384 + ((384  - ATARI_WIDTH) / 2);
	const UBYTE *screen_end = screen_start + (ATARI_HEIGHT - 1) * 384 + ATARI_WIDTH;
	asm_c2p1x1_8_rect(screen_start, screen_end, 320, 384, atari_screen, 320);
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
	atari_screen = (UBYTE *)Mxalloc(ATARI_WIDTH * ATARI_HEIGHT, MX_STRAM);

	VsetMode(PAL | TV | COL40 | BPS8);	// 320x200

	extern int Colours_table[256];
#define Colours_GetR(x) ((UBYTE) (Colours_table[x] >> 16))
#define Colours_GetG(x) ((UBYTE) (Colours_table[x] >> 8))
#define Colours_GetB(x) ((UBYTE) Colours_table[x])
	_RGB palette[256];
	for (int i = 0; i < 256; i++) {
		palette[i].red = Colours_GetR(i);
		palette[i].green = Colours_GetG(i);
		palette[i].blue = Colours_GetB(i);
	}
	VsetRGB(0, 256, palette);

	VsetScreen(SCR_NOCHANGE, atari_screen, SCR_NOCHANGE, SCR_NOCHANGE);
#endif

	for (;;) {
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
	}

	libatari800_exit();
}
