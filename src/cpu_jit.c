/*
 * cpu_jit.c - 6502 CPU emulation using a JIT compiler
 *
 * Copyright (C) 2017 Atari800 development team (see DOC/CREDITS)
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

/*
 * ./configure --host=m68k-atari-mint --enable-jit-compiler --target=falcon --with-sound=no --disable-monitorbreak --disable-crashmenu
 */

#include "config.h"

#include "cpu_jit.h"
#if !defined(__m68k__) || defined(__mcoldfire__)
#error The JIT compiler is not implemented for this platform
#endif

#include <assert.h>
#include <stdlib.h>

#include "antic.h"
#include "cpu.h"
#include "log.h"
#include "memory.h"
#include "util.h"

extern void CPU_JIT_Instance(UBYTE *dst_buf, const struct CPU_JIT_insn_template_t *src_template, UWORD data, int bytes, int cycles);
extern void CPU_JIT_Execute(UBYTE *pCode);

static const struct CPU_JIT_insn_template_t* const JIT_compiler_insn_table[256] = {
	&JIT_insn_opcode_00, &JIT_insn_opcode_01, &JIT_insn_opcode_02, &JIT_insn_opcode_03,
	&JIT_insn_opcode_04, &JIT_insn_opcode_05, &JIT_insn_opcode_06, &JIT_insn_opcode_07,
	&JIT_insn_opcode_08, &JIT_insn_opcode_09, &JIT_insn_opcode_0a, &JIT_insn_opcode_0b,
	&JIT_insn_opcode_0c, &JIT_insn_opcode_0d, &JIT_insn_opcode_0e, &JIT_insn_opcode_0f,

	&JIT_insn_opcode_10, &JIT_insn_opcode_11, &JIT_insn_opcode_12, &JIT_insn_opcode_13,
	&JIT_insn_opcode_14, &JIT_insn_opcode_15, &JIT_insn_opcode_16, &JIT_insn_opcode_17,
	&JIT_insn_opcode_18, &JIT_insn_opcode_19, &JIT_insn_opcode_1a, &JIT_insn_opcode_1b,
	&JIT_insn_opcode_1c, &JIT_insn_opcode_1d, &JIT_insn_opcode_1e, &JIT_insn_opcode_1f,

	&JIT_insn_opcode_20, &JIT_insn_opcode_21, &JIT_insn_opcode_22, &JIT_insn_opcode_23,
	&JIT_insn_opcode_24, &JIT_insn_opcode_25, &JIT_insn_opcode_26, &JIT_insn_opcode_27,
	&JIT_insn_opcode_28, &JIT_insn_opcode_29, &JIT_insn_opcode_2a, &JIT_insn_opcode_2b,
	&JIT_insn_opcode_2c, &JIT_insn_opcode_2d, &JIT_insn_opcode_2e, &JIT_insn_opcode_2f,

	&JIT_insn_opcode_30, &JIT_insn_opcode_31, &JIT_insn_opcode_32, &JIT_insn_opcode_33,
	&JIT_insn_opcode_34, &JIT_insn_opcode_35, &JIT_insn_opcode_36, &JIT_insn_opcode_37,
	&JIT_insn_opcode_38, &JIT_insn_opcode_39, &JIT_insn_opcode_3a, &JIT_insn_opcode_3b,
	&JIT_insn_opcode_3c, &JIT_insn_opcode_3d, &JIT_insn_opcode_3e, &JIT_insn_opcode_3f,

	&JIT_insn_opcode_40, &JIT_insn_opcode_41, &JIT_insn_opcode_42, &JIT_insn_opcode_43,
	&JIT_insn_opcode_44, &JIT_insn_opcode_45, &JIT_insn_opcode_46, &JIT_insn_opcode_47,
	&JIT_insn_opcode_48, &JIT_insn_opcode_49, &JIT_insn_opcode_4a, &JIT_insn_opcode_4b,
	&JIT_insn_opcode_4c, &JIT_insn_opcode_4d, &JIT_insn_opcode_4e, &JIT_insn_opcode_4f,

	&JIT_insn_opcode_50, &JIT_insn_opcode_51, &JIT_insn_opcode_52, &JIT_insn_opcode_53,
	&JIT_insn_opcode_54, &JIT_insn_opcode_55, &JIT_insn_opcode_56, &JIT_insn_opcode_57,
	&JIT_insn_opcode_58, &JIT_insn_opcode_59, &JIT_insn_opcode_5a, &JIT_insn_opcode_5b,
	&JIT_insn_opcode_5c, &JIT_insn_opcode_5d, &JIT_insn_opcode_5e, &JIT_insn_opcode_5f,

	&JIT_insn_opcode_60, &JIT_insn_opcode_61, &JIT_insn_opcode_62, &JIT_insn_opcode_63,
	&JIT_insn_opcode_64, &JIT_insn_opcode_65, &JIT_insn_opcode_66, &JIT_insn_opcode_67,
	&JIT_insn_opcode_68, &JIT_insn_opcode_69, &JIT_insn_opcode_6a, &JIT_insn_opcode_6b,
	&JIT_insn_opcode_6c, &JIT_insn_opcode_6d, &JIT_insn_opcode_6e, &JIT_insn_opcode_6f,

	&JIT_insn_opcode_70, &JIT_insn_opcode_71, &JIT_insn_opcode_72, &JIT_insn_opcode_73,
	&JIT_insn_opcode_74, &JIT_insn_opcode_75, &JIT_insn_opcode_76, &JIT_insn_opcode_77,
	&JIT_insn_opcode_78, &JIT_insn_opcode_79, &JIT_insn_opcode_7a, &JIT_insn_opcode_7b,
	&JIT_insn_opcode_7c, &JIT_insn_opcode_7d, &JIT_insn_opcode_7e, &JIT_insn_opcode_7f,

	&JIT_insn_opcode_80, &JIT_insn_opcode_81, &JIT_insn_opcode_82, &JIT_insn_opcode_83,
	&JIT_insn_opcode_84, &JIT_insn_opcode_85, &JIT_insn_opcode_86, &JIT_insn_opcode_87,
	&JIT_insn_opcode_88, &JIT_insn_opcode_89, &JIT_insn_opcode_8a, &JIT_insn_opcode_8b,
	&JIT_insn_opcode_8c, &JIT_insn_opcode_8d, &JIT_insn_opcode_8e, &JIT_insn_opcode_8f,

	&JIT_insn_opcode_90, &JIT_insn_opcode_91, &JIT_insn_opcode_92, &JIT_insn_opcode_93,
	&JIT_insn_opcode_94, &JIT_insn_opcode_95, &JIT_insn_opcode_96, &JIT_insn_opcode_97,
	&JIT_insn_opcode_98, &JIT_insn_opcode_99, &JIT_insn_opcode_9a, &JIT_insn_opcode_9b,
	&JIT_insn_opcode_9c, &JIT_insn_opcode_9d, &JIT_insn_opcode_9e, &JIT_insn_opcode_9f,

	&JIT_insn_opcode_a0, &JIT_insn_opcode_a1, &JIT_insn_opcode_a2, &JIT_insn_opcode_a3,
	&JIT_insn_opcode_a4, &JIT_insn_opcode_a5, &JIT_insn_opcode_a6, &JIT_insn_opcode_a7,
	&JIT_insn_opcode_a8, &JIT_insn_opcode_a9, &JIT_insn_opcode_aa, &JIT_insn_opcode_ab,
	&JIT_insn_opcode_ac, &JIT_insn_opcode_ad, &JIT_insn_opcode_ae, &JIT_insn_opcode_af,

	&JIT_insn_opcode_b0, &JIT_insn_opcode_b1, &JIT_insn_opcode_b2, &JIT_insn_opcode_b3,
	&JIT_insn_opcode_b4, &JIT_insn_opcode_b5, &JIT_insn_opcode_b6, &JIT_insn_opcode_b7,
	&JIT_insn_opcode_b8, &JIT_insn_opcode_b9, &JIT_insn_opcode_ba, &JIT_insn_opcode_bb,
	&JIT_insn_opcode_bc, &JIT_insn_opcode_bd, &JIT_insn_opcode_be, &JIT_insn_opcode_bf,

	&JIT_insn_opcode_c0, &JIT_insn_opcode_c1, &JIT_insn_opcode_c2, &JIT_insn_opcode_c3,
	&JIT_insn_opcode_c4, &JIT_insn_opcode_c5, &JIT_insn_opcode_c6, &JIT_insn_opcode_c7,
	&JIT_insn_opcode_c8, &JIT_insn_opcode_c9, &JIT_insn_opcode_ca, &JIT_insn_opcode_cb,
	&JIT_insn_opcode_cc, &JIT_insn_opcode_cd, &JIT_insn_opcode_ce, &JIT_insn_opcode_cf,

	&JIT_insn_opcode_d0, &JIT_insn_opcode_d1, &JIT_insn_opcode_d2, &JIT_insn_opcode_d3,
	&JIT_insn_opcode_d4, &JIT_insn_opcode_d5, &JIT_insn_opcode_d6, &JIT_insn_opcode_d7,
	&JIT_insn_opcode_d8, &JIT_insn_opcode_d9, &JIT_insn_opcode_da, &JIT_insn_opcode_db,
	&JIT_insn_opcode_dc, &JIT_insn_opcode_dd, &JIT_insn_opcode_de, &JIT_insn_opcode_df,

	&JIT_insn_opcode_e0, &JIT_insn_opcode_e1, &JIT_insn_opcode_e2, &JIT_insn_opcode_e3,
	&JIT_insn_opcode_e4, &JIT_insn_opcode_e5, &JIT_insn_opcode_e6, &JIT_insn_opcode_e7,
	&JIT_insn_opcode_e8, &JIT_insn_opcode_e9, &JIT_insn_opcode_ea, &JIT_insn_opcode_eb,
	&JIT_insn_opcode_ec, &JIT_insn_opcode_ed, &JIT_insn_opcode_ee, &JIT_insn_opcode_ef,

	&JIT_insn_opcode_f0, &JIT_insn_opcode_f1, &JIT_insn_opcode_f2, &JIT_insn_opcode_f3,
	&JIT_insn_opcode_f4, &JIT_insn_opcode_f5, &JIT_insn_opcode_f6, &JIT_insn_opcode_f7,
	&JIT_insn_opcode_f8, &JIT_insn_opcode_f9, &JIT_insn_opcode_fa, &JIT_insn_opcode_fb,
	&JIT_insn_opcode_fc, &JIT_insn_opcode_fd, &JIT_insn_opcode_fe, &JIT_insn_opcode_ff
};

