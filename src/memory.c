/*
 * memory.c - memory emulation
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2008 Atari800 development team (see DOC/CREDITS)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atari.h"
#include "antic.h"
#include "cpu.h"
#include "cartridge.h"
#include "roms/altirra_5200_os.h"
#include "esc.h"
#include "gtia.h"
#include "log.h"
#include "memory.h"
#include "pbi.h"
#include "pia.h"
#include "pokey.h"
#include "util.h"
#ifndef BASIC
#include "statesav.h"
#endif

UBYTE MEMORY_mem[65536 + 2];

int MEMORY_ram_size = 64;

#ifndef PAGED_ATTRIB

UBYTE MEMORY_attrib[65536];

static UBYTE under_atarixl_os[16384];
static UBYTE under_cartA0BF[8192];

#else /* PAGED_ATTRIB */

MEMORY_rdfunc MEMORY_readmap[256];
MEMORY_wrfunc MEMORY_writemap[256];

typedef struct map_save {
	int     code;
	MEMORY_rdfunc  rdptr;
	MEMORY_wrfunc  wrptr;
} map_save;

void MEMORY_ROM_PutByte(UWORD addr, UBYTE value)
{
}

map_save save_map[2] = {
	{0, NULL, NULL},          /* RAM */
	{1, NULL, MEMORY_ROM_PutByte}    /* ROM */
};

static UBYTE const *cartA0BF = NULL;

#endif /* PAGED_ATTRIB */

UBYTE MEMORY_basic[8192];
UBYTE MEMORY_os[16384];
UBYTE MEMORY_xegame[8192];

int MEMORY_xe_bank = 0;
int MEMORY_selftest_enabled = 0;

static UBYTE under_cart809F[8192];

static int cart809F_enabled = FALSE;
int MEMORY_cartA0BF_enabled = FALSE;

static UBYTE *atarixe_memory = NULL;
static ULONG atarixe_memory_size = 0;

/* RAM shadowed by Self-Test in the XE bank seen by ANTIC, when ANTIC/CPU
   separate XE access is active. */
static UBYTE antic_bank_under_selftest[0x800];

int MEMORY_have_basic = FALSE; /* Atari BASIC image has been successfully read (Atari 800 only) */

/* Axlon and Mosaic RAM expansions for Atari 400/800 only */
static void MosaicPutByte(UWORD addr, UBYTE byte);
static UBYTE MosaicGetByte(UWORD addr, int no_side_effects);
static void AxlonPutByte(UWORD addr, UBYTE byte);
static UBYTE AxlonGetByte(UWORD addr, int no_side_effects);
static UBYTE *axlon_ram = NULL;
static int axlon_current_bankmask = 0;
int axlon_curbank = 0;
int MEMORY_axlon_num_banks = 0x00;
int MEMORY_axlon_0f_mirror = FALSE; /* The real Axlon had a mirror bank register at 0x0fc0-0x0fff, compatibles did not*/
static UBYTE *mosaic_ram = NULL;
static int mosaic_current_num_banks = 0;
static int mosaic_curbank = 0x3f;
int MEMORY_mosaic_num_banks = 0;

int MEMORY_enable_mapram = FALSE;

/* Buffer for storing of MapRAM memory. */
static UBYTE *mapram_memory = NULL;

#ifdef PAGED_ATTRIB
/*
 * $0000 - $4000 (16 KB RAM)
 * $4000 - $5000 (4 KB RAM or extended memory)
 * $5000 - $5800 (Self Test or 2 KB RAM or extended memory)
 * $5800 - $8000 (10 KB RAM or extended memory)
 * $8000 - $A000 (Cartridge B or 8KB RAM)
 * $A000 - $C000 (Cartridge A or 8 KB RAM or BASIC ROM)
 * $C000 - $D000 (4 KB RAM)
 * $D000 - $D800 (hardware)
 * $D800 - $FFFF (14 KB RAM or OS ROM)
 */

static UBYTE bank_read(const UBYTE* addr, int no_side_effects)
{
	/* RAM bank access has no side effects */
	(void)no_side_effects;

	return *addr;
}
static void bank_write(UBYTE* addr, UBYTE value)
{
	*addr = value;
}
static UBYTE mapram_read(UWORD addr, int no_side_effects)
{
	/* MapRAM: 0x5000 - 0x57ff */
	return bank_read(&mapram_memory[addr - 0x5000], no_side_effects);
}
static void mapram_write(UWORD addr, UBYTE value)
{
	/* MapRAM: 0x5000 - 0x57ff */
	bank_write(&mapram_memory[addr - 0x5000], value);
}
static UBYTE atarixe_memory_read(UWORD addr, int no_side_effects)
{
	/* CPU XMS bank: 0x4000 - 0x7fff */
	return bank_read(&atarixe_memory[addr - 0x4000 + (MEMORY_xe_bank << 14)], no_side_effects);
}
static void atarixe_memory_write(UWORD addr, UBYTE value)
{
	/* CPU XMS bank: 0x4000 - 0x7fff */
	bank_write(&atarixe_memory[addr - 0x4000 + (MEMORY_xe_bank << 14)], value);
}
static UBYTE rom_os_read(UWORD addr, int no_side_effects)
{
	/* ROM OS: 0xc000 - 0xcfff; 0xd800 - 0xffff */
	return bank_read(&MEMORY_os[addr - 0xc000], no_side_effects);
}
static UBYTE self_test_read(UWORD addr, int no_side_effects)
{
	/* Self Test: 0xd000 - 0xd7ff -> 0x5000 - 0x57ff */
	return bank_read(&MEMORY_os[addr - 0x5000 + 0x1000], no_side_effects);
}
static UBYTE rom_cartA0BF_read(UWORD addr, int no_side_effects)
{
	/* Cartridge ROM: 0xa000 - 0xbfff */
	return bank_read(&cartA0BF[addr - 0xa000], no_side_effects);
}
#endif

static void DisableMapRAM(void)
{
#ifndef PAGED_ATTRIB
	memcpy(mapram_memory, MEMORY_mem + 0x5000, 0x800);
	memcpy(MEMORY_mem + 0x5000, under_atarixl_os + 0x1000, 0x800);
#else
	MEMORY_SetRAM(0x5000, 0x57ff);
#endif
}
static void EnableMapRAM(void)
{
#ifndef PAGED_ATTRIB
	memcpy(under_atarixl_os + 0x1000, MEMORY_mem + 0x5000, 0x800);
	memcpy(MEMORY_mem + 0x5000, mapram_memory, 0x800);
#else
	MEMORY_SetBank(0x5000, 0x57ff, mapram_read, mapram_write);
#endif
}

