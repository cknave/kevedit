/* libzzt2	-- The ZZT library that behaves like a library
 * $Id: zzt.h,v 1.3 2005/07/03 01:17:20 kvance Exp $
 * Copyright (C) 2001 Kev Vance <kvance@kvance.com>
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
 * Foundation, Inc., 59 Temple Place Suite 330; Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdint.h>

#ifndef LIBZZT2_ZZT_H
#define LIBZZT2_ZZT_H

#ifdef __cplusplus
extern "C" {
#endif


/***** CONSTANTS *****/
/* Board filesize constants */
#define ZZT_BOARD_MAX_FILESIZE	20000
/* Board size constants */
#define ZZT_BOARD_X_SIZE 	60
#define ZZT_BOARD_Y_SIZE 	25
#define ZZT_BOARD_MAX_SIZE 	ZZT_BOARD_X_SIZE * ZZT_BOARD_Y_SIZE
/* Board title size */
#define ZZT_BOARD_TITLE_SIZE	50
/* Maximum params for a board */
#define ZZT_BOARD_MAX_PARAMS 151
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

/* Properties used/not-used by a tile with param data */
#define ZZT_PROPERTY_NOPARAM 0x00
#define ZZT_PROPERTY_NONE    0x01
#define ZZT_PROPERTY_STEP    0x02
#define ZZT_PROPERTY_CYCLE   0x04
#define ZZT_PROPERTY_PROGRAM 0x08
#define ZZT_PROPERTY_LEADER  0x10

/* Uses for the data[] in a param */
#define ZZT_DATAUSE_NONE          0
#define ZZT_DATAUSE_PASSAGEDEST   1
#define ZZT_DATAUSE_DUPRATE       2
#define ZZT_DATAUSE_TIMELEFT      3
#define ZZT_DATAUSE_SENSITIVITY   4
#define ZZT_DATAUSE_INTELLIGENCE  5
#define ZZT_DATAUSE_RESTTIME      6
#define ZZT_DATAUSE_CHAR          7
#define ZZT_DATAUSE_LOCKED        8
#define ZZT_DATAUSE_SPEED         9
#define ZZT_DATAUSE_FIRERATEMODE 10
#define ZZT_DATAUSE_DEVIANCE     11
#define ZZT_DATAUSE_STARTTIME    12
#define ZZT_DATAUSE_PERIOD       13
#define ZZT_DATAUSE_OWNER        14
#define ZZT_DATAUSE_MAX          14

/* ZZT param profile -- description of tile with param data */
typedef struct ZZTprofile {
	uint8_t properties;
	int16_t cycledefault;
	int datause[3];
} ZZTprofile;

/* ZZT board info -- settings for a board */
typedef struct ZZTboardinfo {
	uint8_t maxshots;	/* Max. player bullets that can be onscreen */
	uint8_t darkness;	/* Set if dark */
	uint8_t board_n;	/* Point to board on North border */
	uint8_t board_s;	/* Point to board on South border */
	uint8_t board_w;	/* Point to board on West border */
	uint8_t board_e;	/* Point to board on East border */
	uint8_t reenter;	/* Set for re-enter if zapped */
	uint8_t message[ZZT_MESSAGE_SIZE+1];	/* Board message */
	uint8_t reenter_x;	/* Reenter x-coord */
	uint8_t reenter_y;	/* Reenter y-coord */
	int16_t timelimit;	/* Time limit for board */
	uint16_t paramcount;	/* How many parameter records on board */
} ZZTboardinfo;

/* ZZT parameter record -- objects, scrolls, enemies, etc. */
/* Many fields are used for different things based on the type of enemy.
   Refer to the ZZT File Format */
typedef struct ZZTparam {
	uint8_t index;   /* Position of param in list */
	uint8_t x;		/* X position */
	uint8_t y;		/* Y position */
	int16_t xstep;	/* X step */
	int16_t ystep;	/* Y step */
	int16_t cycle;	/* Cycle (speed) */
	uint8_t data[3];	/* Generic data */
	int16_t leaderindex;	/* Index of leader (usually for centipedes, -1 if none) */
	int16_t followerindex;	/* Index of follower (centipedes, -1 if none) */
	uint8_t utype;		/* Type of tile underneath */
	uint8_t ucolor;	/* Color of tile underneath */
	uint8_t magic[4];	/* UNKNOWN */
	uint16_t instruction;	/* Index to current instruction */
	uint16_t length;	/* Length of program */
	uint8_t *program;	/* Program (if any) */

	uint16_t bindindex;	/* Index of object bound to (zero if none) */

	/* Program hash, used for resolving ambiguities in pasting bound
	   stats. */
	uint32_t program_hash;
} ZZTparam;

