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

		opt		a-;,o4-,o12-,og-				; don't automatically optimize absolute to PC-relative references
												; don't optimize move.l to moveq
												; don't optimize <op>.l #x,An to <op>.w #x,An.
												; disable generic vasm optimizations

		xref	_ANTIC_xpos
		xref	_ANTIC_xpos_limit
		xref	_CPU_IRQ
		xref	_CPU_regA
		xref	_CPU_regX
		xref	_CPU_regY
		xref	_CPU_regP
		xref	_CPU_regS
MEMORY_RAM		equ	0
MEMORY_ROM		equ	1
MEMORY_HARDWARE	equ	2
		xref	_MEMORY_attrib					; UBYTE MEMORY_attrib[65536];
		xref	_MEMORY_HwGetByte				; UBYTE MEMORY_HwGetByte(UWORD addr, int no_side_effects)
		xref	_MEMORY_HwPutByte				; void MEMORY_HwPutByte(UWORD addr, UBYTE byte)
		xref	_MEMORY_mem						; UBYTE MEMORY_mem[65536 + 2]
		xref	_MEMORY_JIT_mem					; struct CPU_JIT_native_code_t MEMORY_JIT_mem[65536 + 2]

		xdef	_host_cpu
		xdef	_CPU_JIT_Execute
		xdef	_CPU_JIT_Instance
		xref	_CPU_JIT_Invalidate				; CPU_JIT_Invalidate(const UWORD addr)

		xref	_GTIA_GetByte
		xref	_POKEY_GetByte
		xref	_PIA_GetByte
		xref	_ANTIC_GetByte

		xref	_GTIA_PutByte
		xref	_POKEY_PutByte
		xref	_PIA_PutByte
		xref	_ANTIC_PutByte

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

unknown	equ		-1
no_stop	equ		0
is_stop	equ		1

reg_A	equr	d2								; .b
reg_X	equr	d3								; .b
reg_Y	equr	d4								; .b
reg_CCR	equr	d5								; .w (only CCR_N & CCR_Z)
V		equr	d6								; .b (<> 0  => V flag set)
C		equr	d7								; .b (< 0   => C flag set)

memory	equr	a2								; .l
mem_attrib equr	a3
memory_jit equr	a4
reg_PC	equr	a5								; .w (must be written as long due to sign extension)
xpos	equr	a6								; .l, ANTIC_xpos mirror

		include	"cpu_m68k_jit_flags.asm"
		include	"cpu_m68k_jit_addr.asm"

		xref	_DUMP
		macro	DUMP
		;move.w	reg_PC,_CPU_regPC
		;move.b	reg_A,_CPU_regA
		;move.b	reg_X,_CPU_regX
		;move.b	reg_Y,_CPU_regY
		;move.b	reg_S+1,_CPU_regS
		;jsr		_DUMP
		endm

; ---------------------------------------------
		section	text
; ---------------------------------------------

; void CPU_JIT_Execute(const UBYTE* pCode);
_CPU_JIT_Execute:
		movea.l	(4,sp),a0						; code
		movem.l	d2-d7/a2-a6,-(sp)

		clr.l	d0
		move.w	_CPU_regPC,d0
		move.l	d0,reg_PC
		clr.l	reg_A
		move.b	_CPU_regA,reg_A
		clr.l	reg_X
		move.b	_CPU_regX,reg_X
		clr.l	reg_Y
		move.b	_CPU_regY,reg_Y
		move.b	_CPU_regS,reg_S+1

		PutStatus _CPU_regP
		and.b	#%00111100,_CPU_regP			; NV*BDIZC -> 00*BDI00

		lea		_MEMORY_mem+$8000,memory
		lea		_MEMORY_attrib+$8000,mem_attrib
		lea		_MEMORY_JIT_mem+$8000,memory_jit
		move.l	_ANTIC_xpos,xpos

		jsr		(a0)

		move.l	xpos,_ANTIC_xpos

		GetStatus d0
		move.b	d0,_CPU_regP

		move.b	reg_S+1,_CPU_regS
		move.b	reg_Y,_CPU_regY
		move.b	reg_X,_CPU_regX
		move.b	reg_A,_CPU_regA
		move.w	reg_PC,_CPU_regPC

		movem.l	(sp)+,d2-d7/a2-a6
		rts

; void CPU_JIT_Instance(UBYTE* dst_buf, const struct CPU_JIT_insn_template_t *src_template,
;                       const UWORD data, const UWORD data_extra,
;                       const int bytes,
;                       const int cycles, const int cycles_extra);
				rsreset
header_stop		rs.w	1
header_addressing rs.w	1
header_h_begin	rs.l	1
header_h_end	rs.l	1
header_f_begin	rs.l	1
header_f_end	rs.l	1
header_cycles	rs.l	1
header_bytes	rs.l	1
header_p_base	rs.l	1						; struct m68k_t*
header_p_hw		rs.l	1						; struct m68k_t* or NULL
header_p_rmw	rs.l	1						; struct m68k_t* or NULL
header_p_hw_rmw	rs.l	1						; struct m68k_t* or NULL

				rsreset							; struct m68k_t
m68k_start		rs.l	1						;
m68k_end		rs.l	1						;
m68k_data1		rs.l	1						;
m68k_data2		rs.l	1						;

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

		move.w	(insn_template_data_offset1,a0),d0
		bmi.b	.no_data1
		move.l	(12,sp),d1						; d1.w: data
		or.w	d1,(0.b,a1,d0.w*1)
.no_data1:
		move.w	(insn_template_data_offset2,a0),d0
		bmi.b	.no_data2
		move.l	(16,sp),d1						; d1.w: data_extra
		or.w	d1,(0.b,a1,d0.w*1)
.no_data2:
		move.w	(insn_template_bytes_offset,a0),d0
		bmi.b	.no_bytes
		move.l	(20,sp),d1						; d1.l: bytes
		move.w	(addql_pc_table.l,pc,d1.l*2),(0.b,a1,d0.w*1)
.no_bytes:
		move.w	(insn_template_cycles_offset1,a0),d0
		bmi.b	.no_cycles1
		move.l	(24,sp),d1						; d1.l: cycles
		move.w	(addql_xpos_table.l,pc,d1.l*2),(0.b,a1,d0.w*1)
