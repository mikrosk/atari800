; cpu_m68k_jit.asm - m68k JIT compiler core
;
; Copyright (C) 2017 Atari800 development team (see DOC/CREDITS)
;
; This file is part of the Atari800 emulator project which emulates
; the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
;
; Atari800 is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; Atari800 is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with Atari800; if not, write to the Free Software
; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

PAGED_ATTRIB	equ	0

		xref	_ANTIC_xpos
		xref	_ANTIC_xpos_limit
		xref	_CPU_IRQ
		xref	_CPU_regA
		xref	_CPU_regX
		xref	_CPU_regY
		xref	_CPU_regP
		xref	_CPU_regS
		ifeq	PAGED_ATTRIB
MEMORY_RAM		equ	0
MEMORY_ROM		equ	1
MEMORY_HARDWARE	equ	2
		xref	_MEMORY_attrib					; UBYTE MEMORY_attrib[65536];
		xref	_MEMORY_HwGetByte				; UBYTE MEMORY_HwGetByte(UWORD addr, int no_side_effects)
		xref	_MEMORY_HwPutByte				; void MEMORY_HwPutByte(UWORD addr, UBYTE byte)
		else
		xref	_MEMORY_readmap					; MEMORY_rdfunc MEMORY_readmap[256]
		xref	_MEMORY_writemap				; MEMORY_writemap MEMORY_readmap[256]
		endif
		xref	_MEMORY_dPutByte				; MEMORY_dPutByte(UWORD addr, UBYTE value);
		xref	_MEMORY_mem

		xdef	_CPU_JIT_Execute
		xdef	_CPU_JIT_Instance

		xdef	_JIT_insn_opcode_00
		xdef	_JIT_insn_opcode_01
		xdef	_JIT_insn_opcode_03
		xdef	_JIT_insn_opcode_64
		xdef	_JIT_insn_opcode_f4
		xdef	_JIT_insn_opcode_e2
		xdef	_JIT_insn_opcode_05
		xdef	_JIT_insn_opcode_06
		xdef	_JIT_insn_opcode_07
		xdef	_JIT_insn_opcode_08
		xdef	_JIT_insn_opcode_09
		xdef	_JIT_insn_opcode_0a
		xdef	_JIT_insn_opcode_2b
		xdef	_JIT_insn_opcode_0c
		xdef	_JIT_insn_opcode_0d
		xdef	_JIT_insn_opcode_0e
		xdef	_JIT_insn_opcode_0f
		xdef	_JIT_insn_opcode_10
		xdef	_JIT_insn_opcode_11
		xdef	_JIT_insn_opcode_13
		xdef	_JIT_insn_opcode_15
		xdef	_JIT_insn_opcode_16
		xdef	_JIT_insn_opcode_17
		xdef	_JIT_insn_opcode_18
		xdef	_JIT_insn_opcode_19
		xdef	_JIT_insn_opcode_1b
		xdef	_JIT_insn_opcode_fc
		xdef	_JIT_insn_opcode_1d
		xdef	_JIT_insn_opcode_1e
		xdef	_JIT_insn_opcode_1f
		xdef	_JIT_insn_opcode_20
		xdef	_JIT_insn_opcode_21
		xdef	_JIT_insn_opcode_23
		xdef	_JIT_insn_opcode_24
		xdef	_JIT_insn_opcode_25
		xdef	_JIT_insn_opcode_26
		xdef	_JIT_insn_opcode_27
		xdef	_JIT_insn_opcode_28
		xdef	_JIT_insn_opcode_29
		xdef	_JIT_insn_opcode_2a
		xdef	_JIT_insn_opcode_2c
		xdef	_JIT_insn_opcode_2d
		xdef	_JIT_insn_opcode_2e
		xdef	_JIT_insn_opcode_2f
		xdef	_JIT_insn_opcode_30
		xdef	_JIT_insn_opcode_31
		xdef	_JIT_insn_opcode_33
		xdef	_JIT_insn_opcode_35
		xdef	_JIT_insn_opcode_36
		xdef	_JIT_insn_opcode_37
		xdef	_JIT_insn_opcode_38
		xdef	_JIT_insn_opcode_39
		xdef	_JIT_insn_opcode_3b
		xdef	_JIT_insn_opcode_3d
		xdef	_JIT_insn_opcode_3e
		xdef	_JIT_insn_opcode_3f
		xdef	_JIT_insn_opcode_40
		xdef	_JIT_insn_opcode_41
		xdef	_JIT_insn_opcode_43
		xdef	_JIT_insn_opcode_45
		xdef	_JIT_insn_opcode_46
		xdef	_JIT_insn_opcode_47
		xdef	_JIT_insn_opcode_48
		xdef	_JIT_insn_opcode_49
		xdef	_JIT_insn_opcode_4a
		xdef	_JIT_insn_opcode_4b
		xdef	_JIT_insn_opcode_4c
		xdef	_JIT_insn_opcode_4d
		xdef	_JIT_insn_opcode_4e
		xdef	_JIT_insn_opcode_4f
		xdef	_JIT_insn_opcode_50
		xdef	_JIT_insn_opcode_51
		xdef	_JIT_insn_opcode_53
		xdef	_JIT_insn_opcode_55
		xdef	_JIT_insn_opcode_56
		xdef	_JIT_insn_opcode_57
		xdef	_JIT_insn_opcode_58
		xdef	_JIT_insn_opcode_59
		xdef	_JIT_insn_opcode_5b
		xdef	_JIT_insn_opcode_5d
		xdef	_JIT_insn_opcode_5e
		xdef	_JIT_insn_opcode_5f
		xdef	_JIT_insn_opcode_60
		xdef	_JIT_insn_opcode_61
		xdef	_JIT_insn_opcode_63
		xdef	_JIT_insn_opcode_65
		xdef	_JIT_insn_opcode_66
		xdef	_JIT_insn_opcode_67
		xdef	_JIT_insn_opcode_68
		xdef	_JIT_insn_opcode_69
		xdef	_JIT_insn_opcode_6a
		xdef	_JIT_insn_opcode_6b
		xdef	_JIT_insn_opcode_6c
		xdef	_JIT_insn_opcode_6d
		xdef	_JIT_insn_opcode_6e
		xdef	_JIT_insn_opcode_6f
		xdef	_JIT_insn_opcode_70
		xdef	_JIT_insn_opcode_71
		xdef	_JIT_insn_opcode_73
		xdef	_JIT_insn_opcode_75
		xdef	_JIT_insn_opcode_76
		xdef	_JIT_insn_opcode_77
		xdef	_JIT_insn_opcode_78
		xdef	_JIT_insn_opcode_79
		xdef	_JIT_insn_opcode_7b
		xdef	_JIT_insn_opcode_7d
		xdef	_JIT_insn_opcode_7e
		xdef	_JIT_insn_opcode_7f
		xdef	_JIT_insn_opcode_81
		xdef	_JIT_insn_opcode_83
		xdef	_JIT_insn_opcode_84
		xdef	_JIT_insn_opcode_85
		xdef	_JIT_insn_opcode_86
		xdef	_JIT_insn_opcode_87
		xdef	_JIT_insn_opcode_88
		xdef	_JIT_insn_opcode_8a
		xdef	_JIT_insn_opcode_8b
		xdef	_JIT_insn_opcode_8c
		xdef	_JIT_insn_opcode_8d
		xdef	_JIT_insn_opcode_8e
		xdef	_JIT_insn_opcode_8f
		xdef	_JIT_insn_opcode_90
		xdef	_JIT_insn_opcode_91
		xdef	_JIT_insn_opcode_93
		xdef	_JIT_insn_opcode_94
		xdef	_JIT_insn_opcode_95
		xdef	_JIT_insn_opcode_96
		xdef	_JIT_insn_opcode_97
		xdef	_JIT_insn_opcode_98
		xdef	_JIT_insn_opcode_99
		xdef	_JIT_insn_opcode_9a
		xdef	_JIT_insn_opcode_9b
		xdef	_JIT_insn_opcode_9c
		xdef	_JIT_insn_opcode_9d
		xdef	_JIT_insn_opcode_9e
		xdef	_JIT_insn_opcode_9f
		xdef	_JIT_insn_opcode_a0
		xdef	_JIT_insn_opcode_a1
		xdef	_JIT_insn_opcode_a2
		xdef	_JIT_insn_opcode_a3
		xdef	_JIT_insn_opcode_a4
		xdef	_JIT_insn_opcode_a5
		xdef	_JIT_insn_opcode_a6
		xdef	_JIT_insn_opcode_a7
		xdef	_JIT_insn_opcode_a8
		xdef	_JIT_insn_opcode_a9
		xdef	_JIT_insn_opcode_aa
		xdef	_JIT_insn_opcode_ab
		xdef	_JIT_insn_opcode_ac
		xdef	_JIT_insn_opcode_ad
		xdef	_JIT_insn_opcode_ae
		xdef	_JIT_insn_opcode_af
		xdef	_JIT_insn_opcode_b0
		xdef	_JIT_insn_opcode_b1
		xdef	_JIT_insn_opcode_b3
		xdef	_JIT_insn_opcode_b4
		xdef	_JIT_insn_opcode_b5
		xdef	_JIT_insn_opcode_b6
		xdef	_JIT_insn_opcode_b7
		xdef	_JIT_insn_opcode_b8
		xdef	_JIT_insn_opcode_b9
		xdef	_JIT_insn_opcode_ba
		xdef	_JIT_insn_opcode_bb
		xdef	_JIT_insn_opcode_bc
		xdef	_JIT_insn_opcode_bd
		xdef	_JIT_insn_opcode_be
		xdef	_JIT_insn_opcode_bf
		xdef	_JIT_insn_opcode_c0
		xdef	_JIT_insn_opcode_c1
		xdef	_JIT_insn_opcode_c3
		xdef	_JIT_insn_opcode_c4
		xdef	_JIT_insn_opcode_c5
		xdef	_JIT_insn_opcode_c6
		xdef	_JIT_insn_opcode_c7
		xdef	_JIT_insn_opcode_c8
		xdef	_JIT_insn_opcode_c9
		xdef	_JIT_insn_opcode_ca
		xdef	_JIT_insn_opcode_cb
		xdef	_JIT_insn_opcode_cc
		xdef	_JIT_insn_opcode_cd
		xdef	_JIT_insn_opcode_ce
		xdef	_JIT_insn_opcode_cf
		xdef	_JIT_insn_opcode_d0
		xdef	_JIT_insn_opcode_d1
		xdef	_JIT_insn_opcode_d3
		xdef	_JIT_insn_opcode_d5
		xdef	_JIT_insn_opcode_d6
		xdef	_JIT_insn_opcode_d7
		xdef	_JIT_insn_opcode_d8
		xdef	_JIT_insn_opcode_d9
		xdef	_JIT_insn_opcode_db
		xdef	_JIT_insn_opcode_dd
		xdef	_JIT_insn_opcode_de
		xdef	_JIT_insn_opcode_df
		xdef	_JIT_insn_opcode_e0
		xdef	_JIT_insn_opcode_e1
		xdef	_JIT_insn_opcode_e3
		xdef	_JIT_insn_opcode_e4
		xdef	_JIT_insn_opcode_e5
		xdef	_JIT_insn_opcode_e6
		xdef	_JIT_insn_opcode_e7
		xdef	_JIT_insn_opcode_e8
		xdef	_JIT_insn_opcode_eb
		xdef	_JIT_insn_opcode_fa
		xdef	_JIT_insn_opcode_ec
		xdef	_JIT_insn_opcode_ed
		xdef	_JIT_insn_opcode_ee
		xdef	_JIT_insn_opcode_ef
		xdef	_JIT_insn_opcode_f0
		xdef	_JIT_insn_opcode_f1
		xdef	_JIT_insn_opcode_f3
		xdef	_JIT_insn_opcode_f5
		xdef	_JIT_insn_opcode_f6
		xdef	_JIT_insn_opcode_f7
		xdef	_JIT_insn_opcode_f8
		xdef	_JIT_insn_opcode_f9
		xdef	_JIT_insn_opcode_fb
		xdef	_JIT_insn_opcode_fd
		xdef	_JIT_insn_opcode_fe
		xdef	_JIT_insn_opcode_ff
		xdef	_JIT_insn_opcode_d2
		xdef	_JIT_insn_opcode_f2
		xdef	_JIT_insn_opcode_b2