/* ZZT tile info -- the basic building-block of a decompressed board */
typedef struct ZZTtile {
	uint8_t type;
	uint8_t color;
	ZZTparam * param;
} ZZTtile;

/* ZZT tile block -- usually a decompressed board */
typedef struct ZZTblock {
	ZZTtile * tiles;
	int width, height;

	/* Array of param pointers. Memory is shared with the params in tiles[] */
	ZZTparam ** params;
	int paramcount;

	/* Maximum number of params (usually ZZT_BOARD_MAX_PARAMS) */
	int maxparams;

} ZZTblock;

/* ZZT board -- fill a ZZT world with these */
typedef struct ZZTboard {
	uint8_t title[ZZT_BOARD_TITLE_SIZE+1];	/* Board title */
	ZZTboardinfo info;			/* Board info */
	uint8_t *packed;			/* RLE packed board data */
	ZZTparam *params;			/* Array of parameters */
	ZZTblock *bigboard;   /* Data & params when unpacked */

	int plx, ply;         /* Player x and y */
} ZZTboard;

/* ZZT world info -- stuff from the ZZT file header */
typedef struct ZZTworldinfo {
	uint16_t boardcount;	/* Boards in world */
	int16_t ammo;		/* Ammo count */
	int16_t gems;		/* Gems count */
	uint8_t keys[7];	/* Keys */
	int16_t health;		/* Health count */
	int16_t startboard;	/* Number of board to start on */
	int16_t torches;	/* Torch count */
	int16_t torchcycles;	/* How many cycles of torch left */
	int16_t energizercycles; /* How many cycles of energizer left */
	int16_t score;		/* Score */
	uint8_t title[ZZT_WORLD_TITLE_SIZE+1];	/* World title */
	uint8_t flags[ZZT_MAX_FLAGS][ZZT_FLAG_SIZE+1];	/* 10 flags, 20 chars each */
	int16_t timepassed;	/* Time passed in savegame */
	int16_t timepassedhsec;	/* Internal 1/100th-seconds timer for passed time */
	uint8_t savegame;	/* Set if savegame */
} ZZTworldinfo;

