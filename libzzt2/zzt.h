/* libzzt2	-- The ZZT library that behaves like a library
 * $Id: zzt.h,v 1.2 2002/02/02 05:19:51 bitman Exp $
 * Copyright (C) 2001 Kev Vance <kev@kvance.com>
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

#include <stdio.h>
#include <sys/types.h>

#ifndef __ZZT_H
#define __ZZT_H

/* Perhaps we can put these type defines in another file?
 * "types.h" seems okay, but something like "sizetypes.h" would be more
 * appropriate. This only seems to be needed by the DOS version of DJGPP */

/* Also, this used to use #ifdef DOS, but that was setup by the KevEdit
 * makefile. Should we do the same thing here, or find another way
 * of determining the necessity of these typdefs? */

/* If you're running DOS, you'll need these defined */
typedef char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef unsigned char u_int8_t;
typedef unsigned short int u_int16_t;
typedef unsigned int u_int32_t;

/***** CONSTANTS *****/
/* Board size constants */
#define ZZT_BOARD_X_SIZE 	60
#define ZZT_BOARD_Y_SIZE 	25
#define ZZT_BOARD_MAX_SIZE 	ZZT_BOARD_X_SIZE * ZZT_BOARD_Y_SIZE
/* Board title size */
#define ZZT_BOARD_TITLE_SIZE	50
/* World title size */
#define ZZT_WORLD_TITLE_SIZE	20
/* Flag size */
#define ZZT_FLAG_SIZE		20
/* How many flags */
#define ZZT_MAX_FLAGS		10
/* Message size */
#define ZZT_MESSAGE_SIZE	58
/* Which key is which in the ZZTinfo key array */
#define ZZT_KEY_BLUE		0
#define ZZT_KEY_GREEN		1
#define ZZT_KEY_CYAN		2
#define ZZT_KEY_RED		3
#define ZZT_KEY_PURPLE		4
#define ZZT_KEY_YELLOW		5
#define ZZT_KEY_WHITE		6

/* ZZT board info -- settings for a board */
typedef struct ZZTboardinfo {
	u_int8_t maxshots;	/* Max. player bullets that can be onscreen */
	u_int8_t darkness;	/* Set if dark */
	u_int8_t board_n;	/* Point to board on North border */
	u_int8_t board_s;	/* Point to board on South border */
	u_int8_t board_w;	/* Point to board on West border */
	u_int8_t board_e;	/* Point to board on East border */
	u_int8_t reenter;	/* Set for re-enter if zapped */
	u_int8_t message[ZZT_MESSAGE_SIZE+1];	/* Board message */
	u_int8_t reenter_x;	/* Reenter x-coord */
	u_int8_t reenter_y;	/* Reenter y-coord */
	u_int16_t timelimit;	/* Time limit for board */
	u_int16_t paramcount;	/* How many parameter records on board */
} ZZTboardinfo;

/* ZZT parameter record -- objects, scrolls, enemies, etc. */
/* Many fields are used for different things based on the type of enemy.
   Refer to the ZZT File Format */
typedef struct ZZTparam {
	u_int8_t x;		/* X position */
	u_int8_t y;		/* Y position */
	u_int16_t xstep;	/* X step */
	u_int16_t ystep;	/* Y step */
	u_int16_t cycle;	/* Cycle (speed) */
	u_int8_t data[3];	/* Generic data */
	u_int8_t magic1[4];	/* UNKNOWN */
	u_int8_t utype;		/* Type of tile underneath */
	u_int8_t ucolor;	/* Color of tile underneath */
	u_int8_t magic2[4];	/* ALSO UNKNOWN */
	u_int16_t instruction;	/* Index to current instruction */
	u_int16_t length;	/* Length of program */
	u_int8_t *program;	/* Program (if any) */
} ZZTparam;

/* ZZT board -- fill a ZZT world with these */
typedef struct ZZTboard {
	u_int8_t title[ZZT_BOARD_TITLE_SIZE+1];	/* Board title */
	ZZTboardinfo info;			/* Board info */
	u_int8_t *packed;			/* RLE packed board data */
	ZZTparam *params;			/* Array of parameters */
} ZZTboard;

