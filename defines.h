#ifndef DEFINES_H
#define DEFINES_H
#define _MAX_PATH 100

typedef unsigned char BYTE;
typedef quint16 UINT;
typedef quint32 DWORD;
typedef quint16 WORD;
#define EDIT_WINDOW_SQ	216						/* Edit window square [dot] */
#define	MAX_FONT_SQ		(EDIT_WINDOW_SQ / 3)	/* Maximum font square [dot] = 72 */
#define MAX_FONT_WB	((MAX_FONT_SQ + 7) / 8)
#define IS_DBC1(c)	(((BYTE)(c) >= 0x81 && (BYTE)(c) <= 0x9F) || ((BYTE)(c) >= 0xE0 && (BYTE)(c) <= 0xFC))
#define IS_DBC2(c)	(((BYTE)(c) >= 0x40 && (BYTE)(c) <= 0x7E) || ((BYTE)(c) >= 0x80 && (BYTE)(c) <= 0xFC))
#define IS_DBC(c)	(IS_DBC1((WORD)(c) >> 8) && IS_DBC2((c) & 0xFF))
#define IS_SBC(c)	(((WORD)c >= 0x20 && (WORD)c <= 0x7E) || ((WORD)c >= 0xA1 && (WORD)c <= 0xDF))

#endif // DEFINES_H