static void DisableSelfTestROM(int antic_bank)
{
	if (MEMORY_ram_size > 20) {
#ifndef PAGED_ATTRIB
		memcpy(MEMORY_mem + 0x5000, under_atarixl_os + 0x1000, 0x800);
#endif
		if (ANTIC_xe_ptr != NULL)
			/* Also disable Self Test from XE bank accessed by ANTIC. */
			memcpy(atarixe_memory + (antic_bank << 14) + 0x1000, antic_bank_under_selftest, 0x800);
		MEMORY_SetRAM(0x5000, 0x57ff);
	}
	else
		MEMORY_dFillMem(0x5000, 0xff, 0x800);
	MEMORY_selftest_enabled = FALSE;
}
static void EnableSelfTestROM(int antic_bank)
{
	if (MEMORY_ram_size > 20) {
#ifndef PAGED_ATTRIB
		memcpy(under_atarixl_os + 0x1000, MEMORY_mem + 0x5000, 0x800);
#endif
		if (ANTIC_xe_ptr != NULL)
			/* Also backup RAM under Self Test from XE bank accessed by ANTIC. */
			memcpy(antic_bank_under_selftest, atarixe_memory + (antic_bank << 14) + 0x1000, 0x800);
#ifndef PAGED_ATTRIB
		MEMORY_SetROM(0x5000, 0x57ff);
#endif
	}
#ifndef PAGED_ATTRIB
	memcpy(MEMORY_mem + 0x5000, MEMORY_os + 0x1000, 0x800);
#else
	MEMORY_SetBank(0x5000, 0x57ff, self_test_read, MEMORY_ROM_PutByte);
#endif
	if (ANTIC_xe_ptr != NULL)
		/* Also enable Self Test in the XE bank accessed by ANTIC. */
		memcpy(atarixe_memory + (antic_bank << 14) + 0x1000, MEMORY_os + 0x1000, 0x800);
	MEMORY_selftest_enabled = TRUE;
}

static void DisableOsROM(void)
{
	if (MEMORY_ram_size > 48) {
#ifndef PAGED_ATTRIB
		memcpy(MEMORY_mem + 0xc000, under_atarixl_os, 0x1000);
		memcpy(MEMORY_mem + 0xd800, under_atarixl_os + 0x1800, 0x2800);
#endif
		MEMORY_SetRAM(0xc000, 0xcfff);
		MEMORY_SetRAM(0xd800, 0xffff);
	} else {
		MEMORY_dFillMem(0xc000, 0xff, 0x1000);
		MEMORY_dFillMem(0xd800, 0xff, 0x2800);
	}
}
static void EnableOsROM(void)
{
	if (MEMORY_ram_size > 48) {
#ifndef PAGED_ATTRIB
		memcpy(under_atarixl_os, MEMORY_mem + 0xc000, 0x1000);
		memcpy(under_atarixl_os + 0x1800, MEMORY_mem + 0xd800, 0x2800);
		MEMORY_SetROM(0xc000, 0xcfff);
		MEMORY_SetROM(0xd800, 0xffff);
#endif
	}
#ifndef PAGED_ATTRIB
	memcpy(MEMORY_mem + 0xc000, MEMORY_os, 0x1000);
	memcpy(MEMORY_mem + 0xd800, MEMORY_os + 0x1800, 0x2800);
#else
	MEMORY_SetBank(0xc000, 0xcfff, rom_os_read, MEMORY_ROM_PutByte);
	MEMORY_SetBank(0xd800, 0xffff, rom_os_read, MEMORY_ROM_PutByte);
#endif
	ESC_PatchOS();
}

static void DisableCartROM(void)
{
	if (MEMORY_ram_size > 40) {
#ifndef PAGED_ATTRIB
		memcpy(MEMORY_mem + 0xa000, under_cartA0BF, 0x2000);
#endif
		MEMORY_SetRAM(0xa000, 0xbfff);
	}
	else
		MEMORY_dFillMem(0xa000, 0xff, 0x2000);
}
static void EnableCartROM(UBYTE const *cart, int already_used)
{
	if (MEMORY_ram_size > 40 && !already_used) {
#ifndef PAGED_ATTRIB
		memcpy(under_cartA0BF, MEMORY_mem + 0xa000, 0x2000);
		MEMORY_SetROM(0xa000, 0xbfff);
#endif
	}
#ifndef PAGED_ATTRIB
	memcpy(MEMORY_mem + 0xa000, cart, 0x2000);
#else
	MEMORY_SetBank(0xa000, 0xbfff, rom_cartA0BF_read, MEMORY_ROM_PutByte);
#endif
}

static void SelectAtariXeBank(int cpu_bank, int new_cpu_bank)
{
#ifndef PAGED_ATTRIB
	memcpy(atarixe_memory + (cpu_bank << 14), MEMORY_mem + 0x4000, 0x4000);
	memcpy(MEMORY_mem + 0x4000, atarixe_memory + (new_cpu_bank << 14), 0x4000);
#else
	MEMORY_SetBank(0x4000, 0x7fff, atarixe_memory_read, atarixe_memory_write);
#endif
}

static void alloc_axlon_memory(void){
	if (MEMORY_axlon_num_banks > 0 && Atari800_machine_type == Atari800_MACHINE_800) {
		int size = MEMORY_axlon_num_banks * 0x4000;
		if (axlon_ram == NULL || axlon_current_bankmask != MEMORY_axlon_num_banks - 1) {
			axlon_current_bankmask = MEMORY_axlon_num_banks - 1;
			axlon_ram = (UBYTE *)Util_realloc(axlon_ram, size);
		}
		memset(axlon_ram, 0, size);
	} else {
		if (axlon_ram != NULL) {
			free(axlon_ram);
			axlon_ram = NULL;
			axlon_current_bankmask = 0;
		}
	}
}

static void alloc_mosaic_memory(void){
	if (MEMORY_mosaic_num_banks > 0 && Atari800_machine_type == Atari800_MACHINE_800) {
		int size = MEMORY_mosaic_num_banks * 0x1000;
		if (mosaic_ram == NULL || mosaic_current_num_banks != MEMORY_mosaic_num_banks) {
			mosaic_current_num_banks = MEMORY_mosaic_num_banks;
			mosaic_ram = (UBYTE *)Util_realloc(mosaic_ram, size);
		}
		memset(mosaic_ram, 0, size);
	} else {
		if (mosaic_ram != NULL) {
			free(mosaic_ram);
			mosaic_ram = NULL;
			mosaic_current_num_banks = 0;
		}
	}
}

static void AllocXEMemory(void)
{
	if (MEMORY_ram_size > 64) {
		/* don't count 64 KB of base memory */
		/* count number of 16 KB banks, add 1 for saving base memory 0x4000-0x7fff */
		ULONG size = (1 + (MEMORY_ram_size - 64) / 16) * 16384;
		if (size != atarixe_memory_size) {
			if (atarixe_memory != NULL)
				free(atarixe_memory);
			atarixe_memory = (UBYTE *) Util_malloc(size);
			atarixe_memory_size = size;
			memset(atarixe_memory, 0, size);
		}
	}
	/* atarixe_memory not needed, free it */
	else if (atarixe_memory != NULL) {
		free(atarixe_memory);
		atarixe_memory = NULL;
		atarixe_memory_size = 0;
	}
}

static void AllocMapRAM(void)
{
	if (MEMORY_enable_mapram && Atari800_machine_type == Atari800_MACHINE_XLXE
	    && MEMORY_ram_size > 20) {
		if (mapram_memory == NULL)
			mapram_memory = (UBYTE *)Util_malloc(0x800);
	}
	else if (mapram_memory != NULL) {
		free(mapram_memory);
		mapram_memory = NULL;
	}
}

int MEMORY_SizeValid(int size)
{
	return size == 8 || size == 16 || size == 24 || size == 32
	       || size == 40 || size == 48 || size == 52 || size == 64
	       || size == 128 || size == 192 || size == MEMORY_RAM_320_RAMBO
	       || size == MEMORY_RAM_320_COMPY_SHOP || size == 576 || size == 1088;
}