/* ZZT world info -- stuff from the ZZT file header */
typedef struct ZZTworldinfo {
	u_int16_t boardcount;	/* Boards in world */
	u_int16_t ammo;		/* Ammo count */
	u_int16_t gems;		/* Gems count */
	u_int8_t keys[7];	/* Keys */
	u_int16_t health;	/* Health count */
	u_int16_t startboard;	/* Number of board to start on */
	u_int16_t torches;	/* Torch count */
	u_int16_t torchcycles;	/* How many cycles of torch left */
	u_int16_t energizercycles; /* How many cycles of energizer left */
	u_int8_t magic1[2];	/* UNKNOWN */
	u_int16_t score;	/* Score */
	u_int8_t title[ZZT_WORLD_TITLE_SIZE+1];	/* World title */
	u_int8_t flags[ZZT_MAX_FLAGS][ZZT_FLAG_SIZE+1];	/* 10 flags, 20 chars each */
	u_int16_t timepassed;	/* Time passed in savegame */
	u_int8_t magic2[2];	/* ALSO UNKNOWN */
	u_int8_t savegame;	/* Set if savegame */
} ZZTworldinfo;

/* World structure -- create one of these */
typedef struct ZZTworld {
	/*** HEADER ***/
	ZZTworldinfo *header;	/* World information */
	/*** BOARD ARRAY ***/
	ZZTboard *boards;	/* Array of boards */
	u_int8_t *bigboard;	/* Fully expanded board (the current one) */
	int cur_board;		/* Board we are accessing */
	char *filename;		/* Filename */
} ZZTworld;

/*****************************************************************/
/***    *****   ***   **    ***   ***   ***   ****    *****   ****/
/***  *  ***  ****  ****  ****  ****  ****  **  **  **  **  ******/
/***  **  **  ****  ****   ****  ****  ***  **  **    *****  *****/
/***      **  ****  ****  ******  ****  **  **  **  **  ****  ****/
/***  **  ***   ***   **    **   ***   ****   ****  **  **   *****/
/*****************************************************************/

/***** WORLD MANIPULATORS *****/
/* zztWorldCreate(filename, title)
 * Make a new world with the given filename and title
 * If filename is NULL, default is untitled.zzt
 * If title is NULL, default is the part before the dot in the filename
 * This does NOT create or load any files
 */
ZZTworld *zztWorldCreate(char *filename, char *title);
/* zztWorldLoad(filename)
 * Load a ZZT game from the given filename
 */
ZZTworld *zztWorldLoad(char *filename);
/* zztWorldFree(world)
 * Free all memory this world is using
 */
void zztWorldFree(ZZTworld *world);
/* zztWorldSave(world)
 * Save the ZZT world to disk
 */
int zztWorldSave(ZZTworld *world);
/* zztWorldSetXXXXX()
 * Set a world info variable
 */
void zztWorldSetBoardcount(ZZTworld *world, u_int16_t count);	/* Don't ever use */
void zztWorldSetAmmo(ZZTworld *world, u_int16_t ammo);
void zztWorldSetGems(ZZTworld *world, u_int16_t gems);
void zztWorldSetKey(ZZTworld *world, int number, u_int8_t state);
void zztWorldSetHealth(ZZTworld *world, u_int16_t health);
void zztWorldSetStartboard(ZZTworld *world, u_int16_t startboard);
void zztWorldSetTorches(ZZTworld *world, u_int16_t torches);
void zztWorldSetTorchcycles(ZZTworld *world, u_int16_t torchcycles);
void zztWorldSetEnergizercycles(ZZTworld *world, u_int16_t energizercycles);
void zztWorldSetScore(ZZTworld *world, u_int16_t score);
void zztWorldSetTitle(ZZTworld *world, char *title);
void zztWorldSetFlag(ZZTworld *world, int number, char *word);
void zztWorldSetTimepassed(ZZTworld *world, u_int16_t time);
void zztWorldSetSavegame(ZZTworld *world, u_int8_t flag);
/* zztWorldSetFilename(world, name)
 * Set the filename of the world
 */
void zztWorldSetFilename(ZZTworld *world, char *name);
/* zztWorldGetXXXXX()
 * Get a world info variable
 */
