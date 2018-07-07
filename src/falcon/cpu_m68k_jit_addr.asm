; base addressing modes (specified by the template in upper byte)
accumulator			equ	$0000
absolute			equ	$1000
absolute_x			equ	$2000
absolute_y			equ	$3000
immediate			equ	$4000
implied				equ	$5000
indirect			equ	$6000
x_indirect			equ	$7000
indirect_y			equ	$8000
relative			equ	$9000
zero_page			equ	$a000
zero_page_x			equ	$b000
zero_page_y			equ	$c000

; addressing mode flags (specified by the JIT in lower byte)
flag_rmw			equ	1<<1
flag_hw				equ	1<<2
flag_special1		equ	1<<5
flag_special2		equ	1<<6
flag_special3		equ	1<<7

absolute_rmw		equ	absolute|flag_rmw
absolute_hw			equ	absolute|flag_hw
absolute_hw_rmw		equ	absolute|flag_hw|flag_rmw
absolute_x_rmw		equ	absolute_x|flag_rmw
absolute_x_hw		equ	absolute_x|flag_hw
absolute_x_hw_rmw	equ	absolute_x|flag_hw|flag_rmw
absolute_y_hw		equ	absolute_y|flag_hw
immediate_ld_le_7f	equ	immediate|flag_special1
immediate_ld_eq_80	equ	immediate|flag_special2
immediate_ld_gt_80	equ	immediate|flag_special3
zero_page_rmw		equ	zero_page|flag_rmw
zero_page_x_rmw		equ	zero_page_x|flag_rmw


		macro	ACCUMULATOR
		dc.b	no_stop
		dc.b	accumulator
		NO_OFFSET_WITH_BYTES_AND_CYCLES
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		clr.l	d0
		endm

		macro	ABSOLUTE
		dc.b	no_stop
		dc.b	absolute
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
.m68k_data:
		move.l	#$00000000,d0
		endm

		macro	ABSOLUTE_JUMP
		dc.b	is_stop
		dc.b	absolute
		OFFSET_WITH_CYCLES 4
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		endm

		macro	ABSOLUTE_X
		dc.b	no_stop
		dc.b	absolute_x
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
.m68k_data:
		move.l	#$00000000,d0
		add.w	reg_X,d0
		endm

		macro	ABSOLUTE_Y
		dc.b	no_stop
		dc.b	absolute_y
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
.m68k_data:
		move.l	#$00000000,d0
		add.w	reg_Y,d0
		endm

		macro	IMMEDIATE
		dc.b	no_stop
		dc.b	immediate
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
.m68k_data:
		move.l	#$00000000,d0
		endm

		macro	IMPLIED
		dc.b	no_stop
		dc.b	implied
		NO_OFFSET_WITH_BYTES_AND_CYCLES
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		clr.l	d0
		endm

		macro	IMPLIED_RETURN
		dc.b	is_stop
		dc.b	implied
		NO_OFFSET_WITH_CYCLES
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		clr.l	d0
		endm

		macro	INDIRECT
		dc.b	is_stop
		dc.b	indirect
		OFFSET_WITH_CYCLES	4
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		endm

		macro	INDIRECT_X
		dc.b	no_stop
		dc.b	x_indirect
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
.m68k_data:
		move.l	#$00000000,d0
		add.b	reg_X,d0
		MEMORY_dGetWord
		endm

		macro	INDIRECT_Y
		dc.b	no_stop
		dc.b	indirect_y
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
.m68k_data:
		move.l	#$00000000,d0
		MEMORY_dGetWord
		add.w	reg_Y,d0
		endm

		macro	RELATIVE
		dc.b	is_stop
		dc.b	relative
		OFFSET_WITH_BYTES_AND_CYCLES_EXTRA 4
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		endm

		macro	ZPAGE
		dc.b	no_stop
		dc.b	zero_page
		OFFSET_WITH_BYTES_AND_CYCLES 2
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		endm

		macro	ZPAGE_RMW
		dc.b	no_stop
		dc.b	zero_page
		OFFSET_WITH_DATA_EXTRA_AND_BYTES_AND_CYCLES 2
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
		endm

		macro	ZPAGE_X
		dc.b	no_stop
		dc.b	zero_page_x
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
.m68k_data:
		move.l	#$00000000,d0
		add.b	reg_X,d0
		endm

		macro	ZPAGE_Y
		dc.b	no_stop
		dc.b	zero_page_y
		OFFSET_WITH_BYTES_AND_CYCLES 4
