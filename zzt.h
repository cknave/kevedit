/* zzt.h        -- ZZT library definitions
 * $Id: zzt.h,v 1.6 2000/11/13 19:56:23 kvance Exp $
 * Copyright (C) 1998-2000 Kev Vance <kvance@tekktonik.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __ZZT_H
#define __ZZT_H 1

#include <sys/types.h>

/* Some basic defines */
#define BOARD_X		60
#define BOARD_Y		25
#define BOARD_MAX	BOARD_X * BOARD_Y
#define TITLE_SIZE	50

#ifdef DOS
/* If you're running DOS, you'll need these defined */
typedef char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef unsigned char u_int8_t;
typedef unsigned short int u_int16_t;
typedef unsigned int u_int32_t;
#endif

/* Read The ZZT File Format for more info on this structure */
typedef struct param {
	u_int8_t x;
	u_int8_t y;
	u_int16_t xstep;
	u_int16_t ystep;
	u_int16_t cycle;
	u_int8_t data1;
	u_int8_t data2;
	u_int8_t data3;
	u_int32_t magic;	/* FFFF FFFF */
	u_int8_t undert;
	u_int8_t underc;
	u_int32_t unused;	/* I _could_ be saving memory here, but nooo :) */
	u_int16_t instruction;
	u_int16_t length;
	u_int8_t pad[8];
	char *moredata;
} param;

/* Read the ZZT File Format for more info on this structure, too */
typedef struct zztheader {
	u_int16_t magicnumber;
	u_int16_t boardcount;
	u_int16_t ammo;
	u_int16_t gems;

	u_int8_t bluekey;
	u_int8_t greenkey;
	u_int8_t cyankey;
	u_int8_t redkey;
	u_int8_t purplekey;
	u_int8_t yellowkey;
	u_int8_t whitekey;

	u_int16_t health;
	u_int16_t startboard;
	u_int16_t torches;
	u_int16_t torchcycles;
	u_int16_t energizercycles;
	u_int16_t unknown;
	u_int16_t score;

	u_int8_t titlelength;
	u_int8_t title[20];

	u_int8_t flag1len;
	u_int8_t flag1[20];
	u_int8_t flag2len;
	u_int8_t flag2[20];
	u_int8_t flag3len;
	u_int8_t flag3[20];
	u_int8_t flag4len;
	u_int8_t flag4[20];
	u_int8_t flag5len;
	u_int8_t flag5[20];
	u_int8_t flag6len;
	u_int8_t flag6[20];
	u_int8_t flag7len;
	u_int8_t flag7[20];
	u_int8_t flag8len;
	u_int8_t flag8[20];
	u_int8_t flag9len;
	u_int8_t flag9[20];
	u_int8_t flag10len;
	u_int8_t flag10[20];

	u_int16_t timepassed;
	u_int16_t nothing;
	u_int8_t savebyte;

	u_int8_t pad[247];
} zztheader;

/* The ZZT File Format is a great document for understanding this structure */
typedef struct boardinfo {
	u_int8_t maxshots;
	u_int8_t darkness;
	u_int8_t board_n;
	u_int8_t board_s;
	u_int8_t board_w;
	u_int8_t board_e;
	u_int8_t reenter;
	u_int8_t messagelen;
	u_int8_t message[58];
	u_int16_t unknown;
	u_int16_t timelimit;
	u_int8_t pad[16];
	u_int16_t objectcount;
} boardinfo;

/* Each board has a title, info, RLE compressed data, and max 151 params */
typedef struct board {
	u_int8_t *title;
	boardinfo *info;
	u_int8_t *data;
	param *params[152];
} board;

/* Each world has a header, and max 255 boards */
typedef struct world {
	zztheader *zhead;
	board *board[255];
} world;

/* rle_encode(uncompressed)
 * Takes a block of data with the structure:
 * [code] [colour] [code] [colour] ...
 * and returns an RLE compressed block with the structure:
 * [number] [code] [colour] */
extern u_int8_t *rle_encode(u_int8_t *);
/* rle_decode(compressed, uncompressed)
 * Decodes the RLE compressed data in the first argument out to a block of
 * data the size of BOARD_MAX * 2 in the second argument */
extern int rle_decode(u_int8_t *, u_int8_t *);
/* rle_lengthof(compressed)
 * Gives the actual size (in bytes) of an rle encoded ZZT board */
extern long rle_lengthof(u_int8_t *);

/* linechar(uncompressed, x, y)
 * Returns the IBM ASCII value for a line on an uncompressed board with a
 * given x and y position (mostly used internally) */
