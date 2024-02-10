#include <stdio.h>
#include <string.h>

#include "libatari800.h"

void asm_c2p1x1_8(const UBYTE *pChunky, const UBYTE *pChunkyEnd, UBYTE *pScreen);

static void debug_screen()
{
	/* print out portion of screen, assuming graphics 0 display list */
	unsigned char *screen = libatari800_get_screen_ptr();
	int x, y;

	asm_c2p1x1_8(screen, screen, screen);

	screen += 384 * 24 + 24;
	for (y = 0; y < 32; y++) {
		for (x = 8; x < 88; x++) {
			unsigned char c = screen[x];
			if (c == 0)
				printf(" ");
			else if (c == 0x94)
				printf(".");
			else if (c == 0x9a)
				printf("X");
			else
				printf("?");
		}
		printf("\n");
		screen += 384;
	}
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

	while (libatari800_get_frame_number() < 200) {
		libatari800_get_current_state(&state);
		cpu = (cpu_state_t *)&state.state[state.tags.cpu];  /* order: A,SR,SP,X,Y */
		pc = (pc_state_t *)&state.state[state.tags.pc];
		printf("frame %d: A=%02x X=%02x Y=%02x SP=%02x SR=%02x PC=%04x\n", libatari800_get_frame_number(), cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S, pc->PC);
		libatari800_next_frame(&input);
		if (libatari800_get_frame_number() > 100) {
			debug_screen();
			input.keychar = 'A';
		}
	}
	libatari800_get_current_state(&state);
	cpu = (cpu_state_t *)&state.state[state.tags.cpu];  /* order: A,SR,SP,X,Y */
	pc = (pc_state_t *)&state.state[state.tags.pc];
	printf("frame %d: A=%02x X=%02x Y=%02x SP=%02x SR=%02x PC=%04x\n", libatari800_get_frame_number(), cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S, pc->PC);

	libatari800_exit();
}
