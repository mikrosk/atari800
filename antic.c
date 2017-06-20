#include <stdio.h>
#include <string.h>

#define LCHOP 3	/* do not build lefmost 0..3 characters in wide mode */
#define RCHOP 3	/* do not build rightmost 0..3 characters in wide mode */

#ifndef AMIGA
#include "config.h"
#endif

#include "atari.h"
#include "rt-config.h"
#include "cpu.h"
#include "gtia.h"
#include "antic.h"
#include "pokey.h"
#include "sio.h"

#define FALSE 0
#define TRUE 1
#define DO_DLI if ((IR & 0x80) && (NMIEN & 0x80)) {NMIST |=0x80;NMI();}

static char *rcsid = "$Id: antic.c,v 1.18 1997/03/22 21:48:27 david Exp $";

UBYTE CHACTL;
UBYTE CHBASE;
UBYTE DLISTH;
UBYTE DLISTL;
UBYTE DMACTL;
UBYTE HSCROL;
UBYTE NMIEN;
UBYTE NMIST;
UBYTE PMBASE;
UBYTE VSCROL;

/*
 * These are defined for Word (2 Byte) memory accesses. I have not
 * defined Longword (4 Byte) values since the Sparc architecture
 * requires that Longword are on a longword boundry.
 *
 * Words accesses don't appear to be a problem because the first
 * pixel plotted on a line will always be at an even offset, and
 * hence on a word boundry.
 *
 * Note: HSCROL is in colour clocks whereas the pixels are emulated
 *       down to a half colour clock - the first pixel plotted is
 *       moved across by 2*HSCROL
 */

#define PF2_COLPF0 0x0404
#define PF2_COLPF1 0x0505
#define PF2_COLPF2 0x0606
#define PF2_COLPF3 0x0707
#define PF2_COLBK 0x0808

#define PF4_COLPF0 0x04040404
#define PF4_COLPF1 0x05050505
#define PF4_COLPF2 0x06060606
#define PF4_COLPF3 0x07070707
#define PF4_COLBK 0x08080808

int ypos;

#define CPUL	114				/* 114 CPU cycles for 1 screenline */
#define DMAR	9				/* 9 cycles for DMA refresh */
#define WSYNC	7				/* 7 cycles for wsync */
#define BEGL	15				/* 15 cycles for begin of each screenline */

/* Table of Antic "beglinecycles" from real atari */
/* middle of sceenline */
static realcyc[128] =
{58, 58, 58, 0, 58, 58, 58, 0,	/* blank lines */
 58, 58, 58, 0, 58, 58, 58, 0,	/* --- */
 27, 23, 15, 0, 41, 37, 31, 0,	/* antic2, gr.0 */
 27, 23, 15, 0, 41, 37, 31, 0,	/* antic3, gr.0(8x10) */
 27, 23, 15, 0, 41, 37, 31, 0,	/* antic4, gr.12 */
 27, 23, 15, 0, 41, 37, 31, 0,	/* antic5, gr.13 */
 40, 36, 32, 0, 49, 47, 45, 0,	/* antic6, gr.1 */
 40, 36, 32, 0, 49, 47, 45, 0,	/* antic7, gr.2 */

 53, 52, 51, 0, 58, 58, 58, 0,	/* antic8, gr.3 */
 53, 52, 51, 0, 58, 58, 58, 0,	/* antic9, gr.4 */
 49, 47, 45, 0, 58, 58, 58, 0,	/* antic10, gr.5 */
 49, 47, 45, 0, 58, 58, 58, 0,	/* antic11, gr.6 */
 49, 47, 45, 0, 58, 58, 58, 0,	/* antic12, gr.14 */
 40, 36, 32, 0, 58, 58, 58, 0,	/* antic13, gr.7 */
 40, 36, 32, 0, 58, 58, 58, 0,	/* antic14, gr.15 */
 40, 36, 32, 0, 58, 58, 58, 0	/* antic15, gr.8 */
};

/* Table of nlines for Antic modes */
static an_nline[16] =
{1, 1, 8, 10, 8, 16, 8, 16,
 8, 4, 4, 2, 1, 2, 1, 1};

/* DLI on first screenline */
#define DLI_FIRSTLINE	if (dlisc==0) { allc -= 8; NMIST |= 0x80; NMI(); } unc = GO(allc + unc);

int dmaw;						/* DMA width = 4,5,6 */

/* have made these globals for now:PM */
int unc;						/* unused cycles (can be < 0 !) */
int dlisc;						/* screenline with horizontal blank interrupt */

int firstlinecycles;			/* DMA cycles for first line */
int nextlinecycles;				/* DMA cycles for next lines */
int pmgdmac;					/* DMA cycles for PMG */
int dldmac;						/* DMA cycles for read display-list */

int allc;						/* temporary (sum of cycles) */
int begcyc;						/* temporary (cycles for begin of screenline) */
int anticmode;					/* temporary (Antic mode) */
int anticm8;					/* temporary (anticmode<<3) */

/*
 * Pre-computed values for improved performance
 */

static UWORD chbase_40;			/* CHBASE for 40 character mode */
static UWORD chbase_20;			/* CHBASE for 20 character mode */
static int scroll_offset;

static UBYTE singleline;
UBYTE player_dma_enabled;
UBYTE player_gra_enabled;
UBYTE missile_dma_enabled;
UBYTE missile_gra_enabled;
UBYTE player_flickering;
UBYTE missile_flickering;

static UWORD maddr_s;			/* Address of Missiles - Single Line Resolution */
static UWORD p0addr_s;			/* Address of Player0 - Single Line Resolution */
static UWORD p1addr_s;			/* Address of Player1 - Single Line Resolution */
static UWORD p2addr_s;			/* Address of Player2 - Single Line Resolution */
static UWORD p3addr_s;			/* Address of Player3 - Single Line Resolution */
static UWORD maddr_d;			/* Address of Missiles - Double Line Resolution */
static UWORD p0addr_d;			/* Address of Player0 - Double Line Resolution */
static UWORD p1addr_d;			/* Address of Player1 - Double Line Resolution */
static UWORD p2addr_d;			/* Address of Player2 - Double Line Resolution */
static UWORD p3addr_d;			/* Address of Player3 - Double Line Resolution */
static UBYTE IR;				/* made a global */

static int foobar;
static int hscrol_flag;
static int chars_read[6];
static int chars_displayed[6];
static int x_min[6];
static int ch_offset[6];
#define NORMAL0 0
#define NORMAL1 1
#define NORMAL2 2
#define SCROLL0 3
#define SCROLL1 4
#define SCROLL2 5
static int base_scroll_char_offset;
static int base_scroll_char_offset2;
static int base_scroll_char_offset3;
static int mode_type;

static int left_border_chars;
static int right_border_chars;
static int right_border_start;
extern UBYTE pm_scanline[ATARI_WIDTH];
extern UBYTE pf_colls[9];
extern int DELAYED_SERIN_IRQ;
extern int DELAYED_SEROUT_IRQ;
extern int DELAYED_XMTDONE_IRQ;

static int normal_lastline;
int wsync_halt = 0;
int antic23f = 0;
int pmg_dma(void);
#define L_PM0 (0*5-4)
#define L_PM1 (1*5-4)
#define L_PM01 (2*5-4)
#define L_PM2 (3*5-4)
#define L_PM3 (4*5-4)
#define L_PM23 (5*5-4)
#define L_PMNONE (6*5-4)
int new_pm_lookup[16] =
{
	L_PMNONE,					/* 0000 - None */
	L_PM0,						/* 0001 - Player 0 */
	L_PM1,						/* 0010 - Player 1 */
	L_PM0,
/* 0011 - Player 0 *//**0OR1   3 */
	L_PM2,						/* 0100 - Player 2 */
	L_PM0,						/* 0101 - Player 0 */
	L_PM1,						/* 0110 - Player 1 */
	L_PM0,
/* 0111 - Player 0 *//**0OR1   7 */
	L_PM3,						/* 1000 - Player 3 */
	L_PM0,						/* 1001 - Player 0 */
	L_PM1,						/* 1010 - Player 1 */
	L_PM0,
/* 1011 - Player 0 *//**0OR1   11 */
	L_PM2,
/* 1100 - Player 2 *//**2OR3   12 */
	L_PM0,						/* 1101 - Player 0 */
	L_PM1,						/* 1110 - Player 1 */
	L_PM0,						/* 1111 - Player 0 */
};
static UBYTE playfield_lookup[256];		/* what size should this be to be fastest? */

/*
   =============================================================
   Define screen as ULONG to ensure that it is Longword aligned.
   This allows special optimisations under certain conditions.
   -------------------------------------------------------------
   The extra 16 scanlines is used as an overflow buffer and has
   enough room for any extra mode line. It is needed on the
   occasions that a valid JVB instruction is not found in the
   display list - An automatic break out will occur when ypos
   is greater than the ATARI_HEIGHT, if its one less than this
   there must be enough space for another mode line.
   =============================================================
 */