/*	0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
static const int JIT_compiler_cycle_table[256] =
{
	7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,		/* 0x */
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,		/* 1x */
	6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,		/* 2x */
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,		/* 3x */

	6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,		/* 4x */
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,		/* 5x */
	6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,		/* 6x */
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,		/* 7x */

	2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,		/* 8x */
	2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,		/* 9x */
	2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,		/* Ax */
	2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,		/* Bx */

	2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,		/* Cx */
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,		/* Dx */
	2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,		/* Ex */
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7		/* Fx */
};

/*	0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
static const int JIT_compiler_bytes_table[256] =
{
	1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,		/* 0x */
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,		/* 1x */
	3, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,		/* 2x */
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,		/* 3x */

	1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,		/* 4x */
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,		/* 5x */
	1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,		/* 6x */
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,		/* 7x */

	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,		/* 8x */
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,		/* 9x */
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,		/* Ax */
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,		/* Bx */

	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,		/* Cx */
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,		/* Dx */
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,		/* Ex */
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3		/* Fx */
};

#define CPUCHECKIRQ /* PLP, RTI, CLI */
#define UPDATE_LOCAL_REGS
#define UPDATE_GLOBAL_REGS

struct JIT_block_t {
	/* address of the first native insn in the block */
	UBYTE* addr;
	/* number of non-modified 6502 instructions in the block */
	size_t current_insn_count;
	/* the original number of 6502 instructions in the block */
	size_t total_insn_count;
	/* 6502 address range */
	UWORD start_addr;
	UWORD end_addr;
};
struct JIT_native_code_info_t {
	/* pointer to the block where this insn belongs to */
	struct JIT_block_t *block;
	/* type of data on given 6502 address */
	enum type_t {
		Data_Type_None,
		Data_Type_Insn,
		Data_Type_OpLo,
		Data_Type_OpHi
	} data_type;
};