.no_cycles1:
		move.w	(insn_template_cycles_offset2,a0),d0
		bmi.b	.no_cycles2
		move.l	(28,sp),d1						; d1.l: cycles_extra
		move.w	(addql_xpos_table.l,pc,d1.l*2),(0.b,a1,d0.w*1)
.no_cycles2:
		movem.l	d2/a2,-(sp)
		pea		.clear_cache
		move.w	#38,-(sp)
		trap	#14
		addq.l	#6,sp
		movem.l	(sp)+,d2/a2
		rts

; TODO: per entry operation
.clear_cache:
		cmpi.l	#40,_host_cpu
		bge.b	.cpu040_60
.cpu030:
		movec	cacr,d0
		or.b	#%00001000,d0					; CI bit (clear i-cache)
		movec	d0,cacr
		rts
.cpu040_60:
		move	sr,d0
		ori		#$0700,sr						; ints off
		nop										; fix for some broken 040s
		cpusha	bc								; flush (and possibly invalidate) d-cache & invalidate i-cache
		move	d0,sr							; ints back
		rts

; ----------------------------------------------

P_BASE	equ		.m68k_p_base
P_HW	equ		.m68k_p_hw
P_RMW	equ		.m68k_p_rmw
P_HW_RMW equ	.m68k_p_hw_rmw
P_NULL	equ		0

		macro	HEADER
		dc.w	\1								; \1: stop
		dc.w	\2								; \2: base addressing
		dc.l	.m68k_header
		dc.l	.m68k_header_end
		dc.l	.m68k_footer
		dc.l	.m68k_footer_end
		dc.l	.m68k_cycles
		dc.l	.m68k_bytes
		dc.l	P_BASE
		dc.l	\3								; \3: p_hw
		dc.l	\4								; \4: p_rmw
		dc.l	\5								; \5: p_hw_rmw
.m68k_header:
		DUMP
.m68k_cycles equ *+0
		addq.l	#1,xpos							; 2 - 8
.m68k_header_end:
		endm

		macro	FOOTER
.m68k_footer:
.m68k_bytes equ *+0
		addq.w	#1,reg_PC						; 1 - 3
		cmp.l	_ANTIC_xpos_limit,xpos
		blt.b	.next_insn
		rts
.next_insn:
.m68k_footer_end:
		endm

		macro	FOOTER_NO_BYTES
.m68k_footer:
.m68k_bytes equ 0
		cmp.l	_ANTIC_xpos_limit,xpos
		blt.b	.next_insn
		rts
.next_insn:
.m68k_footer_end:
		endm

* 		HEADER	no_stop,absolute,P_HW,P_RMW,P_HW_RMW
* 		AND6502	ABSOLUTE,reg_A
* 		AND6502	ABSOLUTE_HW,reg_A
* 		AND6502	ABSOLUTE_RMW,reg_A
* 		AND6502	ABSOLUTE_HW_RMW,reg_A
* 		FOOTER

		macro	AND6502							; \1: <addressing>, \2: reg_A/#imm
		\1		and,\2
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	CMP6502							; \1: <addressing>, \2: reg_[AXY]/#imm
		\1		cmp,\2
		SAVE_NZc
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	EOR6502							; \1: <addressing>, \2: reg_A/#imm
		\1		eor,\2
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	LD6502							; \1: <addressing>, \2: reg_[AXY]/#imm
		\1		move,\2
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	ORA6502							; \1: <addressing>, \2: reg_A/#imm
		\1		or,\2
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	ADC6502							; \1: <addressing>, \2: reg_A/#imm	TODO
		btst	#D_FLAG,_CPU_regP
		beq.b	.binary_adc\@
.decimal_adc\@:
		pea		(.skip\@,pc)
		jmp		decimal_adc
.binary_adc\@:
		LOAD_C
		addx.b	d0,reg_A
		SAVE_NVZC
.skip\@:
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	SBC6502							; \1.b: #imm or dn TODO moveq
		btst	#D_FLAG,_CPU_regP
		beq.b	.binary_sbc\@
.decimal_sbc\@:
		pea		(.skip\@,pc)
		jmp		decimal_sbc
.binary_sbc\@:
		LOAD_c
		subx.b	d0,reg_A
		SAVE_NVZc
.skip\@:
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	ST6502							; \1: <addressing>, \2: reg_[AXY]
		\1		\2
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	BIT6502							; \1: <addressing>, \2: reg_A
		\1		move,V
		move	ccr,reg_CCR						; save N
		and.b	\2,V
		bne.b	.ne\@
		or.b	#(1<<CCR_Z),reg_CCR				; save Z