ULONG *atari_screen = NULL;
UBYTE *scrn_ptr;
UBYTE prior_table[35 * 16];
UBYTE cur_prior[35];
UBYTE p5_mask;
#define R_BLACK 23
#define R_COLPM0OR1 9
#define R_COLPM2OR3 10
#define R_COLPM0_OR_PF0 11
#define R_COLPM1_OR_PF0 12
#define R_COLPM0OR1_OR_PF0 13
#define R_COLPM0_OR_PF1 14
#define R_COLPM1_OR_PF1 15
#define R_COLPM0OR1_OR_PF1 16
#define R_COLPM2_OR_PF2 17
#define R_COLPM3_OR_PF2 18
#define R_COLPM2OR3_OR_PF2 19
#define R_COLPM2_OR_PF3 20
#define R_COLPM3_OR_PF3 21
#define R_COLPM2OR3_OR_PF3 22
#define R_BLACK 23
UWORD cl_word[24];

void initialize_prior_table()
{
	UBYTE player_colreg[] =
	{0, 1, 9, 2, 3, 10, 0};
	int i, j, k;
	for (i = 0; i <= 15; i++) {	/* prior setting */

		for (j = 0; j <= 6; j++) {	/* player */

			for (k = 0; k <= 4; k++) {	/* playfield */

				int c;			/* colreg */

				if (j == 6)
					c = k + 4;	/* no player */

				else if (k == 4)
					c = player_colreg[j];
				else {
					if (k <= 1) {	/* playfields 0 and 1 */

						if (j <= 2) {	/* players01 and 2=0+1 */

							switch (i) {
							case 0:
								if (k == 0) {
									if (j == 0)
										c = R_COLPM0_OR_PF0;
									if (j == 1)
										c = R_COLPM1_OR_PF0;
									if (j == 2)
										c = R_COLPM0OR1_OR_PF0;
								}
								else {
									if (j == 0)
										c = R_COLPM0_OR_PF1;
									if (j == 1)
										c = R_COLPM1_OR_PF1;
									if (j == 2)
										c = R_COLPM0OR1_OR_PF1;
								}
								break;
							case 1:
							case 2:
							case 3:
								c = player_colreg[j];
								break;
							case 4:
							case 8:
							case 12:
								c = k + 4;
								break;
							default:
								c = R_BLACK;
								break;
							}
						}
						else {	/* pf01, players23 and 2OR3 */

							if ((i & 0x01) == 0) {
								c = k + 4;
							}
							else {
								c = player_colreg[j];
							}
						}
					}
					else {		/* playfields 2 and 3 */

						if (j <= 2) {	/* players 01 and 0OR1 */

							if ((i <= 3) || (i >= 8 && i <= 11)) {
								c = player_colreg[j];
							}
							else {
								c = k + 4;
							}
						}
						else {	/* pf23, players23 and 2OR3 */

							switch (i) {
							case 0:
								if (k == 2) {
									if (j == 3)
										c = R_COLPM2_OR_PF2;
									if (j == 4)
										c = R_COLPM3_OR_PF2;
									if (j == 5)
										c = R_COLPM2OR3_OR_PF2;
								}
								else {
									if (j == 3)
										c = R_COLPM2_OR_PF3;
									if (j == 4)
										c = R_COLPM3_OR_PF3;
									if (j == 5)
										c = R_COLPM2OR3_OR_PF3;
								}
								break;
							case 1:
							case 8:
							case 9:
								c = player_colreg[j];
								break;
							case 2:
							case 4:
							case 6:
								c = k + 4;
								break;
							default:
								c = R_BLACK;
								break;
							}
						}
					}
				}
				prior_table[i * 35 + j * 5 + k] = c;
			}
		}
	}
}
void ANTIC_Initialise(int *argc, char *argv[])
{
	int i;
	int j;

	playfield_lookup[0x00] = 8;
	playfield_lookup[0x40] = 4;
	playfield_lookup[0x80] = 5;
	playfield_lookup[0xc0] = 6;
	initialize_prior_table();
	GTIA_PutByte(_PRIOR, 0xff);
	GTIA_PutByte(_PRIOR, 0x00);

	player_dma_enabled = missile_dma_enabled = 0;
	player_gra_enabled = missile_gra_enabled = 0;
	player_flickering = missile_flickering = 0;
	GRAFP0 = GRAFP1 = GRAFP2 = GRAFP3 = GRAFM = 0;
}

/*
   *****************************************************************
   *                                *
   *    Section         :   Antic Display Modes *
   *    Original Author     :   David Firth     *
   *    Date Written        :   28th May 1995       *
   *    Version         :   1.0         *
   *                                *
   *                                *
   *   Description                          *
   *   -----------                          *
   *                                *
   *   Section that handles Antic display modes. Not required   *
   *   for BASIC version.                       *
   *                                *
   *****************************************************************
 */

int xmin;
int xmax;

int dmactl_xmin_noscroll;
int dmactl_xmax_noscroll;
static int dmactl_xmin_scroll;
static int dmactl_xmax_scroll;
static int char_delta;
static int flip_mask;
static int char_offset;
static int invert_mask;
static int blank_mask;

static UWORD screenaddr;
static UWORD lookup1[256];
static UWORD lookup2[256];
static ULONG lookup_gtia[16];

static int vskipbefore = 0;
static int vskipafter = 0;

#define DO_PMG if(pm_pixel){\
	       pf_colls[colreg]|=pm_pixel;\
	       if (pm_pixel&p5_mask) colreg=7;\
               else pm_pixel=pm_pixel|(pm_pixel>>4);\
               colreg=cur_prior[new_pm_lookup[pm_pixel&0x0f]+colreg];\
              }
void do_border(void)
{
	int kk;
	int temp_border_chars;
	int pass;
	UWORD *ptr = (UWORD *) & scrn_ptr[LCHOP * 8];
	ULONG *t_pm_scanline_ptr = (ULONG *) (&pm_scanline[LCHOP*4]);
	ULONG COL_8_LONG;

	COL_8_LONG = cl_word[8] | (cl_word[8] << 16);
	temp_border_chars = left_border_chars;
	pass = 2;
	while (pass--) {
		for (kk = temp_border_chars; kk; kk--) {
			if (!(*t_pm_scanline_ptr)) {
				ULONG *l_ptr = (ULONG *) ptr;

				*l_ptr++ = COL_8_LONG;	/* PF4_COLPF2; */

				*l_ptr++ = COL_8_LONG;	/* PF4_COLPF2; */

				ptr = (UWORD *) l_ptr;
			}
			else {
				UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
				UBYTE pm_pixel;
				int which_player, which_missile;
				UBYTE colreg;
				int k;
				for (k = 1; k <= 4; k++) {
					pm_pixel = *c_pm_scanline_ptr++;
					colreg = 8;
					DO_PMG
						* ptr++ = cl_word[colreg];
				}
			}
			t_pm_scanline_ptr++;
		}
		ptr = (UWORD *) & scrn_ptr[right_border_start];
		t_pm_scanline_ptr = (ULONG *) (&pm_scanline[(right_border_start >> 1)]);
		temp_border_chars = right_border_chars;
	}
}