static struct JIT_block_t *allocate_block()
{
	struct JIT_block_t *block = (struct JIT_block_t *)Util_malloc(sizeof(*block));
	memset(block, 0, sizeof(*block));
	return block;
}
static struct JIT_native_code_info_t *allocate_code_info()
{
	struct JIT_native_code_info_t *info = (struct JIT_native_code_info_t *)Util_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));
	return info;
}

static struct CPU_JIT_native_code_t *find_code(const UWORD pc)
{
	if (MEMORY_JIT_mem[pc].insn_addr == NULL) {
		/* no code found */
		return NULL;
	}

	/* in theory, 6502 code may jump into an operand's field, not supported for now */
	assert(MEMORY_JIT_mem[pc].insn_info->data_type == Data_Type_Insn);

	return &MEMORY_JIT_mem[pc];
}
static struct CPU_JIT_native_code_t *compile_code(const UWORD pc) {
	UBYTE stop = 0;
	UWORD addr = pc;
	ULONG native_size = 0;
	struct JIT_block_t *block;
	UBYTE *code;

	while (!stop) {
		UBYTE insn;
		const struct CPU_JIT_insn_template_t *insn_template;

		insn = MEMORY_mem[addr];
		insn_template = JIT_compiler_insn_table[insn];

		if (insn_template->is_stop == 2) {
			Log_print("insn %02x not implemented", insn);
			/* not implemented */
			return NULL;
		}

		if (MEMORY_JIT_mem[addr].insn_addr != NULL) {
			/* previous code found, insert a jump and stop translation */
			insn_template = &JIT_insn_opcode_4c;
		}

		native_size += insn_template->native_code_size;

		addr += JIT_compiler_bytes_table[insn];
		stop = insn_template->is_stop;
	}