.m68k_start:
		DUMP
.m68k_cycles:
		addq.l	#1,xpos							; 2 - 8
.m68k_data:
		move.l	#$00000000,d0
		add.b	reg_Y,d0
		endm

		macro	NOT_IMPLEMENTED
		dc.b	unknown
		dc.b	unknown
		endm



		macro	ABSOLUTE						; \1: <insn>, \2: reg_[AXY]
.m68k_p_base:
		dc.l	.m68k_start\@
		dc.l	.m68k_end\@
		dc.l	.m68k_data\@
		dc.l	0
.m68k_start\@:
.m68k_data\@: equ *+2
		\1.b	($abcd-$8000.w,memory),\2
.m68k_end\@:
		endm

		macro	ABSOLUTE_RMW					; \1: <insn>, \2: dn
.m68k_p_rmw:
		dc.l	.m68k_start\@
		dc.l	.m68k_end\@
		dc.l	.m68k_data\@
		dc.l	0
.m68k_start\@:
.m68k_data\@: equ *+2
		lea		($abcd-$8000.w,memory),a0
		move.b	(a0),\2
		\1		\2
		move.b	\2,(a0)
.m68k_end\@:
		endm

		macro	ABSOLUTE_HW						; \1: <insn>, \2: reg_[AXY]
.m68k_p_hw:
		dc.l	.m68k_start\@
		dc.l	.m68k_end\@
		dc.l	.m68k_data\@
		dc.l	.m68k_data_extra\@
.m68k_start\@:
		move.l	xpos,_ANTIC_xpos
		clr.l	-(sp)							; FALSE (possible side effects)
.m68k_data\@: equ *+2
		pea		$ffffabcd.w
.m68k_data_extra\@: equ *+2						; this will be handled via hw_get_table[data_extra]
		jsr		_MEMORY_HwGetByte
		addq.l	#8,sp
		\1.b	d0,\2
.m68k_end\@:
		move.l	_ANTIC_xpos,xpos
		endm

		macro	ABSOLUTE_HW_RMW					; \1: <insn>
.m68k_p_hw_rmw:
		dc.l	.m68k_start\@
		dc.l	.m68k_end\@
		dc.l	.m68k_data\@
		dc.l	0
.m68k_start\@:
		move.l	xpos,_ANTIC_xpos
		clr.l	-(sp)							; FALSE (possible side effects)
.m68k_data\@: equ *+2
		pea		$ffffabcd.w
		jsr		_MEMORY_HwGetByte				; d0.b: value
		;move.l	d0,(4,sp)						; replace FALSE with old value
		;subq.l	#1,_ANTIC_xpos
		;jsr		_GTIA_PutByte
		;addq.l	#1,_ANTIC_xpos
		;move.l	(4,sp),d0						; d0.b: value
		\1		d0
		move.l	d0,(4,sp)						; replace FALSE with value
		jsr		_MEMORY_HwPutByte
		addq.l	#8,sp
		move.l	_ANTIC_xpos,xpos
.m68k_end\@:
		endm

		macro	ABSOLUTE_ST						; \1: reg_[AXY]
.m68k_p_base:
		dc.l	.m68k_start\@
		dc.l	.m68k_end\@
		dc.l	.m68k_data\@
		dc.l	0
.m68k_start\@:
.m68k_data\@: equ *+2
		move.b	\1,($abcd-$8000.w,memory)
.m68k_end\@:
		endm

		macro	ABSOLUTE_HW_ST					; \1: reg_[AXY]
.m68k_p_hw:
		dc.l	.m68k_start\@
		dc.l	.m68k_end\@
		dc.l	.m68k_data\@
		dc.l	.m68k_data_extra\@
.m68k_start\@:
		move.l	xpos,_ANTIC_xpos
		move.l	\1,-(sp)
.m68k_data\@: equ *+2
		pea		$ffffabcd.w
.m68k_data_extra\@: equ *+2						; this will be handled via hw_put_table[data_extra]
		jsr		_MEMORY_HwPutByte
		addq.l	#8,sp
		move.l	_ANTIC_xpos,xpos
.m68k_end\@:
		endm

		macro	ZPAGE							; \1: <insn>, \2: reg_[AXY]