u_int16_t zztWorldGetBoardcount(ZZTworld *world);
u_int16_t zztWorldGetAmmo(ZZTworld *world);
u_int16_t zztWorldGetGems(ZZTworld *world);
u_int8_t zztWorldGetKey(ZZTworld *world, int number);
u_int16_t zztWorldGetHealth(ZZTworld *world);
u_int16_t zztWorldGetStartboard(ZZTworld *world);
u_int16_t zztWorldGetTorches(ZZTworld *world);
u_int16_t zztWorldGetTorchcycles(ZZTworld *world);
u_int16_t zztWorldGetEnergizercycles(ZZTworld *world);
u_int16_t zztWorldGetScore(ZZTworld *world);
u_int8_t *zztWorldGetTitle(ZZTworld *world);
u_int8_t *zztWorldGetFlag(ZZTworld *world, int number);
u_int16_t zztWorldGetTimepassed(ZZTworld *world);
u_int8_t zztWorldGetSavegame(ZZTworld *world);
/* zztWorldGetFilename(world)
 * Get the filename of the world
 */
char *zztWorldGetFilename(ZZTworld *world);

/* zztBoardSelect(world, number)
 * Load the given board into memory
 */
int zztBoardSelect(ZZTworld *world, int number);
/* zztBoardCommit(world)
 * Commits changes to board tiles.  Not very useful unless you want to do
 * something to the rle data yourself.  You shouldn't ever have to use this
 * in normal operation.
 */
void zztBoardCommit(ZZTworld *world);
/* zztBoardGetCurrent(world)
 * Return the number of the currently selected board
 */
int zztBoardGetCurrent(ZZTworld *world);
/***** BOARD MANIPULATORS *****/
/* zztWorldAddBoard(world, title)
 * Create a blank board at the end of the world with the given title
 */
void zztWorldAddBoard(ZZTworld *world, char *title);
/* zztWorldDeleteBoard(world, number, relink)
 * Remove a board from the world
 * Make relink non-zero to correct all the board links
 */
int zztWorldDeleteBoard(ZZTworld *world, int number, int relink);
/* zztBoardCreate(title)
 * Create a stand-alone board
 */
ZZTboard *zztBoardCreate(char *title);
/* zztBoardFree(board)
 * Delete a stand-alone board (NOT IN A WORLD!!)
 */
void zztBoardFree(ZZTboard *board);
/* zztBoardCopy(board)
 * Return a copy of an existing board
 */
ZZTboard *zztBoardCopy(ZZTboard *board);
/* zztWorldInsertBoard(world, board, position, relink)
 * Insert a stand-alone board into the given world
 * Make relink non-zero to correct all the board links
 */
int zztWorldInsertBoard(ZZTworld *world, ZZTboard *board, int number, int relink);
/* zztWorldMoveBoard(world, source, destination)
 * Move an existing board # somewhere else in the world, like zztInsertBoard
 */
int zztWorldMoveBoard(ZZTworld *world, int src, int dest);
/* zztBoardSetXXXXX()
 * Set a board info variable
 */ 
void zztBoardSetTitle(ZZTworld *world, char *title);
void zztBoardSetMaxshots(ZZTworld *world, u_int8_t maxshots);
void zztBoardSetDarkness(ZZTworld *world, u_int8_t darkness);
void zztBoardSetBoard_n(ZZTworld *world, u_int8_t board_n);
void zztBoardSetBoard_s(ZZTworld *world, u_int8_t board_s);
void zztBoardSetBoard_w(ZZTworld *world, u_int8_t board_w);
void zztBoardSetBoard_e(ZZTworld *world, u_int8_t board_e);
void zztBoardSetReenter(ZZTworld *world, u_int8_t reenter);
void zztBoardSetMessage(ZZTworld *world, char *message);
void zztBoardSetTimelimit(ZZTworld *world, u_int16_t timelimit);
void zztBoardSetParamcount(ZZTworld *world, u_int16_t paramcount);	/* Don't ever use */
/* zztBoardGetXXXXX()
 * Get a board info variable
 */ 
u_int8_t *zztBoardGetTitle(ZZTworld *world);
u_int8_t zztBoardGetMaxshots(ZZTworld *world);
u_int8_t zztBoardGetDarkness(ZZTworld *world);
u_int8_t zztBoardGetBoard_n(ZZTworld *world);
u_int8_t zztBoardGetBoard_s(ZZTworld *world);
u_int8_t zztBoardGetBoard_w(ZZTworld *world);
u_int8_t zztBoardGetBoard_e(ZZTworld *world);
u_int8_t zztBoardGetReenter(ZZTworld *world);
u_int8_t *zztBoardGetMessage(ZZTworld *world);
u_int16_t zztBoardGetTimelimit(ZZTworld *world);
u_int16_t zztBoardGetParamcount(ZZTworld *world);