	assert(native_size > 0);

	block = allocate_block();
	block->addr = (UBYTE*) Util_malloc(native_size);
	block->start_addr = pc;

	stop = 0;
	addr = pc;
	code = block->addr;

	while (!stop) {
		UBYTE insn;
		UBYTE bytes;
		UBYTE cycles;
		UWORD data = 0;
		const struct CPU_JIT_insn_template_t *insn_template;
		struct CPU_JIT_native_code_t *native_code;

		native_code = &MEMORY_JIT_mem[addr];

		if (native_code->insn_addr == NULL) {
			native_code->insn_addr = code;
			native_code->insn_info = allocate_code_info();
			native_code->insn_info->block = block;
			native_code->insn_info->data_type = Data_Type_Insn;

			block->current_insn_count++;

			insn = MEMORY_mem[addr];
			bytes = JIT_compiler_bytes_table[insn];
			cycles = JIT_compiler_cycle_table[insn];
			insn_template = JIT_compiler_insn_table[insn];

			if (bytes == 2) {
				native_code++;
				native_code->insn_addr = code;
				native_code->insn_info = allocate_code_info();
				native_code->insn_info->block = block;
				native_code->insn_info->data_type = Data_Type_OpLo;

				data = MEMORY_mem[addr+1];
				if (insn_template->is_stop) {
					/* special case: bcc */
					data = addr + ((UWORD) ((SBYTE) data));
				}
			} else if (bytes == 3) {
				native_code++;
				native_code->insn_addr = code;
				native_code->insn_info = allocate_code_info();
				native_code->insn_info->block = block;
				native_code->insn_info->data_type = Data_Type_OpLo;

				native_code++;
				native_code->insn_addr = code;
				native_code->insn_info = allocate_code_info();
				native_code->insn_info->block = block;
				native_code->insn_info->data_type = Data_Type_OpHi;

				data = MEMORY_mem[addr+1] + (MEMORY_mem[addr+2] << 8);
			}
		} else {
			/* previous code found, jump and let it finish */
			insn = 0x4c;
			bytes = 0;	/* virtual instruction */
			cycles = 0;	/* virtual instruction */
			insn_template = JIT_compiler_insn_table[insn];

			data = addr;
		}

		CPU_JIT_Instance(code, insn_template, data, bytes, cycles);

		code += insn_template->native_code_size;
		addr += bytes;
		stop = insn_template->is_stop;
	}

	block->end_addr = addr;
	block->total_insn_count = block->current_insn_count;

	/* all good, whole block translated */
	return &MEMORY_JIT_mem[pc];
}

static void execute_code(struct CPU_JIT_native_code_t *native_code)
{
	assert(native_code->insn_addr != NULL);

	CPU_JIT_Execute(native_code->insn_addr);
}

/* 6502 registers. */
UWORD CPU_regPC;
UBYTE CPU_regA;
UBYTE CPU_regX;
UBYTE CPU_regY;
UBYTE CPU_regP;
UBYTE CPU_regS;
UBYTE CPU_IRQ;

void CPU_GetStatus(void)
{
	/* not needed */
}
void CPU_PutStatus(void)
{
	/* not needed */
}