/* World structure -- create one of these */
typedef struct ZZTworld {
	/*** HEADER ***/
	ZZTworldinfo *header;	/* World information */
	/*** BOARD ARRAY ***/
	ZZTboard *boards;	/* Array of boards */
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
/* zztWorldFree(world)
 * Free all memory this world is using
 */
void zztWorldFree(ZZTworld *world);
/* zztWorldLoad(filename)
 * Load a ZZT game from the given filename
 */
ZZTworld *zztWorldLoad(char *filename);
/* zztWorldSave(world)
 * Save the ZZT world to disk
 */
int zztWorldSave(ZZTworld *world);
/* zztWorldGetSize(world)
 * Determine the total size of the world in bytes
 * TODO: write this function
 */
uint32_t zztWorldGetSize(ZZTworld *world);
/* zztWorldSetXXXXX()
 * Set a world info variable
 */
void zztWorldSetBoardcount(ZZTworld *world, uint16_t count);	/* Don't ever use */
void zztWorldSetAmmo(ZZTworld *world, int16_t ammo);
void zztWorldSetGems(ZZTworld *world, int16_t gems);
void zztWorldSetKey(ZZTworld *world, int number, uint8_t state);
void zztWorldSetHealth(ZZTworld *world, int16_t health);
void zztWorldSetStartboard(ZZTworld *world, int16_t startboard);
void zztWorldSetTorches(ZZTworld *world, int16_t torches);
void zztWorldSetTorchcycles(ZZTworld *world, int16_t torchcycles);
void zztWorldSetEnergizercycles(ZZTworld *world, int16_t energizercycles);
void zztWorldSetScore(ZZTworld *world, int16_t score);
void zztWorldSetTitle(ZZTworld *world, char *title);
void zztWorldSetFlag(ZZTworld *world, int number, char *word);
void zztWorldSetTimepassed(ZZTworld *world, int16_t time);
void zztWorldSetSavegame(ZZTworld *world, uint8_t flag);
/* zztWorldSetFilename(world, name)
 * Set the filename of the world
 */
void zztWorldSetFilename(ZZTworld *world, char *name);
/* zztWorldGetXXXXX()
 * Get a world info variable
 */
uint16_t zztWorldGetBoardcount(ZZTworld *world);
int16_t zztWorldGetAmmo(ZZTworld *world);
int16_t zztWorldGetGems(ZZTworld *world);
uint8_t zztWorldGetKey(ZZTworld *world, int number);
int16_t zztWorldGetHealth(ZZTworld *world);
int16_t zztWorldGetStartboard(ZZTworld *world);
int16_t zztWorldGetTorches(ZZTworld *world);
int16_t zztWorldGetTorchcycles(ZZTworld *world);
int16_t zztWorldGetEnergizercycles(ZZTworld *world);
int16_t zztWorldGetScore(ZZTworld *world);
uint8_t *zztWorldGetTitle(ZZTworld *world);
uint8_t *zztWorldGetFlag(ZZTworld *world, int number);
int16_t zztWorldGetTimepassed(ZZTworld *world);
uint8_t zztWorldGetSavegame(ZZTworld *world);
/* zztWorldGetFilename(world)
 * Get the filename of the world
 */
char *zztWorldGetFilename(ZZTworld *world);

/* zztBoardSelect(world, number)
 * Load the given board into memory
 */
int zztBoardSelect(ZZTworld *world, int number);
/* zztBoardCommit(world)
 * Compresses the current board. Not very useful at all.
 */
void zztBoardCommit(ZZTworld *world);
/* zztBoardClear(world)
 * Clear the current board
 */
int zztBoardClear(ZZTworld *world);
/* zztBoardValidateLinks(world)
 * Remove links to boards beyond the maximum for the current board
 */
int zztBoardValidateLinks(ZZTworld *world);
/* zztBoardGetCurrent(world)
 * Return the number of the currently selected board
 */
int zztBoardGetCurrent(ZZTworld *world);
/* zztBoardGetCurPtr(world)
 * Return the currently selected board itself -- be careful
 */
ZZTboard * zztBoardGetCurPtr(ZZTworld *world);
/* zztBoardGetBlock(world)
 * Return the block of tiles for current board */
ZZTblock * zztBoardGetBlock(ZZTworld *world);

/***** BOARD MANIPULATORS *****/
/* zztBoardCreate(title)
 * Create a stand-alone board
 */
ZZTboard *zztBoardCreate(char *title);
/* zztBoardFree(board)
 * Delete a stand-alone board (NOT IN A WORLD!!)
 */
void zztBoardFree(ZZTboard *board);
/* zztBoardLoad(filename)
 * Load a ZZT board from given filename
 */
ZZTboard *zztBoardLoad(char *filename);
/* zztBoardSave(board)
 * Save the ZZT board to given filename
 */
int zztBoardSave(ZZTboard *board, char *filename);
/* zztBoardCopy(board)
 * Return a copy of an existing board
 */
ZZTboard *zztBoardCopy(ZZTboard *board);
/* zztBoardDecompress()
 * Switch board to decompressed (bigboard) form (do not call manually)
 */
int zztBoardDecompress(ZZTboard *board);
/* zztBoardCompress(board)
 * Switch board to compressed (rle) form (do not call manually)
 */
int zztBoardCompress(ZZTboard *board);
/* zztBoardGetSize(board)
 * Determine the size of a board in bytes
 */
uint32_t zztBoardGetSize(ZZTboard *board);
/* zztWorldAddBoard(world, title)
 * Create a blank board at the end of the world with the given title
 */
void zztWorldAddBoard(ZZTworld *world, char *title);
/* zztWorldDeleteBoard(world, number, relink)
 * Remove a board from the world
 * Make relink non-zero to correct all the board links
 */
int zztWorldDeleteBoard(ZZTworld *world, int number, int relink);
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
void zztBoardSetMaxshots(ZZTworld *world, uint8_t maxshots);
void zztBoardSetDarkness(ZZTworld *world, uint8_t darkness);
void zztBoardSetBoard_n(ZZTworld *world, uint8_t board_n);
void zztBoardSetBoard_s(ZZTworld *world, uint8_t board_s);
void zztBoardSetBoard_w(ZZTworld *world, uint8_t board_w);
void zztBoardSetBoard_e(ZZTworld *world, uint8_t board_e);
void zztBoardSetReenter(ZZTworld *world, uint8_t reenter);
void zztBoardSetReenter_x(ZZTworld *world, uint8_t reenter_x);
void zztBoardSetReenter_y(ZZTworld *world, uint8_t reenter_y);
void zztBoardSetMessage(ZZTworld *world, char *message);
void zztBoardSetTimelimit(ZZTworld *world, int16_t timelimit);
void zztBoardSetParamcount(ZZTworld *world, uint16_t paramcount);	/* Don't ever use */
/* zztBoardGetXXXXX()
 * Get a board info variable
 */ 
uint8_t *zztBoardGetTitle(ZZTworld *world);
uint8_t zztBoardGetMaxshots(ZZTworld *world);
uint8_t zztBoardGetDarkness(ZZTworld *world);
uint8_t zztBoardGetBoard_n(ZZTworld *world);
uint8_t zztBoardGetBoard_s(ZZTworld *world);
uint8_t zztBoardGetBoard_w(ZZTworld *world);
uint8_t zztBoardGetBoard_e(ZZTworld *world);
uint8_t zztBoardGetReenter(ZZTworld *world);
uint8_t zztBoardGetReenter_x(ZZTworld *world);
uint8_t zztBoardGetReenter_y(ZZTworld *world);
uint8_t *zztBoardGetMessage(ZZTworld *world);
int16_t zztBoardGetTimelimit(ZZTworld *world);
uint16_t zztBoardGetParamcount(ZZTworld *world);

/***** FILE I/O ******/
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

/***** BLOCK MANIPULATORS ******/
/* zztBlockCreate(width, height)
 * Create an empty block of tiles
 */
ZZTblock *zztBlockCreate(int width, int height);
/* zztBlockFree(block)
 * Destroy a zztBlock
 */
void zztBlockFree(ZZTblock *block);
/* zztBlockDuplicate(block)
 * Create a copy of a block
 */
ZZTblock *zztBlockDuplicate(ZZTblock *block);
/* zztBlockCopyArea(src, x1, y1, x2, y2)
 * Create a copy of a rectangular area
 */
ZZTblock *zztBlockCopyArea(ZZTblock *src, int x1, int y1, int x2, int y2);
/* zztBlockPaste(dest, src, x, y)
 * Paste the src block onto dest at (x, y)
 * Clipping occurs if src is too big
 */
int zztBlockPaste(ZZTblock *dest, ZZTblock *src, int x, int y);

/***** PARAMETER MANIPULATORS AND CHECKS *****/
/* zztParamRehash()
 * update ZZTparam program hash. */
void zztParamRehash(ZZTparam * param);

/* zztParamEqualProgram(param, param)
 * returns 1 if the params have the same code,
 * 0 otherwise.
 */
int zztParamEqualProgram(const ZZTparam * p1, const ZZTparam * p2);

/* zztParamCreateBlank()
 * Create a blank, typeless param. Use only in advanced situations!
 */
ZZTparam *zztParamCreateBlank(void);
/* zztParamCreate(tile)
 * Create a param structure for the given tile
 */
ZZTparam *zztParamCreate(ZZTtile tile);
/* zztParamFree(param)
 * Free()s a param
 */
int zztParamFree(ZZTparam *param);
/* zztParamDuplicate(param)
 * Create a duplicate of a param w/o any shared memory
 */
ZZTparam *zztParamDuplicate(ZZTparam *param);
/* zztParamGetProperties(tile)
 * Get the parameter properties for the given tile
 */
uint8_t zztParamGetProperties(ZZTtile tile);
/* zztParamDatauseGet(tile, which)
 * Determine the use for data[which] in tile's param
 */
int zztParamDatauseGet(ZZTtile tile, int which);
/* zztParamDatauseGetName(tile, which)
 * Get a description of the data represented by data[which]
 */
const char *zztParamDatauseGetName(ZZTtile tile, int which);
/* zztParamDatauseLocate(datause)
 * Determine which data[] element a given datause represents
 */
int zztParamDatauseLocate(int datause);
/* zztParamGetProperty(param, property)
 * Retrieve a given property (datause) for a param
 */
int zztParamGetProperty(ZZTparam * param, int property);

/***** TILE MANIPULATORS ******/
/* zztTileSet(block, x, y, tile)
 * Set the tile at (x, y) for the given block
 * No protection against overwriting the player
 */
int zztTileSet(ZZTblock * block, int x, int y, ZZTtile tile);
/* zztTilePlot(block, x, y, tile)
 * Plot a tile to the given block at (x, y)
 * Tiles underneath other tiles are handled appropriately
 * No protection against overwriting the player
 */
int zztTilePlot(ZZTblock * block, int x, int y, ZZTtile tile);
/* zztPlot(world, x, y, tile)
 * Plot a tile to world's current board
 * The player will not be overwritten and param count is limited
 */
int zztPlot(ZZTworld * world, int x, int y, ZZTtile tile);
/* zztPlotPlayer(world, x, y)
 * Plot the player
 * This must be seperate from zztPlot to allow player clones
 */
int zztPlotPlayer(ZZTworld * world, int x, int y);
/* zztTileMove(block, fromx, fromy, tox, toy)
 * Move a tile from one place to another
 * Tile params will remain in the same order
 * No protection against overwriting/moving the player
 */
int zztTileMove(ZZTblock * block, int fromx, int fromy, int tox, int toy);
/* zztMove(block, fromx, fromy, tox, toy)
 * Move a tile from one place to another
 * Tile params will remain in the same order
 * This function is PlayerSafe (TM)
 */
int zztMove(ZZTworld * world, int fromx, int fromy, int tox, int toy);
/* zztTileErase(block, x, y)
 * Erase the tile at (x, y)
 * If tile has terrain underneath, the terrain remains
 * Plot an empty instead to completely erase a location
 */
int zztTileErase(ZZTblock * block, int x, int y);
/* zztErase(world, x, y)
 * Erase the tile at (x, y), protecting the player
 */
int zztErase(ZZTworld * world, int x, int y);
/* zztTileAt(block, x, y)
 * Macro to find the tile at a given coordinate in a block
 */
#define zztTileAt(block, x, y) (block->tiles[(block->width)*(y) + (x)])
/* zztTileGet(world, x, y)
 * Gets the tile at (x, y), but DOES NOT COPY PARAM DATA
 */
ZZTtile zztTileGet(ZZTworld * world, int x, int y);
/* zztLoneTileGetDisplayChar(tile)
 * zztTileGetDisplayChar(block, x, y)
 * zztGetDisplayChar(world, x, y)
 * Gets the display character of the tile at (x, y)
 */
uint8_t zztLoneTileGetDisplayChar(ZZTtile tile);
uint8_t zztTileGetDisplayChar(ZZTblock * block, int x, int y);
uint8_t zztGetDisplayChar(ZZTworld * world, int x, int y);
/* zztLoneTileGetDisplayChar(tile)
 * zztTileGetDisplayColor(block, x, y)
 * zztGetDisplayColor(world, x, y)
 * Gets the display colour of the tile at (x, y)
 */
uint8_t zztLoneTileGetDisplayColor(ZZTtile tile);
uint8_t zztTileGetDisplayColor(ZZTblock * block, int x, int y);
uint8_t zztGetDisplayColor(ZZTworld * world, int x, int y);
/* zztTileGetName(tile)
 * Returns a descriptive name for a tile (do not modify!)
 */
const char * zztTileGetName(ZZTtile tile);
/* zztTileGetKind(tile)
 * Returns the kind name used in object code (do not modify!)
 */
const char * zztTileGetKind(ZZTtile tile);

/* zztTileIsText: returns true if the tile is text, false otherwise
 * WARNING: Does not consider unsupported blinking text to be text! */
int zztTileIsText(ZZTtile tile);

/* Make the tile type name and kind tables visible (don't modify!) */
extern const char * _zzt_type_name_table[];
extern const char * _zzt_type_kind_table[];

/***** TILE TYPES *****/
#define ZZT_EMPTY         0x00
#define ZZT_EDGE          0x01
#define ZZT_MESSAGETIMER  0x02
#define ZZT_MONITOR       0x03
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

#define ZZT_MAX_TYPE      0x35

/* Blinking text, usage may lead to undefined behavior in ZZT. */
#define ZZT_BBLUETEXT     0x37
#define ZZT_BGREENTEXT    0x38
#define ZZT_BCYANTEXT     0x39
#define ZZT_BREDTEXT      0x3A
#define ZZT_BPURPLETEXT   0x3B
#define ZZT_BYELLOWTEXT   0x3C
#define ZZT_BWHITETEXT    0x3D

#ifdef __cplusplus
}
#endif

#endif /* LIBZZT2_ZZT_H */