extern u_int8_t linechar(u_int8_t *, u_int8_t, u_int8_t);

/* z_getchar(code, colour, param, uncompressed, x, y)
 * Returns the IBM ASCII value for any code given its colour, parameter
 * record (yes, you must search through the parameter records and pass the
 * correct param to this function), the uncompressed board, and its x and y
 * position */
extern u_int8_t z_getchar(u_int8_t, u_int8_t, param *, u_int8_t *, u_int8_t, u_int8_t);
/* z_getcolour(code, colour, param)
 * Returns the standard PC textmode colour ((back << 4) + fore) of any code
 * given its colour and parameter record (and only /its/ parameter record) */
extern u_int8_t z_getcolour(u_int8_t, u_int8_t, param *);
/* saveworld(file, world)
 * Writes the given world file to disk as the filename file */
extern int saveworld(char *fname, world * wo);
/* loadworld(file)
 * Returns a world structure for a given filename of a ZZT file */
extern world *loadworld(char *fname);
/* z_delete(world)
 * Frees an entire world structure */
extern void z_delete(world * w);
/* z_newworld()
 * Creates a new world structure and returns it */
extern world *z_newworld();
/* z_newboard(title)
 * Creates a new board with the given title and returns it */
extern board *z_newboard(char *);

/* z_newparam_object(x, y, char, undertype, undercolor)
 * Creates a new object paramater record with the given information and a NULL program */
extern param *z_newparam_object(int x, int y, int ch, int undert, int underc);
/* z_newparam_scroll(x, y, undertype, undercolor)
 * Creates a new scroll parameter record with the given information and NULL data */
extern param *z_newparam_scroll(int x, int y, int undert, int underc);
/* z_newparam_conveyer(x, y)
 * Creates a new conveyer parameter at cycle 3 */
extern param *z_newparam_conveyer(int x, int y);
/* z_newparam_conveyer(x, y)
 * Creates a new passage to the given board */
extern param *z_newparam_passage(int x, int y, int b);
/* z_newparam_bomb(x, y)
 * Create a new unlit bomb at x, y */
extern param *z_newparam_bomb(int x, int y);

/* Definitions of codes: */
#define Z_EMPTY		0x00
#define Z_EDGE		0x01
#define Z_PLAYER	0x04
#define Z_AMMO		0x05
#define Z_TORCH		0x06
#define Z_GEM		0x07
#define Z_KEY		0x08
#define Z_DOOR		0x09
#define Z_SCROLL	0x0A
#define Z_PASSAGE	0x0B
#define Z_DUPLICATOR	0x0C
#define Z_BOMB		0x0D
#define Z_ENERGIZER	0x0E
#define Z_STAR		0x0F
#define Z_CWCONV	0x10
#define Z_CCWCONV	0x11
#define Z_BULLET	0x12
#define Z_WATER		0x13
#define Z_FOREST	0x14
#define Z_SOLID		0x15
#define Z_NORMAL	0x16
#define Z_BREAKABLE	0x17
#define Z_BOULDER	0x18
#define Z_NSSLIDER	0x19
#define Z_EWSLIDER	0x1A
#define Z_FAKE		0x1B
#define Z_INVISIBLE	0x1C
#define Z_BLINK		0x1D
#define Z_TRANSPORTER	0x1E
#define Z_LINE		0x1F
#define Z_RICOCHET	0x20
#define Z_BLINKHORIZ	0x21
#define Z_BEAR		0x22
#define Z_RUFFIAN	0x23
#define Z_OBJECT	0x24
#define Z_SLIME		0x25
#define Z_SHARK		0x26
#define Z_SPINNINGGUN	0x27
#define Z_PUSHER	0x28
#define Z_LION		0x29
#define Z_TIGER		0x2A
#define Z_BLINKVERT	0x2B
#define Z_CENTHEAD	0x2C
#define Z_CENTBODY	0x2D
#define Z_BLUETEXT	0x2F
#define Z_GREENTEXT	0x30
#define Z_CYANTEXT	0x31
#define Z_REDTEXT	0x32
#define Z_PURPLETEXT	0x33
#define Z_YELLOWTEXT	0x34
#define Z_WHITETEXT	0x35
#define Z_BBLUETEXT	0x37
#define Z_BGREENTEXT	0x38
#define Z_BCYANTEXT	0x39
#define Z_BREDTEXT	0x3A
#define Z_BPURPLETEXT	0x3B
#define Z_BYELLOWTEXT	0x3C
#define Z_BWHITETEXT	0x3D

#endif