.ne\@:	and.b	#(1<<V_FLAG),V
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	ROL_ONLY						; \1.b: dn
		LOAD_C
		addx.b	\1,\1
		SAVE_NZC
		endm
		macro	ROL6502							; \1: <addressing>, \2: reg_A/dn
		\1		ROL_ONLY,2
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	ROR_ONLY						; \1.b: dn
		LOAD_C
		roxr.b	#1,\1
		SAVE_NZC
		endm
		macro	ROR6502							; \1: <addressing>, \2: reg_A/dn
		\1		ROR_ONLY,\2
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	ASL_ONLY						; \1.b: dn
		add.b	\1,\1
		SAVE_NZC
		endm
		macro	ASL6502							; \1: <addressing>, \2: reg_A/dn
		\1		ASL_ONLY,\2
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	LSR_ONLY						; \1.b: dn
		lsr.b	#1,\1
		SAVE_NZC
		endm
		macro	LSR6502							; \1: <addressing>, \2: reg_A/dn
		\1		LSR_ONLY,\2
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	INC_ONLY						; \1.b: dn
		addq.b	#1,\1
		SAVE_NZ
		endm
		macro	INC6502							; \1: <addressing>, \2: reg_A/dn
		\1		INC_ONLY,\2
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		macro	DEC_ONLY						; \1.b: dn
		subq.b	#1,\1
		SAVE_NZ
		endm
		macro	DEC6502							; \1: <addressing>, \2: reg_A/dn
		\1		DEC_ONLY,\2
		UPDATE_PC
		RETURN_OR_CONTINUE
		endm

		; input:    (clean d0.l)
		; output:   d0.b
		; clobbers: d0.w
		macro	PL
		addq.b	#1,reg_S+1
		move.w	reg_S,d0
		MEMORY_dGetByte
		endm

		; input:    d0.b (clean d0.l)
		; output:   -
		; clobbers: d0.w, d1.b
		macro	PH
		move.b	d0,d1
		move.w	reg_S,d0
		subq.b	#1,reg_S+1
		MEMORY_dPutByte
		endm

		; input:    d0.w (big endian, clean d0.l)
		; output:   -
		; clobbers: d0.w, d1.b, a0
		macro	PHW
		move.l	d0,a0
		lsr.w	#8,d0
		PH
		move.l	a0,d0
		PH
		endm

		; input:    (clean d0.l)
		; output:   d0.w
		; clobbers: d0.w, d1.w
		macro	PLW
		PL
		move.w	d0,d1
		PL
		lsl.w	#8,d0
		move.b	d1,d0
		endm

		; input:    (clean d0.l)
		; output:   -
		; clobbers: d0.w, d1.b
		macro	PHPB0
		GetStatus d0
		;         NV*BDIZC
		and.b	#%11101111,d0
		PH										; push flags with B flag clear (NMI, IRQ)
		endm

		; input:    (clean d0.l)
		; output:   -
		; clobbers: d0.w, d1.b
		macro	PHPB1
		GetStatus d0
		PH										; push flags with B flag set (PHP, BRK)
		endm

		; input:    (clean d0.l)
		; output:   d0.b
		; clobbers: d0.w
		macro	PLP
		PL
		PutStatus d0
		;         NV*BDIZC
		and.b	#%00001100,d0
		add.b	#%00110000,d0
		move.b	d0,_CPU_regP
		endm

		; input:    (clean d0.l)
		; output:   -
		; clobbers: d0.w, d1.b, a0
		macro	PHPC
		move.l	reg_PC,d0
		PHW
		endm

		; input:    -
		; output:   -
		; clobbers: -
		macro	RETURN_OR_CONTINUE
		cmp.l	_ANTIC_xpos_limit,xpos
		blt.b	.next_insn\@
		rts
.next_insn\@:
		endm

		; input:    -
		; output:   -
		; clobbers: d0.l, a0
		macro	RETURN_OR_JUMP
		jmp		return_or_jump
		endm

		; TODO: we know what is the value of the PC and the jump destination
;       so why not calculate it in advance?
		macro	BRANCH							; <cc>
		b\1.b	.taken\@
		UPDATE_PC
		RETURN_OR_JUMP
.taken\@:
.m68k_data:
		move.l	#$00000000,d0
		move.l	d0,reg_PC
.m68k_cycles_extra:
		addq.l	#1,xpos							; 1 - 2
		RETURN_OR_JUMP
		DONE
		endm

		; input:    (clean d0.l)
		; output:   -
		; clobbers: d0.w, d1.b, a0
		macro	CHECKIRQ
		tst.b	_CPU_IRQ
		beq		.skip\@
		btst	#I_FLAG,_CPU_regP
		bne		.skip\@
		cmp.l	_ANTIC_xpos_limit,xpos
		bge		.skip\@
		PHPC
		PHPB0
		SetI
		move.l	#$0000fffe,d0
		MEMORY_dGetWord
		move.l	d0,reg_PC
		addq.l	#7,xpos
		RETURN_OR_JUMP
.skip\@:
		endm

; ----------------------------------------------

return_or_jump:
		cmp.l	_ANTIC_xpos_limit,xpos
		bge.b	.return
		move.l	(-$8000.w,memory_jit,reg_PC.l*8),d0	; UBYTE *insn_addr
		beq.b	.return
		movea.l	d0,a0
		jmp		(a0)
		; not reached...
.return:
		rts

decimal_adc:
		unpk	reg_A,d1,#0
		unpk	d0,V,#0
		LOAD_C
		addx.w	V,d1
		cmp.b	#$0a,d1
		blo.b	.no_carry
		add.w	#$0106,d1
.no_carry:
		pack	d1,d1,#0
		tst.b	d1
		move	ccr,reg_CCR						; save N
		and.w	#(1<<CCR_N),reg_CCR				;
		move.b	reg_A,V
		LOAD_C
		addx.b	d0,V
		move	ccr,V
		or.w	V,reg_CCR						; save Z
		eor.b	d0,reg_A
		not.b	reg_A
		eor.b	d1,d0
		and.b	reg_A,d0
		smi		V								; save V
		move.b	d1,reg_A
		cmp.w	#$0a00,d1
		shs		C								; save C
		blo.b	.no_carry2
		add.b	#$60,reg_A
.no_carry2:
		rts

decimal_sbc:
		move.b	reg_A,V
		unpk	reg_A,reg_A,#0
		unpk	d0,d1,#0
		not.b	C
		LOAD_C
		subx.w	d1,reg_A
		tst.b	reg_A
		bpl.b	.no_carry
		subq.w	#6,reg_A
.no_carry:
		pack	reg_A,reg_A,#0
		tst.w	reg_A
 		bpl.b	.no_carry2
 		sub.b	#$60,reg_A
.no_carry2:
		LOAD_C
		subx.b	d0,V
		SAVE_NVZc
		rts

rol_mem_rmw:
		MEMORY_Hw_RMW ROL_ONLY
		rts
ror_mem_rmw:
		MEMORY_Hw_RMW ROR_ONLY
		rts
asl_mem_rmw:
		MEMORY_Hw_RMW ASL_ONLY
		rts
lsr_mem_rmw:
		MEMORY_Hw_RMW LSR_ONLY
		rts
inc_mem_rmw:
		MEMORY_Hw_RMW INC_ONLY
		rts
dec_mem_rmw:
		MEMORY_Hw_RMW DEC_ONLY
		rts

rol_gtia_rmw:
		GTIA_RMW ROL_ONLY
		rts
ror_gtia_rmw:
		GTIA_RMW ROR_ONLY
		rts
asl_gtia_rmw:
		GTIA_RMW ASL_ONLY
		rts
lsr_gtia_rmw:
		GTIA_RMW LSR_ONLY
		rts
inc_gtia_rmw:
		GTIA_RMW INC_ONLY
		rts
dec_gtia_rmw:
		GTIA_RMW DEC_ONLY
		rts

rol_pokey_rmw:
		POKEY_RMW ROL_ONLY
		rts
ror_pokey_rmw:
		POKEY_RMW ROR_ONLY
		rts
