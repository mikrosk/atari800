#ifndef CPU_JIT_H_
#define CPU_JIT_H_

#include "atari.h"

struct JIT_native_code_info_t;

struct CPU_JIT_native_code_t {
	/* counter part for the 6502 addr */
	UBYTE *insn_addr;
	/* private member */
	struct JIT_native_code_info_t *insn_info;
};

struct CPU_JIT_insn_template_t {
	/* modifies the PC */
	UWORD is_stop;
	UWORD data_offset;
	UWORD bytes_offset;
	UWORD cycles_offset;
	UWORD native_code_size;
	UBYTE *native_code;
};

extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_00;	/* BRK */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_01;	/* ORA (ab,x) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_03;	/* ASO (ab,x) [unofficial - ASL then ORA with Acc] */

#define JIT_insn_opcode_04	JIT_insn_opcode_64			/* NOP ab [unofficial - skip byte] */
#define JIT_insn_opcode_44	JIT_insn_opcode_64
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_64;

#define JIT_insn_opcode_14	JIT_insn_opcode_f4			/* NOP ab,x [unofficial - skip byte] */
#define JIT_insn_opcode_34	JIT_insn_opcode_f4
#define JIT_insn_opcode_54	JIT_insn_opcode_f4
#define JIT_insn_opcode_74	JIT_insn_opcode_f4
#define JIT_insn_opcode_d4	JIT_insn_opcode_f4
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_f4;

#define JIT_insn_opcode_80	JIT_insn_opcode_e2			/* NOP #ab [unofficial - skip byte] */
#define JIT_insn_opcode_82	JIT_insn_opcode_e2
#define JIT_insn_opcode_89	JIT_insn_opcode_e2
#define JIT_insn_opcode_c2	JIT_insn_opcode_e2
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_e2;

extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_05;	/* ORA ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_06;	/* ASL ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_07;	/* ASO ab [unofficial - ASL then ORA with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_08;	/* PHP */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_09;	/* ORA #ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_0a;	/* ASL */

#define JIT_insn_opcode_0b	JIT_insn_opcode_2b			/* ANC #ab [unofficial - AND then copy N to C (Fox) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_2b;

extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_0c;	/* NOP abcd [unofficial - skip word] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_0d;	/* ORA abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_0e;	/* ASL abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_0f;	/* ASO abcd [unofficial - ASL then ORA with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_10;	/* BPL */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_11;	/* ORA (ab),y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_13;	/* ASO (ab),y [unofficial - ASL then ORA with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_15;	/* ORA ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_16;	/* ASL ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_17;	/* ASO ab,x [unofficial - ASL then ORA with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_18;	/* CLC */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_19;	/* ORA abcd,y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_1b;	/* ASO abcd,y [unofficial - ASL then ORA with Acc] */

#define JIT_insn_opcode_1c	JIT_insn_opcode_fc			/* NOP abcd,x [unofficial - skip word] */
#define JIT_insn_opcode_3c	JIT_insn_opcode_fc
#define JIT_insn_opcode_5c	JIT_insn_opcode_fc
#define JIT_insn_opcode_7c	JIT_insn_opcode_fc
#define JIT_insn_opcode_dc	JIT_insn_opcode_fc
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_fc;

extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_1d;	/* ORA abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_1e;	/* ASL abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_1f;	/* ASO abcd,x [unofficial - ASL then ORA with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_20;	/* JSR abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_21;	/* AND (ab,x) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_23;	/* RLA (ab,x) [unofficial - ROL Mem, then AND with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_24;	/* BIT ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_25;	/* AND ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_26;	/* ROL ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_27;	/* RLA ab [unofficial - ROL Mem, then AND with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_28;	/* PLP */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_29;	/* AND #ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_2a;	/* ROL */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_2c;	/* BIT abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_2d;	/* AND abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_2e;	/* ROL abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_2f;	/* RLA abcd [unofficial - ROL Mem, then AND with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_30;	/* BMI */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_31;	/* AND (ab),y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_33;	/* RLA (ab),y [unofficial - ROL Mem, then AND with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_35;	/* AND ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_36;	/* ROL ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_37;	/* RLA ab,x [unofficial - ROL Mem, then AND with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_38;	/* SEC */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_39;	/* AND abcd,y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_3b;	/* RLA abcd,y [unofficial - ROL Mem, then AND with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_3d;	/* AND abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_3e;	/* ROL abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_3f;	/* RLA abcd,x [unofficial - ROL Mem, then AND with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_40;	/* RTI */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_41;	/* EOR (ab,x) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_43;	/* LSE (ab,x) [unofficial - LSR then EOR result with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_45;	/* EOR ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_46;	/* LSR ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_47;	/* LSE ab [unofficial - LSR then EOR result with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_48;	/* PHA */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_49;	/* EOR #ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_4a;	/* LSR */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_4b;	/* ALR #ab [unofficial - Acc AND Data, LSR result] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_4c;	/* JMP abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_4d;	/* EOR abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_4e;	/* LSR abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_4f;	/* LSE abcd [unofficial - LSR then EOR result with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_50;	/* BVC */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_51;	/* EOR (ab),y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_53;	/* LSE (ab),y [unofficial - LSR then EOR result with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_55;	/* EOR ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_56;	/* LSR ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_57;	/* LSE ab,x [unofficial - LSR then EOR result with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_58;	/* CLI */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_59;	/* EOR abcd,y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_5b;	/* LSE abcd,y [unofficial - LSR then EOR result with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_5d;	/* EOR abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_5e;	/* LSR abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_5f;	/* LSE abcd,x [unofficial - LSR then EOR result with A] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_60;	/* RTS */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_61;	/* ADC (ab,x) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_63;	/* RRA (ab,x) [unofficial - ROR Mem, then ADC to Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_65;	/* ADC ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_66;	/* ROR ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_67;	/* RRA ab [unofficial - ROR Mem, then ADC to Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_68;	/* PLA */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_69;	/* ADC #ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_6a;	/* ROR */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_6b;	/* ARR #ab [unofficial - Acc AND Data, ROR result] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_6c;	/* JMP (abcd) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_6d;	/* ADC abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_6e;	/* ROR abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_6f;	/* RRA abcd [unofficial - ROR Mem, then ADC to Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_70;	/* BVS */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_71;	/* ADC (ab),y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_73;	/* RRA (ab),y [unofficial - ROR Mem, then ADC to Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_75;	/* ADC ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_76;	/* ROR ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_77;	/* RRA ab,x [unofficial - ROR Mem, then ADC to Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_78;	/* SEI */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_79;	/* ADC abcd,y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_7b;	/* RRA abcd,y [unofficial - ROR Mem, then ADC to Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_7d;	/* ADC abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_7e;	/* ROR abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_7f;	/* RRA abcd,x [unofficial - ROR Mem, then ADC to Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_81;	/* STA (ab,x) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_83;	/* SAX (ab,x) [unofficial - Store result A AND X */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_84;	/* STY ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_85;	/* STA ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_86;	/* STX ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_87;	/* SAX ab [unofficial - Store result A AND X] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_88;	/* DEY */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_8a;	/* TXA */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_8b;	/* ANE #ab [unofficial - A AND X AND (Mem OR $EF) to Acc] (Fox) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_8c;	/* STY abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_8d;	/* STA abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_8e;	/* STX abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_8f;	/* SAX abcd [unofficial - Store result A AND X] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_90;	/* BCC */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_91;	/* STA (ab),y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_93;	/* SHA (ab),y [unofficial, UNSTABLE - Store A AND X AND (H+1) ?] (Fox) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_94;	/* STY ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_95;	/* STA ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_96;	/* STX ab,y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_97;	/* SAX ab,y [unofficial - Store result A AND X] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_98;	/* TYA */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_99;	/* STA abcd,y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_9a;	/* TXS */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_9b;	/* SHS abcd,y [unofficial, UNSTABLE] (Fox) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_9c;	/* SHY abcd,x [unofficial - Store Y and (H+1)] (Fox) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_9d;	/* STA abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_9e;	/* SHX abcd,y [unofficial - Store X and (H+1)] (Fox) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_9f;	/* SHA abcd,y [unofficial, UNSTABLE - Store A AND X AND (H+1) ?] (Fox) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_a0;	/* LDY #ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_a1;	/* LDA (ab,x) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_a2;	/* LDX #ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_a3;	/* LAX (ab,x) [unofficial] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_a4;	/* LDY ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_a5;	/* LDA ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_a6;	/* LDX ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_a7;	/* LAX ab [unofficial] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_a8;	/* TAY */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_a9;	/* LDA #ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_aa;	/* TAX */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_ab;	/* ANX #ab [unofficial - AND #ab, then TAX] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_ac;	/* LDY abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_ad;	/* LDA abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_ae;	/* LDX abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_af;	/* LAX abcd [unofficial] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_b0;	/* BCS */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_b1;	/* LDA (ab),y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_b3;	/* LAX (ab),y [unofficial] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_b4;	/* LDY ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_b5;	/* LDA ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_b6;	/* LDX ab,y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_b7;	/* LAX ab,y [unofficial] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_b8;	/* CLV */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_b9;	/* LDA abcd,y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_ba;	/* TSX */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_bb;	/* LAS abcd,y [unofficial - AND S with Mem, transfer to A and X (Fox) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_bc;	/* LDY abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_bd;	/* LDA abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_be;	/* LDX abcd,y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_bf;	/* LAX abcd,y [unofficial] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_c0;	/* CPY #ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_c1;	/* CMP (ab,x) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_c3;	/* DCM (ab,x) [unofficial - DEC Mem then CMP with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_c4;	/* CPY ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_c5;	/* CMP ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_c6;	/* DEC ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_c7;	/* DCM ab [unofficial - DEC Mem then CMP with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_c8;	/* INY */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_c9;	/* CMP #ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_ca;	/* DEX */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_cb;	/* SBX #ab [unofficial - store ((A AND X) - Mem) in X] (Fox) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_cc;	/* CPY abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_cd;	/* CMP abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_ce;	/* DEC abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_cf;	/* DCM abcd [unofficial - DEC Mem then CMP with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_d0;	/* BNE */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_d1;	/* CMP (ab),y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_d3;	/* DCM (ab),y [unofficial - DEC Mem then CMP with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_d5;	/* CMP ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_d6;	/* DEC ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_d7;	/* DCM ab,x [unofficial - DEC Mem then CMP with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_d8;	/* CLD */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_d9;	/* CMP abcd,y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_db;	/* DCM abcd,y [unofficial - DEC Mem then CMP with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_dd;	/* CMP abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_de;	/* DEC abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_df;	/* DCM abcd,x [unofficial - DEC Mem then CMP with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_e0;	/* CPX #ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_e1;	/* SBC (ab,x) */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_e3;	/* INS (ab,x) [unofficial - INC Mem then SBC with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_e4;	/* CPX ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_e5;	/* SBC ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_e6;	/* INC ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_e7;	/* INS ab [unofficial - INC Mem then SBC with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_e8;	/* INX */