void MEMORY_InitialiseMachine(void)
{
	int const os_size = Atari800_machine_type == Atari800_MACHINE_800 ? 0x2800
	                    : Atari800_machine_type == Atari800_MACHINE_5200 ? 0x800
	                    : 0x4000;
	int const os_rom_start = 0x10000 - os_size;
	ANTIC_xe_ptr = NULL;
	cart809F_enabled = FALSE;
	MEMORY_cartA0BF_enabled = FALSE;
	if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
		GTIA_TRIG[3] = 0;
		if (GTIA_GRACTL & 4)
			GTIA_TRIG_latch[3] = 0;
	}
	memcpy(MEMORY_mem + os_rom_start, MEMORY_os, os_size);
	switch (Atari800_machine_type) {
	case Atari800_MACHINE_5200:
		MEMORY_dFillMem(0x0000, 0x00, 0xf800);
		MEMORY_SetRAM(0x0000, 0x3fff);
		MEMORY_SetROM(0x4000, 0xffff);
#ifndef PAGED_ATTRIB
		MEMORY_SetHARDWARE(0xc000, 0xcfff);	/* 5200 GTIA Chip */
		MEMORY_SetHARDWARE(0xd400, 0xd4ff);	/* 5200 ANTIC Chip */
		MEMORY_SetHARDWARE(0xe800, 0xefff);	/* 5200 POKEY Chip */
#else
		MEMORY_readmap[0xc0] = GTIA_GetByte;
		MEMORY_readmap[0xc1] = GTIA_GetByte;
		MEMORY_readmap[0xc2] = GTIA_GetByte;
		MEMORY_readmap[0xc3] = GTIA_GetByte;
		MEMORY_readmap[0xc4] = GTIA_GetByte;
		MEMORY_readmap[0xc5] = GTIA_GetByte;
		MEMORY_readmap[0xc6] = GTIA_GetByte;
		MEMORY_readmap[0xc7] = GTIA_GetByte;
		MEMORY_readmap[0xc8] = GTIA_GetByte;
		MEMORY_readmap[0xc9] = GTIA_GetByte;
		MEMORY_readmap[0xca] = GTIA_GetByte;
		MEMORY_readmap[0xcb] = GTIA_GetByte;
		MEMORY_readmap[0xcc] = GTIA_GetByte;
		MEMORY_readmap[0xcd] = GTIA_GetByte;
		MEMORY_readmap[0xce] = GTIA_GetByte;
		MEMORY_readmap[0xcf] = GTIA_GetByte;
		MEMORY_readmap[0xd4] = ANTIC_GetByte;
		MEMORY_readmap[0xe8] = POKEY_GetByte;
		MEMORY_readmap[0xe9] = POKEY_GetByte;
		MEMORY_readmap[0xea] = POKEY_GetByte;
		MEMORY_readmap[0xeb] = POKEY_GetByte;
		MEMORY_readmap[0xec] = POKEY_GetByte;
		MEMORY_readmap[0xed] = POKEY_GetByte;
		MEMORY_readmap[0xee] = POKEY_GetByte;
		MEMORY_readmap[0xef] = POKEY_GetByte;

		MEMORY_writemap[0xc0] = GTIA_PutByte;
		MEMORY_writemap[0xc1] = GTIA_PutByte;
		MEMORY_writemap[0xc2] = GTIA_PutByte;
		MEMORY_writemap[0xc3] = GTIA_PutByte;
		MEMORY_writemap[0xc4] = GTIA_PutByte;
		MEMORY_writemap[0xc5] = GTIA_PutByte;
		MEMORY_writemap[0xc6] = GTIA_PutByte;
		MEMORY_writemap[0xc7] = GTIA_PutByte;
		MEMORY_writemap[0xc8] = GTIA_PutByte;
		MEMORY_writemap[0xc9] = GTIA_PutByte;
		MEMORY_writemap[0xca] = GTIA_PutByte;
		MEMORY_writemap[0xcb] = GTIA_PutByte;
		MEMORY_writemap[0xcc] = GTIA_PutByte;
		MEMORY_writemap[0xcd] = GTIA_PutByte;
		MEMORY_writemap[0xce] = GTIA_PutByte;
		MEMORY_writemap[0xcf] = GTIA_PutByte;
		MEMORY_writemap[0xd4] = ANTIC_PutByte;
		MEMORY_writemap[0xe8] = POKEY_PutByte;
		MEMORY_writemap[0xe9] = POKEY_PutByte;
		MEMORY_writemap[0xea] = POKEY_PutByte;
		MEMORY_writemap[0xeb] = POKEY_PutByte;
		MEMORY_writemap[0xec] = POKEY_PutByte;
		MEMORY_writemap[0xed] = POKEY_PutByte;
		MEMORY_writemap[0xee] = POKEY_PutByte;
		MEMORY_writemap[0xef] = POKEY_PutByte;
#endif
		break;
	default:
		{
			int const base_ram = MEMORY_ram_size > 64 ? 64 * 1024 : MEMORY_ram_size * 1024;
			int const hole_end = (os_rom_start < 0xd000 ? os_rom_start : 0xd000);
			int const hole_start = base_ram > hole_end ? hole_end : base_ram;
			ESC_PatchOS();
			MEMORY_dFillMem(0x0000, 0x00, hole_start);
			MEMORY_SetRAM(0x0000, hole_start - 1);
			if (hole_start < hole_end) {
				MEMORY_dFillMem(hole_start, 0xff, hole_end - hole_start);
				MEMORY_SetROM(hole_start, hole_end - 1);
			}
			if (hole_end < 0xd000)
				MEMORY_SetROM(hole_end, 0xcfff);
			MEMORY_SetROM(0xd800, 0xffff);
#ifndef PAGED_ATTRIB
			MEMORY_SetHARDWARE(0xd000, 0xd7ff);
			if (Atari800_machine_type == Atari800_MACHINE_800) {
				if (MEMORY_mosaic_num_banks > 0) MEMORY_SetHARDWARE(0xff00, 0xffff);
				/* only 0xffc0-0xffff are used, but mark the whole
				 * page to make state saving easier */
				if (MEMORY_axlon_num_banks > 0) {
					MEMORY_SetHARDWARE(0xcf00, 0xcfff);
					if (MEMORY_axlon_0f_mirror) MEMORY_SetHARDWARE(0x0f00, 0x0fff);
					/* only ?fc0-?fff are used, but mark the whole page*/
				}
			}
#else
			MEMORY_readmap[0xd0] = GTIA_GetByte;
			MEMORY_readmap[0xd1] = PBI_D1GetByte;
			MEMORY_readmap[0xd2] = POKEY_GetByte;
			MEMORY_readmap[0xd3] = PIA_GetByte;
			MEMORY_readmap[0xd4] = ANTIC_GetByte;
			MEMORY_readmap[0xd5] = CARTRIDGE_GetByte;
			MEMORY_readmap[0xd6] = PBI_D6GetByte;
			MEMORY_readmap[0xd7] = PBI_D7GetByte;
			MEMORY_writemap[0xd0] = GTIA_PutByte;
			MEMORY_writemap[0xd1] = PBI_D1PutByte;
			MEMORY_writemap[0xd2] = POKEY_PutByte;
			MEMORY_writemap[0xd3] = PIA_PutByte;
			MEMORY_writemap[0xd4] = ANTIC_PutByte;
			MEMORY_writemap[0xd5] = CARTRIDGE_PutByte;
			MEMORY_writemap[0xd6] = PBI_D6PutByte;
			MEMORY_writemap[0xd7] = PBI_D7PutByte;
			if (Atari800_machine_type == Atari800_MACHINE_800) {
				if (MEMORY_mosaic_num_banks > 0) MEMORY_writemap[0xff] = MosaicPutByte;
				if (MEMORY_axlon_num_banks > 0) {
					MEMORY_writemap[0xcf] = AxlonPutByte;
					if (MEMORY_axlon_0f_mirror)
						MEMORY_writemap[0x0f] = AxlonPutByte;
				}
			}
#endif
		}
		break;
	}
	AllocXEMemory();
	alloc_axlon_memory();
	alloc_mosaic_memory();
	axlon_curbank = 0;
	mosaic_curbank = 0x3f;
	AllocMapRAM();
	Atari800_Coldstart();
}

