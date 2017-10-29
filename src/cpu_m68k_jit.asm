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

		xref	_ANTIC_xpos_limit

		xdef	_CPU_JIT_Execute

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

reg_PC	equr	d2								; .w, pointing to MEMORY_mem aligned on 64k
reg_A	equr	d3								; .b
reg_X	equr	d4								; .b
reg_Y	equr	d5								; .b
reg_SP	equr	a2								; .l, pointing to MEMORY_mem aligned on 64k
memory	equr	a3								; .l, pointing to MEMORY_mem aligned on 64k
xpos	equr	a4								; .l, ANTIC_xpos mirror

		macro	COPY_NO_OPERAND_AND_START
		dc.l	.copy
		; UWORD instance(UWORD* buf, UWORD data, int bytes, int cycles)
.copy:	lea		(.m68k_start,pc),a0				; a0:   src buffer
		move.l	(4,sp),a1						; a1:   dst buffer
		tst.l	a1
		beq.b	.m68k_skip
		move.w	#(.m68k_end-.m68k_start)/2-1,d0	; d0.w: # of words to copy
.copy_loop:
		move.w	(a0)+,(a1)+
		dbra	d0,.copy_loop

		move.l	(4,sp),a1						; a1:   dst buffer
		lea		(addql_xpos_table,pc),a0
		move.l	(12,sp),d0						; d0.l: cycles
		move.w	(a0,d0.l*2),(.m68k_cycles-.m68k_start,a1)
		lea		(addql_pc_table,pc),a0
		move.l	(16,sp),d0						; d0.l: bytes
		move.w	(a0,d0.l*2),(.m68k_bytes-.m68k_start,a1)

		;move.l	(8,sp),d0						; d0.w: operand

.m68k_skip:
		move.l	#(.m68k_end-.m68k_start)/2,d0	; number of words
		rts
.m68k_start:
		endm

		macro	M68K_BYTES
.m68k_bytes:
		addq.l	#1,reg_PC						; 1 - 3
		endm

		macro	M68K_CYCLES_AND_END
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		cmp.l	_ANTIC_xpos_limit,xpos
		blt.b	.m68k_end
		rts
.m68k_end:
		endm

;ZFLAG  equr d1 ; Bit 0..7
;NFLAG  equr d1 ; Bit 8..15
;VFLAG  equr d6 ; Bit 7
;DFLAG  equr d6 ; Bit 15
;CFLAG  equr d5 ; Bit 0..7, ( 1 = ff )

;/* 6502 flags local to this module */
;static UBYTE N;					/* bit7 set => N flag set */
;static UBYTE V;                 /* non-zero => V flag set */
;static UBYTE Z;					/* zero     => Z flag set */
;static UBYTE C;					/* must be 0 or 1 */
;/* B, D, I are always in CPU_regP */

;void CPU_GetStatus(void)
;{
;	CPU_regP = (N & 0x80) + (V ? 0x40 : 0) + (CPU_regP & 0x3c) + ((Z == 0) ? 0x02 : 0) + C;
;}

; ---------------------------------------------
		section	text
; ---------------------------------------------

_CPU_JIT_Execute:
		movea.l	(4,sp),a0						; code
		movem.l	d2-d7/a2-a6,-(sp)
		jsr		(a0)
		movem.l	(sp)+,d2-d7/a2-a6
		rts

; ---------------------------------------------
		section	data
; ---------------------------------------------

addql_xpos_table:
		illegal
		addq.l	#1,xpos
		addq.l	#2,xpos
		addq.l	#3,xpos
		addq.l	#4,xpos
		addq.l	#5,xpos
		addq.l	#6,xpos
		addq.l	#7,xpos
		addq.l	#8,xpos

addql_pc_table:
		illegal
		addq.l	#1,reg_PC
		addq.l	#2,reg_PC
		addq.l	#3,reg_PC

_JIT_insn_opcode_00: ;BRK
_JIT_insn_opcode_01: ;ORA (ab,x)
_JIT_insn_opcode_03: ;ASO (ab,x) [unofficial - ASL then ORA with Acc]
_JIT_insn_opcode_64: ;NOP ab [unofficial - skip byte]
		dc.w	no_stop
		COPY_NO_OPERAND_AND_START
		; nothing to do
		M68K_BYTES
		M68K_CYCLES_AND_END