void draw_antic_2(int j, int nchars, UWORD t_screenaddr, char *ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	ULONG COL_6_LONG;
	int i;
	lookup1[0x00] = colour_lookup[6];
	lookup1[0x80] = lookup1[0x40] = lookup1[0x20] =
		lookup1[0x10] = lookup1[0x08] = lookup1[0x04] =
		lookup1[0x02] = lookup1[0x01] = (colour_lookup[6] & 0xf0) | (colour_lookup[5] & 0x0f);
	COL_6_LONG = cl_word[6] | (cl_word[6] << 16);
	for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		UWORD chaddr;
		UBYTE invert;
		UBYTE blank;
		UBYTE chdata;

		chaddr = t_chbase + ((UWORD) (screendata & 0x7f) << 3);
		if (screendata & invert_mask)
			invert = 0xff;
		else
			invert = 0x00;
		if (screendata & blank_mask)
			blank = 0x00;
		else
			blank = 0xff;
		if ((j & 0x0e) == 0x08 && (screendata & 0x60) != 0x60)
			chdata = invert & blank;
		else
			chdata = (memory[chaddr] ^ invert) & blank;
		if (!(*t_pm_scanline_ptr)) {
			if (chdata) {
				*ptr++ = lookup1[chdata & 0x80];
				*ptr++ = lookup1[chdata & 0x40];
				*ptr++ = lookup1[chdata & 0x20];
				*ptr++ = lookup1[chdata & 0x10];
				*ptr++ = lookup1[chdata & 0x08];
				*ptr++ = lookup1[chdata & 0x04];
				*ptr++ = lookup1[chdata & 0x02];
				*ptr++ = lookup1[chdata & 0x01];
			}
			else {
#ifdef UNALIGNED_LONG_OK
				ULONG *l_ptr = (ULONG *) ptr;

				*l_ptr++ = COL_6_LONG;
				*l_ptr++ = COL_6_LONG;

				ptr = (UBYTE *) l_ptr;
#else
				UWORD *w_ptr = (UWORD *) ptr;

				*w_ptr++ = PF2_COLPF2;
				*w_ptr++ = PF2_COLPF2;
				*w_ptr++ = PF2_COLPF2;
				*w_ptr++ = PF2_COLPF2;

				ptr = (UBYTE *) w_ptr;
#endif
			}
		}
		else {
			UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
			UBYTE pm_pixel;
			int which_player, which_missile;
			UBYTE colreg;
			int k;
			for (k = 1; k <= 4; k++) {
				pm_pixel = *c_pm_scanline_ptr++;
				colreg = 6;
				DO_PMG
					if (chdata & 0x80)
					*ptr++ = (cl_word[colreg] & 0xf0) | (COLPF1 & 0x0f);
				else
					*ptr++ = (UBYTE) ((cl_word[colreg] & 0xff));
				if (chdata & 0x40)
					*ptr++ = (cl_word[colreg] & 0xf0) | (COLPF1 & 0x0f);
				else
					*ptr++ = (UBYTE) ((cl_word[colreg] & 0xff));
				chdata <<= 2;
			}

		}
		t_pm_scanline_ptr++;
	}
}
void draw_antic_2_gtia9_11(int j, int nchars, UWORD t_screenaddr, char *t_ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	int i;
	ULONG *ptr = (ULONG *) (t_ptr);
	ULONG temp_count = 0;
	ULONG base_colour = colour_lookup[8];
	ULONG increment;
	base_colour = base_colour | (base_colour << 8);
	base_colour = base_colour | (base_colour << 16);
	if ((PRIOR & 0xC0) == 0x40)
		increment = 0x01010101;
	else
		increment = 0x10101010;
	for (i = 0; i <= 15; i++) {
		lookup_gtia[i] = base_colour | temp_count;
		temp_count += increment;
	}

	for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		UWORD chaddr;
		UBYTE invert;
		UBYTE blank;
		UBYTE chdata;

		chaddr = t_chbase + ((UWORD) (screendata & 0x7f) << 3);
		if (screendata & invert_mask)
			invert = 0xff;
		else
			invert = 0x00;
		if (screendata & blank_mask)
			blank = 0x00;
		else
			blank = 0xff;
		if ((j & 0x0e) == 0x08 && (screendata & 0x60) != 0x60)
			chdata = invert & blank;
		else
			chdata = (memory[chaddr] ^ invert) & blank;
		*ptr++ = lookup_gtia[chdata >> 4];
		*ptr++ = lookup_gtia[chdata & 0x0f];
		if ((*t_pm_scanline_ptr)) {
			UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
			UBYTE pm_pixel;
			int which_player, which_missile;
			UBYTE colreg;
			int k;
			UWORD *w_ptr = (UWORD *) (ptr - 2);
			for (k = 1; k <= 4; k++) {
				pm_pixel = *c_pm_scanline_ptr++;
				colreg = 8;
				if (pm_pixel) {
					pf_colls[colreg] |= pm_pixel;
					if (pm_pixel & p5_mask)
						colreg = 7;
					else
						pm_pixel = pm_pixel | (pm_pixel >> 4);
					colreg = cur_prior[new_pm_lookup[pm_pixel & 0x0f] + colreg];
					*w_ptr = cl_word[colreg];
				}
				w_ptr++;
			}
		}
		t_pm_scanline_ptr++;
	}
}
void draw_antic_3(int j, int nchars, UWORD t_screenaddr, char *ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	ULONG COL_6_LONG;
	int i;
	lookup1[0x00] = colour_lookup[6];
	lookup1[0x80] = lookup1[0x40] = lookup1[0x20] =
		lookup1[0x10] = lookup1[0x08] = lookup1[0x04] =
		lookup1[0x02] = lookup1[0x01] = (colour_lookup[6] & 0xf0) | (colour_lookup[5] & 0x0f);
	COL_6_LONG = cl_word[6] | (cl_word[6] << 16);
	for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		UWORD chaddr;
		UBYTE invert;
		UBYTE blank;
		UBYTE chdata;

		chaddr = t_chbase + ((UWORD) (screendata & 0x7f) << 3);
		if (screendata & invert_mask)
			invert = 0xff;
		else
			invert = 0x00;
		if (screendata & blank_mask)
			blank = 0x00;
		else
			blank = 0xff;
/* only need to change this line from antic_2 vvvvvvvvv */
		/*     if((j&0x0e)==0x08 && (screendata&0x60)!=0x60) */
		if ((((screendata & 0x60) == 0x60) && ((j & 0x0e) == 0x00)) || (((screendata & 0x60) != 0x60) && ((j & 0x0e) == 0x08)))
			chdata = invert & blank;
		else
			chdata = (memory[chaddr] ^ invert) & blank;
		if (!(*t_pm_scanline_ptr)) {
			if (chdata) {
				*ptr++ = lookup1[chdata & 0x80];
				*ptr++ = lookup1[chdata & 0x40];
				*ptr++ = lookup1[chdata & 0x20];
				*ptr++ = lookup1[chdata & 0x10];
				*ptr++ = lookup1[chdata & 0x08];
				*ptr++ = lookup1[chdata & 0x04];
				*ptr++ = lookup1[chdata & 0x02];
				*ptr++ = lookup1[chdata & 0x01];
			}
			else {
#ifdef UNALIGNED_LONG_OK
				ULONG *l_ptr = (ULONG *) ptr;

				*l_ptr++ = COL_6_LONG;
				*l_ptr++ = COL_6_LONG;

				ptr = (UBYTE *) l_ptr;
#else
				UWORD *w_ptr = (UWORD *) ptr;

				*w_ptr++ = PF2_COLPF2;
				*w_ptr++ = PF2_COLPF2;
				*w_ptr++ = PF2_COLPF2;
				*w_ptr++ = PF2_COLPF2;

				ptr = (UBYTE *) w_ptr;
#endif
			}
		}
		else {
			UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
			UBYTE pm_pixel;
			int which_player, which_missile;
			UBYTE colreg;
			int k;
			for (k = 1; k <= 4; k++) {
				pm_pixel = *c_pm_scanline_ptr++;
				colreg = 6;
				DO_PMG
					if (chdata & 0x80)
					*ptr++ = (cl_word[colreg] & 0xf0) | (COLPF1 & 0x0f);
				else
					*ptr++ = (UBYTE) ((cl_word[colreg] & 0xff));
				if (chdata & 0x40)
					*ptr++ = (cl_word[colreg] & 0xf0) | (COLPF1 & 0x0f);
				else
					*ptr++ = (UBYTE) ((cl_word[colreg] & 0xff));
				chdata <<= 2;
			}

		}
		t_pm_scanline_ptr++;
	}
}

void draw_antic_4(int j, int nchars, UWORD t_screenaddr, char *t_ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	int i;
	UWORD *ptr = (UWORD *) (t_ptr);
#define LOOKUP1_4COL  lookup1[0x00] = cl_word[8];\
  lookup1[0x40] = lookup1[0x10] = lookup1[0x04] = lookup1[0x01] = cl_word[4];\
  lookup1[0x80] = lookup1[0x20] = lookup1[0x08] = lookup1[0x02] = cl_word[5];\
  lookup1[0xc0] = lookup1[0x30] = lookup1[0x0c] = lookup1[0x03] = cl_word[6];
	LOOKUP1_4COL
/*
   ==================================
   Pixel values when character >= 128
   ==================================
 */
		lookup2[0x00] = cl_word[8];
	lookup2[0x40] = lookup2[0x10] = lookup2[0x04] = lookup2[0x01] = cl_word[4];
	lookup2[0x80] = lookup2[0x20] = lookup2[0x08] = lookup2[0x02] = cl_word[5];
	lookup2[0xc0] = lookup2[0x30] = lookup2[0x0c] = lookup2[0x03] = cl_word[7];

	for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		UWORD chaddr;
		UWORD *lookup;
		UBYTE chdata;
		chaddr = t_chbase + ((UWORD) (screendata & 0x7f) << 3);
		if (screendata & 0x80)
			lookup = lookup2;
		else
			lookup = lookup1;
		chdata = memory[chaddr];
		if (!(*t_pm_scanline_ptr)) {
			if (chdata) {
				UWORD colour;

				colour = lookup[chdata & 0xc0];
				*ptr++ = colour;

				colour = lookup[chdata & 0x30];
				*ptr++ = colour;

				colour = lookup[chdata & 0x0c];
				*ptr++ = colour;

				colour = lookup[chdata & 0x03];
				*ptr++ = colour;
			}
			else {
				*ptr++ = lookup1[0];
				*ptr++ = lookup1[0];
				*ptr++ = lookup1[0];
				*ptr++ = lookup1[0];
			}
		}
		else {
			UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
			UBYTE pm_pixel;
			int which_player, which_missile;
			UBYTE colreg;
			int k;
			for (k = 1; k <= 4; k++) {
				pm_pixel = *c_pm_scanline_ptr++;
				colreg = playfield_lookup[chdata & 0xc0];
				if ((screendata & 0x80) && (colreg == 6))
					colreg = 7;
				DO_PMG
					* ptr++ = cl_word[colreg];
				chdata <<= 2;
			}

		}
		t_pm_scanline_ptr++;
	}
}