#define JIT_insn_opcode_e9	JIT_insn_opcode_eb			/* SBC #ab */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_eb;	/* SBC #ab [unofficial] */

#define JIT_insn_opcode_ea	JIT_insn_opcode_fa			/* NOP */
#define JIT_insn_opcode_1a	JIT_insn_opcode_fa			/* NOP [unofficial] */
#define JIT_insn_opcode_3a	JIT_insn_opcode_fa
#define JIT_insn_opcode_5a	JIT_insn_opcode_fa
#define JIT_insn_opcode_7a	JIT_insn_opcode_fa
#define JIT_insn_opcode_da	JIT_insn_opcode_fa
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_fa;

extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_ec;	/* CPX abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_ed;	/* SBC abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_ee;	/* INC abcd */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_ef;	/* INS abcd [unofficial - INC Mem then SBC with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_f0;	/* BEQ */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_f1;	/* SBC (ab),y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_f3;	/* INS (ab),y [unofficial - INC Mem then SBC with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_f5;	/* SBC ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_f6;	/* INC ab,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_f7;	/* INS ab,x [unofficial - INC Mem then SBC with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_f8;	/* SED */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_f9;	/* SBC abcd,y */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_fb;	/* INS abcd,y [unofficial - INC Mem then SBC with Acc] */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_fd;	/* SBC abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_fe;	/* INC abcd,x */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_ff;	/* INS abcd,x [unofficial - INC Mem then SBC with Acc] */

extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_d2;	/* ESCRTS #ab (CIM) - on Atari is here instruction CIM [unofficial] !RS! */
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_f2;	/* ESC #ab (CIM) - on Atari is here instruction CIM [unofficial] !RS! */

#define JIT_insn_opcode_02	JIT_insn_opcode_b2			/* CIM [unofficial - crash intermediate] */
#define JIT_insn_opcode_12	JIT_insn_opcode_b2
#define JIT_insn_opcode_22	JIT_insn_opcode_b2
#define JIT_insn_opcode_32	JIT_insn_opcode_b2
#define JIT_insn_opcode_42	JIT_insn_opcode_b2
#define JIT_insn_opcode_52	JIT_insn_opcode_b2
#define JIT_insn_opcode_62	JIT_insn_opcode_b2
#define JIT_insn_opcode_72	JIT_insn_opcode_b2
#define JIT_insn_opcode_92	JIT_insn_opcode_b2
extern const struct CPU_JIT_insn_template_t JIT_insn_opcode_b2;

/* Invalidates the host instructions specified by the address. Returns FALSE if not found. */
extern int CPU_JIT_Invalidate(const UWORD addr);
/* Invalidates block(s) specified by the range. */
extern void CPU_JIT_InvalidateMem(const UWORD from, const UWORD to);
/* Invalidates the blocks specified. */
extern void CPU_JIT_InvalidateAllocatedCode(struct CPU_JIT_native_code_t *native_code, const int count);

#endif /* CPU_JIT_H_ */
