#ifndef _ATARI_H_
#define _ATARI_H_

#include "config.h"
#include <stdint.h>
#include <stddef.h> /* size_t */

/* Fundamental declarations ---------------------------------------------- */

#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1
#endif

/* SBYTE and UBYTE must be exactly 1 byte long. */
/* SWORD and UWORD must be exactly 2 bytes long. */
/* SLONG and ULONG must be exactly 4 bytes long. */
#define SBYTE int8_t
#define SWORD int16_t
#define SLONG int32_t
#define UBYTE uint8_t
#define UWORD uint16_t
#ifndef WIN32
/* Windows headers typedef ULONG */
#define ULONG uint32_t
#endif
/* Note: in various parts of the emulator we assume that char is 1 byte
   and int is 4 bytes. */


/* Public interface ------------------------------------------------------ */

#define ATARI_VISIBLE_WIDTH 336
#define ATARI_LEFT_MARGIN 24

/* RAM size in kilobytes.
   The only valid value for MACHINE_5200 is 16. */

/* Video system. */
#define TV_PAL 312
#define TV_NTSC 262
extern int tv_mode;

/* Dimensions of atari_screen.
   atari_screen is ATARI_WIDTH * ATARI_HEIGHT bytes.
   Each byte is an Atari color code - use Palette_Get[RGB] functions
   to get actual RGB codes.
   You should never display anything outside the middle 336 columns. */
#define ATARI_WIDTH  384
#define ATARI_HEIGHT 240

#define SOUND_SAMPLE_RATE 44100

/* File types returned by Atari800_DetectFileType() and Atari800_OpenFile(). */
#define AFILE_ERROR      0
#define AFILE_ATR        1
#define AFILE_XFD        2
#define AFILE_ATR_GZ     3
#define AFILE_XFD_GZ     4
#define AFILE_DCM        5
#define AFILE_XEX        6
#define AFILE_BAS        7
#define AFILE_LST        8
#define AFILE_CART       9
#define AFILE_ROM        10
#define AFILE_CAS        11
#define AFILE_BOOT_TAPE  12
#define AFILE_STATE      13
#define AFILE_STATE_GZ   14

/* Initializes Atari800 emulation core. */
void Atari800_Initialise(void);

/* Emulates one frame (1/50sec for PAL, 1/60sec for NTSC). */
void Atari800_Frame(void);

/* Reboots the emulated Atari. */
void Coldstart(void);

/* Presses the Reset key in the emulated Atari. */
void Warmstart(void);

/* Auto-detects file type and mounts the file in the emulator.
   reboot: Coldstart() for disks, cartridges and tapes
   diskno: drive number for disks (1-8)
   readonly: mount disks as read-only */
int Atari800_OpenFile(const uint8_t *data, size_t size);

/* Shuts down Atari800 emulation core. */
int Atari800_Exit(void);


/* Private interface ----------------------------------------------------- */
/* Don't use outside the emulation core! */

/* ATR format header */
struct ATR_Header {
	unsigned char magic1;
	unsigned char magic2;
	unsigned char seccountlo;
	unsigned char seccounthi;
	unsigned char secsizelo;
	unsigned char secsizehi;
	unsigned char hiseccountlo;
	unsigned char hiseccounthi;
	unsigned char gash[7];
	unsigned char writeprotect;
};

/* First two bytes of an ATR file. */
#define MAGIC1  0x96
#define MAGIC2  0x02

/* Current clock cycle in a scanline.
   Normally 0 <= xpos && xpos < LINE_C, but in some cases xpos >= LINE_C,
   which means that we are already in line (ypos + 1). */
extern int xpos;

/* xpos limit for the currently running 6502 emulation. */
extern int xpos_limit;

/* Number of cycles per scanline. */
#define LINE_C   114

/* STA WSYNC resumes here. */
#define WSYNC_C  106

/* Number of memory refresh cycles per scanline.
   In the first scanline of a font mode there are actually less than DMAR
   memory refresh cycles. */
#define DMAR     9

/* Number of scanlines per frame. */
#define max_ypos tv_mode

/* Main clock value at the beginning of the current scanline. */
extern unsigned int screenline_cpu_clock;

/* Current main clock value. */
#define cpu_clock (screenline_cpu_clock + xpos)

/* STAT_UNALIGNED_WORDS is solely for benchmarking purposes.
   8-element arrays (stat_arr) represent number of accesses with the given
   value of 3 least significant bits of address. This gives us information
   about the ratio of aligned vs unaligned accesses. */