void draw_antic_6(int j, int nchars, UWORD t_screenaddr, char *t_ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	int i;
	UWORD *ptr = (UWORD *) (t_ptr);
	for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		UWORD chaddr;
		UBYTE chdata;
		UWORD colour;
		int k, kk;
		chaddr = t_chbase + ((UWORD) (screendata & 0x3f) << 3);
		switch (screendata & 0xc0) {
		case 0x00:
			colour = cl_word[4];
			break;
		case 0x40:
			colour = cl_word[5];
			break;
		case 0x80:
			colour = cl_word[6];
			break;
		case 0xc0:
			colour = cl_word[7];
			break;
		}
		chdata = memory[chaddr];
		for (kk = 0; kk < 2; kk++) {
			if (!(*t_pm_scanline_ptr)) {
				if (chdata & 0xf0) {
					if (chdata & 0x80)
						*ptr++ = colour;
					else
						*ptr++ = cl_word[8];
					if (chdata & 0x40)
						*ptr++ = colour;
					else
						*ptr++ = cl_word[8];
					if (chdata & 0x20)
						*ptr++ = colour;
					else
						*ptr++ = cl_word[8];
					if (chdata & 0x10)
						*ptr++ = colour;
					else
						*ptr++ = cl_word[8];
				}
				else {
					*ptr++ = cl_word[8];
					*ptr++ = cl_word[8];
					*ptr++ = cl_word[8];
					*ptr++ = cl_word[8];
				}
				chdata = chdata << 4;
			}
			else {
				UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
				UBYTE pm_pixel;
				int which_player, which_missile;
				UBYTE colreg;
				int k;
				for (k = 1; k <= 4; k++) {
					pm_pixel = *c_pm_scanline_ptr++;
					colreg = (chdata & 0x80) ? ((screendata & 0xc0) >> 6) + 4 : 8;

					DO_PMG
						* ptr++ = cl_word[colreg];
					chdata <<= 1;
				}

			}
			t_pm_scanline_ptr++;
		}
	}
}
void draw_antic_8(int j, int nchars, UWORD t_screenaddr, char *t_ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	int i;
	UWORD *ptr = (UWORD *) (t_ptr);
	LOOKUP1_4COL
		for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		int k, kk;
		for (kk = 0; kk < 4; kk++) {
			if (!(*t_pm_scanline_ptr)) {
				*ptr++ = lookup1[screendata & 0xC0];
				*ptr++ = lookup1[screendata & 0xC0];
				*ptr++ = lookup1[screendata & 0xC0];
				*ptr++ = lookup1[screendata & 0xC0];
				screendata <<= 2;
			}
			else {
				UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
				UBYTE pm_pixel;
				int which_player, which_missile;
				UBYTE colreg;
				int k;
				for (k = 0; k <= 3; k++) {
					pm_pixel = *c_pm_scanline_ptr++;
					colreg = playfield_lookup[screendata & 0xc0];
					DO_PMG
						* ptr++ = cl_word[colreg];
				}
				screendata <<= 2;
			}
			t_pm_scanline_ptr++;
		}
	}
}
void draw_antic_9(int j, int nchars, UWORD t_screenaddr, char *t_ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	int i;
	UWORD *ptr = (UWORD *) (t_ptr);
	lookup1[0x00] = cl_word[8];
	lookup1[0x80] = lookup1[0x40] = lookup1[0x20] = lookup1[0x10] = cl_word[4];
	for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		int k, kk;
		for (kk = 0; kk < 4; kk++) {
			if (!(*t_pm_scanline_ptr)) {
				*ptr++ = lookup1[screendata & 0x80];
				*ptr++ = lookup1[screendata & 0x80];
				*ptr++ = lookup1[screendata & 0x40];
				*ptr++ = lookup1[screendata & 0x40];
				screendata <<= 2;
			}
			else {
				UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
				UBYTE pm_pixel;
				int which_player, which_missile;
				UBYTE colreg;
				int k;
				for (k = 0; k <= 3; k++) {
					pm_pixel = *c_pm_scanline_ptr++;
					colreg = (screendata & 0x80) ? 4 : 8;

					DO_PMG
						* ptr++ = cl_word[colreg];
					if (k & 0x01)
						screendata <<= 1;
				}
			}
			t_pm_scanline_ptr++;
		}
	}
}

void draw_antic_a(int j, int nchars, UWORD t_screenaddr, char *t_ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	int i;
	UWORD *ptr = (UWORD *) (t_ptr);
	LOOKUP1_4COL
		for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		int k, kk;
		for (kk = 0; kk < 2; kk++) {
			if (!(*t_pm_scanline_ptr)) {
				*ptr++ = lookup1[screendata & 0xC0];
				*ptr++ = lookup1[screendata & 0xC0];
				*ptr++ = lookup1[screendata & 0x30];
				*ptr++ = lookup1[screendata & 0x30];
				screendata <<= 4;
			}
			else {
				UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
				UBYTE pm_pixel;
				int which_player, which_missile;
				UBYTE colreg;
				int k;
				for (k = 0; k <= 3; k++) {
					pm_pixel = *c_pm_scanline_ptr++;
					colreg = playfield_lookup[screendata & 0xc0];
					DO_PMG
						* ptr++ = cl_word[colreg];
					if (k & 0x01)
						screendata <<= 2;
				}
			}
			t_pm_scanline_ptr++;
		}
	}
}

void draw_antic_c(int j, int nchars, UWORD t_screenaddr, char *t_ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	int i;
	UWORD *ptr = (UWORD *) (t_ptr);
	lookup1[0x00] = cl_word[8];
	lookup1[0x80] = lookup1[0x40] = lookup1[0x20] = lookup1[0x10] = cl_word[4];
	for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		int k, kk;
		for (kk = 0; kk < 2; kk++) {
			if (!(*t_pm_scanline_ptr)) {
				*ptr++ = lookup1[screendata & 0x80];
				*ptr++ = lookup1[screendata & 0x40];
				*ptr++ = lookup1[screendata & 0x20];
				*ptr++ = lookup1[screendata & 0x10];
				screendata <<= 4;
			}
			else {
				UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
				UBYTE pm_pixel;
				int which_player, which_missile;
				UBYTE colreg;
				int k;
				for (k = 1; k <= 4; k++) {
					pm_pixel = *c_pm_scanline_ptr++;
					colreg = (screendata & 0x80) ? 4 : 8;
					DO_PMG
						* ptr++ = cl_word[colreg];
					screendata <<= 1;
				}
			}
			t_pm_scanline_ptr++;
		}
	}
}

