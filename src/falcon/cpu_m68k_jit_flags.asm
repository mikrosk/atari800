; Bit      : 76543210
; CCR      : ***XNZVC
; CPU_regP : NV*BDIZC

CCR_N	equ		3
CCR_Z	equ		2

N_FLAG	equ		7
V_FLAG	equ		6
B_FLAG	equ		4
D_FLAG	equ		3
I_FLAG	equ		2
Z_FLAG	equ		1
C_FLAG	equ		0

		macro	GetStatus
		move.b	_CPU_regP,\1					; \1.b: 00*BDI00
		move	reg_CCR,ccr
.n\@:	bpl.b	.z\@
		;bset	#N_FLAG,\1
		ori.b	#(1<<N_FLAG),\1
.z\@:	move	reg_CCR,ccr						; comment out to ignore N=1 Z=1 (possible by BIT and PLP)
		bne.b	.v\@
		;bset	#Z_FLAG,\1
		addq.b	#(1<<Z_FLAG),\1
.v\@:	tst.b	V
		beq.b	.c\@
		;bset	#V_FLAG,\1
		ori.b	#(1<<V_FLAG),\1
.c\@:	tst.b	C
		beq.b	.done\@
		;bset	#C_FLAG,\1
		addq.b	#(1<<C_FLAG),\1
.done\@:										; \1.b:   NV*BDIZC
		endm

		macro	PutStatus						; \1.b:    NV*BDIZC
		move.b	\1,C							; dC.b:    NV*BDIZC
		add.b	C,C								; dC.b: [N]V*BDIZC0
		smi		V
		subx.b	reg_CCR,reg_CCR					; dc.b:    NNNNNNNN
		lsl.b	#6,C							; dC.b: [Z]C0000000
		smi		C
		addx.b	reg_CCR,reg_CCR					; dc.b:    NNNNNNNZ
		lsl.b	#2,reg_CCR						; dc.b:    NNNNNZ00
		endm

		macro	ClrV
		clr.b	V
		endm
		macro	SetD
		ori.b	#(1<<D_FLAG),_CPU_regP
		endm
		macro	ClrD
		andi.b	#~(1<<D_FLAG),_CPU_regP
		endm
		macro	SetI
		ori.b	#(1<<I_FLAG),_CPU_regP
		endm
		macro	ClrI
		andi.b	#~(1<<I_FLAG),_CPU_regP
		endm
		macro	SetC
		st		C
		endm
		macro	ClrC
		clr.b	C
		endm

		macro	SAVE_NZ
		move	ccr,reg_CCR
		endm

		macro	SAVE_NZC
		move	ccr,reg_CCR
		subx.b	C,C
		endm

		macro	SAVE_NZc
		move	ccr,reg_CCR
		scc		C								; inverted C
		endm

		macro	SAVE_NVZC
		move	ccr,reg_CCR
		svs		V
		subx.b	C,C
		endm

		macro	SAVE_NVZc
		move	ccr,reg_CCR
		svs		V
		scc		C								; inverted C
		endm

		macro	LOAD_C
		add.b	C,C
		endm

		macro	LOAD_c
		subq.b	#1,C							; inverted C
		endm

		macro	TEST_NZ
		move	reg_CCR,ccr
		endm

		macro	TEST_V
		tst.b	V
		endm

		macro	TEST_C
		tst.b	C
		endm
