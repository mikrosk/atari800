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