no_stop	equ		0
is_stop	equ		1
unknown	equ		2

reg_PC	equr	d2								; .w
reg_A	equr	d3								; .b
reg_X	equr	d4								; .b
reg_Y	equr	d5								; .b
reg_S	equr	d6								; .b
memory	equr	a2								; .l
		ifeq	PAGED_ATTRIB
mem_attrib equr	a3
		else
mem_readmap equr a3
mem_writemap equr a4
		endif
xpos	equr	a6								; .l, ANTIC_xpos mirror

; ----------------------------------------------

		macro	MEMORY_dGetByte					; d0.l: addr
		move.b	(0.b,memory,d0.l*1),d0
		endm

		macro	MEMORY_dGetWord					; d0.l: addr
		move.w	(0.b,memory,d0.l*1),d0
		ror.w	#8,d0
		endm

		macro	MEMORY_dPutByte					; d0.l: addr, d1.b: value
		move.l	d1,-(sp)
		move.l	d0,-(sp)
		jsr		_MEMORY_dPutByte
		addq.l	#8,sp
		endm

		macro	MEMORY_GetByte					; d0.l: addr
		ifeq	PAGED_ATTRIB
		cmpi.b	#MEMORY_HARDWARE,(0.b,mem_attrib,d0.l*1)
		bne.b	.no_hardware\@
		clr.l	-(sp)							; FALSE (no side effects)
		move.l	d0,-(sp)						; addr
		move.l	xpos,_ANTIC_xpos
		jsr		_MEMORY_HwGetByte
		move.l	_ANTIC_xpos,xpos
		addq.l	#8,sp
		bra.b	.skip\@
		else
		move.l	d0,d1
		lsr.l	#8,d1
		move.l	(0.b,mem_readmap,d1.l*4),d1
		beq.b	.no_hardware\@
		movea.l	d1,a0
		clr.l	-(sp)							; FALSE (no side effects)
		move.l	d0,-(sp)						; addr
		move.l	xpos,_ANTIC_xpos
		jsr		(a0)
		move.l	_ANTIC_xpos,xpos
		addq.l	#8,sp
		bra.b	.skip\@
		endif
.no_hardware\@:
		move.b	(0.b,memory,d0.l*1),d0
.skip\@:
		endm

		macro	MEMORY_PutByte					; d0.l: addr, d1.b: value
		ifeq	PAGED_ATTRIB
		cmpi.b	#MEMORY_RAM,(0.b,mem_attrib,d0.l*1)
		bne.b	.no_ram\@
		move.l	d1,-(sp)						; value
		move.l	d0,-(sp)						; addr
		jsr		_MEMORY_dPutByte
		bra.b	.skip\@
.no_ram\@:
		cmpi.b	#MEMORY_HARDWARE,(0.b,mem_attrib,d0.l*1)
		bne.b	.no_hardware\@
		move.l	d1,-(sp)						; value
		move.l	d0,-(sp)						; addr
		move.l	xpos,_ANTIC_xpos
		jsr		_MEMORY_HwPutByte
		move.l	_ANTIC_xpos,xpos
.no_hardware\@:
		else
		movea.l	d0,a0							; backup
		lsr.l	#8,d0
		movea.l	(0.b,mem_writemap,d0.l*4),a1
		move.l	d1,-(sp)						; value
		move.l	a0,-(sp)						; addr
		move.l	xpos,_ANTIC_xpos
		jsr		(a1)
		move.l	_ANTIC_xpos,xpos
		endif