#ifndef BASIC

void MEMORY_StateSave(UBYTE SaveVerbose)
{
	int temp;
	UBYTE byte;

	/* Axlon/Mosaic for 400/800 */
	if (Atari800_machine_type == Atari800_MACHINE_800) {
		StateSav_SaveINT(&MEMORY_axlon_num_banks, 1);
		if (MEMORY_axlon_num_banks > 0){
			StateSav_SaveINT(&axlon_curbank, 1);
			StateSav_SaveINT(&MEMORY_axlon_0f_mirror, 1);
			StateSav_SaveUBYTE(axlon_ram, MEMORY_axlon_num_banks * 0x4000);
		}
		StateSav_SaveINT(&mosaic_current_num_banks, 1);
		if (mosaic_current_num_banks > 0) {
			StateSav_SaveINT(&mosaic_curbank, 1);
			StateSav_SaveUBYTE(mosaic_ram, mosaic_current_num_banks * 0x1000);
		}
	}

	/* Save amount of base RAM in kilobytes. */
	temp = MEMORY_ram_size > 64 ? 64 : MEMORY_ram_size;
	StateSav_SaveINT(&temp, 1);
	StateSav_SaveUBYTE(&MEMORY_mem[0], 65536);
#ifndef PAGED_ATTRIB
	StateSav_SaveUBYTE(&MEMORY_attrib[0], 65536);
#else
	{
		/* I assume here that consecutive calls to StateSav_SaveUBYTE()
		   are equivalent to a single call with all the values
		   (i.e. StateSav_SaveUBYTE() doesn't write any headers). */
		UBYTE attrib_page[256];
		int i;
		for (i = 0; i < 256; i++) {
			if (MEMORY_writemap[i] == NULL)
				memset(attrib_page, MEMORY_RAM, 256);
			else if (MEMORY_writemap[i] == MEMORY_ROM_PutByte)
				memset(attrib_page, MEMORY_ROM, 256);
			else if (i == 0x4f || i == 0x5f || i == 0x8f || i == 0x9f) {
				/* special case: Bounty Bob bank switching registers */
				memset(attrib_page, MEMORY_ROM, 256);
				attrib_page[0xf6] = MEMORY_HARDWARE;
				attrib_page[0xf7] = MEMORY_HARDWARE;
				attrib_page[0xf8] = MEMORY_HARDWARE;
				attrib_page[0xf9] = MEMORY_HARDWARE;
			}
			else {
				memset(attrib_page, MEMORY_HARDWARE, 256);
			}
			StateSav_SaveUBYTE(&attrib_page[0], 256);
		}
	}
#endif

	if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
		if (SaveVerbose != 0)
			StateSav_SaveUBYTE(&MEMORY_basic[0], 8192);
#ifndef PAGED_ATTRIB
		StateSav_SaveUBYTE(&under_cartA0BF[0], 8192);
#endif
		if (SaveVerbose != 0)
			StateSav_SaveUBYTE(&MEMORY_os[0], 16384);
#ifndef PAGED_ATTRIB
		StateSav_SaveUBYTE(&under_atarixl_os[0], 16384);
#endif
		if (SaveVerbose != 0)
			StateSav_SaveUBYTE(MEMORY_xegame, 0x2000);
	}

	/* Save amount of XE RAM in 16KB banks. */
	temp = (MEMORY_ram_size - 64) / 16;
	if (temp < 0)
		temp = 0;
	StateSav_SaveINT(&temp, 1);
	if (MEMORY_ram_size == MEMORY_RAM_320_RAMBO || MEMORY_ram_size == MEMORY_RAM_320_COMPY_SHOP) {
		/* Save specific banking type. */
		temp = MEMORY_ram_size - 320;
		StateSav_SaveINT(&temp, 1);
	}
	byte = PIA_PORTB | PIA_PORTB_mask;
	StateSav_SaveUBYTE(&byte, 1);

	StateSav_SaveINT(&MEMORY_cartA0BF_enabled, 1);

	if (MEMORY_ram_size > 64) {
		StateSav_SaveUBYTE(&atarixe_memory[0], atarixe_memory_size);
		if (ANTIC_xe_ptr != NULL && MEMORY_selftest_enabled)
			StateSav_SaveUBYTE(antic_bank_under_selftest, 0x800);
	}

	/* Simius XL/XE MapRAM expansion */
	if (Atari800_machine_type == Atari800_MACHINE_XLXE && MEMORY_ram_size > 20) {
		StateSav_SaveINT(&MEMORY_enable_mapram, 1);
		if (MEMORY_enable_mapram) {
			StateSav_SaveUBYTE( mapram_memory, 0x800 );
		}
	}
}

