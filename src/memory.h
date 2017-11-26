#ifndef MEMORY_H_
#define MEMORY_H_

#include "config.h"

#include "atari.h"
#ifdef CPU_JIT
#include "cpu_jit.h"
#endif

#define MEMORY_dGetByte(x)				(MEMORY_mem[x])
#ifndef CPU_JIT
#define MEMORY_dPutByte(x, y)			(MEMORY_mem[x] = y)
#else
void MEMORY_dPutByte(UWORD addr, UBYTE value);
#endif

#if !defined(WORDS_BIGENDIAN) && defined(WORDS_UNALIGNED_OK) && !defined(CPU_JIT)
#define MEMORY_dGetWord(x)				UNALIGNED_GET_WORD(MEMORY_mem+(x), memory_read_word_stat)
#define MEMORY_dPutWord(x, y)			UNALIGNED_PUT_WORD(MEMORY_mem+(x), (y), memory_write_word_stat)
#define MEMORY_dGetWordAligned(x)		UNALIGNED_GET_WORD(MEMORY_mem+(x), memory_read_aligned_word_stat)
#define MEMORY_dPutWordAligned(x, y)	UNALIGNED_PUT_WORD(MEMORY_mem+(x), (y), memory_write_aligned_word_stat)
#else
/* can't or don't want to do word optimizations */
#define MEMORY_dGetWord(x)				(MEMORY_mem[x] + (MEMORY_mem[(x) + 1] << 8))
#ifndef CPU_JIT
#define MEMORY_dPutWord(x, y)			(MEMORY_mem[x] = (UBYTE) (y), MEMORY_mem[(x) + 1] = (UBYTE) ((y) >> 8))
#else
void MEMORY_dPutWord(UWORD addr, UWORD value);
#endif
/* faster versions of MEMORY_dGetWord and MEMORY_dPutWord for even addresses */
/* TODO: guarantee that memory is UWORD-aligned and use UWORD access */
#define MEMORY_dGetWordAligned(x)		MEMORY_dGetWord(x)
#define MEMORY_dPutWordAligned(x, y)	MEMORY_dPutWord(x, y)
#endif

void MEMORY_dCopyFromMem(UWORD from, void* to, size_t size);
void MEMORY_dCopyToMem(const void* from, UWORD to, size_t size);
void MEMORY_dFillMem(UWORD addr1, UBYTE value, size_t length);

extern UBYTE MEMORY_mem[65536 + 2];
#ifdef CPU_JIT
extern struct CPU_JIT_native_code_t MEMORY_JIT_mem[65536 + 2];
#define MEMORY_JIT_SIZE(x) ((x) * sizeof(MEMORY_JIT_mem[0]))
#endif

/* RAM size in kilobytes.
   Valid values for Atari800_MACHINE_800 are: 16, 48, 52.
   Valid values for Atari800_MACHINE_XLXE are: 16, 64, 128, 192, RAM_320_RAMBO,
   RAM_320_COMPY_SHOP, 576, 1088.
   The only valid value for Atari800_MACHINE_5200 is 16. */
#define MEMORY_RAM_320_RAMBO       320
#define MEMORY_RAM_320_COMPY_SHOP  321
extern int MEMORY_ram_size;

#ifndef PAGED_ATTRIB

#define MEMORY_RAM       0
#define MEMORY_ROM       1
#define MEMORY_HARDWARE  2

extern UBYTE MEMORY_attrib[65536];
/* Reads a byte from ADDR. Can potentially have side effects, when reading
   from hardware area. */