void CPU_NMI(void)
{
}

void CPU_GO(int limit)
{
	if (ANTIC_wsync_halt) {
		if (ANTIC_DRAWING_SCREEN) {
			if (limit < ANTIC_antic2cpu_ptr[ANTIC_WSYNC_C] + ANTIC_delayed_wsync)
				return;
			ANTIC_xpos = ANTIC_antic2cpu_ptr[ANTIC_WSYNC_C] + ANTIC_delayed_wsync;
		}
		else {
			if (limit < (ANTIC_WSYNC_C + ANTIC_delayed_wsync))
				return;
			ANTIC_xpos = ANTIC_WSYNC_C;
		}
		ANTIC_delayed_wsync = 0;
		ANTIC_wsync_halt = 0;
	}
	ANTIC_xpos_limit = limit;

	UPDATE_LOCAL_REGS;

	CPUCHECKIRQ;

	while (ANTIC_xpos < ANTIC_xpos_limit) {
		struct CPU_JIT_native_code_t *native_code = find_code(CPU_regPC);
		if (native_code == NULL) {
			native_code = compile_code(CPU_regPC);
			if (native_code == NULL) {
				/* fatal error */
				exit(EXIT_FAILURE);
			}
			Log_print("code compiled for %x", CPU_regPC);
		}

		execute_code(native_code);
	}

	UPDATE_GLOBAL_REGS;
}

int CPU_JIT_Invalidate(UWORD addr)
{
	struct CPU_JIT_native_code_t *native_code = &MEMORY_JIT_mem[addr];
	struct JIT_native_code_info_t *info;

	if (native_code->insn_addr == NULL) {
		return FALSE;
	}

	info = native_code->insn_info;
	assert(info != NULL && info->data_type != Data_Type_None);

	if (info->data_type == Data_Type_OpLo) {
		/* process insn */
		return CPU_JIT_Invalidate(addr-1);
	} else if (info->data_type == Data_Type_OpHi) {
		/* process insn */
		return CPU_JIT_Invalidate(addr-2);
	} else {
		struct JIT_block_t *block = info->block;
		int i;
		int size;

		block->current_insn_count--;
		/* TODO: maybe a different policy? like if >= 1/3 of the block is gone? */
		if (block->current_insn_count == 0) {
			/* this is the last insn, release the whole block */
			free(block->addr);
			/* release the block info as well */
			free(block);
		} else {
			/* NOTE: this assumes that native "return from subroutine" will never take more space than already allocated */
			assert(JIT_insn_opcode_60.is_stop != 2);
			CPU_JIT_Instance(native_code->insn_addr, &JIT_insn_opcode_60, 0, 0, 0);
		}

		size = JIT_compiler_bytes_table[MEMORY_mem[addr]];
		for (i = 0; i < size; i++) {
			MEMORY_JIT_mem[addr+i].insn_addr = NULL;
			free(MEMORY_JIT_mem[addr+i].insn_info);
			MEMORY_JIT_mem[addr+i].insn_info = NULL;
		}
	}

	return TRUE;
}

void CPU_JIT_InvalidateMem(UWORD from, UWORD to)
{
	UWORD addr;
	for (addr = from; addr <= to; addr++) {
		CPU_JIT_Invalidate(addr);
	}
}

void CPU_JIT_InvalidateAllocatedCode(struct CPU_JIT_native_code_t *native_code, int count)
{
	int i;
	for (i = 0; i < count; i++) {
		struct JIT_native_code_info_t *info = native_code[i].insn_info;

		if (native_code[i].insn_addr == NULL) {
			assert(info == NULL);
			continue;
		}
		assert(info != NULL);

		if (info->data_type == Data_Type_Insn) {
			struct JIT_block_t *block = info->block;

			block->current_insn_count--;
			if (block->current_insn_count == 0) {
				/* this is the last insn, release the whole block */
				free(block->addr);
				/* release the block info as well */
				free(block);
			} else {
				/* NOTE: this assumes that native "return from subroutine" will never take more space than already allocated */
				assert(JIT_insn_opcode_60.is_stop != 2);
				CPU_JIT_Instance(native_code[i].insn_addr, &JIT_insn_opcode_60, 0, 0, 0);
			}
		}

		native_code[i].insn_addr = NULL;
		free(native_code[i].insn_info);
		native_code[i].insn_info = NULL;
	}
}