void MEMORY_StateRead(UBYTE SaveVerbose, UBYTE StateVersion)
{
	int base_ram_kb;
	int num_xe_banks;
	UBYTE portb;

	/* Axlon/Mosaic for 400/800 */
	if (Atari800_machine_type == Atari800_MACHINE_800 && StateVersion >= 5) {
		StateSav_ReadINT(&MEMORY_axlon_num_banks, 1);
		if (MEMORY_axlon_num_banks > 0){
			StateSav_ReadINT(&axlon_curbank, 1);
			if (StateVersion < 7) {
				/* Read bank mask, then increase by 1 to get number of banks. */
				StateSav_ReadINT(&MEMORY_axlon_num_banks, 1);
				++ MEMORY_axlon_num_banks;
			}
			StateSav_ReadINT(&MEMORY_axlon_0f_mirror, 1);
			if (StateVersion < 7) {
				int temp;
				/* Ignore saved RAM size - can be derived. */
				StateSav_ReadINT(&temp, 1);
			}
			alloc_axlon_memory();
			StateSav_ReadUBYTE(axlon_ram, MEMORY_axlon_num_banks * 0x4000);
		}
		StateSav_ReadINT(&MEMORY_mosaic_num_banks, 1);
		if (MEMORY_mosaic_num_banks > 0) {
			StateSav_ReadINT(&mosaic_curbank, 1);
			if (StateVersion < 7) {
				int temp;
				/* Read max bank number, then increase by 1 to get number of banks. */
				StateSav_ReadINT(&MEMORY_mosaic_num_banks, 1);
				++ MEMORY_mosaic_num_banks;
				StateSav_ReadINT(&temp, 1); /* Ignore Mosaic RAM size - can be derived. */
			}
			alloc_mosaic_memory();
			StateSav_ReadUBYTE(mosaic_ram, mosaic_current_num_banks * 0x1000);
		}
	}

	if (StateVersion >= 7)
		/* Read amount of base RAM in kilobytes. */
		StateSav_ReadINT(&base_ram_kb, 1);
	StateSav_ReadUBYTE(&MEMORY_mem[0], 65536);
#ifndef PAGED_ATTRIB
	StateSav_ReadUBYTE(&MEMORY_attrib[0], 65536);
#else
	{
		UBYTE attrib_page[256];
		int i;
		for (i = 0; i < 256; i++) {
			StateSav_ReadUBYTE(&attrib_page[0], 256);
			/* note: 0x40 is intentional here:
			   we want ROM on page 0xd1 if H: patches are enabled */
			switch (attrib_page[0x40]) {
			case MEMORY_RAM:
				MEMORY_readmap[i] = NULL;
				MEMORY_writemap[i] = NULL;
				break;
			case MEMORY_ROM:
				if (i != 0xd1 && attrib_page[0xf6] == MEMORY_HARDWARE) {
					if (i == 0x4f || i == 0x8f) {
						MEMORY_readmap[i] = CARTRIDGE_BountyBob1GetByte;
						MEMORY_writemap[i] = CARTRIDGE_BountyBob1PutByte;
					}
					else if (i == 0x5f || i == 0x9f) {
						MEMORY_readmap[i] = CARTRIDGE_BountyBob2GetByte;
						MEMORY_writemap[i] = CARTRIDGE_BountyBob2PutByte;
					}
					/* else something's wrong, so we keep current values */
				}
				else {
					MEMORY_readmap[i] = NULL;
					MEMORY_writemap[i] = MEMORY_ROM_PutByte;
				}
				break;
			case MEMORY_HARDWARE:
				switch (i) {
				case 0xc0:
				case 0xd0:
					MEMORY_readmap[i] = GTIA_GetByte;
					MEMORY_writemap[i] = GTIA_PutByte;
					break;
				case 0xd1:
					MEMORY_readmap[i] = PBI_D1GetByte;
					MEMORY_writemap[i] = PBI_D1PutByte;
					break;
				case 0xd2:
				case 0xe8:
				case 0xeb:
					MEMORY_readmap[i] = POKEY_GetByte;
					MEMORY_writemap[i] = POKEY_PutByte;
					break;
				case 0xd3:
					MEMORY_readmap[i] = PIA_GetByte;
					MEMORY_writemap[i] = PIA_PutByte;
					break;
				case 0xd4:
					MEMORY_readmap[i] = ANTIC_GetByte;
					MEMORY_writemap[i] = ANTIC_PutByte;
					break;
				case 0xd5:
					MEMORY_readmap[i] = CARTRIDGE_GetByte;
					MEMORY_writemap[i] = CARTRIDGE_PutByte;
					break;
				case 0xd6:
					MEMORY_readmap[i] = PBI_D6GetByte;
					MEMORY_writemap[i] = PBI_D6PutByte;
					break;
				case 0xd7:
					MEMORY_readmap[i] = PBI_D7GetByte;
					MEMORY_writemap[i] = PBI_D7PutByte;
					break;
				case 0xff:
					if (MEMORY_mosaic_num_banks > 0) MEMORY_writemap[0xff] = MosaicPutByte;
					break;
				case 0xcf:
					if (MEMORY_axlon_num_banks > 0) MEMORY_writemap[0xcf] = AxlonPutByte;
					break;
				case 0x0f:
					if (MEMORY_axlon_num_banks > 0 && MEMORY_axlon_0f_mirror) MEMORY_writemap[0x0f] = AxlonPutByte;
					break;
				default:
					/* something's wrong, so we keep current values */
					break;
				}
				break;
			default:
				/* something's wrong, so we keep current values */
				break;
			}
		}
	}
#endif

	if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
		if (SaveVerbose)
			StateSav_ReadUBYTE(&MEMORY_basic[0], 8192);
#ifndef PAGED_ATTRIB
		StateSav_ReadUBYTE(&under_cartA0BF[0], 8192);
#endif

		if (SaveVerbose)
			StateSav_ReadUBYTE(&MEMORY_os[0], 16384);
#ifndef PAGED_ATTRIB
		StateSav_ReadUBYTE(&under_atarixl_os[0], 16384);
#endif
		if (StateVersion >= 7 && SaveVerbose)
			StateSav_ReadUBYTE(MEMORY_xegame, 0x2000);
	}

	if (StateVersion >= 7) {
		/* Read amount of XE RAM in 16KB banks. */
		StateSav_ReadINT(&num_xe_banks, 1);
		/* Compute value of MEMORY_ram_size. */
		MEMORY_ram_size = base_ram_kb + num_xe_banks * 16;
		if (MEMORY_ram_size == 320) {
			/* There are 2 different memory mappings for 320 KB. */
			/* In savestate version <= 6 this variable is read in PIA_StateRead. */
			int xe_type;
			StateSav_ReadINT(&xe_type, 1);
			MEMORY_ram_size += xe_type;
		}
		if (!MEMORY_SizeValid(MEMORY_ram_size)) {
			MEMORY_ram_size = 64;
			Log_print("Warning: Bad RAM size read in from state save, defaulting to 64 KB");
		}

		/* Read PORTB and set variables that are based on it. */
		StateSav_ReadUBYTE(&portb, 1);
		MEMORY_xe_bank = 0;
		if (MEMORY_ram_size > 64 && (portb & 0x30) != 0x30) {
			switch (MEMORY_ram_size) {
			case 128:
				MEMORY_xe_bank = ((portb & 0x0c) >> 2) + 1;
				break;
			case 192:
				MEMORY_xe_bank = (((portb & 0x0c) + ((portb & 0x40) >> 2)) >> 2) + 1;
				break;
			case MEMORY_RAM_320_RAMBO:
				MEMORY_xe_bank = (((portb & 0x0c) + ((portb & 0x60) >> 1)) >> 2) + 1;
				break;
			case MEMORY_RAM_320_COMPY_SHOP:
				MEMORY_xe_bank = (((portb & 0x0c) + ((portb & 0xc0) >> 2)) >> 2) + 1;
				break;
			case 576:
				MEMORY_xe_bank = (((portb & 0x0e) + ((portb & 0x60) >> 1)) >> 1) + 1;
				break;
			case 1088:
				MEMORY_xe_bank = (((portb & 0x0e) + ((portb & 0xe0) >> 1)) >> 1) + 1;
				break;
			}
		}
		/* In savestate version <= 6 this variable is read in PIA_StateRead. */
		MEMORY_selftest_enabled = (portb & 0x81) == 0x01
		                          && !((portb & 0x30) != 0x30 && MEMORY_ram_size == MEMORY_RAM_320_COMPY_SHOP)
		                          && !((portb & 0x10) == 0 && MEMORY_ram_size == 1088);

		StateSav_ReadINT(&MEMORY_cartA0BF_enabled, 1);
		if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
			GTIA_TRIG[3] = MEMORY_cartA0BF_enabled;
			if (MEMORY_cartA0BF_enabled == 0 && (GTIA_GRACTL & 4))
				GTIA_TRIG_latch[3] = 0;
		}
	}
	ANTIC_xe_ptr = NULL;
	AllocXEMemory();
	if (MEMORY_ram_size > 64) {
		StateSav_ReadUBYTE(&atarixe_memory[0], atarixe_memory_size);
		/* a hack that makes state files compatible with previous versions:
		   for 130 XE there's written 192 KB of unused data */
		if (MEMORY_ram_size == 128 && StateVersion <= 6) {
			UBYTE buffer[256];
			int i;
			for (i = 0; i < 192 * 4; i++)
				StateSav_ReadUBYTE(&buffer[0], 256);
		}
		if (StateVersion >= 7 && (MEMORY_ram_size == 128 || MEMORY_ram_size == MEMORY_RAM_320_COMPY_SHOP)) {
			switch (portb & 0x30) {
			case 0x20:	/* ANTIC: base, CPU: extended */
				ANTIC_xe_ptr = atarixe_memory;
				break;
			case 0x10:	/* ANTIC: extended, CPU: base */
				ANTIC_xe_ptr = atarixe_memory + (MEMORY_xe_bank << 14);
				break;
			default:	/* ANTIC same as CPU */
				ANTIC_xe_ptr = NULL;
				break;
			}

			if (ANTIC_xe_ptr != NULL && MEMORY_selftest_enabled)
				/* Also read ANTIC-visible memory shadowed by Self Test. */
				StateSav_ReadUBYTE(antic_bank_under_selftest, 0x800);

		}
	}

	/* Simius XL/XE MapRAM expansion */
	if (StateVersion >= 7 && Atari800_machine_type == Atari800_MACHINE_XLXE && MEMORY_ram_size > 20) {
		StateSav_ReadINT(&MEMORY_enable_mapram, 1);
		AllocMapRAM();
		if (mapram_memory != NULL) {
			StateSav_ReadUBYTE(mapram_memory, 0x800);
		}
	}
}