_JIT_insn_opcode_f4: ;NOP ab,x [unofficial - skip byte]
		dc.w	no_stop
		COPY_NO_OPERAND_AND_START
		; nothing to do
		M68K_BYTES
		M68K_CYCLES_AND_END

_JIT_insn_opcode_e2: ;NOP #ab [unofficial - skip byte]
		dc.w	no_stop
		COPY_NO_OPERAND_AND_START
		; nothing to do
		M68K_BYTES
		M68K_CYCLES_AND_END

_JIT_insn_opcode_05: ;ORA ab
_JIT_insn_opcode_06: ;ASL ab
_JIT_insn_opcode_07: ;ASO ab [unofficial - ASL then ORA with Acc]
_JIT_insn_opcode_08: ;PHP
_JIT_insn_opcode_09: ;ORA #ab
_JIT_insn_opcode_0a: ;ASL
_JIT_insn_opcode_2b: ;ANC #ab [unofficial - AND then copy N to C (Fox)
_JIT_insn_opcode_0c: ;NOP abcd [unofficial - skip word]
		dc.w	no_stop
		COPY_NO_OPERAND_AND_START
		; nothing to do
		M68K_BYTES
		M68K_CYCLES_AND_END

_JIT_insn_opcode_0d: ;ORA abcd
_JIT_insn_opcode_0e: ;ASL abcd
_JIT_insn_opcode_0f: ;ASO abcd [unofficial - ASL then ORA with Acc]
_JIT_insn_opcode_10: ;BPL
_JIT_insn_opcode_11: ;ORA (ab),y
_JIT_insn_opcode_13: ;ASO (ab),y [unofficial - ASL then ORA with Acc]
_JIT_insn_opcode_15: ;ORA ab,x
_JIT_insn_opcode_16: ;ASL ab,x
_JIT_insn_opcode_17: ;ASO ab,x [unofficial - ASL then ORA with Acc]
_JIT_insn_opcode_18: ;CLC
_JIT_insn_opcode_19: ;ORA abcd,y
_JIT_insn_opcode_1b: ;ASO abcd,y [unofficial - ASL then ORA with Acc]
_JIT_insn_opcode_fc: ;NOP abcd,x [unofficial - skip word]
		dc.w	no_stop
		COPY_NO_OPERAND_AND_START
		; nothing to do
		M68K_BYTES
		M68K_CYCLES_AND_END