#ifdef STAT_UNALIGNED_WORDS
#define UNALIGNED_STAT_DEF(stat_arr)             unsigned int stat_arr[8];
#define UNALIGNED_STAT_DECL(stat_arr)            extern unsigned int stat_arr[8];
#define UNALIGNED_GET_WORD(ptr, stat_arr)        (stat_arr[(unsigned int) (ptr) & 7]++, *(const UWORD *) (ptr))
#define UNALIGNED_PUT_WORD(ptr, value, stat_arr) (stat_arr[(unsigned int) (ptr) & 7]++, *(UWORD *) (ptr) = (value))
#define UNALIGNED_GET_LONG(ptr, stat_arr)        (stat_arr[(unsigned int) (ptr) & 7]++, *(const ULONG *) (ptr))
#define UNALIGNED_PUT_LONG(ptr, value, stat_arr) (stat_arr[(unsigned int) (ptr) & 7]++, *(ULONG *) (ptr) = (value))
UNALIGNED_STAT_DECL(atari_screen_write_long_stat)
UNALIGNED_STAT_DECL(pm_scanline_read_long_stat)
UNALIGNED_STAT_DECL(memory_read_word_stat)
UNALIGNED_STAT_DECL(memory_write_word_stat)
UNALIGNED_STAT_DECL(memory_read_aligned_word_stat)
UNALIGNED_STAT_DECL(memory_write_aligned_word_stat)
#else
#define UNALIGNED_STAT_DEF(stat_arr)
#define UNALIGNED_STAT_DECL(stat_arr)
#define UNALIGNED_GET_WORD(ptr, stat_arr)        (*(const UWORD *) (ptr))
#define UNALIGNED_PUT_WORD(ptr, value, stat_arr) (*(UWORD *) (ptr) = (value))
#define UNALIGNED_GET_LONG(ptr, stat_arr)        (*(const ULONG *) (ptr))
#define UNALIGNED_PUT_LONG(ptr, value, stat_arr) (*(ULONG *) (ptr) = (value))
#endif

/* Escape codes used to mark places in 6502 code that must
   be handled specially by the emulator. An escape sequence
   is an illegal 6502 opcode 0xF2 or 0xD2 followed
   by one of these escape codes: */
enum ESCAPE {

	/* SIO patch. */
	ESC_SIOV,

	/* stdio-based handlers for the BASIC version
	   and handlers for Atari Basic loader. */
	ESC_EHOPEN,
	ESC_EHCLOS,
	ESC_EHREAD,
	ESC_EHWRIT,
	ESC_EHSTAT,
	ESC_EHSPEC,

	ESC_KHOPEN,
	ESC_KHCLOS,
	ESC_KHREAD,
	ESC_KHWRIT,
	ESC_KHSTAT,
	ESC_KHSPEC,

	/* Atari executable loader. */
	ESC_BINLOADER_CONT,

	/* Cassette emulation. */
	ESC_COPENLOAD = 0xa8,
	ESC_COPENSAVE = 0xa9,

	/* Printer. */
	ESC_PHOPEN = 0xb0,
	ESC_PHCLOS = 0xb1,
	ESC_PHREAD = 0xb2,
	ESC_PHWRIT = 0xb3,
	ESC_PHSTAT = 0xb4,
	ESC_PHSPEC = 0xb5,
	ESC_PHINIT = 0xb6,

	/* H: device. */
	ESC_HHOPEN = 0xc0,
	ESC_HHCLOS = 0xc1,
	ESC_HHREAD = 0xc2,
	ESC_HHWRIT = 0xc3,
	ESC_HHSTAT = 0xc4,
	ESC_HHSPEC = 0xc5,
	ESC_HHINIT = 0xc6
};

/* A function called to handle an escape sequence. */
typedef void (*EscFunctionType)(void);

extern int hold_start;
extern int press_space;

extern int start_binloading;

void Atari_Initialise(void);

/* Puts an escape sequence at the specified address. */
void Atari800_AddEsc(UWORD address, UBYTE esc_code, EscFunctionType function);

/* Puts an escape sequence followed by the RTS instruction. */
void Atari800_AddEscRts(UWORD address, UBYTE esc_code, EscFunctionType function);

/* Unregisters an escape sequence. You must cleanup the Atari memory yourself. */
void Atari800_RemoveEsc(UBYTE esc_code);

/* Handles an escape sequence. */
void Atari800_RunEsc(UBYTE esc_code);

/* Reads a byte from the specified special address (not RAM or ROM). */
UBYTE Atari800_GetByte(UWORD addr);

/* Stores a byte at the specified special address (not RAM or ROM). */
void Atari800_PutByte(UWORD addr, UBYTE byte);

unsigned int Atari_PORT(unsigned int num);
unsigned int Atari_TRIG(unsigned int num);
unsigned int Atari_POT(unsigned int num);

#endif /* _ATARI_H_ */