.skip\@:
		addq.l	#8,sp
		endm

; ----------------------------------------------

		macro	NO_STOP
		dc.w	no_stop
		endm

		macro	IS_STOP
		dc.w	is_stop
		endm

		macro	NOT_IMPLEMENTED
		dc.w	unknown
		endm

; ----------------------------------------------

		macro	NO_OFFSET_WITH_BYTES_AND_CYCLES
		dc.w	-1
		dc.w	.m68k_bytes-.m68k_start
		dc.w	.m68k_cycles-.m68k_start
		dc.w	.m68k_end-.m68k_start
		endm

		macro	NO_OFFSET_WITH_CYCLES
		dc.w	-1
		dc.w	-1
		dc.w	.m68k_cycles-.m68k_start
		dc.w	.m68k_end-.m68k_start
		endm

		macro	OFFSET_WITH_CYCLES				; offset
		dc.w	\1
		dc.w	-1
		dc.w	.m68k_cycles-.m68k_start
		dc.w	.m68k_end-.m68k_start
		endm

		macro	OFFSET_WITH_BYTES_AND_CYCLES	; offset
		dc.w	\1
		dc.w	.m68k_bytes-.m68k_start
		dc.w	.m68k_cycles-.m68k_start
		dc.w	.m68k_end-.m68k_start
		endm

		macro	BRANCH_WITH_BYTES_AND_CYCLES
		dc.w	.m68k_branch-.m68k_start+2
		dc.w	.m68k_bytes-.m68k_start
		dc.w	.m68k_cycles-.m68k_start
		dc.w	.m68k_end-.m68k_start
		endm

		macro	JUMP_WITH_CYCLES
		dc.w	.m68k_jump-.m68k_start+2
		dc.w	-1
		dc.w	.m68k_cycles-.m68k_start
		dc.w	.m68k_end-.m68k_start
		endm

; ----------------------------------------------

		macro	ACCUMULATOR
		NO_OFFSET_WITH_BYTES_AND_CYCLES
.m68k_start:
		endm

		macro	IMPLIED
		NO_OFFSET_WITH_BYTES_AND_CYCLES
.m68k_start:
		endm

		macro	IMMEDIATE
		OFFSET_WITH_BYTES_AND_CYCLES 2
.m68k_start:
		move.b	#$00ab,d0
		endm

		macro	ABSOLUTE
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		move.l	#$0000abcd,d0
		endm

		macro	ZPAGE
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		move.l	#$000000ab,d0
		endm

		macro	ABSOLUTE_X
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		move.l	#$0000abcd,d0
		add.w	reg_X,d0
		endm

		macro	ABSOLUTE_Y
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		move.l	#$0000abcd,d0
		add.w	reg_Y,d0
		endm

		macro	INDIRECT_X
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		move.l	#$000000ab,d0
		add.b	reg_X,d0
		MEMORY_dGetWord
		endm

		macro	INDIRECT_Y
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		move.l	#$000000ab,d0
		MEMORY_dGetWord
		add.w	reg_Y,d0
		endm

		macro	ZPAGE_X
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		move.l	#$000000ab,d0
		add.b	reg_X,d0
		endm

		macro	ZPAGE_Y
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		move.l	#$000000ab,d0
		add.b	reg_Y,d0
		endm

; ----------------------------------------------

; 1 extra cycle for X (or Y) index overflow
		macro	NCYCLES_X
		cmp.b	d0,reg_X						; if ((UBYTE) addr < X) ANTIC_xpos++
		bls.b	.no_extra_cycle\@
		addq.l	#1,xpos
.no_extra_cycle\@:
		endm

		macro	NCYCLES_Y
		cmp.b	d0,reg_Y						; if ((UBYTE) addr < Y) ANTIC_xpos++
		bls.b	.no_extra_cycle\@
		addq.l	#1,xpos
.no_extra_cycle\@:
		endm

; ----------------------------------------------

		macro	BRANCH
.m68k_branch:
		move.w	#$abcd,d0
		eor.w	d0,reg_PC
		and.w	#$ff00,reg_PC
		beq.b	.same_page\@
		addq.l	#1,xpos
.same_page\@:
		; CHEATING!!! add the base two cycles + 1 'branch taken' and return
		addq.l	#2+1,xpos
		move.w	d0,reg_PC
		rts
		endm

		macro	M68K_BYTES_TEMPLATE
.m68k_bytes:
		addq.l	#1,reg_PC						; 1 - 3
		endm

		macro	M68K_CYCLES_TEMPLATE
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		cmp.l	_ANTIC_xpos_limit,xpos
		blt.b	.next_insn\@
		rts
.next_insn\@:
		endm

		macro	DONE
.m68k_end:
		endm

; ---------------------------------------------
		section	text
; ---------------------------------------------

; void CPU_JIT_Execute(UBYTE* pCode);
_CPU_JIT_Execute:
		movea.l	(4,sp),a0						; code
		movem.l	d2-d7/a2-a6,-(sp)

		clr.l	reg_PC
		move.w	_CPU_regPC,reg_PC
		clr.l	reg_A
		move.b	_CPU_regA,reg_A
		clr.l	reg_X
		move.b	_CPU_regX,reg_X
		clr.l	reg_Y
		move.b	_CPU_regY,reg_Y
		; TODO: reg_P
		clr.l	reg_S
		move.b	_CPU_regS,reg_S

		ifeq	PAGED_ATTRIB
		lea		_MEMORY_attrib,mem_attrib
		else
		lea		_MEMORY_readmap,mem_readmap
		lea		_MEMORY_writemap,mem_writemap
		endif
		lea		_MEMORY_mem,memory
		move.l	_ANTIC_xpos,xpos

		jsr		(a0)

		move.l	xpos,_ANTIC_xpos

		move.b	reg_S,_CPU_regS
		; TODO: reg_P
		move.b	reg_Y,_CPU_regY
		move.b	reg_X,_CPU_regX
		move.b	reg_A,_CPU_regA
		move.w	reg_PC,_CPU_regPC

		movem.l	(sp)+,d2-d7/a2-a6
		rts

; void CPU_JIT_Instance(UBYTE* dst_buf, const struct CPU_JIT_insn_template_t *src_template, UWORD data, int bytes, int cycles);
		rsreset
insn_template_is_stop: 		rs.w	1
insn_template_data_offset:	rs.w	1
insn_template_bytes_offset:	rs.w	1
insn_template_cycles_offset:rs.w	1
insn_template_code_size:	rs.w	1
insn_template_code:			rs.b	0

_CPU_JIT_Instance:
		movea.l	(8,sp),a0						; a0: insn template
		move.w	(insn_template_code_size,a0),d0	; d0.w: code template size
		lsr.w	#1,d0
		subq.w	#1,d0
		lea		(insn_template_code,a0),a0		; a0: src buffer (code template)
		movea.l	(4,sp),a1						; a1: dst buffer
.copy_loop:
		move.w	(a0)+,(a1)+
		dbra	d0,.copy_loop

		movea.l	(8,sp),a0						; a0: insn template
		movea.l	(4,sp),a1						; a1: dst buffer

		move.w	(insn_template_data_offset,a0),d0
		bmi.b	.no_data
		move.l	(12,sp),d1						; d1.w: data
		move.w	d1,(0.b,a1,d0.w*1)
.no_data:
		move.w	(insn_template_bytes_offset,a0),d0
		bmi.b	.no_bytes
		move.l	(16,sp),d1						; d1.l: bytes
		move.w	(addql_pc_table.l,pc,d1.l*2),(0.b,a1,d0.w*1)
.no_bytes:
		move.w	(insn_template_cycles_offset,a0),d0
		bmi.b	.no_cycles
		move.l	(20,sp),d1						; d1.l: cycles
		move.w	(addql_xpos_table.l,pc,d1.l*2),(0.b,a1,d0.w*1)
.no_cycles:
		rts

; ----------------------------------------------