#endif /* BASIC */

void MEMORY_CopyFromMem(UWORD from, UBYTE *to, int size)
{
	while (--size >= 0) {
		*to++ = MEMORY_GetByte(from);
		from++;
	}
}

void MEMORY_CopyToMem(const UBYTE *from, UWORD to, int size)
{
	while (--size >= 0) {
		MEMORY_PutByte(to, *from);
		from++;
		to++;
	}
}


/* Returns NULL if both builtin BASIC and XEGS game are disabled.
   Otherwise returns a pointer to an 8KB array containing either
   BASIC or XEGS game ROM contents. */
static UBYTE const * builtin_cart(UBYTE portb)
{
	/* Normally BASIC is enabled by clearing bit 1 of PORTB, but it's disabled
	   when using 576K and 1088K memory expansions, where bit 1 is used for
	   selecting extended memory bank number. */
	if (Atari800_builtin_basic
	    && (portb & 0x02) == 0
	    && ((portb & 0x10) != 0 || (MEMORY_ram_size != 576 && MEMORY_ram_size != 1088)))
		return MEMORY_basic;
	/* The builtin XEGS game is disabled when BASIC is enabled. It is enabled
	   by setting bit 6 of PORTB, but it's disabled when using 320K and larger
	   XE memory expansions, where bit 6 is used for selecting extended memory
	   bank number. */
	if (Atari800_builtin_game
	    && (portb & 0x40) == 0
	    && ((portb & 0x10) != 0 || MEMORY_ram_size < 320))
		return MEMORY_xegame;
	return NULL;
}

/* Note: this function is only for XL/XE! */
void MEMORY_HandlePORTB(UBYTE byte, UBYTE oldval)
{
	int antic_bank = 0;
	int mapram_selected = FALSE;
	int new_mapram_selected = FALSE;

	/* MapRAM is selected if RAM > 20 KB, Self Test is enabled while OS ROM is disabled,
	   and both CPU & ANTIC have access to base RAM. */
	if (mapram_memory != NULL && MEMORY_ram_size > 20) {
		mapram_selected = (oldval & 0xb1) == 0x30;
		new_mapram_selected = (byte & 0xb1) == 0x30;
	}

	if (mapram_selected && !new_mapram_selected) {
		DisableMapRAM();
	}

	/* Switch XE memory bank in 0x4000-0x7fff */
	if (MEMORY_ram_size > 64) {
		int bank = 0;
		int cpu_bank, new_cpu_bank, new_antic_bank;
		/* bank = 0 : base RAM */
		/* bank = 1..64 : extended RAM */
		if ((byte & 0x30) != 0x30)
			switch (MEMORY_ram_size) {
			case 128:
				bank = ((byte & 0x0c) >> 2) + 1;
				break;
			case 192:
				bank = (((byte & 0x0c) + ((byte & 0x40) >> 2)) >> 2) + 1;
				break;
			case MEMORY_RAM_320_RAMBO:
				bank = (((byte & 0x0c) + ((byte & 0x60) >> 1)) >> 2) + 1;
				break;
			case MEMORY_RAM_320_COMPY_SHOP:
				bank = (((byte & 0x0c) + ((byte & 0xc0) >> 2)) >> 2) + 1;
				break;
			case 576:
				bank = (((byte & 0x0e) + ((byte & 0x60) >> 1)) >> 1) + 1;
				break;
			case 1088:
				bank = (((byte & 0x0e) + ((byte & 0xe0) >> 1)) >> 1) + 1;
				break;
			}
		cpu_bank = (oldval & 0x10) ? 0 : MEMORY_xe_bank;
		new_cpu_bank = (byte & 0x10) ? 0 : bank;
		antic_bank = (oldval & 0x20) ? 0 : MEMORY_xe_bank;
		new_antic_bank = (byte & 0x20) ? 0 : bank;

		/* Note: in Compy Shop bit 5 (ANTIC access) disables Self Test */
		if (MEMORY_selftest_enabled
		    && (cpu_bank != new_cpu_bank
		        || antic_bank != new_antic_bank
		        || (MEMORY_ram_size == MEMORY_RAM_320_COMPY_SHOP && (byte & 0x20) == 0))) {
			DisableSelfTestROM(antic_bank);
		}
		if (cpu_bank != new_cpu_bank) {
			SelectAtariXeBank(cpu_bank, new_cpu_bank);
		}

		if (MEMORY_ram_size == 128 || MEMORY_ram_size == MEMORY_RAM_320_COMPY_SHOP)
			ANTIC_xe_ptr = new_antic_bank == new_cpu_bank ? NULL : atarixe_memory + (new_antic_bank << 14);

		MEMORY_xe_bank = bank;
		antic_bank = new_antic_bank;
	}

	/* Enable/disable OS ROM in 0xc000-0xcfff and 0xd800-0xffff */
	if ((oldval ^ byte) & 0x01) {
		if (byte & 0x01) {
			EnableOsROM();
		}
		else {
			DisableOsROM();
			/* When OS ROM is disabled we also have to disable Self Test - Jindroush */
			if (MEMORY_selftest_enabled) {
				DisableSelfTestROM(antic_bank);
			}
		}
	}

	/* Enable/disable BASIC/game ROM in 0xa000-0xbfff */
	if (!MEMORY_cartA0BF_enabled) {
		UBYTE const *builtin_cart_new = builtin_cart(byte);
		UBYTE const *builtin_cart_old = builtin_cart(oldval);
		if (builtin_cart_old != builtin_cart_new) {
			if (builtin_cart_new != NULL) {
				EnableCartROM(builtin_cart_new, builtin_cart_old != NULL);
			}
			else {
				DisableCartROM();
			}
#ifdef PAGED_ATTRIB
			cartA0BF = builtin_cart_new;
#endif
		}
	}

	/* Enable/disable Self Test ROM in 0x5000-0x57ff */
	if (byte & 0x80) {
		if (MEMORY_selftest_enabled) {
			DisableSelfTestROM(antic_bank);
		}
	}
	else {
		/* We can enable Self Test only if the OS ROM is enabled */
		/* and we're not accessing extended 320K Compy Shop or 1088K memory */
		/* Note: in Compy Shop bit 5 (ANTIC access) disables Self Test */
		if (!MEMORY_selftest_enabled && (byte & 0x01)
			&& !((byte & 0x30) != 0x30 && MEMORY_ram_size == MEMORY_RAM_320_COMPY_SHOP)
			&& !((byte & 0x10) == 0 && MEMORY_ram_size == 1088)) {
			EnableSelfTestROM(antic_bank);
		}
		else if (!mapram_selected && new_mapram_selected) {
			EnableMapRAM();
		}
	}
}

