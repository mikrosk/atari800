/*
 * diskled.c - disk drive LED emulation
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2003 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "atari.h"
#include "diskled.h"

#define DISKLED_FONT_WIDTH		5
#define DISKLED_FONT_HEIGHT		7
#define DISKLED_FONT_CHARSIZE	(DISKLED_FONT_WIDTH * DISKLED_FONT_HEIGHT)

int led_status = 0;
int led_off_delay = -1;

static unsigned char DiskLED[]= {
172,172,172,172,172,
172,172,000,172,172,
172,000,000,172,172,
172,172,000,172,172,
172,172,000,172,172,
172,172,000,172,172,
172,172,172,172,172,
		
172,172,172,172,172,
172,000,000,172,172,
172,172,172,000,172,
172,172,000,172,172,
172,000,172,172,172,
172,000,000,000,172,
172,172,172,172,172,
		
172,172,172,172,172,
172,000,000,172,172,
172,172,172,000,172,
172,172,000,172,172,
172,172,172,000,172,
172,000,000,172,172,
172,172,172,172,172,

172,172,172,172,172,
172,172,172,000,172,
172,172,000,000,172,
172,000,172,000,172,
172,000,000,000,172,
172,172,172,000,172,
172,172,172,172,172,

172,172,172,172,172,
172,000,000,000,172,
172,000,172,172,172,
172,000,000,000,172,
172,172,172,000,172,
172,000,000,172,172,
172,172,172,172,172,

172,172,172,172,172,
172,172,000,172,172,
172,000,172,172,172,
172,000,000,172,172,
172,000,172,000,172,
172,172,000,172,172,
172,172,172,172,172,

172,172,172,172,172,
172,000,000,000,172,
172,172,172,000,172,
172,172,000,172,172,
172,172,000,172,172,
172,172,000,172,172,
172,172,172,172,172,

172,172,172,172,172,
172,172,000,172,172,
172,000,172,000,172,
172,172,000,172,172,
172,000,172,000,172,
172,172,000,172,172,
172,172,172,172,172,

172,172,172,172,172,		
172,172,000,172,172,
172,000,172,000,172,
172,000,172,000,172,
172,000,172,000,172,
172,172,000,172,172,
172,172,172,172,172,	/* End of read LEDs	*/

053,053,053,053,053,	/* Start of write LEDs */
053,053,000,053,053,
053,000,000,053,053,
053,053,000,053,053,
053,053,000,053,053,
053,053,000,053,053,
053,053,053,053,053,

053,053,053,053,053,
053,000,000,053,053,
053,053,053,000,053,
053,053,000,053,053,
053,000,053,053,053,
053,000,000,000,053,
053,053,053,053,053,

053,053,053,053,053,
053,000,000,053,053,
053,053,053,000,053,
053,053,000,053,053,
053,053,053,000,053,
053,000,000,053,053,
053,053,053,053,053,

053,053,053,053,053,
053,053,053,000,053,
053,053,000,000,053,
053,000,053,000,053,
053,000,000,000,053,
053,053,053,000,053,
053,053,053,053,053,


053,053,053,053,053,
053,000,000,000,053,
053,000,053,053,053,
053,000,000,000,053,
053,053,053,000,053,
053,000,000,053,053,
053,053,053,053,053,

053,053,053,053,053,
053,053,000,053,053,
053,000,053,053,053,
053,000,000,053,053,
053,000,053,000,053,
053,053,000,053,053,
053,053,053,053,053,

053,053,053,053,053,
053,000,000,000,053,
053,053,053,000,053,
053,053,000,053,053,
053,053,000,053,053,
053,053,000,053,053,
053,053,053,053,053,

053,053,053,053,053,
053,053,000,053,053,
053,000,053,000,053,
053,053,000,053,053,
053,000,053,000,053,
053,053,000,053,053,
053,053,053,053,053,

053,053,053,053,053,
053,053,000,053,053,
053,000,053,000,053,
053,000,053,000,053,
053,000,053,000,053,
053,053,000,053,053,	
053,053,053,053,053	/* End of write LEDs	*/
};


void LED_Frame(void)
{
	if (led_off_delay >= 0)
		if (--led_off_delay < 0)
			led_status = 0;

	if (led_status) {
		UBYTE *shape = DiskLED + (led_status - 1) * DISKLED_FONT_CHARSIZE;
		UBYTE *scrn = (UBYTE *) atari_screen
					  + (screen_visible_y2 - DISKLED_FONT_HEIGHT) * ATARI_WIDTH
					  + screen_visible_x2 - DISKLED_FONT_WIDTH;
		int x;
		int y;
		for (y = 0; y < DISKLED_FONT_HEIGHT; y++) {
			for (x = 0; x < DISKLED_FONT_WIDTH; x++)
				*scrn++ = *shape++;
			scrn += ATARI_WIDTH - DISKLED_FONT_WIDTH;
		}
	}
}