_JIT_insn_opcode_1d: ;ORA abcd,x
_JIT_insn_opcode_1e: ;ASL abcd,x
_JIT_insn_opcode_1f: ;ASO abcd,x [unofficial - ASL then ORA with Acc]
_JIT_insn_opcode_20: ;JSR abcd
_JIT_insn_opcode_21: ;AND (ab,x)
_JIT_insn_opcode_23: ;RLA (ab,x) [unofficial - ROL Mem, then AND with A]
_JIT_insn_opcode_24: ;BIT ab
_JIT_insn_opcode_25: ;AND ab
_JIT_insn_opcode_26: ;ROL ab
_JIT_insn_opcode_27: ;RLA ab [unofficial - ROL Mem, then AND with A]
_JIT_insn_opcode_28: ;PLP
_JIT_insn_opcode_29: ;AND #ab
_JIT_insn_opcode_2a: ;ROL
_JIT_insn_opcode_2c: ;BIT abcd
_JIT_insn_opcode_2d: ;AND abcd
_JIT_insn_opcode_2e: ;ROL abcd
_JIT_insn_opcode_2f: ;RLA abcd [unofficial - ROL Mem, then AND with A]
_JIT_insn_opcode_30: ;BMI
_JIT_insn_opcode_31: ;AND (ab),y
_JIT_insn_opcode_33: ;RLA (ab),y [unofficial - ROL Mem, then AND with A]
_JIT_insn_opcode_35: ;AND ab,x
_JIT_insn_opcode_36: ;ROL ab,x
_JIT_insn_opcode_37: ;RLA ab,x [unofficial - ROL Mem, then AND with A]
_JIT_insn_opcode_38: ;SEC
_JIT_insn_opcode_39: ;AND abcd,y
_JIT_insn_opcode_3b: ;RLA abcd,y [unofficial - ROL Mem, then AND with A]
_JIT_insn_opcode_3d: ;AND abcd,x
_JIT_insn_opcode_3e: ;ROL abcd,x
_JIT_insn_opcode_3f: ;RLA abcd,x [unofficial - ROL Mem, then AND with A]
_JIT_insn_opcode_40: ;RTI
_JIT_insn_opcode_41: ;EOR (ab,x)
_JIT_insn_opcode_43: ;LSE (ab,x) [unofficial - LSR then EOR result with A]
_JIT_insn_opcode_45: ;EOR ab
_JIT_insn_opcode_46: ;LSR ab
_JIT_insn_opcode_47: ;LSE ab [unofficial - LSR then EOR result with A]
_JIT_insn_opcode_48: ;PHA
_JIT_insn_opcode_49: ;EOR #ab
_JIT_insn_opcode_4a: ;LSR
_JIT_insn_opcode_4b: ;ALR #ab [unofficial - Acc AND Data, LSR result]
_JIT_insn_opcode_4c: ;JMP abcd
_JIT_insn_opcode_4d: ;EOR abcd
_JIT_insn_opcode_4e: ;LSR abcd
_JIT_insn_opcode_4f: ;LSE abcd [unofficial - LSR then EOR result with A]
_JIT_insn_opcode_50: ;BVC
_JIT_insn_opcode_51: ;EOR (ab),y
_JIT_insn_opcode_53: ;LSE (ab),y [unofficial - LSR then EOR result with A]
_JIT_insn_opcode_55: ;EOR ab,x
_JIT_insn_opcode_56: ;LSR ab,x
_JIT_insn_opcode_57: ;LSE ab,x [unofficial - LSR then EOR result with A]
_JIT_insn_opcode_58: ;CLI
_JIT_insn_opcode_59: ;EOR abcd,y
_JIT_insn_opcode_5b: ;LSE abcd,y [unofficial - LSR then EOR result with A]
_JIT_insn_opcode_5d: ;EOR abcd,x
_JIT_insn_opcode_5e: ;LSR abcd,x
_JIT_insn_opcode_5f: ;LSE abcd,x [unofficial - LSR then EOR result with A]
_JIT_insn_opcode_60: ;RTS
_JIT_insn_opcode_61: ;ADC (ab,x)
_JIT_insn_opcode_63: ;RRA (ab,x) [unofficial - ROR Mem, then ADC to Acc]
_JIT_insn_opcode_65: ;ADC ab
_JIT_insn_opcode_66: ;ROR ab
_JIT_insn_opcode_67: ;RRA ab [unofficial - ROR Mem, then ADC to Acc]
_JIT_insn_opcode_68: ;PLA
_JIT_insn_opcode_69: ;ADC #ab
_JIT_insn_opcode_6a: ;ROR
_JIT_insn_opcode_6b: ;ARR #ab [unofficial - Acc AND Data, ROR result]
_JIT_insn_opcode_6c: ;JMP (abcd)
_JIT_insn_opcode_6d: ;ADC abcd
_JIT_insn_opcode_6e: ;ROR abcd
_JIT_insn_opcode_6f: ;RRA abcd [unofficial - ROR Mem, then ADC to Acc]
_JIT_insn_opcode_70: ;BVS
_JIT_insn_opcode_71: ;ADC (ab),y
_JIT_insn_opcode_73: ;RRA (ab),y [unofficial - ROR Mem, then ADC to Acc]
_JIT_insn_opcode_75: ;ADC ab,x
_JIT_insn_opcode_76: ;ROR ab,x
_JIT_insn_opcode_77: ;RRA ab,x [unofficial - ROR Mem, then ADC to Acc]
_JIT_insn_opcode_78: ;SEI
_JIT_insn_opcode_79: ;ADC abcd,y
_JIT_insn_opcode_7b: ;RRA abcd,y [unofficial - ROR Mem, then ADC to Acc]
_JIT_insn_opcode_7d: ;ADC abcd,x
_JIT_insn_opcode_7e: ;ROR abcd,x
_JIT_insn_opcode_7f: ;RRA abcd,x [unofficial - ROR Mem, then ADC to Acc]
_JIT_insn_opcode_81: ;STA (ab,x)
_JIT_insn_opcode_83: ;SAX (ab,x) [unofficial - Store result A AND X
_JIT_insn_opcode_84: ;STY ab
_JIT_insn_opcode_85: ;STA ab
_JIT_insn_opcode_86: ;STX ab
_JIT_insn_opcode_87: ;SAX ab [unofficial - Store result A AND X]
_JIT_insn_opcode_88: ;DEY
_JIT_insn_opcode_8a: ;TXA
_JIT_insn_opcode_8b: ;ANE #ab [unofficial - A AND X AND (Mem OR $EF) to Acc] (Fox)
_JIT_insn_opcode_8c: ;STY abcd
_JIT_insn_opcode_8d: ;STA abcd
_JIT_insn_opcode_8e: ;STX abcd
_JIT_insn_opcode_8f: ;SAX abcd [unofficial - Store result A AND X]
_JIT_insn_opcode_90: ;BCC
_JIT_insn_opcode_91: ;STA (ab),y
_JIT_insn_opcode_93: ;SHA (ab),y [unofficial, UNSTABLE - Store A AND X AND (H+1) ?] (Fox)
_JIT_insn_opcode_94: ;STY ab,x
_JIT_insn_opcode_95: ;STA ab,x
_JIT_insn_opcode_96: ;STX ab,y
_JIT_insn_opcode_97: ;SAX ab,y [unofficial - Store result A AND X]
_JIT_insn_opcode_98: ;TYA
_JIT_insn_opcode_99: ;STA abcd,y
_JIT_insn_opcode_9a: ;TXS
_JIT_insn_opcode_9b: ;SHS abcd,y [unofficial, UNSTABLE] (Fox)
_JIT_insn_opcode_9c: ;SHY abcd,x [unofficial - Store Y and (H+1)] (Fox)
_JIT_insn_opcode_9d: ;STA abcd,x
_JIT_insn_opcode_9e: ;SHX abcd,y [unofficial - Store X and (H+1)] (Fox)
_JIT_insn_opcode_9f: ;SHA abcd,y [unofficial, UNSTABLE - Store A AND X AND (H+1) ?] (Fox)
_JIT_insn_opcode_a0: ;LDY #ab
_JIT_insn_opcode_a1: ;LDA (ab,x)
_JIT_insn_opcode_a2: ;LDX #ab
_JIT_insn_opcode_a3: ;LAX (ab,x) [unofficial]
_JIT_insn_opcode_a4: ;LDY ab
_JIT_insn_opcode_a5: ;LDA ab
_JIT_insn_opcode_a6: ;LDX ab
_JIT_insn_opcode_a7: ;LAX ab [unofficial]
_JIT_insn_opcode_a8: ;TAY
_JIT_insn_opcode_a9: ;LDA #ab
_JIT_insn_opcode_aa: ;TAX
_JIT_insn_opcode_ab: ;ANX #ab [unofficial - AND #ab, then TAX]
_JIT_insn_opcode_ac: ;LDY abcd
_JIT_insn_opcode_ad: ;LDA abcd
_JIT_insn_opcode_ae: ;LDX abcd
_JIT_insn_opcode_af: ;LAX abcd [unofficial]
_JIT_insn_opcode_b0: ;BCS
_JIT_insn_opcode_b1: ;LDA (ab),y
_JIT_insn_opcode_b3: ;LAX (ab),y [unofficial]
_JIT_insn_opcode_b4: ;LDY ab,x
_JIT_insn_opcode_b5: ;LDA ab,x
_JIT_insn_opcode_b6: ;LDX ab,y
_JIT_insn_opcode_b7: ;LAX ab,y [unofficial]
_JIT_insn_opcode_b8: ;CLV
_JIT_insn_opcode_b9: ;LDA abcd,y
_JIT_insn_opcode_ba: ;TSX
_JIT_insn_opcode_bb: ;LAS abcd,y [unofficial - AND S with Mem, transfer to A and X (Fox)
_JIT_insn_opcode_bc: ;LDY abcd,x
_JIT_insn_opcode_bd: ;LDA abcd,x
_JIT_insn_opcode_be: ;LDX abcd,y
_JIT_insn_opcode_bf: ;LAX abcd,y [unofficial]
_JIT_insn_opcode_c0: ;CPY #ab
_JIT_insn_opcode_c1: ;CMP (ab,x)
_JIT_insn_opcode_c3: ;DCM (ab,x) [unofficial - DEC Mem then CMP with Acc]
_JIT_insn_opcode_c4: ;CPY ab
_JIT_insn_opcode_c5: ;CMP ab
_JIT_insn_opcode_c6: ;DEC ab
_JIT_insn_opcode_c7: ;DCM ab [unofficial - DEC Mem then CMP with Acc]
_JIT_insn_opcode_c8: ;INY
_JIT_insn_opcode_c9: ;CMP #ab
_JIT_insn_opcode_ca: ;DEX
_JIT_insn_opcode_cb: ;SBX #ab [unofficial - store ((A AND X) - Mem) in X] (Fox)
_JIT_insn_opcode_cc: ;CPY abcd
_JIT_insn_opcode_cd: ;CMP abcd
_JIT_insn_opcode_ce: ;DEC abcd
_JIT_insn_opcode_cf: ;DCM abcd [unofficial - DEC Mem then CMP with Acc]
_JIT_insn_opcode_d0: ;BNE
_JIT_insn_opcode_d1: ;CMP (ab),y
_JIT_insn_opcode_d3: ;DCM (ab),y [unofficial - DEC Mem then CMP with Acc]
_JIT_insn_opcode_d5: ;CMP ab,x
_JIT_insn_opcode_d6: ;DEC ab,x
_JIT_insn_opcode_d7: ;DCM ab,x [unofficial - DEC Mem then CMP with Acc]
_JIT_insn_opcode_d8: ;CLD
_JIT_insn_opcode_d9: ;CMP abcd,y
_JIT_insn_opcode_db: ;DCM abcd,y [unofficial - DEC Mem then CMP with Acc]
_JIT_insn_opcode_dd: ;CMP abcd,x
_JIT_insn_opcode_de: ;DEC abcd,x
_JIT_insn_opcode_df: ;DCM abcd,x [unofficial - DEC Mem then CMP with Acc]
_JIT_insn_opcode_e0: ;CPX #ab
_JIT_insn_opcode_e1: ;SBC (ab,x)
_JIT_insn_opcode_e3: ;INS (ab,x) [unofficial - INC Mem then SBC with Acc]
_JIT_insn_opcode_e4: ;CPX ab
_JIT_insn_opcode_e5: ;SBC ab
_JIT_insn_opcode_e6: ;INC ab
_JIT_insn_opcode_e7: ;INS ab [unofficial - INC Mem then SBC with Acc]
_JIT_insn_opcode_e8: ;INX
_JIT_insn_opcode_eb: ;SBC #ab [unofficial]; e9 official
_JIT_insn_opcode_fa: ;NOP [unofficial]; ea official
		dc.w	no_stop
		COPY_NO_OPERAND_AND_START
		; nothing to do
		M68K_BYTES
		M68K_CYCLES_AND_END

_JIT_insn_opcode_ec: ;CPX abcd
_JIT_insn_opcode_ed: ;SBC abcd
_JIT_insn_opcode_ee: ;INC abcd
_JIT_insn_opcode_ef: ;INS abcd [unofficial - INC Mem then SBC with Acc]
_JIT_insn_opcode_f0: ;BEQ
_JIT_insn_opcode_f1: ;SBC (ab),y
_JIT_insn_opcode_f3: ;INS (ab),y [unofficial - INC Mem then SBC with Acc]
_JIT_insn_opcode_f5: ;SBC ab,x
_JIT_insn_opcode_f6: ;INC ab,x
_JIT_insn_opcode_f7: ;INS ab,x [unofficial - INC Mem then SBC with Acc]
_JIT_insn_opcode_f8: ;SED
_JIT_insn_opcode_f9: ;SBC abcd,y
_JIT_insn_opcode_fb: ;INS abcd,y [unofficial - INC Mem then SBC with Acc]
_JIT_insn_opcode_fd: ;SBC abcd,x
_JIT_insn_opcode_fe: ;INC abcd,x
_JIT_insn_opcode_ff: ;INS abcd,x [unofficial - INC Mem then SBC with Acc]
_JIT_insn_opcode_d2: ;ESCRTS #ab (CIM) - on Atari is here instruction CIM [unofficial] !RS!
_JIT_insn_opcode_f2: ;ESC #ab (CIM) - on Atari is here instruction CIM [unofficial] !RS!
_JIT_insn_opcode_b2: ;CIM [unofficial - crash intermediate]