/* Mosaic banking scheme: writing to 0xffc0+<n> selects ram bank <n>, if
 * that is past the last available bank, selects rom.  Banks are 4k,
 * located at 0xc000-0xcfff.  Tested: Rambrandt (drawing program), Topdos1.5.
 * Reverse engineered from software that uses it.  May be incorrect in some
 * details.  Unknown:  were there mirrors of the bank addresses?  Was the RAM
 * enabled at coldstart? Did the Mosaic home-bank on reset?
 * The Topdos 1.5 manual has some information.
 */
static void MosaicPutByte(UWORD addr, UBYTE byte)
{
	int newbank;
	if (addr < 0xffc0) return;
#ifdef DEBUG
	Log_print("MosaicPutByte:%4X:%2X",addr,byte);
#endif
	newbank = addr - 0xffc0;
	if (newbank == mosaic_curbank || (newbank >= mosaic_current_num_banks && mosaic_curbank >= mosaic_current_num_banks)) return; /*same bank or rom -> rom*/
	if (newbank >= mosaic_current_num_banks && mosaic_curbank < mosaic_current_num_banks) {
		/*ram ->rom*/
		memcpy(mosaic_ram + mosaic_curbank*0x1000, MEMORY_mem + 0xc000,0x1000);
		MEMORY_dFillMem(0xc000, 0xff, 0x1000);
		MEMORY_SetROM(0xc000, 0xcfff);
	}
	else if (newbank < mosaic_current_num_banks && mosaic_curbank >= mosaic_current_num_banks) {
		/*rom->ram*/
		memcpy(MEMORY_mem + 0xc000, mosaic_ram+newbank*0x1000,0x1000);
		MEMORY_SetRAM(0xc000, 0xcfff);
	}
	else {
		/*ram -> ram*/
		memcpy(mosaic_ram + mosaic_curbank*0x1000, MEMORY_mem + 0xc000, 0x1000);
		memcpy(MEMORY_mem + 0xc000, mosaic_ram + newbank*0x1000, 0x1000);
		MEMORY_SetRAM(0xc000, 0xcfff);
	}
	mosaic_curbank = newbank;
}

static UBYTE MosaicGetByte(UWORD addr, int no_side_effects)
{
#ifdef DEBUG
	Log_print("MosaicGetByte%4X",addr);
#endif
	return MEMORY_mem[addr];
}

/* Axlon banking scheme: writing <n> to 0xcfc0-0xcfff selects a bank.  The Axlon
 * used 3 bits, giving 8 banks.  Extended versions were constructed that
 * used additional bits, for up to 256 banks.  Banks were 16k, at 0x4000-0x7fff.
 * The total ram was 32+16*numbanks k.  The Axlon did homebank on reset,
 * compatibles did not.  The Axlon had a shadow address at 0x0fc0-0x0fff.
 * A possible explaination for the shadow address is that it allowed the
 * Axlon to work in any 800 slot due to a hardware limitation.
 * The shadow address could cause compatibility problems.  The compatibles
 * did not implement that shadow address.
 * Source: comp.sys.atari.8bit postings, Andreas Magenheimer's FAQ
 */
static void AxlonPutByte(UWORD addr, UBYTE byte)
{
	int newbank;
	/*Write-through to RAM if it is the page 0x0f shadow*/
	if ((addr&0xff00) == 0x0f00) MEMORY_mem[addr] = byte;
	if ((addr&0xff) < 0xc0) return; /*0xffc0-0xffff and 0x0fc0-0x0fff only*/
#ifdef DEBUG
	Log_print("AxlonPutByte:%4X:%2X", addr, byte);
#endif
	newbank = (byte&axlon_current_bankmask);
	if (newbank == axlon_curbank) return;
	memcpy(axlon_ram + axlon_curbank*0x4000, MEMORY_mem + 0x4000, 0x4000);
	memcpy(MEMORY_mem + 0x4000, axlon_ram + newbank*0x4000, 0x4000);
	axlon_curbank = newbank;
}

static UBYTE AxlonGetByte(UWORD addr, int no_side_effects)
{
#ifdef DEBUG
	Log_print("AxlonGetByte%4X",addr);
#endif
	return MEMORY_mem[addr];
}

void MEMORY_Cart809fDisable(void)
{
	if (cart809F_enabled) {
		if (MEMORY_ram_size > 32) {
			memcpy(MEMORY_mem + 0x8000, under_cart809F, 0x2000);
			MEMORY_SetRAM(0x8000, 0x9fff);
		}
		else
			MEMORY_dFillMem(0x8000, 0xff, 0x2000);
		cart809F_enabled = FALSE;
	}
}

void MEMORY_Cart809fEnable(void)
{
	if (!cart809F_enabled) {
		if (MEMORY_ram_size > 32) {
			memcpy(under_cart809F, MEMORY_mem + 0x8000, 0x2000);
			MEMORY_SetROM(0x8000, 0x9fff);
		}
		cart809F_enabled = TRUE;
	}
}

void MEMORY_CartA0bfDisable(void)
{
	if (MEMORY_cartA0BF_enabled) {
		/* No BASIC if not XL/XE or bit 1 of PORTB set */
		/* or accessing extended 576K or 1088K memory */
		if (MEMORY_ram_size > 40 && builtin_cart(PIA_PORTB | PIA_PORTB_mask) == NULL) {
#ifndef PAGED_ATTRIB
			memcpy(MEMORY_mem + 0xa000, under_cartA0BF, 0x2000);
#endif
			MEMORY_SetRAM(0xa000, 0xbfff);
		}
		else
			MEMORY_dFillMem(0xa000, 0xff, 0x2000);

		MEMORY_cartA0BF_enabled = FALSE;
		if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
			GTIA_TRIG[3] = 0;
			if (GTIA_GRACTL & 4)
				GTIA_TRIG_latch[3] = 0;
		}
	}
}

