/* tiles.c	-- All those ZZT tiles
 * $Id: tiles.c,v 1.7 2002/02/19 03:32:28 bitman Exp $
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

#include <stdlib.h>
#include <string.h>

#include "zzt.h"

/* Look-up table for tile type names */
const char * _zzt_type_name_table[] = {
	/* ZZT_EMPTY          */ "Empty",
	/* ZZT_EDGE           */ "Board Edge",
	/* Invalid            */ "Unknown",
	/* Invalid            */ "Unknown",
	/* ZZT_PLAYER         */ "Player",
	/* ZZT_AMMO           */ "Ammo",
	/* ZZT_TORCH          */ "Torch",
	/* ZZT_GEM            */ "Gem",
	/* ZZT_KEY            */ "Key",
	/* ZZT_DOOR           */ "Door",
	/* ZZT_SCROLL         */ "Scroll",
	/* ZZT_PASSAGE        */ "Passage",
	/* ZZT_DUPLICATOR     */ "Duplicator",
	/* ZZT_BOMB           */ "Bomb",
	/* ZZT_ENERGIZER      */ "Energizer",
	/* ZZT_STAR           */ "Throwstar",
	/* ZZT_CWCONV         */ "Clockwise Conveyor",
	/* ZZT_CCWCONV        */ "Counter-Clockwise Conveyor",
	/* ZZT_BULLET         */ "Bullet",
	/* ZZT_WATER          */ "Water",
	/* ZZT_FOREST         */ "Forest",
	/* ZZT_SOLID          */ "Solid Wall",
	/* ZZT_NORMAL         */ "Normal Wall",
	/* ZZT_BREAKABLE      */ "Breakable Wall",
	/* ZZT_BOULDER        */ "Boulder",
	/* ZZT_NSSLIDER       */ "North-South Slider",
	/* ZZT_EWSLIDER       */ "East-West Slider",
	/* ZZT_FAKE           */ "Fake Wall / Floor",
	/* ZZT_INVISIBLE      */ "Invisible Wall",
	/* ZZT_BLINK          */ "Blink Wall",
	/* ZZT_TRANSPORTER    */ "Transporter",
	/* ZZT_LINE           */ "Line",
	/* ZZT_RICOCHET       */ "Richochet",
	/* ZZT_BLINKHORIZ     */ "Horizontal Blink Wall",
	/* ZZT_BEAR           */ "Bear",
	/* ZZT_RUFFIAN        */ "Ruffian",
	/* ZZT_OBJECT         */ "Object",
	/* ZZT_SLIME          */ "Slime",
	/* ZZT_SHARK          */ "Shark",
	/* ZZT_SPINNINGGUN    */ "Spinning Gun",
	/* ZZT_PUSHER         */ "Pusher",
	/* ZZT_LION           */ "Lion",
	/* ZZT_TIGER          */ "Tiger",
	/* ZZT_BLINKVERT      */ "Vertical Blink Wall",
	/* ZZT_CENTHEAD       */ "Centipede Head",
	/* ZZT_CENTBODY       */ "Centipede Body",
	/* ZZT_BLUETEXT       */ "Blue Text",
	/* ZZT_GREENTEXT      */ "Green Text",
	/* ZZT_CYANTEXT       */ "Cyan Text",
	/* ZZT_REDTEXT        */ "Red Text",
	/* ZZT_PURPLETEXT     */ "Purple Text",
	/* ZZT_YELLOWTEXT     */ "Yellow Text",
	/* ZZT_WHITETEXT      */ "White Text",
	/* Invalid type       */ "Unknown",
};