.m68k_data equ *+2
		\1.b	($00ab-$8000.w,memory),\2
		endm

		macro	ZPAGE_ST						; \1: reg_[AXY]
.m68k_data equ *+2
		move.b	\1,($00ab-$8000.w,memory)
		endm

		macro	ABSOLUTE_X						; \1: <insn>, \2: reg_[AY]
.m68k_data_extra equ *+1						; this will be handled a move.b <data>,(extra,an)
		moveq	#$cd,d0
		add.b	reg_X,d0
		subx.l	d0,d0
		sub.l	d0,xpos
.m68k_data equ *+4
		\1.b	($abcd-$8000.w,memory,reg_X.w),\2
		endm

		macro	ABSOLUTE_X_ST					; \1: reg_A
.m68k_data equ *+4
		move.b	\1,($abcd-$8000.w,memory,reg_X.w)
		endm

		macro	ABSOLUTE_Y						; \1: <insn>, \2: reg_[AX]
.m68k_data_extra equ *+1						; this will be handled a move.b <data>,(extra,an)
		moveq	#$cd,d0
		add.b	reg_Y,d0
		subx.l	d0,d0
		sub.l	d0,xpos
.m68k_data equ *+4
		\1.b	($abcd-$8000.w,memory,reg_Y.w),\2
		endm

		macro	ABSOLUTE_Y_ST					; \1: reg_A
.m68k_data equ *+4
		move.b	\1,($abcd-$8000.w,memory,reg_Y.w)
		endm

		macro	IMMEDIATE_A						; \1: <insn>, \2: #imm
		\1.b	\2,reg_A
		endm
		macro	IMMEDIATE_X						; \1: <insn>, \2: #imm
		\1.b	\2,reg_X
		endm
		macro	IMMEDIATE_Y						; \1: <insn>, \2: #imm
		\1.b	\2,reg_Y
		endm

		macro	INDIRECT_X						; \1: <insn>, \2: reg_A
.m68k_data equ *+2
		move.w	#$abcd-$8000,d0
		add.b	reg_X,d0
		move.w (0.b,memory,d0.w),d0
		ror.w	#8,d0
		\1.b	(-$8000.w,memory,d0.w),\2		; TODO: this could be a HW address
		endm

		macro	INDIRECT_X_ST					; \1: reg_A
.m68k_data equ *+2
		move.w	#$abcd-$8000,d0
		add.b	reg_X,d0
		move.w (0.b,memory,d0.w),d0
		ror.w	#8,d0
		move.b	\1,(-$8000.w,memory,d0.w)
		endm

		macro	INDIRECT_Y						; \1: <insn>, \2: reg_A
.m68k_data equ *+2
		move.w ($00ab-$8000.w,memory),d0
		ror.w	#8,d0
		add.b	reg_Y,d0
		subx.l	d1,d1							; d1.l: $ffffffff if C=1, $00000000 if C=0
		suba.l	d1,xpos							; flags not affected
		negx.b	d1								; d1.w: $ff00 if C=1, $0000 if C=0
		sub.w	d1,d0
		\1.b	(-$8000.w,memory,d0.w),\2		; TODO: this could be a HW address
		endm

		macro	INDIRECT_Y_ST					; \1: reg_A
.m68k_data equ *+2
		move.w ($00ab-$8000.w,memory),d0
		ror.w	#8,d0
		add.w	reg_Y,d0
		move.b	\1,(-$8000.w,memory,d0.w)
		endm

		macro	ZPAGE_X							; \1: <insn>, \2: reg_[AY]
.m68k_data equ *+2
		move.w	#$00ab-$8000,d0
		add.b	reg_X,d0
		\1.b	(0.b,memory,d0.w),\2
		endm

		macro	ZPAGE_X_ST						; \1: reg_[AY]
.m68k_data equ *+2
		move.w	#$00ab-$8000,d0
		add.b	reg_X,d0
		move.b	\1,(0.b,memory,d0.w)
		endm

		macro	ZPAGE_Y							; \1: <insn>, \2: reg_X
.m68k_data equ *+2
		move.w	#$00ab-$8000,d0
		add.b	reg_Y,d0
		\1.b	(0.b,memory,d0.w),\2
		endm

		macro	ZPAGE_Y_ST						; \1: reg_X
.m68k_data equ *+2
		move.w	#$00ab-$8000,d0
		add.b	reg_Y,d0
		move.b	\1,(0.b,memory,d0.w)
		endm