asl_pokey_rmw:
		POKEY_RMW ASL_ONLY
		rts
lsr_pokey_rmw:
		POKEY_RMW LSR_ONLY
		rts
inc_pokey_rmw:
		POKEY_RMW INC_ONLY
		rts
dec_pokey_rmw:
		POKEY_RMW DEC_ONLY
		rts

rol_pia_rmw:
		PIA_RMW ROL_ONLY
		rts
ror_pia_rmw:
		PIA_RMW ROR_ONLY
		rts
asl_pia_rmw:
		PIA_RMW ASL_ONLY
		rts
lsr_pia_rmw:
		PIA_RMW LSR_ONLY
		rts
inc_pia_rmw:
		PIA_RMW INC_ONLY
		rts
dec_pia_rmw:
		PIA_RMW DEC_ONLY
		rts

rol_antic_rmw:
		ANTIC_RMW ROL_ONLY
		rts
ror_antic_rmw:
		ANTIC_RMW ROR_ONLY
		rts
asl_antic_rmw:
		ANTIC_RMW ASL_ONLY
		rts
lsr_antic_rmw:
		ANTIC_RMW LSR_ONLY
		rts
inc_antic_rmw:
		ANTIC_RMW INC_ONLY
		rts
dec_antic_rmw:
		ANTIC_RMW DEC_ONLY
		rts

JIT_Invalidate:
		move.l	d1,-(sp)
		move.l	d0,-(sp)
		jsr		_CPU_JIT_Invalidate
		move.l	(sp)+,d0
		move.l	(sp)+,d1
		rts

; ----------------------------------------------

_JIT_insn_opcode_00: ;BRK
		IMPLIED_RETURN
		addq.w	#1,reg_PC						; XXX: if reg_PC is an An, this is handled as .l
		PHPC
		PHPB1
		SetI
		move.l	#$0000fffe,d0
		MEMORY_dGetWord
		move.l	d0,reg_PC
		RETURN_OR_JUMP
		DONE

_JIT_insn_opcode_01: ;ORA (ab,x)
		INDIRECT_X
		MEMORY_GetByte
		ORA6502
		DONE

