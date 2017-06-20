
#include "config.h"

#ifdef DMASOUND
#include <osbind.h>
#include "pokeysnd.h"

static char *rcsid = "$Id: sound_falcon.c,v 1.2 1998/02/21 15:00:23 david Exp $";

#define FALSE 0
#define TRUE 1

extern int get_cookie(long cookie, long *value);

static char *dsp_buffer1, *dsp_endbuf1;
static char *dsp_buffer2, *dsp_endbuf2;

static sound_enabled = FALSE;

static int dsp_sample_rate = 12517;
static int sndbufsize = 600;
static int AUDCTL = 0x00;
static int AUDF[4] =
{0, 0, 0, 0};
static int AUDC[4] =
{0, 0, 0, 0};

/* Atari DMA sound hardware */
extern void timer_A(void);

#ifndef UBYTE
#define UBYTE	unsigned char
#endif

#define	TIMERA		*(long *)0x134

#define	TACTRL		*(UBYTE *)0xFFFA19
#define	TADATA		*(UBYTE *)0xFFFA1F

#define	IEA			*(UBYTE *)0xFFFA07
#define	ISRA		*(UBYTE *)0xFFFA0F
#define	IMA			*(UBYTE *)0xFFFA13
#define IVECTOR		*(UBYTE *)0xFFFA17

long old_timer_A;
UBYTE old_tactrl, old_tadata, old_ivector, old_iea, old_isra, old_ima;
short *DMActrlptr = (short *) 0xff8900;

void Setbuffer(long bufbeg, long bufsize)
{
	long bufend = bufbeg + bufsize;
	DMActrlptr[1] = (bufbeg >> 16) & 0xff;
	DMActrlptr[2] = (bufbeg >> 8) & 0xff;
	DMActrlptr[3] = bufbeg & 0xff;
	DMActrlptr[7] = (bufend >> 16) & 0xff;
	DMActrlptr[8] = (bufend >> 8) & 0xff;
	DMActrlptr[9] = bufend & 0xff;
}

void timer_A_v_C(void)
{
	static int first = FALSE;	/* start computing second buffer */

	if (first) {
		Setbuffer(dsp_buffer1, sndbufsize);		/* set next DMA buffer */
		Pokey_process(dsp_buffer1, sndbufsize);		/* quickly compute it */
		first = FALSE;
	}
	else {
		Setbuffer(dsp_buffer2, sndbufsize);
		Pokey_process(dsp_buffer2, sndbufsize);
		first = TRUE;
	}
}

void MFP_IRQ_on(void)
{
	Setbuffer(dsp_buffer1, sndbufsize);		/* start playing first buffer */
	DMActrlptr[0x10] = 0x80 | 1;	/* mono 12 kHz */
	DMActrlptr[0] = 0x400 | 3;	/* play until stopped, interrupt at end of frame */

	Mfpint(13, timer_A);
	Xbtimer(0, 8, 1, timer_A);	/* event count mode, interrupt after 1st frame */
	IVECTOR &= ~(1 << 3);		/* turn on AEO */
	Jenabint(13);
}

void MFP_IRQ_off(void)
{
	Jdisint(13);
	DMActrlptr[0] = 0;			/* stop playing */
}

void Sound_Initialise(int *argc, char *argv[])
{
	int i, j;

	for (i = j = 1; i < *argc; i++) {
		if (strcmp(argv[i], "-sound") == 0)
			sound_enabled = TRUE;
		else if (strcmp(argv[i], "-nosound") == 0)
			sound_enabled = FALSE;
		else
			argv[j++] = argv[i];
	}

	*argc = j;

	/* test of Sound hardware availability */
	if (sound_enabled) {
		long val;

		if (get_cookie('_SND', &val)) {
			if (!(val & 2))
				sound_enabled = FALSE;	/* Sound DMA hardware is missing */
		}
		else
			sound_enabled = FALSE;	/* CookieJar is missing */
	}

	if (sound_enabled) {
		dsp_buffer1 = (char *) Mxalloc(2 * sndbufsize, 0);
		dsp_buffer2 = dsp_endbuf1 = dsp_buffer1 + sndbufsize;
		dsp_endbuf2 = dsp_buffer2 + sndbufsize;
		memset(dsp_buffer1, 0, sndbufsize);
		memset(dsp_buffer2, 127, sndbufsize);
		if (!dsp_buffer1) {
			printf("can't allocate sound buffer\n");
			exit(1);
		}

		Pokey_sound_init(FREQ_17_EXACT, dsp_sample_rate, 1);
		Supexec(MFP_IRQ_on);
	}
}

void Sound_Exit(void)
{
	if (sound_enabled) {
		Supexec(MFP_IRQ_off);
		Mfree(dsp_buffer1);
	}
}

void Atari_AUDC(int channel, int byte)
{
	channel--;
	Update_pokey_sound(0xd201 + channel + channel, byte, 0, 4);
}

void Atari_AUDF(int channel, int byte)
{
	channel--;
	Update_pokey_sound(0xd200 + channel + channel, byte, 0, 4);
}

void Atari_AUDCTL(int byte)
{
	Update_pokey_sound(0xd208, byte, 0, 4);
}

#else
void Atari_AUDC(int channel, int byte)
{
}

void Atari_AUDF(int channel, int byte)
{
}

void Atari_AUDCTL(int byte)
{
}

#endif