/* Look-up table for converting zzt types to display chars */
const u_int8_t _zzt_display_char_table[] = {
	' ', /* ZZT_EMPTY          */
	'E', /* ZZT_EDGE           */
	'?', /* Invalid            */
	'?', /* Invalid            */
	2,   /* ZZT_PLAYER         */
	132, /* ZZT_AMMO           */
	157, /* ZZT_TORCH          */
	4,   /* ZZT_GEM            */
	12,  /* ZZT_KEY            */
	10,  /* ZZT_DOOR           */
	232, /* ZZT_SCROLL         */
	240, /* ZZT_PASSAGE        */
	250, /* ZZT_DUPLICATOR     */
	11,  /* ZZT_BOMB           */
	127, /* ZZT_ENERGIZER      */
	'*', /* ZZT_STAR           */
	179, /* ZZT_CWCONV         */
	'\\', /* ZZT_CCWCONV       */
	248, /* ZZT_BULLET         */
	176, /* ZZT_WATER          */
	176, /* ZZT_FOREST         */
	219, /* ZZT_SOLID          */
	178, /* ZZT_NORMAL         */
	177, /* ZZT_BREAKABLE      */
	254, /* ZZT_BOULDER        */
	18,  /* ZZT_NSSLIDER       */
	29,  /* ZZT_EWSLIDER       */
	178, /* ZZT_FAKE           */
	176, /* ZZT_INVISIBLE      */
	206, /* ZZT_BLINK          */
	'<', /* ZZT_TRANSPORTER    */
	250, /* ZZT_LINE           */
	'*', /* ZZT_RICOCHET       */
	205, /* ZZT_BLINKHORIZ     */
	153, /* ZZT_BEAR           */
	5,   /* ZZT_RUFFIAN        */
	2,   /* ZZT_OBJECT         */
	'*', /* ZZT_SLIME          */
	'^', /* ZZT_SHARK          */
	24,  /* ZZT_SPINNINGGUN    */
	17,  /* ZZT_PUSHER         */
	234, /* ZZT_LION           */
	227, /* ZZT_TIGER          */
	186, /* ZZT_BLINKVERT      */
	233, /* ZZT_CENTHEAD       */
	'O', /* ZZT_CENTBODY       */
	/* Text must be handled specially */
};

/* Look-up table for line characters */
const u_int8_t _zzt_display_char_line_table[] = {
	249, /*  (none) */
	208, /* n       */
	210, /*   s     */
	186, /* n s     */
	181, /*     w   */
	188, /* n   w   */
	187, /*   s w   */
	185, /* n s w   */
	198, /*       e */
	200, /* n     e */
	201, /*   s   e */
	204, /* n s   e */
	205, /*     w e */
	202, /* n   w e */
	203, /*   s w e */
	206, /* n s w e */
};

ZZTblock *zztBlockCreate(int width, int height)
{
	ZZTblock *block;
	int i;

	/* Make room for the block */
	block = (ZZTblock*) malloc(sizeof(ZZTblock));
	block->width = width;
	block->height = height;

	/* Allocate the tile space */
	block->tiles = (ZZTtile*) malloc(sizeof(ZZTtile) * width * height);
	/* Fill the tiles with blanks */
	for (i = 0; i < width * height; i++) {
		block->tiles[i].type = ZZT_EMPTY;
		block->tiles[i].color = 0x0F;
		block->tiles[i].param = NULL;
	}

	return block;
}

void zztBlockFree(ZZTblock *block)
{
	int i;

	/* Free any params in the tile list */
	for (i = 0; i < block->width * block->height; i++) {
		if (block->tiles[i].param != NULL) {
			/* Free program if present */
			if (block->tiles[i].param->program != NULL)
				free(block->tiles[i].param->program);
			free(block->tiles[i].param);
		}
	}
	/* Free the tile list and the block */
	free(block->tiles);
	free(block);
}

ZZTblock *zztBlockDuplicate(ZZTblock *block)
{
	ZZTblock *dest;
	int tilecount;
	int i;

	if (block == NULL)
		return NULL;

	dest = (ZZTblock *) malloc(sizeof(ZZTblock));
	tilecount = block->width * block->height;

	/* Copy dimensions */
	dest->width = block->width; dest->height = block->height;

	if (block->tiles == NULL)   /* Anything is possible... */
		return dest;

	/* Copy tile data */
	dest->tiles = (ZZTtile *) malloc(sizeof(ZZTtile) * tilecount);
	memcpy(dest->tiles, block->tiles, sizeof(ZZTtile) * tilecount);

	/* Copy param data */
	for (i = 0; i < tilecount; i++) {
		if (block->tiles[i].param != NULL) {
			dest->tiles[i].param = zztParamDuplicate(block->tiles[i].param);
		}
	}
	
	return dest;
}

int zztTilePlot(ZZTblock * block, int x, int y, ZZTtile tile)
{
	ZZTtile undertile;

	/* Destination must be within bounds */
	if (x < 0 || x >= block->width || y < 0 || y >= block->height)
		return 0;

	/* Remember the old tile in case we are plotting a param tile */
	undertile = zztTileAt(block, x, y);

	zztTileAt(block, x, y).type = tile.type;
	zztTileAt(block, x, y).color = tile.color;
	
	/* Remove any existing param */
	if (zztTileAt(block, x, y).param != NULL) {
		/* Remeber the under type/color in case of plotting param */
		undertile.type = undertile.param->utype;
		undertile.color = undertile.param->ucolor;
		zztParamFree(zztTileAt(block, x, y).param);
		zztTileAt(block, x, y).param = NULL;
	}
	/* Check to see if we are writing a param */
	if (tile.param != NULL) {
		zztTileAt(block, x, y).param = zztParamDuplicate(tile.param);
		/* Tell the param where it now lives */
		zztTileAt(block, x, y).param->x = x;
		zztTileAt(block, x, y).param->y = y;
		if (undertile.type == ZZT_EMPTY || undertile.type == ZZT_FAKE ||
				undertile.type == ZZT_WATER) {
			zztTileAt(block, x, y).param->utype = undertile.type;
			zztTileAt(block, x, y).param->ucolor = undertile.color;
		}
	}

	/* Success! */
	return 1;
}

