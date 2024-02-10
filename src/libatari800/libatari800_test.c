#include <mint/falcon.h>
#include <mint/osbind.h>

#include <stdio.h>
#include <string.h>

#include "libatari800.h"

void asm_c2p1x1_8(const UBYTE *pChunky, const UBYTE *pChunkyEnd, UBYTE *pScreen);

static UBYTE* atari_screen;

static void debug_screen()
{
	/* print out portion of screen, assuming graphics 0 display list */
	unsigned char *screen = libatari800_get_screen_ptr();
	asm_c2p1x1_8(screen, screen + 384*240, atari_screen);
}


int main(int argc, char **argv) {
	input_template_t input;

	/* force the 400/800 OS to get the Memo Pad */
	char *test_args[] = {
		"-atari",
		NULL,
	};
	libatari800_init(-1, test_args);

	libatari800_clear_input_array(&input);

	emulator_state_t state;
	cpu_state_t *cpu;
	pc_state_t *pc;

	printf("emulation: fps=%f\n", libatari800_get_fps());

	atari_screen = (UBYTE *)Mxalloc(384 * 240, MX_STRAM);

	VsetMode(OVERSCAN | PAL | TV | COL40 | BPS8);	// 384x240

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

	while (libatari800_get_frame_number() < 200) {
#if 0
		libatari800_get_current_state(&state);
		cpu = (cpu_state_t *)&state.state[state.tags.cpu];  /* order: A,SR,SP,X,Y */
		pc = (pc_state_t *)&state.state[state.tags.pc];
		printf("frame %d: A=%02x X=%02x Y=%02x SP=%02x SR=%02x PC=%04x\n", libatari800_get_frame_number(), cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S, pc->PC);
#endif
		libatari800_next_frame(&input);
		if (libatari800_get_frame_number() > 100) {
			debug_screen();
			input.keychar = 'A';
		}
	}
#if 0
	libatari800_get_current_state(&state);
	cpu = (cpu_state_t *)&state.state[state.tags.cpu];  /* order: A,SR,SP,X,Y */
	pc = (pc_state_t *)&state.state[state.tags.pc];
	printf("frame %d: A=%02x X=%02x Y=%02x SP=%02x SR=%02x PC=%04x\n", libatari800_get_frame_number(), cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S, pc->PC);
#endif

	libatari800_exit();
}