N:		ds.b	1								; bit7 set => N flag set
V:		ds.b	1								; non-zero => V flag set
Z:		ds.b	1								; zero     => Z flag set
C:		ds.b	1								; non-zero => C flag set

N_FLAG	equ		7
V_FLAG	equ		6
B_FLAG	equ		4
D_FLAG	equ		3
I_FLAG	equ		2
Z_FLAG	equ		1
C_FLAG	equ		0

		macro	SetN
		bset	#N_FLAG,_CPU_regP
		endm
		macro	ClrN
		bclr	#N_FLAG,_CPU_regP
		endm
		macro	SetV
		bset	#V_FLAG,_CPU_regP
		endm
		macro	ClrV
		bclr	#V_FLAG,_CPU_regP
		endm
		macro	SetB
		bset	#B_FLAG,_CPU_regP
		endm
		macro	ClrB
		bclr	#B_FLAG,_CPU_regP
		endm
		macro	SetD
		bset	#D_FLAG,_CPU_regP
		endm
		macro	ClrD
		bclr	#D_FLAG,_CPU_regP
		endm
		macro	SetI
		bset	#I_FLAG,_CPU_regP
		endm
		macro	ClrI
		bclr	#I_FLAG,_CPU_regP
		endm
		macro	SetZ
		bset	#Z_FLAG,_CPU_regP
		endm
		macro	ClrZ
		bclr	#Z_FLAG,_CPU_regP
		endm
		macro	SetC
		bset	#C_FLAG,_CPU_regP
		endm
		macro	ClrC
		bclr	#C_FLAG,_CPU_regP
		endm

		macro	AND6502
		and.b	d0,reg_A
		move.b	reg_A,N
		move.b	reg_A,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		endm

		macro	CMP6502
		move.b	reg_A,Z
		sub.b	d0,Z
		scc		C								; C is inverted on the 6502
		move.b	Z,N
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		endm

		macro	CPX6502
		move.b	reg_X,Z
		sub.b	d0,Z
		scc		C
		move.b	Z,N
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		endm

		macro	CPY6502
		move.b	reg_Y,Z
		sub.b	d0,Z
		scc		C
		move.b	Z,N
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		endm

		macro	EOR6502
		eor.b	d0,reg_A
		move.b	reg_A,N
		move.b	reg_A,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		endm

		macro	LDA6502
		move.b	d0,reg_A
		move.b	reg_A,N
		move.b	reg_A,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		endm

		macro	LDX6502
		move.b	d0,reg_X
		move.b	reg_X,N
		move.b	reg_X,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		endm

		macro	LDY6502
		move.b	d0,reg_Y
		move.b	reg_Y,N
		move.b	reg_Y,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		endm

		macro	ORA6502
		or.b	d0,reg_A
		move.b	reg_A,N
		move.b	reg_A,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		endm

		macro	BIT6502
		move.b	d0,N
		move.b	d0,V
		and.b	#$40,V
		and.b	reg_A,d0
		move.b	d0,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		endm

		macro	ADC6502
		btst	#D_FLAG,_CPU_regP
		beq.b	.binary_adc\@

.decimal_adc\@:
		add.b	C,C
		move.b	reg_A,Z
		addx.b	d0,Z
		move.b	reg_A,V
		eor.b	d0,V
		not.b	V
		unpk	reg_A,reg_A,#0
		unpk	d0,d7,#0
		add.b	C,C
		addx.w	d7,reg_A
		cmp.b	#$0a,reg_A
		blo.b	.no_carry\@
		add.w	#$0106,reg_A
.no_carry\@:
		pack	reg_A,reg_A,#0
		move.b	reg_A,N
		eor.b	reg_A,d0
		and.b	V,d0
		smi		V
		cmp.w	#$0a00,reg_A
		shs		C
		blo.b	.no_carry2\@
		add.b	#$60,reg_A
.no_carry2\@:
		bra.b	.skip\@

.binary_adc\@:
		add.b	C,C
		addx.b	d0,reg_A
		svs		V
		scs		C
		move.b	reg_A,N
		move.b	reg_A,Z
.skip\@:
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		endm

		macro	SBC6502
		btst	#D_FLAG,_CPU_regP
		beq.b	.binary_sbc\@

.decimal_sbc\@:
		move.b	reg_A,Z
		unpk	reg_A,reg_A,#0
		unpk	d0,d7,#0
		not.b	C
		add.b	C,C
		subx.w	d7,reg_A
		tst.b	reg_A
		bpl.b	.no_carry\@
		subq.w	#6,reg_A
.no_carry\@:
		pack	reg_A,reg_A,#0
		tst.w	reg_A
 		bpl.b	.no_carry2\@
 		sub.b	#$60,reg_A
.no_carry2\@:
		add.b	C,C
		subx.b	d0,Z
		svs		V
		scc		C
		move.b	Z,N
		bra.b	.skip\@

.binary_sbc\@:
		subq.b	#1,C
		subx.b	d0,reg_A
		svs		V
		scc		C
		move.b	reg_A,N
		move.b	reg_A,Z
.skip\@:
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		endm

		macro	ROL_ONLY						; src/dst reg
		add.b	C,C
		addx.b	\1,\1
		scs		C
		move.b	\1,N
		move.b	\1,Z
		endm

		macro	ROR_ONLY						; src/dst reg
		add.b	C,C
		roxr.b	#1,\1
		scs		C
		move.b	\1,N
		move.b	\1,Z
		endm

		macro	ASL_ONLY						; src/dst reg
		add.b	\1,\1
		scs		C
		move.b	\1,N
		move.b	\1,Z
		endm

		macro	LSR_ONLY						; src/dst reg
		lsr.b	#1,\1
		scs		C
		move.b	\1,N
		move.b	\1,Z
		endm

		; input:    -
		; output:   d0.b
		; clobbers: d0.w
		macro	PL
		addq.b	#1,reg_S
		move.l	reg_S,d0
		add.w	#$0100,d0
		MEMORY_dGetByte
		endm

		; input:    d0.b
		; output:   -
		; clobbers: d0.l-d1.l/a0-a1
		macro	PH
		move.b	d0,d1
		move.l	reg_S,d0
		add.w	#$0100,d0
		subq.b	#1,reg_S
		MEMORY_dPutByte
		endm

		; input:    d0.w (big endian)
		; output:   -
		; clobbers: d0.l-d1.l/a0-a1
		macro	PHW
		move.w	d0,-(sp)
		lsr.w	#8,d0
		PH
		move.w	(sp)+,d0
		PH
		endm

		; input:    -
		; output:   d0.w
		; clobbers: d0.w, d1.w
		macro	PLW
		PL
		move.w	d0,d1
		PL
		lsl.w	#8,d0
		or.w	d1,d0
		endm

		; input:    \1 (mask)
		; output:   -
		; clobbers: d0.l-d1.l/a0-a1
		macro	PHP								; mask
		move.b	_CPU_regP,d0
		and.b	\1,d0
.n\@:	tst.b	N
		bpl.b	.v\@
		bset	#N_FLAG,d0
.v\@:	tst.b	V
		beq.b	.z\@
		bset	#V_FLAG,d0
.z\@:	tst.b	Z
		bne.b	.c\@
		bset	#Z_FLAG,d0
.c\@:	tst.b	C
		beq.b	.done\@
		bset	#C_FLAG,d0