int zztPlot(ZZTworld * world, int x, int y, ZZTtile tile)
{
	ZZTboard* brd = zztBoardGetCurPtr(world);

	/* Error if board cannot be decompressed */
	if (!zztBoardDecompress(brd))
		return 0;

	/* No writing over the player */
	if (x == brd->plx && y == brd->ply)
		return 0;

	/* Determine whether we are adding a param to the list */
	if (tile.param != NULL && zztTileAt(brd->bigboard, x, y).param == NULL) {
		/* No exceeding max params */
		if (zztBoardGetParamcount(world) >= ZZT_BOARD_MAX_PARAMS)
			return 0;
		zztBoardSetParamcount(world, zztBoardGetParamcount(world) + 1);
	} else if (zztTileAt(brd->bigboard, x, y).param != NULL && tile.param == NULL) {
		zztBoardSetParamcount(world, zztBoardGetParamcount(world) - 1);
	}

	zztTilePlot(brd->bigboard, x, y, tile);
	
	return 1;
}

int zztPlotPlayer(ZZTworld * world, int x, int y)
{
	ZZTboard* brd = zztBoardGetCurPtr(world);
	ZZTtile player;

	/* Simple case */
	if (x == brd->plx && y == brd->ply)
		return 1;

	/* Error if board cannot be decompressed */
	if (!zztBoardDecompress(brd))
		return 0;

	/* Pick up the player */
	player = zztTileGet(world, brd->plx, brd->ply);

	/* Plot the player to the new location -- param data is copied */
	zztTilePlot(brd->bigboard, x, y, player);

	/* Erase the plotted player, leaving terrain underneath */
	zztTileErase(brd->bigboard, brd->plx, brd->ply);

	/* Record the change */
	brd->plx = x; brd->ply = y;

	return 1;
}

int zztTileErase(ZZTblock * block, int x, int y)
{
	/* Erase a tile, bringing whatever is underneath to the top */
	ZZTtile over = zztTileAt(block, x, y);
	ZZTtile under = { ZZT_EMPTY, 0x0F, NULL };

	if (over.param != NULL) {
		under.type = over.param->utype;
		under.color = over.param->ucolor;
	}

	return zztTilePlot(block, x, y, under);
}

int zztErase(ZZTworld * world, int x, int y)
{
	ZZTboard* brd = zztBoardGetCurPtr(world);

	/* Error if board cannot be decompressed */
	if (!zztBoardDecompress(brd))
		return 0;

	/* No erasing the player */
	if (x == brd->plx && y == brd->ply)
		return 0;

	zztTileErase(brd->bigboard, x, y);
	
	return 1;
}

ZZTtile zztTileGet(ZZTworld * world, int x, int y)
{
	ZZTboard* brd = zztBoardGetCurPtr(world);
	ZZTtile empty = { ZZT_EMPTY, 0x0F, NULL };

	/* Error if board cannot be decompressed */
	if (!zztBoardDecompress(brd))
		return empty;

	if (x < 0 || x >= brd->bigboard->width || y < 0 || y >= brd->bigboard->height)
		return empty;

	return zztTileAt(brd->bigboard, x, y);
}

/* Helper function for zztTileGetDisplayChar */
u_int8_t _zzt_display_char_line(ZZTblock * block, int x, int y)
{
	u_int8_t flags;

	flags = 0;

	if (y == 0 || (zztTileAt(block, x, y - 1).type == ZZT_LINE))
		flags |= 1;
	if (y == block->height - 1 || (zztTileAt(block, x, y + 1).type == ZZT_LINE))
		flags |= 2;
	if (x == 0 || (zztTileAt(block, x - 1, y).type == ZZT_LINE))
		flags |= 4;
	if (x == block->width - 1 || (zztTileAt(block, x + 1, y).type == ZZT_LINE))
		flags |= 8;

	return _zzt_display_char_line_table[flags];
}

