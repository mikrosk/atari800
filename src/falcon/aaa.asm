		macro	MEMORY_HwRMW					; \1: <insn> macro
		clr.l	-(sp)							; FALSE (possible side effects)
.m68k_data:
		pea		$ffffabcd.w
		jsr		_MEMORY_HwGetByte
		; d0.b: value
		\1		d0
		move.l	d0,(4,sp)						; replace FALSE with value
		jsr		_MEMORY_HwPutByte
		addq.l	#8,sp
		endm

		macro	GTIA_RMW						; \1: <insn> macro
		move.l	xpos,_ANTIC_xpos
		clr.l	-(sp)							; FALSE (possible side effects)
.m68k_data:
		pea		$ffffabcd.w
		jsr		_GTIA_GetByte
		; d0.b: value
		move.l	d0,(4,sp)						; replace FALSE with value
		subq.l	#1,_ANTIC_xpos
		jsr		_GTIA_PutByte
		addq.l	#1,_ANTIC_xpos
		move.l	(4,sp),d0						; d0.b: value
		\1		d0 ; XXXXXXXX co tak (6,sp) ?
		move.l	d0,(4,sp)						; update value
		jsr		_GTIA_PutByte
		addq.l	#8,sp
		endm

		macro	POKEY_RMW						; \1: <insn> macro
		move.l	xpos,_ANTIC_xpos
		clr.l	-(sp)							; FALSE (possible side effects)
.m68k_data:
		pea		$ffffabcd.w
		jsr		_POKEY_GetByte
		; d0.b: value
		\1		d0
		move.l	d0,(4,sp)						; replace FALSE with value
		jsr		_POKEY_PutByte
		addq.l	#8,sp
		endm

		macro	PIA_RMW							; \1: <insn> macro
		clr.l	-(sp)							; FALSE (possible side effects)
.m68k_data:
		pea		$ffffabcd.w
		jsr		_PIA_GetByte
		; d0.b: value
		\1		d0
		move.l	d0,(4,sp)						; replace FALSE with value
		jsr		_PIA_PutByte
		addq.l	#8,sp
		endm

		macro	ANTIC_RMW						; \1: <insn> macro
		move.l	xpos,_ANTIC_xpos
		clr.l	-(sp)							; FALSE (possible side effects)
.m68k_data:
		pea		$ffffabcd.w
		jsr		_ANTIC_GetByte
		; d0.b: value
		\1		d0
		move.l	d0,(4,sp)						; replace FALSE with value
		jsr		_ANTIC_PutByte
		addq.l	#8,sp
		move.l	_ANTIC_xpos,xpos
		endm