.done\@:
		PH
		endm

		; input:    -
		; output:   d0.b
		; clobbers: d0.l-d1.l/a0-a1
		macro	PHPB0
		PHP		#$2c							; push flags with B flag clear (NMI, IRQ)
		endm

		; input:    -
		; output:   d0.b
		; clobbers: d0.l-d1.l/a0-a1
		macro	PHPB1
		PHP		#$3c							; push flags with B flag set (PHP, BRK)
		endm

		; input:    -
		; output:   d0.b
		; clobbers: d0.w
		macro	PLP
		PL
		btst	#C_FLAG,d0
		sne		C
		btst	#Z_FLAG,d0
		seq		Z
		btst	#V_FLAG,d0
		sne		V
		btst	#N_FLAG,d0
		sne		N
		and.b	#$0c,d0
		add.b	#$30,d0
		move.b	d0,_CPU_regP
		endm

		; input:    -
		; output:   -
		; clobbers: d0.l-d1.l/a0-a1
		macro	PHPC
		move.l	reg_PC,d0
		PHW
		endm

		; input:    -
		; output:   -
		; clobbers: d0.l-d1.l/a0-a1
		macro	CHECKIRQ
		tst.b	_CPU_IRQ
		beq.b	.skip\@
		btst	#I_FLAG,_CPU_regP
		bne		.skip\@
		cmp.l	_ANTIC_xpos_limit,xpos
		bge.b	.skip\@
		PHPC
		PHPB0
		SetI
		move.l	#$0000fffe,d0
		MEMORY_dGetWord
		move.l	d0,reg_PC
		addq.l	#7,xpos
		rts