void draw_antic_e(int j, int nchars, UWORD t_screenaddr, char *t_ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	int i;
	UWORD *ptr = (UWORD *) (t_ptr);
#ifdef UNALIGNED_LONG_OK
	int background;
#endif
	LOOKUP1_4COL
#ifdef UNALIGNED_LONG_OK
		background = (lookup1[0x00] << 16) | lookup1[0x00];
#endif

	for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		if (!(*t_pm_scanline_ptr)) {
			if (screendata) {
				*ptr++ = lookup1[screendata & 0xc0];
				*ptr++ = lookup1[screendata & 0x30];
				*ptr++ = lookup1[screendata & 0x0c];
				*ptr++ = lookup1[screendata & 0x03];
			}
			else {
#ifdef UNALIGNED_LONG_OK
				ULONG *l_ptr = (ULONG *) ptr;

				*l_ptr++ = background;
				*l_ptr++ = background;

				ptr = (UWORD *) l_ptr;
#else
				*ptr++ = lookup1[0x00];
				*ptr++ = lookup1[0x00];
				*ptr++ = lookup1[0x00];
				*ptr++ = lookup1[0x00];
#endif
			}
		}
		else {
			UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
			UBYTE pm_pixel;
			int which_player, which_missile;
			UBYTE colreg;
			int k;
			for (k = 1; k <= 4; k++) {
				pm_pixel = *c_pm_scanline_ptr++;
				colreg = playfield_lookup[screendata & 0xc0];
				DO_PMG
					* ptr++ = cl_word[colreg];
				screendata <<= 2;
			}

		}
		t_pm_scanline_ptr++;
	}
}
void draw_antic_f(int j, int nchars, UWORD t_screenaddr, char *ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	ULONG COL_6_LONG;
	int i;
	lookup1[0x00] = colour_lookup[6];
	lookup1[0x80] = lookup1[0x40] = lookup1[0x20] =
		lookup1[0x10] = lookup1[0x08] = lookup1[0x04] =
		lookup1[0x02] = lookup1[0x01] = (colour_lookup[6] & 0xf0) | (colour_lookup[5] & 0x0f);
	COL_6_LONG = cl_word[6] | (cl_word[6] << 16);
	for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		if (!(*t_pm_scanline_ptr)) {
			if (screendata) {
				*ptr++ = lookup1[screendata & 0x80];
				*ptr++ = lookup1[screendata & 0x40];
				*ptr++ = lookup1[screendata & 0x20];
				*ptr++ = lookup1[screendata & 0x10];
				*ptr++ = lookup1[screendata & 0x08];
				*ptr++ = lookup1[screendata & 0x04];
				*ptr++ = lookup1[screendata & 0x02];
				*ptr++ = lookup1[screendata & 0x01];
			}
			else {
#ifdef UNALIGNED_LONG_OK
				ULONG *l_ptr = (ULONG *) ptr;

				*l_ptr++ = COL_6_LONG;
				*l_ptr++ = COL_6_LONG;

				ptr = (UBYTE *) l_ptr;
#else
				UWORD *w_ptr = (UWORD *) ptr;

				*w_ptr++ = PF2_COLPF2;
				*w_ptr++ = PF2_COLPF2;
				*w_ptr++ = PF2_COLPF2;
				*w_ptr++ = PF2_COLPF2;

				ptr = (UBYTE *) w_ptr;
#endif
			}
		}
		else {
			UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
			UBYTE pm_pixel;
			int which_player, which_missile;
			UBYTE colreg;
			int k;
			for (k = 1; k <= 4; k++) {
				pm_pixel = *c_pm_scanline_ptr++;
				colreg = 6;
				DO_PMG
					if (screendata & 0x80)
					*ptr++ = (cl_word[colreg] & 0xf0) | (COLPF1 & 0x0f);
				else
					*ptr++ = (UBYTE) ((cl_word[colreg] & 0xff));
				if (screendata & 0x40)
					*ptr++ = (cl_word[colreg] & 0xf0) | (COLPF1 & 0x0f);
				else
					*ptr++ = (UBYTE) ((cl_word[colreg] & 0xff));
				screendata <<= 2;
			}
		}
		t_pm_scanline_ptr++;
	}
}
void draw_antic_f_gtia9_11(int j, int nchars, UWORD t_screenaddr, char *t_ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	int i;
	ULONG *ptr = (ULONG *) (t_ptr);
	ULONG temp_count = 0;
	ULONG base_colour = colour_lookup[8];
	ULONG increment;
	base_colour = base_colour | (base_colour << 8);
	base_colour = base_colour | (base_colour << 16);
	if ((PRIOR & 0xC0) == 0x40)
		increment = 0x01010101;
	else
		increment = 0x10101010;
	for (i = 0; i <= 15; i++) {
		lookup_gtia[i] = base_colour | temp_count;
		temp_count += increment;
	}

	for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		*ptr++ = lookup_gtia[screendata >> 4];
		*ptr++ = lookup_gtia[screendata & 0x0f];
		if ((*t_pm_scanline_ptr)) {
			UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
			UBYTE pm_pixel;
			int which_player, which_missile;
			UBYTE colreg;
			int k;
			UWORD *w_ptr = (UWORD *) (ptr - 2);
			for (k = 1; k <= 4; k++) {
				pm_pixel = *c_pm_scanline_ptr++;
				colreg = 8;
				if (pm_pixel) {
					pf_colls[colreg] |= pm_pixel;
					if (pm_pixel & p5_mask)
						colreg = 7;
					else
						pm_pixel = pm_pixel | (pm_pixel >> 4);
					colreg = cur_prior[new_pm_lookup[pm_pixel & 0x0f] + colreg];
					*w_ptr = cl_word[colreg];
				}
				w_ptr++;
			}
		}
		t_pm_scanline_ptr++;
	}
}
static UBYTE gtia_10_lookup[] =
{8, 8, 8, 8, 4, 5, 6, 7, 8, 8, 8, 8, 4, 5, 6, 7};
static UBYTE gtia_10_pm[] =
{0x01, 0x02, 0x04, 0x08, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
void draw_antic_f_gtia10(int j, int nchars, UWORD t_screenaddr, char *t_ptr, ULONG * t_pm_scanline_ptr, UWORD t_chbase)
{
	int i;
	ULONG *ptr = (ULONG *) (t_ptr + 2);
	lookup_gtia[0] = cl_word[0] | (cl_word[0] << 16);
	lookup_gtia[1] = cl_word[1] | (cl_word[1] << 16);
	lookup_gtia[2] = cl_word[2] | (cl_word[2] << 16);
	lookup_gtia[3] = cl_word[3] | (cl_word[3] << 16);
	lookup_gtia[12] = lookup_gtia[4] = cl_word[4] | (cl_word[4] << 16);
	lookup_gtia[13] = lookup_gtia[5] = cl_word[5] | (cl_word[5] << 16);
	lookup_gtia[14] = lookup_gtia[6] = cl_word[6] | (cl_word[6] << 16);
	lookup_gtia[15] = lookup_gtia[7] = cl_word[7] | (cl_word[7] << 16);
	lookup_gtia[8] = lookup_gtia[9] = lookup_gtia[10] = lookup_gtia[11] = cl_word[8] | (cl_word[8] << 16);
	if (!(*((char *) (t_pm_scanline_ptr)))) {
		*((UWORD *) t_ptr) = cl_word[0];
	}
	else {
		UBYTE pm_pixel = *((char *) t_pm_scanline_ptr);
		UBYTE colreg = 8;
		pm_pixel |= 0x01;
		if (pm_pixel & p5_mask)
			colreg = 7;
		else
			pm_pixel = pm_pixel | (pm_pixel >> 4);
		colreg = cur_prior[new_pm_lookup[pm_pixel & 0x0f] + colreg];
		*((UWORD *) t_ptr) = cl_word[colreg];
	}
/* should prolly fix the border code instead..right border is still not right. */
	/* needs to be moved over 1 colour clock */
	t_pm_scanline_ptr = (ULONG *) (((UBYTE *) t_pm_scanline_ptr) + 1);
	for (i = 0; i < nchars; i++) {
		UBYTE screendata = memory[t_screenaddr++];
		*ptr++ = lookup_gtia[screendata >> 4];
		*ptr++ = lookup_gtia[screendata & 0x0f];
		if ((*t_pm_scanline_ptr)) {
			UBYTE *c_pm_scanline_ptr = (char *) t_pm_scanline_ptr;
			UBYTE pm_pixel;
			int which_player, which_missile;
			UBYTE colreg;
			int k;
			UWORD *w_ptr = (UWORD *) (ptr - 2);
			UBYTE t_screendata = screendata >> 4;
			for (k = 0; k <= 3; k++) {
				pm_pixel = *c_pm_scanline_ptr++;
				colreg = gtia_10_lookup[t_screendata];
				if (pm_pixel) {
					pf_colls[colreg] |= pm_pixel;
					pm_pixel |= gtia_10_pm[t_screendata];
					if (pm_pixel & p5_mask)
						colreg = 7;
					else
						pm_pixel = pm_pixel | (pm_pixel >> 4);
					colreg = cur_prior[new_pm_lookup[pm_pixel & 0x0f] + colreg];
					*w_ptr = cl_word[colreg];
				}
				*w_ptr++;
				if (k & 0x01)
					t_screendata = screendata & 0x0f;
			}
		}
		t_pm_scanline_ptr++;
	}
}


void do_antic()
{
	UWORD t_screenaddr;
	int j;
	int lastline;
	int md = hscrol_flag + mode_type;
	int temp_chars_read = chars_read[md];
	int thislinecycles = firstlinecycles;
	int first_line_flag = -4;

	lastline = vskipafter < 16 ? vskipafter : normal_lastline;
	j = vskipbefore;
	do {
		int temp_left_border_chars;
		ULONG *t_pm_scanline_ptr;
		char *ptr;
		UWORD t_chbase;
		extern void new_pm_scanline(void);
		int temp_xmin = x_min[md];

		j &= 0x0f;
		/* if(j==lastline) DO_DLI */

#ifdef POKEY_UPDATE
		pokey_update();
#endif
/* Pokey stuff begin (should probably be moved into pokey_update() as well */
		if (DELAYED_SERIN_IRQ > 0) {
			if (--DELAYED_SERIN_IRQ == 0) {
				/* IRQST &= 0xdf; */
				if (IRQEN & 0x20) {
#ifdef DEBUG2
					printf("SERIO: SERIN Interrupt triggered\n");
#endif
					IRQST &= 0xdf;
					IRQ = 1;
				}
#ifdef DEBUG2
				else {
					printf("SERIO: SERIN Interrupt missed\n");
				}
#endif
			}
		}

		if (DELAYED_SEROUT_IRQ > 0) {
			if (--DELAYED_SEROUT_IRQ == 0) {
				/* IRQST &= 0xef; */
				if (IRQEN & 0x10) {
#ifdef DEBUG2
					printf("SERIO: SEROUT Interrupt triggered\n");
#endif
					IRQST &= 0xef;
					IRQ = 1;
				}
#ifdef DEBUG2
				else {
					/* sigint_handler(1); */
					printf("SERIO: SEROUT Interrupt missed\n");
				}
#endif
				DELAYED_XMTDONE_IRQ += XMTDONE_INTERVAL;

			}
		}

		if (DELAYED_XMTDONE_IRQ > 0) {
			if (--DELAYED_XMTDONE_IRQ == 0) {
				/* IRQST &= 0xf7; */
				if (IRQEN & 0x08) {
#ifdef DEBUG2
					printf("SERIO: XMTDONE Interrupt triggered\n");
#endif
					IRQST &= 0xf7;
					IRQ = 1;
				}
#ifdef DEBUG2
				else {
					printf("SERIO: XMTDONE Interrupt missed\n");
				}
#endif
			}
		}
/* Pokey stuff ends */

		/* if (!wsync_halt) GO(107); */
		pmgdmac = pmg_dma();
		begcyc = BEGL - pmgdmac - dldmac;
		dldmac = 0;				/* subsequent lines have none; */
		/* PART 1 */

		unc = GO(begcyc + unc);	/* cycles for begin of each screen line */
		allc = realcyc[anticm8 + first_line_flag + dmaw] - BEGL;
		/* ^^^^^cycles for part 2; (realcyc is for parts 1+ 2. then -BEGL for 2 */
		/* ^^^ first line flag is either -4 or 0. when added to dmaw */
		/*  is 0-3 or 4-7.  (first and subsequent lines) */
		if (j == lastline && (IR & 0x80) && (NMIEN & 0x80)) {
			allc -= 8;
			NMIST |= 0x80;
			NMI();
		}
		unc = GO(allc + unc);
		/* ^^PART 2.(after the DLI) */
		new_pm_scanline();
		temp_left_border_chars = left_border_chars;
		switch (IR & 0x0f) {
		case 2:
		case 3:
		case 4:
			t_chbase = chbase_40 + ((j & 0x07) ^ flip_mask);
			break;
		case 5:
			t_chbase = chbase_40 + ((j >> 1) ^ flip_mask);
			break;
		case 6:
			t_chbase = chbase_20 + ((j & 0x07) ^ flip_mask);
			break;
		case 7:
			t_chbase = chbase_20 + ((j >> 1) ^ flip_mask);
			break;
			/* default: */
			break;
		}

		t_pm_scanline_ptr = (ULONG *) (&pm_scanline[temp_xmin >> 1]);
		ptr = &scrn_ptr[temp_xmin];

		t_screenaddr = screenaddr + ch_offset[md];
		switch (IR & 0x0f) {
		case 0:
			temp_chars_read = 0;
			left_border_chars = (48 - LCHOP - RCHOP) - right_border_chars;
			break;
		case 2:
			if ((PRIOR & 0x40) == 0x40)
				draw_antic_2_gtia9_11(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			else
				draw_antic_2(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			break;
		case 3:
			draw_antic_3(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			break;
		case 4:
		case 5:
			draw_antic_4(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			break;
		case 6:
		case 7:
			draw_antic_6(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			break;
		case 0x08:
			draw_antic_8(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			break;
		case 0x09:
			draw_antic_9(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			break;
		case 0x0a:
			draw_antic_a(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			break;
		case 0x0b:
		case 0x0c:
			draw_antic_c(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			break;
		case 0x0d:
		case 0x0e:
			draw_antic_e(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			break;
		case 0x0f:
			if ((PRIOR & 0x40) == 0x40)
				draw_antic_f_gtia9_11(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			else if ((PRIOR & 0xC0) == 0x80)
				draw_antic_f_gtia10(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			else
				draw_antic_f(j, chars_displayed[md], t_screenaddr, ptr, t_pm_scanline_ptr, t_chbase);
			break;
		default:
			break;
		}
		do_border();
		left_border_chars = temp_left_border_chars;
		/* memset(pm_scanline, 0, ATARI_WIDTH / 2); *//* Perry disabled 98/03/14 */
/* part 3 */
		allc = CPUL - WSYNC - realcyc[anticm8 + first_line_flag + dmaw] - thislinecycles;
		unc = GO(allc + unc);
		thislinecycles = nextlinecycles;
		first_line_flag = 0;
/* ^^^^ adjust for subsequent lines; */
		wsync_halt = 0;
		unc = GO(WSYNC + unc);
/* ^^^^ part 4. */

		ypos++;
/*if (wsync_halt)
   {
   if (!(--wsync_halt))
   {
   GO (7);
   }
   }
   else
   {
   GO (7);
   }
 */
		scrn_ptr += ATARI_WIDTH;
	} while ((j++) != lastline);
	screenaddr += temp_chars_read;
}

/*
   *****************************************************************
   *                                *
   *    Section         :   Display List        *
   *    Original Author     :   David Firth     *
   *    Date Written        :   28th May 1995       *
   *    Version         :   1.0         *
   *                                *
   *   Description                          *
   *   -----------                          *
   *                                *
   *   Section that handles Antic Display List. Not required for    *
   *   BASIC version.                       *
   *                                                               *
   *****************************************************************
 */

int pmg_dma(void)
{
	int pmd;
	pmd = 0;
	if (missile_dma_enabled) {
		if (singleline) {
			pmd = 1;
			if (missile_gra_enabled)
				GRAFM = memory[maddr_s + ypos];
		}
		else {
			if (!(ypos & 0x01))
				pmd = 1;
			if (missile_gra_enabled)
				GRAFM = memory[maddr_d + (ypos >> 1)];
		}
	}

	if (player_dma_enabled) {
		if (singleline) {
			pmd = 5;			/* 4+1 ...no player DMA without missile DMA */
			if (player_gra_enabled) {
				GRAFP0 = memory[p0addr_s + ypos];
				GRAFP1 = memory[p1addr_s + ypos];
				GRAFP2 = memory[p2addr_s + ypos];
				GRAFP3 = memory[p3addr_s + ypos];
			}
		}
		else {
			if (!(ypos & 0x01))
				pmd = 5;		/* 4+1 ...no player DMA without missile DMA */
			if (player_gra_enabled) {
				GRAFP0 = memory[p0addr_d + (ypos >> 1)];
				GRAFP1 = memory[p1addr_d + (ypos >> 1)];
				GRAFP2 = memory[p2addr_d + (ypos >> 1)];
				GRAFP3 = memory[p3addr_d + (ypos >> 1)];
			}
		}
	}
	return pmd;
}

void ANTIC_RunDisplayList(void)
{
	UWORD dlist;
	int JVB;
	int vscrol_flag;
	int nlines;
	int i;

	wsync_halt = 0;
	unc = 0;

	/*
	 * VCOUNT must equal zero for some games but the first line starts
	 * when VCOUNT=4. This portion processes when VCOUNT=0, 1, 2 and 3
	 */

	for (ypos = 0; ypos < 8; ypos++) {
#ifdef POKEY_UPDATE
		pokey_update();
#endif
		unc = GO(CPUL - DMAR - WSYNC + unc);
		wsync_halt = 0;
		unc = GO(WSYNC + unc);
		/* GO (114); */
	}
	NMIST = 0x00;				/* Reset VBLANK */

	scrn_ptr = (UBYTE *) atari_screen;

	ypos = 8;
	vscrol_flag = FALSE;

	dlist = (DLISTH << 8) | DLISTL;
	JVB = FALSE;

	while ((DMACTL & 0x20) && !JVB && (ypos < (ATARI_HEIGHT + 8))) {
		UBYTE colpf1;

		antic23f = FALSE;
		IR = memory[dlist++];
		colpf1 = COLPF1;

		/* PMG flickering :-) (Raster) */
		if (player_flickering) {
			GRAFP0 = GRAFP1 = GRAFP2 = GRAFP3 = rand();
		}
		if (missile_flickering) {
			GRAFM = rand();
		}


		firstlinecycles = nextlinecycles = DMAR;
		anticmode = (IR & 0x0f);
		anticm8 = (anticmode << 3);

		switch (anticmode) {
		case 0x00:
			{
				normal_lastline = ((IR >> 4) & 0x07);
				dldmac = 1;
				/* IR=0; leave dli bit alone. */
				do_antic();
				nlines = 0;
			}
			break;
		case 0x01:
			if (IR & 0x40) {
				nlines = 0;
				JVB = TRUE;
			}
			else {
				dlist = (memory[dlist + 1] << 8) | memory[dlist];
				normal_lastline = 0;
				IR &= 0xf0;		/* important:must preserve DLI bit. */
				/* maybe should just add 0x01 as a case in antic code? */

				dldmac = 3;
				do_antic();
				nlines = 0;
				/* Jump aparently uses 1 scan line */
			}
			break;
		default:
			if (IR & 0x40) {
				screenaddr = (memory[dlist + 1] << 8) | memory[dlist];
				dlist += 2;
				dldmac = 3;
			}
			else
				dldmac = 1;

			if (IR & 0x20) {
				if (!vscrol_flag) {
					vskipbefore = VSCROL;
					vscrol_flag = TRUE;
				}
			}
			else if (vscrol_flag) {
				vskipafter = VSCROL;
				vscrol_flag = FALSE;
			}
			hscrol_flag = 0;
			if (IR & 0x10) {
				xmin = dmactl_xmin_scroll;
				xmax = dmactl_xmax_scroll;
				scroll_offset = HSCROL + HSCROL;
				hscrol_flag = 3;
			}
			else {
				xmin = dmactl_xmin_noscroll;
				xmax = dmactl_xmax_noscroll;
				scroll_offset = 0;
			}

			switch (IR & 0x0f) {
			case 0x02:
				nlines = 0;
				mode_type = 0;
				normal_lastline = 7;
				antic23f = TRUE;
				firstlinecycles = (dmaw << 4) + 6 - dmaw;	/* 0,1,2 */
				nextlinecycles = (dmaw << 3) + DMAR;
				do_antic();
				break;
			case 0x03:
				nlines = 0;
				mode_type = 0;
				normal_lastline = 9;
				antic23f = TRUE;
				firstlinecycles = (dmaw << 4) + 6 - dmaw;	/* 0,1,2 */
				nextlinecycles = (dmaw << 3) + DMAR;
				do_antic();
				break;
			case 0x04:
				mode_type = 0;
				normal_lastline = 7;
				nlines = 0;
				firstlinecycles = (dmaw << 4) + 6 - dmaw;	/* 0,1,2 */
				nextlinecycles = (dmaw << 3) + DMAR;
				do_antic();
				break;
			case 0x05:
				mode_type = 0;
				normal_lastline = 15;
				nlines = 0;
				firstlinecycles = (dmaw << 4) + 6 - dmaw;	/* 0,1,2 */
				nextlinecycles = (dmaw << 3) + DMAR;
				do_antic();
				break;
			case 0x06:
				mode_type = 1;
				normal_lastline = 7;
				nlines = 0;
				firstlinecycles = (dmaw << 3) + DMAR;
				nextlinecycles = (dmaw << 2) + DMAR;
				do_antic();
				break;
			case 0x07:
				mode_type = 1;
				normal_lastline = 15;
				nlines = 0;
				firstlinecycles = (dmaw << 3) + DMAR;
				nextlinecycles = (dmaw << 2) + DMAR;
				do_antic();
				break;
			case 0x08:
				mode_type = 2;
				normal_lastline = 7;
				nlines = 0;
				firstlinecycles = (dmaw << 1) + DMAR;
				do_antic();
				break;
			case 0x09:
				mode_type = 2;
				normal_lastline = 3;
				nlines = 0;
				firstlinecycles = (dmaw << 1) + DMAR;
				do_antic();
				break;
			case 0x0a:
				nlines = 0;
				mode_type = 1;
				normal_lastline = 3;
				firstlinecycles = (dmaw << 2) + DMAR;
				do_antic();
				break;
			case 0x0b:
				nlines = 0;
				mode_type = 1;
				normal_lastline = 1;
				firstlinecycles = (dmaw << 2) + DMAR;
				do_antic();
				break;
			case 0x0c:
				nlines = 0;
				mode_type = 1;
				normal_lastline = 0;
				firstlinecycles = (dmaw << 2) + DMAR;
				do_antic();
				break;
			case 0x0d:
				nlines = 0;
				mode_type = 0;
				normal_lastline = 1;
				firstlinecycles = (dmaw << 3) + DMAR;
				do_antic();
				break;
			case 0x0e:
				nlines = 0;
				mode_type = 0;
				normal_lastline = 0;
				firstlinecycles = (dmaw << 3) + DMAR;
				do_antic();
				break;
			case 0x0f:
				nlines = 0;
				mode_type = 0;
				normal_lastline = 0;
				antic23f = TRUE;
				firstlinecycles = (dmaw << 3) + DMAR;
				do_antic();
				break;
			default:
				nlines = 0;
				JVB = TRUE;
				break;
			}
			break;
		}
		vskipbefore = 0;
		vskipafter = 99;
	}
	IR = 0;
	normal_lastline = 0;
	for (i = (ATARI_HEIGHT + 8 - ypos); i > 0; i--) {
		do_antic();
	}
	nlines = 0;
	NMIST = 0x40;				/* Set VBLANK */
	if (NMIEN & 0x40) {
		GO(1);					/* Needed for programs that monitor NMIST (Spy's Demise) */
		NMI();
	}

	for (ypos = 248; ypos < (tv_mode == PAL ? 312 : 262); ypos++) {
#ifdef POKEY_UPDATE
		pokey_update();
#endif
		unc = GO(CPUL - WSYNC + unc - DMAR);
		wsync_halt = 0;
		unc = GO(WSYNC + unc);
	}
}

UBYTE ANTIC_GetByte(UWORD addr)
{
	UBYTE byte;

	addr &= 0xff0f;
	switch (addr) {
	case _CHBASE:
		byte = CHBASE;
		break;
	case _CHACTL:
		byte = CHACTL;
		break;
	case _DLISTL:
		byte = DLISTL;
		break;
	case _DLISTH:
		byte = DLISTH;
		break;
	case _DMACTL:
		byte = DMACTL;
		break;
	case _PENH:
	case _PENV:
		byte = 0x00;
		break;
	case _VCOUNT:
		byte = ypos >> 1;
		break;
	case _NMIEN:
		byte = NMIEN;
		break;
	case _NMIST:
		byte = NMIST;
		break;
	case _WSYNC:
/*       wsync_halt++; */
		byte = 0xff;			/* tested on real Atari !RS! */
		break;
	}

	return byte;
}

int ANTIC_PutByte(UWORD addr, UBYTE byte)
{
	int abort = FALSE;

	addr &= 0xff0f;
	switch (addr) {
	case _CHBASE:
		CHBASE = byte;
		chbase_40 = (byte << 8) & 0xfc00;
		chbase_20 = (byte << 8) & 0xfe00;
		break;
	case _CHACTL:
		CHACTL = byte;
/*
   =================================================================
   Check for vertical reflect, video invert and character blank bits
   =================================================================
 */
		switch (CHACTL & 0x07) {
		case 0x00:
			char_offset = 0;
			char_delta = 1;
			flip_mask = 0x00;
			invert_mask = 0x00;
			blank_mask = 0x00;
			break;
		case 0x01:
			char_offset = 0;
			char_delta = 1;
			flip_mask = 0x00;
			invert_mask = 0x00;
			blank_mask = 0x80;
			break;
		case 0x02:
			char_offset = 0;
			char_delta = 1;
			flip_mask = 0x00;
			invert_mask = 0x80;
			blank_mask = 0x00;
			break;
		case 0x03:
			char_offset = 0;
			char_delta = 1;
			flip_mask = 0x00;
			invert_mask = 0x80;
			blank_mask = 0x80;
			break;
		case 0x04:
			char_offset = 7;
			char_delta = -1;
			flip_mask = 0x07;
			invert_mask = 0x00;
			blank_mask = 0x00;
			break;
		case 0x05:
			char_offset = 7;
			char_delta = -1;
			flip_mask = 0x07;
			invert_mask = 0x00;
			blank_mask = 0x80;
			break;
		case 0x06:
			char_offset = 7;
			char_delta = -1;
			flip_mask = 0x07;
			invert_mask = 0x80;
			blank_mask = 0x00;
			break;
		case 0x07:
			char_offset = 7;
			char_delta = -1;
			flip_mask = 0x07;
			invert_mask = 0x80;
			blank_mask = 0x80;
			break;
		}
		break;
	case _DLISTL:
		DLISTL = byte;
		break;
	case _DLISTH:
		DLISTH = byte;
		break;
	case _DMACTL:
		DMACTL = byte;
		switch (DMACTL & 0x03) {
		case 0x00:
			dmactl_xmin_noscroll = dmactl_xmax_noscroll = 0;
			dmactl_xmin_scroll = dmactl_xmax_scroll = 0;
			chars_read[NORMAL0] = 0;
			chars_read[NORMAL1] = 0;
			chars_read[NORMAL2] = 0;
			chars_read[SCROLL0] = 0;
			chars_read[SCROLL1] = 0;
			chars_read[SCROLL2] = 0;
			chars_displayed[NORMAL0] = 0;
			chars_displayed[NORMAL1] = 0;
			chars_displayed[NORMAL2] = 0;
			x_min[NORMAL0] = 0;
			x_min[NORMAL1] = 0;
			x_min[NORMAL2] = 0;
			ch_offset[NORMAL0] = 0;
			ch_offset[NORMAL1] = 0;
			ch_offset[NORMAL2] = 0;
			base_scroll_char_offset = 0;
			base_scroll_char_offset2 = 0;
			base_scroll_char_offset3 = 0;
			left_border_chars = 24 - LCHOP;
			right_border_chars = 24 - RCHOP;
			right_border_start = ATARI_WIDTH >> 1;
			dmaw = 5;
			break;
		case 0x01:
			dmactl_xmin_noscroll = 64;
			dmactl_xmax_noscroll = ATARI_WIDTH - 64;
			dmactl_xmin_scroll = 32;
			dmactl_xmax_scroll = ATARI_WIDTH - 32;
			chars_read[NORMAL0] = 32;
			chars_read[NORMAL1] = 16;
			chars_read[NORMAL2] = 8;
			chars_read[SCROLL0] = 40;
			chars_read[SCROLL1] = 20;
			chars_read[SCROLL2] = 10;
			chars_displayed[NORMAL0] = 32;
			chars_displayed[NORMAL1] = 16;
			chars_displayed[NORMAL2] = 8;
			x_min[NORMAL0] = 64;
			x_min[NORMAL1] = 64;
			x_min[NORMAL2] = 64;
			ch_offset[NORMAL0] = 0;
			ch_offset[NORMAL1] = 0;
			ch_offset[NORMAL2] = 0;
			base_scroll_char_offset = 4;
			base_scroll_char_offset2 = 2;
			base_scroll_char_offset3 = 1;
			left_border_chars = 8 - LCHOP;
			right_border_chars = 8 - RCHOP;
			right_border_start = ATARI_WIDTH - 64 /* - 1 */ ;	/* RS! */
			dmaw = 4;
			break;
		case 0x02:
			dmactl_xmin_noscroll = 32;
			dmactl_xmax_noscroll = ATARI_WIDTH - 32;
			dmactl_xmin_scroll = 0;
			dmactl_xmax_scroll = ATARI_WIDTH;
			chars_read[NORMAL0] = 40;
			chars_read[NORMAL1] = 20;
			chars_read[NORMAL2] = 10;
			chars_read[SCROLL0] = 48;
			chars_read[SCROLL1] = 24;
			chars_read[SCROLL2] = 12;
			chars_displayed[NORMAL0] = 40;
			chars_displayed[NORMAL1] = 20;
			chars_displayed[NORMAL2] = 10;
			x_min[NORMAL0] = 32;
			x_min[NORMAL1] = 32;
			x_min[NORMAL2] = 32;
			ch_offset[NORMAL0] = 0;
			ch_offset[NORMAL1] = 0;
			ch_offset[NORMAL2] = 0;
			base_scroll_char_offset = 4;
			base_scroll_char_offset2 = 2;
			base_scroll_char_offset3 = 1;
			left_border_chars = 4 - LCHOP;
			right_border_chars = 4 - RCHOP;
			right_border_start = ATARI_WIDTH - 32 /* - 1 */ ;	/* RS! */
			dmaw = 5;
			break;
		case 0x03:
			dmactl_xmin_noscroll = dmactl_xmin_scroll = 0;
			dmactl_xmax_noscroll = dmactl_xmax_scroll = ATARI_WIDTH;
			chars_read[NORMAL0] = 48;
			chars_read[NORMAL1] = 24;
			chars_read[NORMAL2] = 12;
			chars_read[SCROLL0] = 48;
			chars_read[SCROLL1] = 24;
			chars_read[SCROLL2] = 12;
			chars_displayed[NORMAL0] = 44;
			chars_displayed[NORMAL1] = 23;
			chars_displayed[NORMAL2] = 12;
			x_min[NORMAL0] = 24;
			x_min[NORMAL1] = 16;
			x_min[NORMAL2] = 0;
			ch_offset[NORMAL0] = 3;
			ch_offset[NORMAL1] = 1;
			ch_offset[NORMAL2] = 0;
			base_scroll_char_offset = 3;
			base_scroll_char_offset2 = 1;
			base_scroll_char_offset3 = 0;
			left_border_chars = 3 - LCHOP;
			right_border_chars = ((1 - LCHOP) < 0) ? (0) : (1 - LCHOP);
			right_border_start = ATARI_WIDTH - 8 /* - 1 */ ;	/* RS! */
			dmaw = 6;
			break;
		}

		missile_dma_enabled = (DMACTL & 0x04);
		player_dma_enabled = (DMACTL & 0x08);
		singleline = (DMACTL & 0x10);
		player_flickering = ((player_dma_enabled | player_gra_enabled) == 0x02);
		missile_flickering = ((missile_dma_enabled | missile_gra_enabled) == 0x01);

		ANTIC_PutByte(_HSCROL, HSCROL);		/* reset values in hscrol */

		break;
	case _HSCROL:

		{

			int char_whole, char_remainder;
			HSCROL = byte & 0x0f;

			if ((DMACTL & 0x03) == 0x00) {	/* no playfield */

				chars_displayed[SCROLL0] = 0;
				chars_displayed[SCROLL1] = 0;
				chars_displayed[SCROLL2] = 0;
				x_min[SCROLL0] = 0;
				x_min[SCROLL1] = 0;
				x_min[SCROLL2] = 0;
				ch_offset[SCROLL0] = 0;
				ch_offset[SCROLL1] = 0;
				ch_offset[SCROLL2] = 0;
			}
			else {
				char_whole = HSCROL >> 2;
				char_remainder = (HSCROL & 0x03) << 1;
				chars_displayed[SCROLL0] = chars_displayed[NORMAL0];
				ch_offset[SCROLL0] = base_scroll_char_offset - char_whole;
				if (char_remainder != 0) {
					char_remainder -= 8;
					chars_displayed[SCROLL0]++;
					ch_offset[SCROLL0]--;
				}
				x_min[SCROLL0] = x_min[NORMAL0] + char_remainder;

				if ((DMACTL & 0x03) == 0x03) {	/* wide playfield */

					if (HSCROL == 4 && HSCROL == 12)
						chars_displayed[SCROLL1] = 22;
					else
						chars_displayed[SCROLL1] = 23;
					char_whole = 0;
					if (HSCROL <= 4) {
						ch_offset[SCROLL1] = 1;
						x_min[SCROLL1] = 16 + (HSCROL << 1);
					}
					else if (HSCROL >= 5 && HSCROL <= 12) {
						ch_offset[SCROLL1] = 0;
						x_min[SCROLL1] = ((HSCROL - 4) << 1) + 8;
					}
					else {
						ch_offset[SCROLL1] = -1;
						x_min[SCROLL1] = ((HSCROL - 12) << 1) + 8;
					}
				}
				else {
					chars_displayed[SCROLL1] = chars_displayed[NORMAL1];
					char_whole = HSCROL >> 3;
					char_remainder = (HSCROL & 0x07) << 1;
					ch_offset[SCROLL1] = base_scroll_char_offset2 - char_whole;
					if (char_remainder != 0) {
						char_remainder -= 16;
						chars_displayed[SCROLL1]++;
						ch_offset[SCROLL1]--;
					}
					x_min[SCROLL1] = x_min[NORMAL0] + char_remainder;
				}
				ch_offset[SCROLL2] = base_scroll_char_offset3;
				if ((DMACTL & 0x03) == 0x03) {	/* wide playfield */

					chars_displayed[SCROLL2] = chars_displayed[NORMAL2];
					x_min[SCROLL2] = x_min[NORMAL2] + (HSCROL << 1);
				}
				else {
					if (HSCROL != 0) {
						x_min[SCROLL2] = x_min[NORMAL2] - 32 + (HSCROL << 1);
						chars_displayed[SCROLL2] = chars_displayed[NORMAL2] + 1;
						ch_offset[SCROLL2]--;
					}
					else {
						chars_displayed[SCROLL2] = chars_displayed[NORMAL2];
						x_min[SCROLL2] = x_min[NORMAL2];
					}
				}
			}
		}
		break;
	case _NMIEN:
		NMIEN = byte;
		break;
	case _NMIRES:
		NMIST = 0x00;
		break;
	case _PMBASE:
		{
			UWORD pmbase_s;
			UWORD pmbase_d;

			PMBASE = byte;

			pmbase_s = (PMBASE & 0xf8) << 8;
			pmbase_d = (PMBASE & 0xfc) << 8;

			maddr_s = pmbase_s + 768;
			p0addr_s = pmbase_s + 1024;
			p1addr_s = pmbase_s + 1280;
			p2addr_s = pmbase_s + 1536;
			p3addr_s = pmbase_s + 1792;

			maddr_d = pmbase_d + 384;
			p0addr_d = pmbase_d + 512;
			p1addr_d = pmbase_d + 640;
			p2addr_d = pmbase_d + 768;
			p3addr_d = pmbase_d + 896;
		}
		break;
	case _VSCROL:
		VSCROL = byte & 0x0f;
		break;
	case _WSYNC:
		wsync_halt++;
		abort = TRUE;
		break;
	}

	return abort;
}