u_int8_t zztLoneTileGetDisplayChar(ZZTtile tile)
{
	if (tile.type > ZZT_BWHITETEXT)
		return '?';
	if (tile.type >= ZZT_BLUETEXT)
		return tile.color;

	switch (tile.type) {
		case ZZT_TRANSPORTER:
			if (tile.param == NULL) break;
			if (tile.param->xstep == 0xFFFF) return '<';
			if (tile.param->xstep == 0x0001) return '>';
			if (tile.param->ystep == 0xFFFF) return '^';
			return 'v';
		case ZZT_OBJECT:
			if (tile.param == NULL) break;
			return tile.param->data[0];
		case ZZT_PUSHER:
			if (tile.param == NULL) break;
			if (tile.param->xstep == 0xFFFF) return 17;
			if (tile.param->xstep == 0x0001) return 16;
			if (tile.param->ystep == 0xFFFF) return 30;
			return 31;
	}

	/* Use lookup table by default */
	return _zzt_display_char_table[tile.type];
}

u_int8_t zztLoneTileGetDisplayColor(ZZTtile tile)
{
	switch (tile.type) {
		case ZZT_EMPTY:  return 0x0F;
		case ZZT_EDGE:   return 0x4C;
		case ZZT_PLAYER: return 0x1F;
		case ZZT_SCROLL:
		case ZZT_DUPLICATOR:
		case ZZT_BOMB:
		case ZZT_STAR:
		case ZZT_BULLET:
		case ZZT_BLINK:
		case ZZT_TRANSPORTER:
		case ZZT_BEAR:
		case ZZT_RUFFIAN:
		case ZZT_OBJECT:
		case ZZT_SLIME:
		case ZZT_SPINNINGGUN:
		case ZZT_PUSHER:
		case ZZT_LION:
		case ZZT_TIGER:
		case ZZT_CENTHEAD:
		case ZZT_CENTBODY:
			if (tile.param == NULL || tile.param->utype == ZZT_EMPTY)
				break;
			return (tile.param->ucolor & 0xf0) + (tile.color & 0x0f);
		case ZZT_BLUETEXT:    return 0x1f;
		case ZZT_GREENTEXT:   return 0x2f;
		case ZZT_CYANTEXT:    return 0x3f;
		case ZZT_REDTEXT:     return 0x4f;
		case ZZT_PURPLETEXT:  return 0x5f;
		case ZZT_YELLOWTEXT:  return 0x6f;
		case ZZT_WHITETEXT:   return 0x0f;
		case ZZT_BBLUETEXT:   return 0x9f;
		case ZZT_BGREENTEXT:  return 0xaf;
		case ZZT_BCYANTEXT:   return 0xbf;
		case ZZT_BREDTEXT:    return 0xcf;
		case ZZT_BPURPLETEXT: return 0xdf;
		case ZZT_BYELLOWTEXT: return 0xef;
		case ZZT_BWHITETEXT:  return 0xff;
	}

	/* Return the color by default */
	return tile.color;
}

u_int8_t zztTileGetDisplayChar(ZZTblock * block, int x, int y)
{
	ZZTtile tile = zztTileAt(block, x, y);

	/* Only the line type is dependant on position */
	if (tile.type == ZZT_LINE)
		return _zzt_display_char_line(block, x, y);

	return zztLoneTileGetDisplayChar(tile);
}

u_int8_t zztTileGetDisplayColor(ZZTblock * block, int x, int y)
{
	ZZTtile tile = zztTileAt(block, x, y);

	return zztLoneTileGetDisplayColor(tile);
}

u_int8_t zztGetDisplayChar(ZZTworld * world, int x, int y) {
	ZZTboard* brd = zztBoardGetCurPtr(world);

	/* Error if board cannot be decompressed */
	if (!zztBoardDecompress(brd))
		return '?';

	return zztTileGetDisplayChar(brd->bigboard, x, y);
}

u_int8_t zztGetDisplayColor(ZZTworld * world, int x, int y) {
	ZZTboard* brd = zztBoardGetCurPtr(world);

	/* Error if board cannot be decompressed */
	if (!zztBoardDecompress(brd))
		return '?';

	return zztTileGetDisplayColor(brd->bigboard, x, y);
}

const char * zztTileGetName(ZZTtile tile)
{
	if (tile.type <= ZZT_WHITETEXT) {
		return _zzt_type_name_table[tile.type];
	}
	/* Default to string at end of table */
	return _zzt_type_name_table[ZZT_WHITETEXT + 1];
}