_JIT_insn_opcode_03: ;ASO (ab,x) [unofficial - ASL then ORA with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_64: ;NOP ab [unofficial - skip byte]
		IMPLIED
		; nothing to do
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_f4: ;NOP ab,x [unofficial - skip byte]
		IMPLIED
		; nothing to do
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_e2: ;NOP #ab [unofficial - skip byte]
		IMPLIED
		; nothing to do
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_05: ;ORA ab
		ZPAGE
.m68k_data:
		or.b	($0000.w,memory),reg_A
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_06: ;ASL ab
		ZPAGE_RMW
.m68k_data:
		move.b	($0000.w,memory),d0
		ASL_ONLY d0
.m68k_data_extra:
		move.b	d0,($0000.w,memory)
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_07: ;ASO ab [unofficial - ASL then ORA with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_08: ;PHP
		IMPLIED
		PHPB1
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_09: ;ORA #ab
		IMMEDIATE
		ORA6502
		DONE

_JIT_insn_opcode_0a: ;ASL
		ACCUMULATOR
		ASL_ONLY reg_A
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_2b: ;ANC #ab [unofficial - AND then copy N to C (Fox)
		NOT_IMPLEMENTED

_JIT_insn_opcode_0c: ;NOP abcd [unofficial - skip word]
		IMPLIED
		; nothing to do
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_0d: ;ORA abcd
		ABSOLUTE
		MEMORY_GetByte
		ORA6502
		DONE

_JIT_insn_opcode_0e: ;ASL abcd
		ABSOLUTE
		move.l	d0,-(sp)
		RMW_GetByte
		ASL_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_0f: ;ASO abcd [unofficial - ASL then ORA with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_10: ;BPL
		RELATIVE
		TEST_NZ
		BRANCH	pl

_JIT_insn_opcode_11: ;ORA (ab),y
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		ORA6502
		DONE

_JIT_insn_opcode_13: ;ASO (ab),y [unofficial - ASL then ORA with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_15: ;ORA ab,x
		ZPAGE_X
		MEMORY_dGetByte
		ORA6502
		DONE

_JIT_insn_opcode_16: ;ASL ab,x
		ZPAGE_X
		move.l	d0,-(sp)
		MEMORY_dGetByte
		ASL_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte_ZP
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_17: ;ASO ab,x [unofficial - ASL then ORA with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_18: ;CLC
		IMPLIED
		ClrC
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_19: ;ORA abcd,y
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		ORA6502
		DONE

_JIT_insn_opcode_1b: ;ASO abcd,y [unofficial - ASL then ORA with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_fc: ;NOP abcd,x [unofficial - skip word]
		IMPLIED
		; nothing to do
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_1d: ;ORA abcd,x
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		ORA6502
		DONE

_JIT_insn_opcode_1e: ;ASL abcd,x
		ABSOLUTE_X
		move.l	d0,-(sp)
		RMW_GetByte
		ASL_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_1f: ;ASO abcd,x [unofficial - ASL then ORA with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_20: ;JSR abcd
		ABSOLUTE_JUMP
		move.l	reg_PC,d0
		addq.w	#1+1,d0									; store PC one byte before next insn
		PHW
.m68k_data:
		move.l	#$00000000,reg_PC
		RETURN_OR_JUMP
		DONE

_JIT_insn_opcode_21: ;AND (ab,x)
		INDIRECT_X
		MEMORY_GetByte
		AND6502
		DONE

_JIT_insn_opcode_23: ;RLA (ab,x) [unofficial - ROL Mem, then AND with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_24: ;BIT ab
		ZPAGE
.m68k_data:
		move.b	($0000.w,memory),V
		move	ccr,reg_CCR						; save N
		and.b	reg_A,V
		bne.b	.ne
		or.b	#(1<<CCR_Z),reg_CCR				; save Z
.ne:	and.b	#(1<<V_FLAG),V
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_25: ;AND ab
		ZPAGE
.m68k_data:
		and.b	($0000.w,memory),reg_A
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_26: ;ROL ab
		ZPAGE_RMW
		LOAD_C
.m68k_data:
		move.b	($0000.w,memory),d0
		ROL_ONLY d0
.m68k_data_extra:
		move.b	d0,($0000.w,memory)
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_27: ;RLA ab [unofficial - ROL Mem, then AND with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_28: ;PLP
		IMPLIED
		PLP
		UPDATE_PC
		RETURN_OR_CONTINUE
		CHECKIRQ
		DONE

_JIT_insn_opcode_29: ;AND #ab
		IMMEDIATE
		AND6502
		DONE

_JIT_insn_opcode_2a: ;ROL
		ACCUMULATOR
		ROL_ONLY reg_A
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_2c: ;BIT abcd
		ABSOLUTE
		MEMORY_GetByte
		BIT6502
		DONE

_JIT_insn_opcode_2d: ;AND abcd
		ABSOLUTE
		MEMORY_GetByte
		AND6502
		DONE

_JIT_insn_opcode_2e: ;ROL abcd
		ABSOLUTE
		move.l	d0,-(sp)
		RMW_GetByte
		ROL_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_2f: ;RLA abcd [unofficial - ROL Mem, then AND with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_30: ;BMI
		RELATIVE
		TEST_NZ
		BRANCH	mi

_JIT_insn_opcode_31: ;AND (ab),y
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		AND6502
		DONE

_JIT_insn_opcode_33: ;RLA (ab),y [unofficial - ROL Mem, then AND with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_35: ;AND ab,x
		ZPAGE_X
		MEMORY_dGetByte
		AND6502
		DONE

_JIT_insn_opcode_36: ;ROL ab,x
		ZPAGE_X
		move.l	d0,-(sp)
		MEMORY_dGetByte
		ROR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte_ZP
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_37: ;RLA ab,x [unofficial - ROL Mem, then AND with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_38: ;SEC
		IMPLIED
		SetC
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_39: ;AND abcd,y
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		AND6502
		DONE

_JIT_insn_opcode_3b: ;RLA abcd,y [unofficial - ROL Mem, then AND with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_3d: ;AND abcd,x
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		AND6502
		DONE

_JIT_insn_opcode_3e: ;ROL abcd,x
		ABSOLUTE_X
		move.l	d0,-(sp)
		RMW_GetByte
		ROL_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_3f: ;RLA abcd,x [unofficial - ROL Mem, then AND with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_40: ;RTI
		IMPLIED_RETURN
		PLP
		PLW
		move.l	d0,reg_PC
		CHECKIRQ
		RETURN_OR_JUMP
		DONE

_JIT_insn_opcode_41: ;EOR (ab,x)
		INDIRECT_X
		MEMORY_GetByte
		EOR6502
		DONE

_JIT_insn_opcode_43: ;LSE (ab,x) [unofficial - LSR then EOR result with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_45: ;EOR ab
		ZPAGE
.m68k_data:
		move.b	($0000.w,memory),d0
		EOR6502
		DONE

_JIT_insn_opcode_46: ;LSR ab
		ZPAGE_RMW
.m68k_data:
		move.b	($0000.w,memory),d0
		LSR_ONLY d0
.m68k_data_extra:
		move.b	d0,($0000.w,memory)
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_47: ;LSE ab [unofficial - LSR then EOR result with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_48: ;PHA
		IMPLIED
		move.l	reg_A,d0
		PH
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_49: ;EOR #ab
		IMMEDIATE
		EOR6502
		DONE

_JIT_insn_opcode_4a: ;LSR
		ACCUMULATOR
		LSR_ONLY reg_A
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_4b: ;ALR #ab [unofficial - Acc AND Data, LSR result]
		NOT_IMPLEMENTED

_JIT_insn_opcode_4c: ;JMP abcd
		ABSOLUTE_JUMP
.m68k_data:
		move.l	#$00000000,reg_PC
		RETURN_OR_JUMP
		DONE

_JIT_insn_opcode_4d: ;EOR abcd
		ABSOLUTE
		MEMORY_GetByte
		EOR6502
		DONE

_JIT_insn_opcode_4e: ;LSR abcd
		ABSOLUTE
		move.l	d0,-(sp)
		RMW_GetByte
		LSR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_4f: ;LSE abcd [unofficial - LSR then EOR result with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_50: ;BVC
		RELATIVE
		TEST_V
		BRANCH	eq

_JIT_insn_opcode_51: ;EOR (ab),y
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		EOR6502
		DONE

_JIT_insn_opcode_53: ;LSE (ab),y [unofficial - LSR then EOR result with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_55: ;EOR ab,x
		ZPAGE_X
		MEMORY_dGetByte
		EOR6502
		DONE

_JIT_insn_opcode_56: ;LSR ab,x
		ZPAGE_X
		move.l	d0,-(sp)
		MEMORY_dGetByte
		LSR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte_ZP
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_57: ;LSE ab,x [unofficial - LSR then EOR result with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_58: ;CLI
		IMPLIED
		ClrI
		UPDATE_PC
		RETURN_OR_CONTINUE
		CHECKIRQ
		DONE

_JIT_insn_opcode_59: ;EOR abcd,y
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		EOR6502
		DONE

_JIT_insn_opcode_5b: ;LSE abcd,y [unofficial - LSR then EOR result with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_5d: ;EOR abcd,x
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		EOR6502
		DONE

_JIT_insn_opcode_5e: ;LSR abcd,x
		ABSOLUTE_X
		move.l	d0,-(sp)
		RMW_GetByte
		LSR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_5f: ;LSE abcd,x [unofficial - LSR then EOR result with A]
		NOT_IMPLEMENTED

_JIT_insn_opcode_60: ;RTS
		IMPLIED_RETURN
		PLW
		addq.w	#1,d0
		move.l	d0,reg_PC
		; TODO (incl. local->global...)
		;if (CPU_rts_handler != NULL) {
		;	CPU_rts_handler();
		;	CPU_rts_handler = NULL;
		;}
		RETURN_OR_JUMP
		DONE

_JIT_insn_opcode_61: ;ADC (ab,x)
		INDIRECT_X
		MEMORY_GetByte
		ADC6502
		DONE

_JIT_insn_opcode_63: ;RRA (ab,x) [unofficial - ROR Mem, then ADC to Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_65: ;ADC ab
		ZPAGE
.m68k_data:
		move.b	($0000.w,memory),d0				; TODO: replace at least binary version
		ADC6502
		DONE

_JIT_insn_opcode_66: ;ROR ab
		ZPAGE_RMW
		LOAD_C
.m68k_data:
		move.b	($0000.w,memory),d0
		ROR_ONLY d0
.m68k_data_extra:
		move.b	d0,($0000.w,memory)
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_67: ;RRA ab [unofficial - ROR Mem, then ADC to Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_68: ;PLA
		IMPLIED
		PL
		move.b	d0,reg_A
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_69: ;ADC #ab
		IMMEDIATE
		ADC6502
		DONE

_JIT_insn_opcode_6a: ;ROR
		ACCUMULATOR
		ROR_ONLY reg_A
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_6b: ;ARR #ab [unofficial - Acc AND Data, ROR result]
		NOT_IMPLEMENTED

_JIT_insn_opcode_6c: ;JMP (abcd)
		INDIRECT
.m68k_data:
		move.l	#$00000000,d0
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
		move.l	d0,reg_PC
		RETURN_OR_JUMP
		DONE

_JIT_insn_opcode_6d: ;ADC abcd
		ABSOLUTE
		MEMORY_GetByte
		ADC6502
		DONE

_JIT_insn_opcode_6e: ;ROR abcd
		ABSOLUTE
		move.l	d0,-(sp)
		RMW_GetByte
		ROR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_6f: ;RRA abcd [unofficial - ROR Mem, then ADC to Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_70: ;BVS
		RELATIVE
		TEST_V
		BRANCH	ne

_JIT_insn_opcode_71: ;ADC (ab),y
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		ADC6502
		DONE

_JIT_insn_opcode_73: ;RRA (ab),y [unofficial - ROR Mem, then ADC to Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_75: ;ADC ab,x
		ZPAGE_X
		MEMORY_dGetByte
		ADC6502
		DONE

_JIT_insn_opcode_76: ;ROR ab,x
		ZPAGE_X
		move.l	d0,-(sp)
		MEMORY_dGetByte
		ROR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte_ZP
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_77: ;RRA ab,x [unofficial - ROR Mem, then ADC to Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_78: ;SEI
		IMPLIED
		SetI
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_79: ;ADC abcd,y
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		ADC6502
		DONE

_JIT_insn_opcode_7b: ;RRA abcd,y [unofficial - ROR Mem, then ADC to Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_7d: ;ADC abcd,x
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		ADC6502
		DONE

_JIT_insn_opcode_7e: ;ROR abcd,x
		ABSOLUTE_X
		move.l	d0,-(sp)
		RMW_GetByte
		ROR_ONLY d0
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_7f: ;RRA abcd,x [unofficial - ROR Mem, then ADC to Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_81: ;STA (ab,x)
		INDIRECT_X
		move.b	reg_A,d1
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_83: ;SAX (ab,x) [unofficial - Store result A AND X
		NOT_IMPLEMENTED

_JIT_insn_opcode_84: ;STY ab
		ZPAGE
.m68k_data:
		move.b	reg_Y,($0000.w,memory)
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_85: ;STA ab
		ZPAGE
.m68k_data:
		move.b	reg_A,($0000.w,memory)
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_86: ;STX ab
		ZPAGE
.m68k_data:
		move.b	reg_X,($0000.w,memory)
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_87: ;SAX ab [unofficial - Store result A AND X]
		NOT_IMPLEMENTED

_JIT_insn_opcode_88: ;DEY
		IMPLIED
		subq.b	#1,reg_Y
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_8a: ;TXA
		IMPLIED
		move.b	reg_X,reg_A
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_8b: ;ANE #ab [unofficial - A AND X AND (Mem OR $EF) to Acc] (Fox)
		NOT_IMPLEMENTED

_JIT_insn_opcode_8c: ;STY abcd
		ABSOLUTE
		move.b	reg_Y,d1
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_8d: ;STA abcd
		ABSOLUTE
		move.b	reg_A,d1
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_8e: ;STX abcd
		ABSOLUTE
		move.b	reg_X,d1
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_8f: ;SAX abcd [unofficial - Store result A AND X]
		NOT_IMPLEMENTED

_JIT_insn_opcode_90: ;BCC
		RELATIVE
		TEST_C
		BRANCH	eq

_JIT_insn_opcode_91: ;STA (ab),y
		INDIRECT_Y
		move.b	reg_A,d1
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_93: ;SHA (ab),y [unofficial, UNSTABLE - Store A AND X AND (H+1) ?] (Fox)
		NOT_IMPLEMENTED

_JIT_insn_opcode_94: ;STY ab,x
		ZPAGE_X
		move.b	reg_Y,d1
		MEMORY_PutByte_ZP
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_95: ;STA ab,x
		ZPAGE_X
		move.b	reg_A,d1
		MEMORY_PutByte_ZP
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_96: ;STX ab,y
		ZPAGE_X
		move.b	reg_X,d1
		MEMORY_PutByte_ZP
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_97: ;SAX ab,y [unofficial - Store result A AND X]
		NOT_IMPLEMENTED

_JIT_insn_opcode_98: ;TYA
		IMPLIED
		move.b	reg_Y,reg_A
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_99: ;STA abcd,y
		ABSOLUTE_Y
		move.b	reg_A,d1
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_9a: ;TXS
		IMPLIED
		move.b	reg_X,reg_S+1
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_9b: ;SHS abcd,y [unofficial, UNSTABLE] (Fox)
		NOT_IMPLEMENTED

_JIT_insn_opcode_9c: ;SHY abcd,x [unofficial - Store Y and (H+1)] (Fox)
		NOT_IMPLEMENTED

_JIT_insn_opcode_9d: ;STA abcd,x
		ABSOLUTE_X
		move.b	reg_A,d1
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_9e: ;SHX abcd,y [unofficial - Store X and (H+1)] (Fox)
		NOT_IMPLEMENTED

_JIT_insn_opcode_9f: ;SHA abcd,y [unofficial, UNSTABLE - Store A AND X AND (H+1) ?] (Fox)
		NOT_IMPLEMENTED

_JIT_insn_opcode_a0: ;LDY #ab
		IMMEDIATE
		LDY6502
		DONE

_JIT_insn_opcode_a1: ;LDA (ab,x)
		INDIRECT_X
		MEMORY_GetByte
		LDA6502
		DONE

_JIT_insn_opcode_a2: ;LDX #ab
		IMMEDIATE
		LDX6502
		DONE

_JIT_insn_opcode_a3: ;LAX (ab,x) [unofficial]
		NOT_IMPLEMENTED

_JIT_insn_opcode_a4: ;LDY ab
		ZPAGE
.m68k_data:
		move.b	($0000.w,memory),reg_Y
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_a5: ;LDA ab
		ZPAGE
.m68k_data:
		move.b	($0000.w,memory),reg_A
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_a6: ;LDX ab
		ZPAGE
.m68k_data:
		move.b	($0000.w,memory),reg_X
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_a7: ;LAX ab [unofficial]
		NOT_IMPLEMENTED

_JIT_insn_opcode_a8: ;TAY
		IMPLIED
		move.b	reg_A,reg_Y
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_a9: ;LDA #ab
		IMMEDIATE
		LDA6502
		DONE

_JIT_insn_opcode_aa: ;TAX
		IMPLIED
		move.b	reg_A,reg_X
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_ab: ;ANX #ab [unofficial - AND #ab, then TAX]
		NOT_IMPLEMENTED

_JIT_insn_opcode_ac: ;LDY abcd
		ABSOLUTE
		MEMORY_GetByte
		LDY6502
		DONE

_JIT_insn_opcode_ad: ;LDA abcd
		ABSOLUTE
		MEMORY_GetByte
		LDA6502
		DONE

_JIT_insn_opcode_ae: ;LDX abcd
		ABSOLUTE
		MEMORY_GetByte
		LDX6502
		DONE

_JIT_insn_opcode_af: ;LAX abcd [unofficial]
		NOT_IMPLEMENTED

_JIT_insn_opcode_b0: ;BCS
		RELATIVE
		TEST_C
		BRANCH	ne

_JIT_insn_opcode_b1: ;LDA (ab),y
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		LDA6502
		DONE

_JIT_insn_opcode_b3: ;LAX (ab),y [unofficial]
		NOT_IMPLEMENTED

_JIT_insn_opcode_b4: ;LDY ab,x
		ZPAGE_X
		MEMORY_dGetByte
		LDY6502
		DONE

_JIT_insn_opcode_b5: ;LDA ab,x
		ZPAGE_X
		MEMORY_dGetByte
		LDA6502
		DONE

_JIT_insn_opcode_b6: ;LDX ab,y
		ZPAGE_Y
		MEMORY_dGetByte
		LDX6502
		DONE

_JIT_insn_opcode_b7: ;LAX ab,y [unofficial]
		NOT_IMPLEMENTED

_JIT_insn_opcode_b8: ;CLV
		IMPLIED
		ClrV
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_b9: ;LDA abcd,y
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		LDA6502
		DONE

_JIT_insn_opcode_ba: ;TSX
		IMPLIED
		move.b	reg_S+1,reg_X
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_bb: ;LAS abcd,y [unofficial - AND S with Mem, transfer to A and X (Fox)
		NOT_IMPLEMENTED

_JIT_insn_opcode_bc: ;LDY abcd,x
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		LDY6502
		DONE

_JIT_insn_opcode_bd: ;LDA abcd,x
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		LDA6502
		DONE

_JIT_insn_opcode_be: ;LDX abcd,y
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		LDX6502
		DONE

_JIT_insn_opcode_bf: ;LAX abcd,y [unofficial]
		NOT_IMPLEMENTED

_JIT_insn_opcode_c0: ;CPY #ab
		IMMEDIATE
		CPY6502
		DONE

_JIT_insn_opcode_c1: ;CMP (ab,x)
		INDIRECT_X
		MEMORY_GetByte
		CMP6502
		DONE

_JIT_insn_opcode_c3: ;DCM (ab,x) [unofficial - DEC Mem then CMP with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_c4: ;CPY ab
		ZPAGE
.m68k_data:
		cmp.b	($0000.w,memory),reg_Y
		SAVE_NZc
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_c5: ;CMP ab
		ZPAGE
.m68k_data:
		cmp.b	($0000.w,memory),reg_A
		SAVE_NZc
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_c6: ;DEC ab
		ZPAGE
.m68k_data:
		subq.b	#1,($0000.w,memory)
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_c7: ;DCM ab [unofficial - DEC Mem then CMP with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_c8: ;INY
		IMPLIED
		addq.b	#1,reg_Y
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_c9: ;CMP #ab
		IMMEDIATE
		CMP6502
		DONE

_JIT_insn_opcode_ca: ;DEX
		IMPLIED
		subq.b	#1,reg_X
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_cb: ;SBX #ab [unofficial - store ((A AND X) - Mem) in X] (Fox)
		NOT_IMPLEMENTED

_JIT_insn_opcode_cc: ;CPY abcd
		ABSOLUTE
		MEMORY_GetByte
		CPY6502
		DONE

_JIT_insn_opcode_cd: ;CMP abcd
		ABSOLUTE
		MEMORY_GetByte
		CMP6502
		DONE

_JIT_insn_opcode_ce: ;DEC abcd
		ABSOLUTE
		move.l	d0,-(sp)
		RMW_GetByte
		subq.b	#1,d0
		SAVE_NZ
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_cf: ;DCM abcd [unofficial - DEC Mem then CMP with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_d0: ;BNE
		RELATIVE
		TEST_NZ
		BRANCH	ne

_JIT_insn_opcode_d1: ;CMP (ab),y
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		CMP6502
		DONE

_JIT_insn_opcode_d3: ;DCM (ab),y [unofficial - DEC Mem then CMP with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_d5: ;CMP ab,x
		ZPAGE_X
		MEMORY_dGetByte
		CMP6502
		DONE

_JIT_insn_opcode_d6: ;DEC ab,x
		ZPAGE_X
		move.l	d0,-(sp)
		MEMORY_dGetByte
		subq.b	#1,d0
		SAVE_NZ
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte_ZP
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_d7: ;DCM ab,x [unofficial - DEC Mem then CMP with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_d8: ;CLD
		IMPLIED
		ClrD
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_d9: ;CMP abcd,y
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		CMP6502
		DONE

_JIT_insn_opcode_db: ;DCM abcd,y [unofficial - DEC Mem then CMP with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_dd: ;CMP abcd,x
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		CMP6502
		DONE

_JIT_insn_opcode_de: ;DEC abcd,x
		ABSOLUTE_X
		move.l	d0,-(sp)
		RMW_GetByte
		subq.b	#1,d0
		SAVE_NZ
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_df: ;DCM abcd,x [unofficial - DEC Mem then CMP with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_e0: ;CPX #ab
		IMMEDIATE
		CPX6502
		DONE

_JIT_insn_opcode_e1: ;SBC (ab,x)
		INDIRECT_X
		MEMORY_GetByte
		SBC6502
		DONE

_JIT_insn_opcode_e3: ;INS (ab,x) [unofficial - INC Mem then SBC with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_e4: ;CPX ab
		ZPAGE
.m68k_data:
		cmp.b	($0000.w,memory),reg_X
		SAVE_NZc
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_e5: ;SBC ab
		ZPAGE
.m68k_data:
		move.b	($0000.w,memory),d0				; TODO: replace at least binary version
		SBC6502
		DONE

_JIT_insn_opcode_e6: ;INC ab
		ZPAGE
.m68k_data:
		addq.b	#1,($0000.w,memory)
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_e7: ;INS ab [unofficial - INC Mem then SBC with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_e8: ;INX
		IMPLIED
		addq.b	#1,reg_X
		SAVE_NZ
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_eb: ;SBC #ab [unofficial]; e9 official
		IMMEDIATE
		SBC6502
		DONE

_JIT_insn_opcode_fa: ;NOP [unofficial]; ea official
		IMPLIED
		; nothing to do
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_ec: ;CPX abcd
		ABSOLUTE
		MEMORY_GetByte
		CPX6502
		DONE

_JIT_insn_opcode_ed: ;SBC abcd
		ABSOLUTE
		MEMORY_GetByte
		SBC6502
		DONE

_JIT_insn_opcode_ee: ;INC abcd
		ABSOLUTE
		move.l	d0,-(sp)
		RMW_GetByte
		addq.b	#1,d0
		SAVE_NZ
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_ef: ;INS abcd [unofficial - INC Mem then SBC with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_f0: ;BEQ
		RELATIVE
		TEST_NZ
		BRANCH	eq

_JIT_insn_opcode_f1: ;SBC (ab),y
		INDIRECT_Y
		NCYCLES_Y
		MEMORY_GetByte
		SBC6502
		DONE

_JIT_insn_opcode_f3: ;INS (ab),y [unofficial - INC Mem then SBC with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_f5: ;SBC ab,x
		ZPAGE_X
		MEMORY_dGetByte
		SBC6502
		DONE

_JIT_insn_opcode_f6: ;INC ab,x
		ZPAGE_X
		move.l	d0,-(sp)
		MEMORY_dGetByte
		addq.b	#1,d0
		SAVE_NZ
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte_ZP
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_f7: ;INS ab,x [unofficial - INC Mem then SBC with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_f8: ;SED
		IMPLIED
		SetD
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_f9: ;SBC abcd,y
		ABSOLUTE_Y
		NCYCLES_Y
		MEMORY_GetByte
		SBC6502
		DONE

_JIT_insn_opcode_fb: ;INS abcd,y [unofficial - INC Mem then SBC with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_fd: ;SBC abcd,x
		ABSOLUTE_X
		NCYCLES_X
		MEMORY_GetByte
		SBC6502
		DONE

_JIT_insn_opcode_fe: ;INC abcd,x
		ABSOLUTE_X
		move.l	d0,-(sp)
		RMW_GetByte
		addq.b	#1,d0
		SAVE_NZ
		move.b	d0,d1
		move.l	(sp)+,d0
		MEMORY_PutByte
		UPDATE_PC
		RETURN_OR_CONTINUE
		DONE

_JIT_insn_opcode_ff: ;INS abcd,x [unofficial - INC Mem then SBC with Acc]
		NOT_IMPLEMENTED

_JIT_insn_opcode_d2: ;ESCRTS #ab (CIM) - on Atari is here instruction CIM [unofficial] !RS!
		NOT_IMPLEMENTED

_JIT_insn_opcode_f2: ;ESC #ab (CIM) - on Atari is here instruction CIM [unofficial] !RS!
		NOT_IMPLEMENTED

_JIT_insn_opcode_b2: ;CIM [unofficial - crash intermediate]
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
		addq.w	#1,reg_PC						; XXX: if reg_PC is an An, this is handled as .l
		addq.w	#2,reg_PC						;
		addq.w	#3,reg_PC						;

reg_S:	dc.w	$0100

hw_get_table:
		dc.l	_GTIA_GetByte
		dc.l	_POKEY_GetByte
		dc.l	_PIA_GetByte
		dc.l	_ANTIC_GetByte

hw_put_table:
		dc.l	_GTIA_PutByte
		dc.l	_POKEY_PutByte
		dc.l	_PIA_PutByte
		dc.l	_ANTIC_PutByte

rol_rmw_table:
		dc.l	rol_mem_rmw
		dc.l	rol_gtia_rmw
		dc.l	rol_pokey_rmw
		dc.l	rol_pia_rmw
		dc.l	rol_antic_rmw

ror_rmw_table:
		dc.l	ror_mem_rmw
		dc.l	ror_gtia_rmw
		dc.l	ror_pokey_rmw
		dc.l	ror_pia_rmw
		dc.l	ror_antic_rmw

asl_rmw_table:
		dc.l	asl_mem_rmw
		dc.l	asl_gtia_rmw
		dc.l	asl_pokey_rmw
		dc.l	asl_pia_rmw
		dc.l	asl_antic_rmw

lsr_rmw_table:
		dc.l	lsr_mem_rmw
		dc.l	lsr_gtia_rmw
		dc.l	lsr_pokey_rmw
		dc.l	lsr_pia_rmw
		dc.l	lsr_antic_rmw

inc_rmw_table:
		dc.l	inc_mem_rmw
		dc.l	inc_gtia_rmw
		dc.l	inc_pokey_rmw
		dc.l	inc_pia_rmw
		dc.l	inc_antic_rmw

dec_rmw_table:
		dc.l	dec_mem_rmw
		dc.l	dec_gtia_rmw
		dc.l	dec_pokey_rmw
		dc.l	dec_pia_rmw
		dc.l	dec_antic_rmw

; ---------------------------------------------
		section	bss
; ---------------------------------------------

_host_cpu:
		ds.l	1								; 30 = 68030, 40 = 68040, 60 = 68060
reg_ccr:
		ds.w	1