void MEMORY_CartA0bfEnable(void)
{
	if (!MEMORY_cartA0BF_enabled) {
		/* No BASIC if not XL/XE or bit 1 of PORTB set */
		/* or accessing extended 576K or 1088K memory */
		if (MEMORY_ram_size > 40 && builtin_cart(PIA_PORTB | PIA_PORTB_mask) == NULL) {
#ifndef PAGED_ATTRIB
			/* Back-up 0xa000-0xbfff RAM */
			memcpy(under_cartA0BF, MEMORY_mem + 0xa000, 0x2000);
#endif
			MEMORY_SetROM(0xa000, 0xbfff);
		}
		MEMORY_cartA0BF_enabled = TRUE;
		if (Atari800_machine_type == Atari800_MACHINE_XLXE)
			GTIA_TRIG[3] = 1;
	}
}

void MEMORY_GetCharset(UBYTE *cs)
{
	/* copy font, but change screencode order to ATASCII order */
	memcpy(cs, ROM_altirra_5200_os + 0x200, 0x100); /* control chars */
	memcpy(cs + 0x100, ROM_altirra_5200_os, 0x200); /* !"#$..., uppercase letters */
	memcpy(cs + 0x300, ROM_altirra_5200_os + 0x300, 0x100); /* lowercase letters */
}

#ifndef PAGED_MEM
UBYTE MEMORY_HwGetByte(UWORD addr, int no_side_effects)
{
	UBYTE byte = 0xff;
	switch (addr & 0xff00) {
	case 0x4f00:
	case 0x8f00:
		if (!no_side_effects)
			CARTRIDGE_BountyBob1(addr);
		byte = 0;
		break;
	case 0x5f00:
	case 0x9f00:
		if (!no_side_effects)
			CARTRIDGE_BountyBob2(addr);
		byte = 0;
		break;
	case 0xd000:				/* GTIA */
	case 0xc000:				/* GTIA - 5200 */
	case 0xc100:				/* GTIA - 5200 */
	case 0xc200:				/* GTIA - 5200 */
	case 0xc300:				/* GTIA - 5200 */
	case 0xc400:				/* GTIA - 5200 */
	case 0xc500:				/* GTIA - 5200 */
	case 0xc600:				/* GTIA - 5200 */
	case 0xc700:				/* GTIA - 5200 */
	case 0xc800:				/* GTIA - 5200 */
	case 0xc900:				/* GTIA - 5200 */
	case 0xca00:				/* GTIA - 5200 */
	case 0xcb00:				/* GTIA - 5200 */
	case 0xcc00:				/* GTIA - 5200 */
	case 0xcd00:				/* GTIA - 5200 */
	case 0xce00:				/* GTIA - 5200 */
		byte = GTIA_GetByte(addr, no_side_effects);
		break;
	case 0xd200:				/* POKEY */
	case 0xe800:				/* POKEY - 5200 */
	case 0xe900:				/* POKEY - 5200 */
	case 0xea00:				/* POKEY - 5200 */
	case 0xeb00:				/* POKEY - 5200 */
	case 0xec00:				/* POKEY - 5200 */
	case 0xed00:				/* POKEY - 5200 */
	case 0xee00:				/* POKEY - 5200 */
	case 0xef00:				/* POKEY - 5200 */
		byte = POKEY_GetByte(addr, no_side_effects);
		break;
	case 0xd300:				/* PIA */
		byte = PIA_GetByte(addr, no_side_effects);
		break;
	case 0xd400:				/* ANTIC */
		byte = ANTIC_GetByte(addr, no_side_effects);
		break;
	case 0xd500:				/* bank-switching cartridges, RTIME-8 */
		byte = CARTRIDGE_GetByte(addr, no_side_effects);
		break;
	case 0xff00:				/* Mosaic memory expansion for 400/800 */
		byte = MosaicGetByte(addr, no_side_effects);
		break;
	case 0xcf00:				/* Axlon memory expansion for 800 */
	case 0x0f00:				/* Axlon shadow */
		if (Atari800_machine_type == Atari800_MACHINE_5200) {
			byte = GTIA_GetByte(addr, no_side_effects); /* GTIA-5200 cfxx */
		}
		else {
			byte = AxlonGetByte(addr, no_side_effects);
		}
		break;
	case 0xd100:				/* PBI page D1 */
		byte = PBI_D1GetByte(addr, no_side_effects);
		break;
	case 0xd600:				/* PBI page D6 */
		byte = PBI_D6GetByte(addr, no_side_effects);
		break;
	case 0xd700:				/* PBI page D7 */
		byte = PBI_D7GetByte(addr, no_side_effects);
		break;
	default:
		break;
	}

	return byte;
}

void MEMORY_HwPutByte(UWORD addr, UBYTE byte)
{
	switch (addr & 0xff00) {
	case 0x4f00:
	case 0x8f00:
		CARTRIDGE_BountyBob1(addr);
		break;
	case 0x5f00:
	case 0x9f00:
		CARTRIDGE_BountyBob2(addr);
		break;
	case 0xd000:				/* GTIA */
	case 0xc000:				/* GTIA - 5200 */
	case 0xc100:				/* GTIA - 5200 */
	case 0xc200:				/* GTIA - 5200 */
	case 0xc300:				/* GTIA - 5200 */
	case 0xc400:				/* GTIA - 5200 */
	case 0xc500:				/* GTIA - 5200 */
	case 0xc600:				/* GTIA - 5200 */
	case 0xc700:				/* GTIA - 5200 */
	case 0xc800:				/* GTIA - 5200 */
	case 0xc900:				/* GTIA - 5200 */
	case 0xca00:				/* GTIA - 5200 */
	case 0xcb00:				/* GTIA - 5200 */
	case 0xcc00:				/* GTIA - 5200 */
	case 0xcd00:				/* GTIA - 5200 */
	case 0xce00:				/* GTIA - 5200 */
		GTIA_PutByte(addr, byte);
		break;
	case 0xd200:				/* POKEY */
	case 0xe800:				/* POKEY - 5200 */
	case 0xe900:				/* POKEY - 5200 */
	case 0xea00:				/* POKEY - 5200 */
	case 0xeb00:				/* POKEY - 5200 */
	case 0xec00:				/* POKEY - 5200 */
	case 0xed00:				/* POKEY - 5200 */
	case 0xee00:				/* POKEY - 5200 */
	case 0xef00:				/* POKEY - 5200 */
		POKEY_PutByte(addr, byte);
		break;
	case 0xd300:				/* PIA */
		PIA_PutByte(addr, byte);
		break;
	case 0xd400:				/* ANTIC */
		ANTIC_PutByte(addr, byte);
		break;
	case 0xd500:				/* bank-switching cartridges, RTIME-8 */
		CARTRIDGE_PutByte(addr, byte);
		break;
	case 0xff00:				/* Mosaic memory expansion for 400/800 */
		MosaicPutByte(addr,byte);
		break;
	case 0xcf00:				/* Axlon memory expansion for 800 */
	case 0x0f00:				/* Axlon shadow */
		if (Atari800_machine_type == Atari800_MACHINE_5200) {
			GTIA_PutByte(addr, byte); /* GTIA-5200 cfxx */
		}
		else {
			AxlonPutByte(addr,byte);
		}
		break;
	case 0xd100:				/* PBI page D1 */
		PBI_D1PutByte(addr, byte);
		break;
	case 0xd600:				/* PBI page D6 */
		PBI_D6PutByte(addr, byte);
		break;
	case 0xd700:				/* PBI page D7 */
		PBI_D7PutByte(addr, byte);
		break;
	default:
		break;
	}
}
#endif /* PAGED_MEM */