#define MEMORY_GetByte(addr)		(MEMORY_attrib[addr] == MEMORY_HARDWARE ? MEMORY_HwGetByte(addr, FALSE) : MEMORY_mem[addr])
/* Reads a byte from ADDR, but without any side effects. */
#define MEMORY_SafeGetByte(addr)		(MEMORY_attrib[addr] == MEMORY_HARDWARE ? MEMORY_HwGetByte(addr, TRUE) : MEMORY_mem[addr])
#define MEMORY_PutByte(addr, byte)	 do { if (MEMORY_attrib[addr] == MEMORY_RAM) MEMORY_dPutByte(addr, byte); else if (MEMORY_attrib[addr] == MEMORY_HARDWARE) MEMORY_HwPutByte(addr, byte); } while (0)
void MEMORY_SetHARDWARE(UWORD addr1, UWORD addr2);

#else /* PAGED_ATTRIB */

typedef UBYTE (*MEMORY_rdfunc)(UWORD addr, int no_side_effects);
typedef void (*MEMORY_wrfunc)(UWORD addr, UBYTE value);
extern MEMORY_rdfunc MEMORY_readmap[256];
extern MEMORY_rdfunc MEMORY_safe_readmap[256];
extern MEMORY_wrfunc MEMORY_writemap[256];
void MEMORY_ROM_PutByte(UWORD addr, UBYTE byte);
/* Reads a byte from ADDR. Can potentially have side effects, when reading
   from hardware area. */
#define MEMORY_GetByte(addr)		(MEMORY_readmap[(addr) >> 8] ? (*MEMORY_readmap[(addr) >> 8])(addr, FALSE) : MEMORY_mem[addr])
/* Reads a byte from ADDR, but without any side effects. */
#define MEMORY_SafeGetByte(addr)		(MEMORY_readmap[(addr) >> 8] ? (*MEMORY_readmap[(addr) >> 8])(addr, TRUE) : MEMORY_mem[addr])
#define MEMORY_PutByte(addr,byte)	(MEMORY_writemap[(addr) >> 8] ? ((*MEMORY_writemap[(addr) >> 8])(addr, byte), 0) : (MEMORY_dPutByte(addr, byte)))

#endif /* PAGED_ATTRIB */

extern UBYTE MEMORY_basic[8192];
extern UBYTE MEMORY_os[16384];
extern UBYTE MEMORY_xegame[8192];

extern int MEMORY_xe_bank;
extern int MEMORY_selftest_enabled;

extern int MEMORY_have_basic;
extern int MEMORY_cartA0BF_enabled;

/* Verifies if SIZE is a correct value for RAM size. */
int MEMORY_SizeValid(int size);
void MEMORY_InitialiseMachine(void);
void MEMORY_StateSave(UBYTE SaveVerbose);
void MEMORY_StateRead(UBYTE SaveVerbose, UBYTE StateVersion);
void MEMORY_CopyFromMem(UWORD from, UBYTE *to, int size);
void MEMORY_CopyToMem(const UBYTE *from, UWORD to, int size);
void MEMORY_HandlePORTB(UBYTE byte, UBYTE oldval);
void MEMORY_Cart809fDisable(void);
void MEMORY_Cart809fEnable(void);
void MEMORY_CartA0bfDisable(void);
void MEMORY_CartA0bfEnable(void);
void MEMORY_CopyROM(UWORD addr1, UWORD addr2, const void* src);
void MEMORY_GetCharset(UBYTE *cs);
void MEMORY_SetRAM(UWORD addr1, UWORD addr2);
void MEMORY_SetROM(UWORD addr1, UWORD addr2);

/* Mosaic and Axlon 400/800 RAM extensions */
extern int MEMORY_mosaic_num_banks;
extern int MEMORY_axlon_0f_mirror;
extern int MEMORY_axlon_num_banks;

/* Controls presence of MapRAM memory modification for XL/XE mode. */
extern int MEMORY_enable_mapram;

#ifndef PAGED_MEM
/* Reads a byte from the specified special address (not RAM or ROM). */
UBYTE MEMORY_HwGetByte(UWORD addr, int safe);

/* Stores a byte at the specified special address (not RAM or ROM). */
void MEMORY_HwPutByte(UWORD addr, UBYTE byte);
#endif /* PAGED_MEM */

#endif /* MEMORY_H_ */