.skip\@:
		endm

		; input:    d0.l
		; output:   d0.b
		; clobbers: d0.l-d1.l/a0-a1
		macro	RMW_GetByte
		move.l	d0,-(sp)
		MEMORY_GetByte
		; d0.b: value
		movea.l	(sp)+,a0
		move.l	a0,d1
		and.w	#$ef00,d1
		cmp.w	#$c000,d1
		bne.b	.skip_rmw\@
		move.b	d0,-(sp)
		move.l	d1,-(sp)						; value (should be the old one but it doesn't matter)
		move.l	a0,-(sp)						; addr
		subq.l	#1,xpos
		move.l	xpos,_ANTIC_xpos
		jsr		_MEMORY_HwPutByte
		move.l	_ANTIC_xpos,xpos
		addq.l	#1,xpos
		addq.l	#8,sp
		move.b	(sp)+,d0
.skip_rmw\@:
		endm

; ----------------------------------------------

_JIT_insn_opcode_00: ;BRK
		IS_STOP
		NO_OFFSET_WITH_CYCLES
.m68k_start:
		addq.l	#1,reg_PC
		PHPC
		PHPB1
		SetI
		move.l	#$0000fffe,d0
		MEMORY_dGetWord
		move.l	d0,reg_PC
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		rts
		DONE

_JIT_insn_opcode_01: ;ORA (ab,x)
		NO_STOP
		INDIRECT_X
		MEMORY_GetByte
		ORA6502
		DONE

_JIT_insn_opcode_03: ;ASO (ab,x) [unofficial - ASL then ORA with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_64: ;NOP ab [unofficial - skip byte]
		NO_STOP
		IMPLIED
		; nothing to do
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_f4: ;NOP ab,x [unofficial - skip byte]
		NO_STOP
		IMPLIED
		; nothing to do
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_e2: ;NOP #ab [unofficial - skip byte]
		NO_STOP
		IMPLIED
		; nothing to do
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_05: ;ORA ab
		NO_STOP
		ZPAGE
		MEMORY_dGetByte
		ORA6502
		DONE

_JIT_insn_opcode_06: ;ASL ab
		NO_STOP
		ZPAGE
		move.l	d0,-(sp)
		MEMORY_dGetByte
		ASL_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_07: ;ASO ab [unofficial - ASL then ORA with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_08: ;PHP
		NO_STOP
		IMPLIED
		PHPB1
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_09: ;ORA #ab
		NO_STOP
		IMMEDIATE
		ORA6502
		DONE

_JIT_insn_opcode_0a: ;ASL
		NO_STOP
		ACCUMULATOR
		ASL_ONLY reg_A
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_2b: ;ANC #ab [unofficial - AND then copy N to C (Fox)
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_0c: ;NOP abcd [unofficial - skip word]
		NO_STOP
		IMPLIED
		; nothing to do
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_0d: ;ORA abcd
		NO_STOP
		ABSOLUTE
		MEMORY_GetByte
		ORA6502
		DONE

_JIT_insn_opcode_0e: ;ASL abcd
		NO_STOP
		ABSOLUTE
		move.l	d0,-(sp)
		RMW_GetByte
		ASL_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_0f: ;ASO abcd [unofficial - ASL then ORA with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_10: ;BPL
		IS_STOP
		BRANCH_WITH_BYTES_AND_CYCLES
.m68k_start:
		tst.b	N
		bmi.b	.next
		BRANCH
.next:	M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_11: ;ORA (ab),y
		NO_STOP
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		ORA6502
		DONE

_JIT_insn_opcode_13: ;ASO (ab),y [unofficial - ASL then ORA with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_15: ;ORA ab,x
		NO_STOP
		ZPAGE_X
		MEMORY_dGetByte
		ORA6502
		DONE

_JIT_insn_opcode_16: ;ASL ab,x
		NO_STOP
		ZPAGE_X
		move.l	d0,-(sp)
		MEMORY_dGetByte
		ASL_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_17: ;ASO ab,x [unofficial - ASL then ORA with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_18: ;CLC
		NO_STOP
		IMPLIED
		clr.b	C
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_19: ;ORA abcd,y
		NO_STOP
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		ORA6502
		DONE

_JIT_insn_opcode_1b: ;ASO abcd,y [unofficial - ASL then ORA with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_fc: ;NOP abcd,x [unofficial - skip word]
		NO_STOP
		IMPLIED
		; nothing to do
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_1d: ;ORA abcd,x
		NO_STOP
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		ORA6502
		DONE

_JIT_insn_opcode_1e: ;ASL abcd,x
		NO_STOP
		ABSOLUTE_X
		move.l	d0,-(sp)
		RMW_GetByte
		ASL_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_1f: ;ASO abcd,x [unofficial - ASL then ORA with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_20: ;JSR abcd
		IS_STOP
		JUMP_WITH_CYCLES
.m68k_start:
		move.l	reg_PC,d0
		addq.l	#1+1,d0									; store PC one byte before next insn
		PHW
.m68k_jump:
		move.w	#$abcd,reg_PC
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		rts
		DONE

_JIT_insn_opcode_21: ;AND (ab,x)
		NO_STOP
		INDIRECT_X
		MEMORY_GetByte
		AND6502
		DONE

_JIT_insn_opcode_23: ;RLA (ab,x) [unofficial - ROL Mem, then AND with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_24: ;BIT ab
		NO_STOP
		ZPAGE
		MEMORY_dGetByte
		BIT6502
		DONE

_JIT_insn_opcode_25: ;AND ab
		NO_STOP
		ZPAGE
		MEMORY_dGetByte
		AND6502
		DONE

_JIT_insn_opcode_26: ;ROL ab
		NO_STOP
		ZPAGE
		move.l	d0,-(sp)
		MEMORY_dGetByte
		ROL_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_27: ;RLA ab [unofficial - ROL Mem, then AND with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_28: ;PLP
		NO_STOP
		IMPLIED
		PLP
		CHECKIRQ
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_29: ;AND #ab
		NO_STOP
		IMMEDIATE
		AND6502
		DONE

_JIT_insn_opcode_2a: ;ROL
		NO_STOP
		ACCUMULATOR
		ROL_ONLY reg_A
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_2c: ;BIT abcd
		NO_STOP
		ABSOLUTE
		MEMORY_GetByte
		BIT6502
		DONE

_JIT_insn_opcode_2d: ;AND abcd
		NO_STOP
		ABSOLUTE
		MEMORY_GetByte
		AND6502
		DONE

_JIT_insn_opcode_2e: ;ROL abcd
		NO_STOP
		ABSOLUTE
		move.l	d0,-(sp)
		RMW_GetByte
		ROL_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_2f: ;RLA abcd [unofficial - ROL Mem, then AND with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_30: ;BMI
		IS_STOP
		BRANCH_WITH_BYTES_AND_CYCLES
.m68k_start:
		tst.b	N
		bpl.b	.next
		BRANCH
.next:	M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_31: ;AND (ab),y
		NO_STOP
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		AND6502
		DONE

_JIT_insn_opcode_33: ;RLA (ab),y [unofficial - ROL Mem, then AND with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_35: ;AND ab,x
		NO_STOP
		ZPAGE_X
		MEMORY_dGetByte
		AND6502
		DONE

_JIT_insn_opcode_36: ;ROL ab,x
		NO_STOP
		ZPAGE_X
		move.l	d0,-(sp)
		MEMORY_dGetByte
		ROR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_37: ;RLA ab,x [unofficial - ROL Mem, then AND with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_38: ;SEC
		NO_STOP
		IMPLIED
		st		C
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_39: ;AND abcd,y
		NO_STOP
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		AND6502
		DONE

_JIT_insn_opcode_3b: ;RLA abcd,y [unofficial - ROL Mem, then AND with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_3d: ;AND abcd,x
		NO_STOP
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		AND6502
		DONE

_JIT_insn_opcode_3e: ;ROL abcd,x
		NO_STOP
		ABSOLUTE_X
		move.l	d0,-(sp)
		RMW_GetByte
		ROL_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_3f: ;RLA abcd,x [unofficial - ROL Mem, then AND with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_40: ;RTI
		IS_STOP
		NO_OFFSET_WITH_CYCLES
.m68k_start:
		PLP
		PLW
		move.w	d0,reg_PC
		CHECKIRQ
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		rts
		DONE

_JIT_insn_opcode_41: ;EOR (ab,x)
		NO_STOP
		INDIRECT_X
		MEMORY_GetByte
		EOR6502
		DONE

_JIT_insn_opcode_43: ;LSE (ab,x) [unofficial - LSR then EOR result with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_45: ;EOR ab
		NO_STOP
		ZPAGE
		MEMORY_dGetByte
		EOR6502
		DONE

_JIT_insn_opcode_46: ;LSR ab
		NO_STOP
		ZPAGE
		move.l	d0,-(sp)
		MEMORY_dGetByte
		LSR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_47: ;LSE ab [unofficial - LSR then EOR result with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_48: ;PHA
		NO_STOP
		IMPLIED
		move.b	reg_A,d0
		PH
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_49: ;EOR #ab
		NO_STOP
		IMMEDIATE
		EOR6502
		DONE

_JIT_insn_opcode_4a: ;LSR
		NO_STOP
		ACCUMULATOR
		LSR_ONLY reg_A
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_4b: ;ALR #ab [unofficial - Acc AND Data, LSR result]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_4c: ;JMP abcd
		IS_STOP
		JUMP_WITH_CYCLES
.m68k_start:
.m68k_jump:
		move.w	#$abcd,reg_PC
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		rts
		DONE

_JIT_insn_opcode_4d: ;EOR abcd
		NO_STOP
		ABSOLUTE
		MEMORY_GetByte
		EOR6502
		DONE

_JIT_insn_opcode_4e: ;LSR abcd
		NO_STOP
		ABSOLUTE
		move.l	d0,-(sp)
		RMW_GetByte
		LSR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_4f: ;LSE abcd [unofficial - LSR then EOR result with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_50: ;BVC
		IS_STOP
		BRANCH_WITH_BYTES_AND_CYCLES
.m68k_start:
		tst.b	V
		bne.b	.next
		BRANCH
.next:	M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_51: ;EOR (ab),y
		NO_STOP
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		EOR6502
		DONE

_JIT_insn_opcode_53: ;LSE (ab),y [unofficial - LSR then EOR result with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_55: ;EOR ab,x
		NO_STOP
		ZPAGE_X;
		MEMORY_dGetByte
		EOR6502
		DONE

_JIT_insn_opcode_56: ;LSR ab,x
		NO_STOP
		ZPAGE_X
		move.l	d0,-(sp)
		MEMORY_dGetByte
		LSR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_57: ;LSE ab,x [unofficial - LSR then EOR result with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_58: ;CLI
		NO_STOP
		IMPLIED
		ClrI
		CHECKIRQ
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_59: ;EOR abcd,y
		NO_STOP
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		EOR6502
		DONE

_JIT_insn_opcode_5b: ;LSE abcd,y [unofficial - LSR then EOR result with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_5d: ;EOR abcd,x
		NO_STOP
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		EOR6502
		DONE

_JIT_insn_opcode_5e: ;LSR abcd,x
		NO_STOP
		ABSOLUTE_X
		move.l	d0,-(sp)
		RMW_GetByte
		LSR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_5f: ;LSE abcd,x [unofficial - LSR then EOR result with A]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_60: ;RTS
		IS_STOP
		NO_OFFSET_WITH_CYCLES
.m68k_start:
		PLW
		addq.w	#1,d0
		move.w	d0,reg_PC
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		; TODO (incl. local->global...)
		;if (CPU_rts_handler != NULL) {
		;	CPU_rts_handler();
		;	CPU_rts_handler = NULL;
		;}
		rts
		DONE

_JIT_insn_opcode_61: ;ADC (ab,x)
		NO_STOP
		INDIRECT_X
		MEMORY_GetByte
		ADC6502
		DONE

_JIT_insn_opcode_63: ;RRA (ab,x) [unofficial - ROR Mem, then ADC to Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_65: ;ADC ab
		NO_STOP
		ZPAGE
		MEMORY_dGetByte
		ADC6502
		DONE

_JIT_insn_opcode_66: ;ROR ab
		NO_STOP
		ZPAGE
		move.l	d0,-(sp)
		MEMORY_dGetByte
		ROR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_67: ;RRA ab [unofficial - ROR Mem, then ADC to Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_68: ;PLA
		NO_STOP
		IMPLIED
		PL
		move.b	d0,reg_A
		move.b	d0,N
		move.b	d0,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_69: ;ADC #ab
		NO_STOP
		IMMEDIATE
		ADC6502
		DONE

_JIT_insn_opcode_6a: ;ROR
		NO_STOP
		ACCUMULATOR
		ROR_ONLY reg_A
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_6b: ;ARR #ab [unofficial - Acc AND Data, ROR result]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_6c: ;JMP (abcd)
		IS_STOP
		OFFSET_WITH_CYCLES 4
.m68k_start:
		move.l	#$0000abcd,d0
		cmp.b	#$ff,d0
		bne.b	.no_bug
		move.l	d0,d1
		clr.b	d0
		MEMORY_dGetByte
		lsl.w	#8,d0
		; CHEATING, MEMORY_dGetByte
		move.b	(0.b,memory,d1.l*1),d0
		bra.b	.skip
.no_bug:
		MEMORY_dGetWord
.skip:
		move.w	d0,reg_PC
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		rts
		DONE

_JIT_insn_opcode_6d: ;ADC abcd
		NO_STOP
		ABSOLUTE
		MEMORY_GetByte
		ADC6502
		DONE

_JIT_insn_opcode_6e: ;ROR abcd
		NO_STOP
		ABSOLUTE
		move.l	d0,-(sp)
		RMW_GetByte
		ROR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_6f: ;RRA abcd [unofficial - ROR Mem, then ADC to Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_70: ;BVS
		IS_STOP
		BRANCH_WITH_BYTES_AND_CYCLES
.m68k_start:
		tst.b	V
		beq.b	.next
		BRANCH
.next:	M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_71: ;ADC (ab),y
		NO_STOP
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		ADC6502
		DONE

_JIT_insn_opcode_73: ;RRA (ab),y [unofficial - ROR Mem, then ADC to Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_75: ;ADC ab,x
		NO_STOP
		ZPAGE_X
		MEMORY_dGetByte
		ADC6502
		DONE

_JIT_insn_opcode_76: ;ROR ab,x
		NO_STOP
		ZPAGE_X
		move.l	d0,-(sp)
		MEMORY_dGetByte
		ROR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_77: ;RRA ab,x [unofficial - ROR Mem, then ADC to Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_78: ;SEI
		NO_STOP
		IMPLIED
		SetI
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_79: ;ADC abcd,y
		NO_STOP
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		ADC6502
		DONE

_JIT_insn_opcode_7b: ;RRA abcd,y [unofficial - ROR Mem, then ADC to Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_7d: ;ADC abcd,x
		NO_STOP
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		ADC6502
		DONE

_JIT_insn_opcode_7e: ;ROR abcd,x
		NO_STOP
		ABSOLUTE_X
		move.l	d0,-(sp)
		RMW_GetByte
		ROR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_7f: ;RRA abcd,x [unofficial - ROR Mem, then ADC to Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_81: ;STA (ab,x)
		NO_STOP
		INDIRECT_X
		move.b	reg_A,d1
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_83: ;SAX (ab,x) [unofficial - Store result A AND X
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_84: ;STY ab
		NO_STOP
		ZPAGE
		move.b	reg_Y,d1
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_85: ;STA ab
		NO_STOP
		ZPAGE
		move.b	reg_A,d1
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_86: ;STX ab
		NO_STOP
		ZPAGE
		move.b	reg_X,d1
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_87: ;SAX ab [unofficial - Store result A AND X]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_88: ;DEY
		NO_STOP
		IMPLIED
		subq.b	#1,reg_Y
		move.b	reg_Y,N
		move.b	reg_Y,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_8a: ;TXA
		NO_STOP
		IMPLIED
		move.b	reg_X,reg_A
		move.b	reg_A,N
		move.b	reg_A,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_8b: ;ANE #ab [unofficial - A AND X AND (Mem OR $EF) to Acc] (Fox)
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_8c: ;STY abcd
		NO_STOP
		ABSOLUTE
		move.b	reg_Y,d1
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_8d: ;STA abcd
		NO_STOP
		ABSOLUTE
		move.b	reg_A,d1
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_8e: ;STX abcd
		NO_STOP
		ABSOLUTE
		move.b	reg_X,d1
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_8f: ;SAX abcd [unofficial - Store result A AND X]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_90: ;BCC
		IS_STOP
		BRANCH_WITH_BYTES_AND_CYCLES
.m68k_start:
		tst.b	C
		bne.b	.next
		BRANCH
.next:	M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_91: ;STA (ab),y
		NO_STOP
		INDIRECT_Y
		move.b	reg_A,d1
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_93: ;SHA (ab),y [unofficial, UNSTABLE - Store A AND X AND (H+1) ?] (Fox)
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_94: ;STY ab,x
		NO_STOP
		ZPAGE_X
		move.b	reg_Y,d1
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_95: ;STA ab,x
		NO_STOP
		ZPAGE_X
		move.b	reg_A,d1
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_96: ;STX ab,y
		NO_STOP
		ZPAGE_X
		move.b	reg_X,d1
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_97: ;SAX ab,y [unofficial - Store result A AND X]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_98: ;TYA
		NO_STOP
		IMPLIED
		move.b	reg_Y,reg_A
		move.b	reg_A,N
		move.b	reg_A,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_99: ;STA abcd,y
		NO_STOP
		ABSOLUTE_Y
		move.b	reg_A,d1
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_9a: ;TXS
		NO_STOP
		IMPLIED
		move.b	reg_X,reg_S
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_9b: ;SHS abcd,y [unofficial, UNSTABLE] (Fox)
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_9c: ;SHY abcd,x [unofficial - Store Y and (H+1)] (Fox)
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_9d: ;STA abcd,x
		NO_STOP
		ABSOLUTE_X
		move.b	reg_A,d1
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_9e: ;SHX abcd,y [unofficial - Store X and (H+1)] (Fox)
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_9f: ;SHA abcd,y [unofficial, UNSTABLE - Store A AND X AND (H+1) ?] (Fox)
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_a0: ;LDY #ab
		NO_STOP
		IMMEDIATE
		LDY6502
		DONE

_JIT_insn_opcode_a1: ;LDA (ab,x)
		NO_STOP
		INDIRECT_X
		MEMORY_GetByte
		LDA6502
		DONE

_JIT_insn_opcode_a2: ;LDX #ab
		NO_STOP
		IMMEDIATE
		LDX6502
		DONE

_JIT_insn_opcode_a3: ;LAX (ab,x) [unofficial]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_a4: ;LDY ab
		NO_STOP
		ZPAGE
		MEMORY_dGetByte
		LDY6502
		DONE

_JIT_insn_opcode_a5: ;LDA ab
		NO_STOP
		ZPAGE
		MEMORY_dGetByte
		LDA6502
		DONE

_JIT_insn_opcode_a6: ;LDX ab
		NO_STOP
		ZPAGE
		MEMORY_dGetByte
		LDX6502
		DONE

_JIT_insn_opcode_a7: ;LAX ab [unofficial]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_a8: ;TAY
		NO_STOP
		IMPLIED
		move.b	reg_A,reg_Y
		move.b	reg_Y,N
		move.b	reg_Y,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_a9: ;LDA #ab
		NO_STOP
		IMMEDIATE
		LDA6502
		DONE

_JIT_insn_opcode_aa: ;TAX
		NO_STOP
		IMPLIED
		move.b	reg_A,reg_X
		move.b	reg_X,N
		move.b	reg_X,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_ab: ;ANX #ab [unofficial - AND #ab, then TAX]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_ac: ;LDY abcd
		NO_STOP
		ABSOLUTE
		MEMORY_GetByte
		LDY6502
		DONE

_JIT_insn_opcode_ad: ;LDA abcd
		NO_STOP
		ABSOLUTE
		MEMORY_GetByte
		LDA6502
		DONE

_JIT_insn_opcode_ae: ;LDX abcd
		NO_STOP
		ABSOLUTE
		MEMORY_GetByte
		LDX6502
		DONE

_JIT_insn_opcode_af: ;LAX abcd [unofficial]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_b0: ;BCS
		IS_STOP
		BRANCH_WITH_BYTES_AND_CYCLES
.m68k_start:
		tst.b	C
		beq.b	.next
		BRANCH
.next:	M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_b1: ;LDA (ab),y
		NO_STOP
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		LDA6502
		DONE

_JIT_insn_opcode_b3: ;LAX (ab),y [unofficial]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_b4: ;LDY ab,x
		NO_STOP
		ZPAGE_X
		MEMORY_dGetByte
		LDY6502
		DONE

_JIT_insn_opcode_b5: ;LDA ab,x
		NO_STOP
		ZPAGE_X
		MEMORY_dGetByte
		LDA6502
		DONE

_JIT_insn_opcode_b6: ;LDX ab,y
		NO_STOP
		ZPAGE_Y
		MEMORY_dGetByte
		LDX6502
		DONE

_JIT_insn_opcode_b7: ;LAX ab,y [unofficial]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_b8: ;CLV
		NO_STOP
		IMPLIED
		clr.b	V
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_b9: ;LDA abcd,y
		NO_STOP
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		LDA6502
		DONE

_JIT_insn_opcode_ba: ;TSX
		NO_STOP
		IMPLIED
		move.b	reg_S,reg_X
		move.b	reg_X,N
		move.b	reg_X,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_bb: ;LAS abcd,y [unofficial - AND S with Mem, transfer to A and X (Fox)
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_bc: ;LDY abcd,x
		NO_STOP
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		LDY6502
		DONE

_JIT_insn_opcode_bd: ;LDA abcd,x
		NO_STOP
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		LDA6502
		DONE

_JIT_insn_opcode_be: ;LDX abcd,y
		NO_STOP
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		LDX6502
		DONE

_JIT_insn_opcode_bf: ;LAX abcd,y [unofficial]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_c0: ;CPY #ab
		NO_STOP
		IMMEDIATE
		CPY6502
		DONE

_JIT_insn_opcode_c1: ;CMP (ab,x)
		NO_STOP
		INDIRECT_X
		MEMORY_GetByte
		CMP6502
		DONE

_JIT_insn_opcode_c3: ;DCM (ab,x) [unofficial - DEC Mem then CMP with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_c4: ;CPY ab
		NO_STOP
		ZPAGE
		MEMORY_dGetByte
		CPY6502
		DONE

_JIT_insn_opcode_c5: ;CMP ab
		NO_STOP
		ZPAGE
		MEMORY_dGetByte
		CMP6502
		DONE

_JIT_insn_opcode_c6: ;DEC ab
		NO_STOP
		ZPAGE
		move.l	d0,-(sp)
		MEMORY_dGetByte
		subq.b	#1,d0
		move.b	d0,N
		move.b	d0,Z
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_c7: ;DCM ab [unofficial - DEC Mem then CMP with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_c8: ;INY
		NO_STOP
		IMPLIED
		addq.b	#1,reg_Y
		move.b	reg_Y,N
		move.b	reg_Y,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_c9: ;CMP #ab
		NO_STOP
		IMMEDIATE
		CMP6502
		DONE

_JIT_insn_opcode_ca: ;DEX
		NO_STOP
		IMPLIED
		subq.b	#1,reg_X
		move.b	reg_X,N
		move.b	reg_X,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_cb: ;SBX #ab [unofficial - store ((A AND X) - Mem) in X] (Fox)
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_cc: ;CPY abcd
		NO_STOP
		ABSOLUTE
		MEMORY_GetByte
		CPY6502
		DONE

_JIT_insn_opcode_cd: ;CMP abcd
		NO_STOP
		ABSOLUTE
		MEMORY_GetByte
		CMP6502
		DONE

_JIT_insn_opcode_ce: ;DEC abcd
		NO_STOP
		ABSOLUTE
		move.l	d0,-(sp)
		RMW_GetByte
		subq.b	#1,d0
		move.b	d0,N
		move.b	d0,Z
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_cf: ;DCM abcd [unofficial - DEC Mem then CMP with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_d0: ;BNE
		IS_STOP
		BRANCH_WITH_BYTES_AND_CYCLES
.m68k_start:
		tst.b	Z
		beq.b	.next
		BRANCH
.next:	M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_d1: ;CMP (ab),y
		NO_STOP
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		CMP6502
		DONE

_JIT_insn_opcode_d3: ;DCM (ab),y [unofficial - DEC Mem then CMP with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_d5: ;CMP ab,x
		NO_STOP
		ZPAGE_X
		MEMORY_dGetByte
		CMP6502
		DONE

_JIT_insn_opcode_d6: ;DEC ab,x
		NO_STOP
		ZPAGE_X
		move.l	d0,-(sp)
		MEMORY_dGetByte
		subq.b	#1,d0
		move.b	d0,N
		move.b	d0,Z
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_d7: ;DCM ab,x [unofficial - DEC Mem then CMP with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_d8: ;CLD
		NO_STOP
		IMPLIED
		ClrD
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_d9: ;CMP abcd,y
		NO_STOP
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		CMP6502
		DONE

_JIT_insn_opcode_db: ;DCM abcd,y [unofficial - DEC Mem then CMP with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_dd: ;CMP abcd,x
		NO_STOP
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		CMP6502
		DONE

_JIT_insn_opcode_de: ;DEC abcd,x
		NO_STOP
		ABSOLUTE_X
		move.l	d0,-(sp)
		RMW_GetByte
		subq.b	#1,d0
		move.b	d0,N
		move.b	d0,Z
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_df: ;DCM abcd,x [unofficial - DEC Mem then CMP with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_e0: ;CPX #ab
		NO_STOP
		IMMEDIATE
		CPX6502
		DONE

_JIT_insn_opcode_e1: ;SBC (ab,x)
		NO_STOP
		INDIRECT_X
		MEMORY_GetByte
		SBC6502
		DONE

_JIT_insn_opcode_e3: ;INS (ab,x) [unofficial - INC Mem then SBC with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_e4: ;CPX ab
		NO_STOP
		ZPAGE
		MEMORY_dGetByte
		CPX6502
		DONE

_JIT_insn_opcode_e5: ;SBC ab
		NO_STOP
		ZPAGE
		MEMORY_dGetByte
		SBC6502
		DONE

_JIT_insn_opcode_e6: ;INC ab
		NO_STOP
		ZPAGE
		move.l	d0,-(sp)
		MEMORY_dGetByte
		addq.b	#1,d0
		move.b	d0,N
		move.b	d0,Z
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_e7: ;INS ab [unofficial - INC Mem then SBC with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_e8: ;INX
		NO_STOP
		IMPLIED
		addq.b	#1,reg_X
		move.b	reg_X,N
		move.b	reg_X,Z
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_eb: ;SBC #ab [unofficial]; e9 official
		NO_STOP
		IMMEDIATE
		SBC6502
		DONE

_JIT_insn_opcode_fa: ;NOP [unofficial]; ea official
		NO_STOP
		IMPLIED
		; nothing to do
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_ec: ;CPX abcd
		NO_STOP
		ABSOLUTE
		MEMORY_GetByte
		CPX6502
		DONE

_JIT_insn_opcode_ed: ;SBC abcd
		NO_STOP
		ABSOLUTE
		MEMORY_GetByte
		SBC6502
		DONE

_JIT_insn_opcode_ee: ;INC abcd
		NO_STOP
		ABSOLUTE
		move.l	d0,-(sp)
		RMW_GetByte
		addq.b	#1,d0
		move.b	d0,N
		move.b	d0,Z
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_ef: ;INS abcd [unofficial - INC Mem then SBC with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_f0: ;BEQ
		IS_STOP
		BRANCH_WITH_BYTES_AND_CYCLES
.m68k_start:
		tst.b	Z
		bne.b	.next
		BRANCH
.next:	M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_f1: ;SBC (ab),y
		NO_STOP
		INDIRECT_Y
		NCYCLES_Y;
		MEMORY_GetByte
		SBC6502
		DONE

_JIT_insn_opcode_f3: ;INS (ab),y [unofficial - INC Mem then SBC with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_f5: ;SBC ab,x
		NO_STOP
		ZPAGE_X
		MEMORY_dGetByte
		SBC6502
		DONE

_JIT_insn_opcode_f6: ;INC ab,x
		NO_STOP
		ZPAGE_X
		move.l	d0,-(sp)
		MEMORY_dGetByte
		addq.b	#1,d0
		move.b	d0,N
		move.b	d0,Z
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_dPutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_f7: ;INS ab,x [unofficial - INC Mem then SBC with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_f8: ;SED
		NO_STOP
		IMPLIED
		SetD
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_f9: ;SBC abcd,y
		NO_STOP
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		SBC6502
		DONE

_JIT_insn_opcode_fb: ;INS abcd,y [unofficial - INC Mem then SBC with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_fd: ;SBC abcd,x
		NO_STOP
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		SBC6502
		DONE

_JIT_insn_opcode_fe: ;INC abcd,x
		NO_STOP
		ABSOLUTE_X
		move.l	d0,-(sp)
		RMW_GetByte
		addq.b	#1,d0
		move.b	d0,N
		move.b	d0,Z
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		M68K_BYTES_TEMPLATE
		M68K_CYCLES_TEMPLATE
		DONE

_JIT_insn_opcode_ff: ;INS abcd,x [unofficial - INC Mem then SBC with Acc]
		;NO_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_d2: ;ESCRTS #ab (CIM) - on Atari is here instruction CIM [unofficial] !RS!
		;IS_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_f2: ;ESC #ab (CIM) - on Atari is here instruction CIM [unofficial] !RS!
		;IS_STOP
		NOT_IMPLEMENTED

_JIT_insn_opcode_b2: ;CIM [unofficial - crash intermediate]
		;IS_STOP
		NOT_IMPLEMENTED

; ---------------------------------------------
		section	data
; ---------------------------------------------

addql_xpos_table:
		nop
		addq.l	#1,xpos
		addq.l	#2,xpos
		addq.l	#3,xpos
		addq.l	#4,xpos
		addq.l	#5,xpos
		addq.l	#6,xpos
		addq.l	#7,xpos
		addq.l	#8,xpos

addql_pc_table:
		nop
		addq.l	#1,reg_PC
		addq.l	#2,reg_PC
		addq.l	#3,reg_PC