/* zztWorldWrite(world, fp)
 * Write a whole world to an open file
 */
int zztWorldWrite(ZZTworld *world, FILE *fp);
/* zztBoardWrite(board, fp)
 * Write a single board to an open file
 */
int zztBoardWrite(ZZTboard *board, FILE *fp);
/* zztWorldRead(fp)
 * Read in a whole world from an open file
 */
ZZTworld *zztWorldRead(FILE *fp);
/* zztBoardRead(fp)
 * Read in a single board from an open file
 */
ZZTboard *zztBoardRead(FILE *fp);

/***** PARAMETER MANIPULATORS *****/
/* zztInsertParam(world, param)
 * Insert the given parameter into the current board
 */
int zztParamInsert(ZZTworld *world, ZZTparam *param);
/* zztDeleteParam(world, n)
 * Delete the nth parameter in the list -- not very useful
 */
int zztParamDelete(ZZTworld *world, int number);
/* zztDeleteParamAt(world, x, y)
 * Delete the parameter at the given coordinates
 */
int zztParamDeleteAt(ZZTworld *world, int x, int y);
/* zztPlot(world, x, y, type, color)
 * Plot the given type/color at the given coords.
 * NO PARAMETERS ARE CREATED
 */
int zztPlot(ZZTworld *world, int x, int y, u_int8_t type, u_int8_t color);
/* zztPlotPlayer(world, x, y)
 * Plot a player, create player param
 */
int zztPlotPlayer(ZZTworld *world, int x, int y);

/***** TILE TYPES *****/
#define ZZT_EMPTY         0x00
#define ZZT_EDGE          0x01
#define ZZT_PLAYER        0x04
#define ZZT_AMMO          0x05
#define ZZT_TORCH         0x06
#define ZZT_GEM           0x07
#define ZZT_KEY           0x08
#define ZZT_DOOR          0x09
#define ZZT_SCROLL        0x0A
#define ZZT_PASSAGE       0x0B
#define ZZT_DUPLICATOR    0x0C
#define ZZT_BOMB          0x0D
#define ZZT_ENERGIZER     0x0E
#define ZZT_STAR          0x0F
#define ZZT_CWCONV        0x10
#define ZZT_CCWCONV       0x11
#define ZZT_BULLET        0x12
#define ZZT_WATER         0x13
#define ZZT_FOREST        0x14
#define ZZT_SOLID         0x15
#define ZZT_NORMAL        0x16
#define ZZT_BREAKABLE     0x17
#define ZZT_BOULDER       0x18
#define ZZT_NSSLIDER      0x19
#define ZZT_EWSLIDER      0x1A
#define ZZT_FAKE          0x1B
#define ZZT_INVISIBLE     0x1C
#define ZZT_BLINK         0x1D
#define ZZT_TRANSPORTER   0x1E
#define ZZT_LINE          0x1F
#define ZZT_RICOCHET      0x20
#define ZZT_BLINKHORIZ    0x21
#define ZZT_BEAR          0x22
#define ZZT_RUFFIAN       0x23
#define ZZT_OBJECT        0x24
#define ZZT_SLIME         0x25
#define ZZT_SHARK         0x26
#define ZZT_SPINNINGGUN   0x27
#define ZZT_PUSHER        0x28
#define ZZT_LION          0x29
#define ZZT_TIGER         0x2A
#define ZZT_BLINKVERT     0x2B
#define ZZT_CENTHEAD      0x2C
#define ZZT_CENTBODY      0x2D
#define ZZT_BLUETEXT      0x2F
#define ZZT_GREENTEXT     0x30
#define ZZT_CYANTEXT      0x31
#define ZZT_REDTEXT       0x32
#define ZZT_PURPLETEXT    0x33
#define ZZT_YELLOWTEXT    0x34
#define ZZT_WHITETEXT     0x35
#define ZZT_BBLUETEXT     0x37
#define ZZT_BGREENTEXT    0x38
#define ZZT_BCYANTEXT     0x39
#define ZZT_BREDTEXT      0x3A
#define ZZT_BPURPLETEXT   0x3B
#define ZZT_BYELLOWTEXT   0x3C
#define ZZT_BWHITETEXT    0x3D

#endif /* __ZZT_H */
