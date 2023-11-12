/* tiles.c	-- All those ZZT tiles
 * $Id: tiles.c,v 1.5 2005/06/29 03:20:34 kvance Exp $
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "zzt.h"

/* The all-powerful min/max/swap macros */
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define swap(a, b, type) { type c = (a); (a) = (b); (b) = c; }

/* Look-up table for tile type names */
const char * _zzt_type_name_table[] = {
	/* ZZT_EMPTY          */ "Empty",
	/* ZZT_EDGE           */ "Board Edge",
	/* ZZT_MESSAGETIMER   */ "Message Timer",
	/* ZZT_MONITOR        */ "Monitor",
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
	/* ZZT_RICOCHET       */ "Ricochet",
	/* ZZT_BLINKHORIZ     */ "Horizontal Blink Wall Ray",
	/* ZZT_BEAR           */ "Bear",
	/* ZZT_RUFFIAN        */ "Ruffian",
	/* ZZT_OBJECT         */ "Object",
	/* ZZT_SLIME          */ "Slime",
	/* ZZT_SHARK          */ "Shark",
	/* ZZT_SPINNINGGUN    */ "Spinning Gun",
	/* ZZT_PUSHER         */ "Pusher",
	/* ZZT_LION           */ "Lion",
	/* ZZT_TIGER          */ "Tiger",
	/* ZZT_BLINKVERT      */ "Vertical Blink Wall Ray",
	/* ZZT_CENTHEAD       */ "Centipede Head",
	/* ZZT_CENTBODY       */ "Centipede Body",
	/* Invalid            */ "Unknown",
	/* ZZT_BLUETEXT       */ "Blue Text",
	/* ZZT_GREENTEXT      */ "Green Text",
	/* ZZT_CYANTEXT       */ "Cyan Text",
	/* ZZT_REDTEXT        */ "Red Text",
	/* ZZT_PURPLETEXT     */ "Purple Text",
	/* ZZT_YELLOWTEXT     */ "Yellow Text",
	/* ZZT_WHITETEXT      */ "White Text",
	/* Invalid type       */ "Unknown"
};

const char * _zzt_type_kind_table[] = {
	/* ZZT_EMPTY          */ "empty",
	/* ZZT_EDGE           */ "",
	/* Invalid            */ "(message timer)",
	/* ZZT_MONITOR        */ "monitor",
	/* ZZT_PLAYER         */ "player",
	/* ZZT_AMMO           */ "ammo",
	/* ZZT_TORCH          */ "torch",
	/* ZZT_GEM            */ "gem",
	/* ZZT_KEY            */ "key",
	/* ZZT_DOOR           */ "door",
	/* ZZT_SCROLL         */ "scroll",
	/* ZZT_PASSAGE        */ "passage",
	/* ZZT_DUPLICATOR     */ "duplicator",
	/* ZZT_BOMB           */ "bomb",
	/* ZZT_ENERGIZER      */ "energizer",
	/* ZZT_STAR           */ "star",
	/* ZZT_CWCONV         */ "clockwise",
	/* ZZT_CCWCONV        */ "counter",
	/* ZZT_BULLET         */ "bullet",
	/* ZZT_WATER          */ "water",
	/* ZZT_FOREST         */ "forest",
	/* ZZT_SOLID          */ "solid",
	/* ZZT_NORMAL         */ "normal",
	/* ZZT_BREAKABLE      */ "breakable",
	/* ZZT_BOULDER        */ "boulder",
	/* ZZT_NSSLIDER       */ "sliderns",
	/* ZZT_EWSLIDER       */ "sliderew",
	/* ZZT_FAKE           */ "fake",
	/* ZZT_INVISIBLE      */ "invisible",
	/* ZZT_BLINK          */ "blinkwall",
	/* ZZT_TRANSPORTER    */ "transporter",
	/* ZZT_LINE           */ "line",
	/* ZZT_RICOCHET       */ "ricochet",
	/* ZZT_BLINKHORIZ     */ "(horizontal blink wall ray)",
	/* ZZT_BEAR           */ "bear",
	/* ZZT_RUFFIAN        */ "ruffian",
	/* ZZT_OBJECT         */ "object",
	/* ZZT_SLIME          */ "slime",
	/* ZZT_SHARK          */ "shark",
	/* ZZT_SPINNINGGUN    */ "spinninggun",
	/* ZZT_PUSHER         */ "pusher",
	/* ZZT_LION           */ "lion",
	/* ZZT_TIGER          */ "tiger",
	/* ZZT_BLINKVERT      */ "(vertical blink wall ray)",
	/* ZZT_CENTHEAD       */ "head",
	/* ZZT_CENTBODY       */ "segment",
	/* Invalid            */ "(unknown)",
	/* ZZT_BLUETEXT       */ "(blue text)",
	/* ZZT_GREENTEXT      */ "(green text)",
	/* ZZT_CYANTEXT       */ "(cyan text)",
	/* ZZT_REDTEXT        */ "(red text)",
	/* ZZT_PURPLETEXT     */ "(purple text)",
	/* ZZT_YELLOWTEXT     */ "(yellow text)",
	/* ZZT_WHITETEXT      */ "(white text)",
	/* Invalid type       */ "(unknown)"
};

/* Look-up table for converting zzt types to display chars */
const uint8_t _zzt_display_char_table[] = {
	' ', /* ZZT_EMPTY          */
	'E', /* ZZT_EDGE           */
	'T', /* ZZT_MESSAGETIMER   */
	'M', /* ZZT_MONITOR        */
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
	'/', /* ZZT_STAR           */
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
	0x1F, /* ZZT_PUSHER         */
	234, /* ZZT_LION           */
	227, /* ZZT_TIGER          */
	186, /* ZZT_BLINKVERT      */
	233, /* ZZT_CENTHEAD       */
	'O', /* ZZT_CENTBODY       */
	'?', /* Unknown            */
	/* Text must be handled specially */
};

/* Look-up table for line characters */
const uint8_t _zzt_display_char_line_table[] = {
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

/* Relink a param: references greater than start will be decreased by one,
 * references equal to start will be reset to their default value. */
void _zzt_relink_param(ZZTparam * param, int start)
{
	if (param->leaderindex > start && param->leaderindex != -1)
		param->leaderindex--;
	else if (param->leaderindex == start)
		param->leaderindex = -1;

	if (param->followerindex > start && param->followerindex != -1)
		param->followerindex--;
	else if (param->followerindex == start)
		param->followerindex = -1;

	if (param->bindindex > start)
		param->bindindex--;
	else if (param->bindindex == start)
		param->bindindex = 0;
}

/* Remove a param from the a block's list of params */
void _zzt_block_remove_param_from_list(ZZTblock * block, int index)
{
	int i;

	/* No invalid indexes allowed */
	if (index < 0)
		return;

	for (i = index + 1; i < block->paramcount; i++) {
		block->params[i - 1] = block->params[i];
		block->params[i - 1]->index = i - 1;
	}

	/* Number of params has decreased */
	block->paramcount--;

	/* Relink all references to the removed params */
	for (i = 0; i < block->paramcount; i++) {
		_zzt_relink_param(block->params[i], index);
	}
}

/* Determine type/color to use as undertile for a tile */
ZZTtile _zzt_get_undertile(ZZTtile tile)
{
	ZZTtile undertile = { ZZT_EMPTY, 0x0F, NULL };

	if (tile.param != NULL) {
		/* Steal the under type/color from the given tile */
		undertile.type = tile.param->utype;
		undertile.color = tile.param->ucolor;
	} else {
		if (tile.type == ZZT_EMPTY || tile.type == ZZT_FAKE ||
				tile.type == ZZT_WATER) {
			/* Steal the type and color from the given tile itself */
			undertile = tile;
		}
	}

	return undertile;
}

ZZTblock *zztBlockCreate(int width, int height)
{
	ZZTblock *block;
	int i;

	/* Make room for the block */
	block = (ZZTblock*) malloc(sizeof(ZZTblock));
	block->width = width;
	block->height = height;

	/* Use max params for a board by default + 1 for the player */
	block->maxparams = ZZT_BOARD_MAX_PARAMS + 1;

	/* Allocate the tile space */
	block->tiles = (ZZTtile*) malloc(sizeof(ZZTtile) * width * height);
	/* Fill the tiles with blanks */
	for (i = 0; i < width * height; i++) {
		block->tiles[i].type = ZZT_EMPTY;
		block->tiles[i].color = 0x0F;
		block->tiles[i].param = NULL;
	}

	/* No params just yet */
	block->paramcount = 0;
	block->params = NULL;

	return block;
}

void zztBlockFree(ZZTblock *block)
{
	int i;

	/* Free all params */
	for (i = 0; i < block->paramcount; i++) {
		if (block->params[i] != NULL) {
			zztParamFree(block->params[i]);
		}
	}

	/* Free the params list */
	if (block->params != NULL)
		free(block->params);

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
	dest->paramcount = block->paramcount;
	dest->maxparams  = block->maxparams;

	if (block->tiles == NULL)   /* Anything is possible... */
		return dest;

	/* Copy tile data */
	dest->tiles = (ZZTtile *) malloc(sizeof(ZZTtile) * tilecount);
	memcpy(dest->tiles, block->tiles, sizeof(ZZTtile) * tilecount);

	/* Copy param data */
	dest->params = (ZZTparam **) malloc(sizeof(ZZTparam*) * dest->paramcount);

	for (i = 0; i < block->paramcount; i++) {
		if (block->params[i] != NULL) {
			int x, y;
			dest->params[i] = zztParamDuplicate(block->params[i]);
			x = dest->params[i]->x;
			y = dest->params[i]->y;

			/* Update the tile for this param */
			if (x < block->width && y < block->height &&
					x >= 0           && y >= 0)
				zztTileAt(dest, x, y).param = dest->params[i];
		} else {
			dest->params[i] = NULL;
		}
	}
	
	return dest;
}

ZZTblock *zztBlockCopyArea(ZZTblock *src, int x1, int y1, int x2, int y2)
{
	int row;          /* Current row in source */
	int destx;        /* Current column in the destination block */
	ZZTblock * dest;  /* Destination block */

	/* Make sure (x1, y1) is upper left corner and (x2, y2) is lower right */
	if (x1 > x2) swap(x1, x2, int);
	if (y1 > y2) swap(y1, y2, int);

	/* Make sure coords are within range */
	if (x1 >= src->width)  x1 = src->width  - 1;
	if (x2 >= src->width)  x2 = src->width  - 1;
	if (y1 >= src->height) y1 = src->height - 1;
	if (y2 >= src->height) y2 = src->height - 1;

	/* Create the destination block */
	/* Endpoints are included in the copy */
	dest = zztBlockCreate(x2 - x1 + 1, y2 - y1 + 1);

	if (dest == NULL)
		return NULL;

	/* Copy */

	for (row = y1; row <= y2; row++) {
		/* Find start and end indexes within src */
		int srcpos = row * src->width + x1;
		int endpos = row * src->width + x2;
		for (destx = 0; srcpos <= endpos && destx < dest->width; srcpos++, destx++) {
			zztTileSet(dest, destx, row - y1, src->tiles[srcpos]);
		}
	}

	return dest;
}

int zztBlockPaste(ZZTblock *dest, ZZTblock *src,
#ifdef SELECTION
									selection destsel, selection srcsel,
#endif
									int x, int y)
{
	int srcpos;     /* Current index in source */
	int row, col;   /* Current row and col in dest */

	/* Paste */

	srcpos = 0;     /* Start at beginning of source object */
	for (row = y; row < src->height + y && row < dest->height; row++) {
		for (col = x; col < src->width + x && col < dest->width; col++, srcpos++) {
			/* Paste the currently indexed tile from source to (row, col) in dest */

			if (row < 0 || col < 0)
				continue;

#ifdef SELECTION
			/* Only copy selected tiles */
			if (!isselected(destsel, col, row) || !isselected(srcsel, col - x, row - y))
				continue;
#endif

			/* Can't use plot because we want to maintain terrain under creatures
			 * from the source block, not the destination block */
			zztTileSet(dest, col, row, src->tiles[srcpos]);
		}
		/* If the loop stopped short of using every column in src, advance
		 * the srcpos index to ignore these columns */
		srcpos += (src->width + x) - col;
	}

	/* Success! */
	return 1;
}

int zztTileSet(ZZTblock * block, int x, int y, ZZTtile tile)
{
	/* Param for the tile being overwritten */
	ZZTparam * oldparam = zztTileAt(block, x, y).param;

	/* Do not continue if we will exceed maxparams */
	if (block->paramcount >= block->maxparams &&
			tile.param != NULL && oldparam == NULL)
		return 0;  /* Fail */

	/* Copy type and color */
	zztTileAt(block, x, y).type = tile.type;
	zztTileAt(block, x, y).color = tile.color;

	/* If we are given the same param that's already there, we are done. */
	if (tile.param == oldparam)
		return 1;  /* Success */

	/* Duplicate the param we are given so that we have our own copy. */
	tile.param = zztParamDuplicate(tile.param);
	
	/* Plot the new param (NULL or not) */
	zztTileAt(block, x, y).param = tile.param;

	/* If we are plotting a paramed tile */
	if (tile.param != NULL) {

		/* Tell the param where it now lives */
		tile.param->x = x;
		tile.param->y = y;

		/* Update the param list */
		if (oldparam != NULL) {
			/* Put the new param where the old one used to be */
			tile.param->index = oldparam->index;
			block->params[tile.param->index] = tile.param;
		} else {
			/* Add the param to the end of the list */
			tile.param->index = block->paramcount++;
			block->params = (ZZTparam **) realloc(block->params, sizeof(ZZTparam*) * block->paramcount);
			block->params[block->paramcount - 1] = tile.param;
		}
	} else {
		if (oldparam != NULL) {
			/* Remove the old param from the param list */
			_zzt_block_remove_param_from_list(block, oldparam->index);
		}
	}

	/* Remove the old param since we don't need it any more. */
	if (oldparam != NULL) {
		zztParamFree(oldparam);
	}

	return 1;  /* Success */
}

int zztTilePlot(ZZTblock * block, int x, int y, ZZTtile tile)
{
	/* Destination must be within bounds */
	if (x < 0 || x >= block->width || y < 0 || y >= block->height)
		return 0;

	if (tile.param != NULL) {
		int success;
		ZZTtile undertile = _zzt_get_undertile(zztTileAt(block, x, y));

		/* Make a copy of a param, change the under info, and plot that */
		tile.param = zztParamDuplicate(tile.param);
		tile.param->utype  = undertile.type;
		tile.param->ucolor = undertile.color;

		success = zztTileSet(block, x, y, tile);

		zztParamFree(tile.param);
		return success;
	}

	return zztTileSet(block, x, y, tile);
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

	/* Plot the tile */
	zztTilePlot(brd->bigboard, x, y, tile);

	/* Update the paramcount for the board just to be nice. */
	zztBoardSetParamcount(world, brd->bigboard->paramcount);
	
	return 1;
}

int zztPlotPlayer(ZZTworld * world, int x, int y)
{
	ZZTboard* brd = zztBoardGetCurPtr(world);

	/* Error if board cannot be decompressed */
	if (!zztBoardDecompress(brd))
		return 0;

	if (x != brd->plx || y != brd->ply)
		zztTileMove(brd->bigboard, brd->plx, brd->ply, x, y);

	/* Record the change */
	brd->plx = x; brd->ply = y;
	zztTileAt(brd->bigboard, x, y).type = ZZT_PLAYER;

	return 1;
}

int zztTileMove(ZZTblock * block, int fromx, int fromy, int tox, int toy)
{
	ZZTtile from, to, underfrom, underto;

	if (fromx > block->width || fromy > block->height ||
			tox   > block->width || toy   > block->height)
		return 0;  /* Fail */

	/* Grab from and to tiles */
	from = zztTileAt(block, fromx, fromy);
	to   = zztTileAt(block, tox,   toy);

	/* Grab undertiles */
	underfrom = _zzt_get_undertile(from);
	underto   = _zzt_get_undertile(to);

	/* Remove any params we may be overwriting */
	zztTileErase(block, tox, toy);

	/* Move from to its new home */
	zztTileAt(block, tox, toy) = from;

	/* Update from's params */
	if (from.param != NULL) {
		from.param->utype  = underto.type;
		from.param->ucolor = underto.color;

		from.param->x = tox;
		from.param->y = toy;
	}
	
	/* Replace from with its undertile */
	zztTileAt(block, fromx, fromy) = underfrom;

	return 1;
}

int zztMove(ZZTworld * world, int fromx, int fromy, int tox, int toy)
{
	ZZTboard * brd = zztBoardGetCurPtr(world);

	/* Can't move onto the player */
	if (tox == brd->plx && toy == brd->ply)
		return 0;

	if (fromx == brd->plx && fromy == brd->ply) {
		/* Player is being moved -- use zztPlotPlayer() */
		return zztPlotPlayer(world, tox, toy);
	}

	return zztTileMove(brd->bigboard, fromx, fromy, tox, toy);
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

	/* Reduce the param count if we are removing a tile */
	if (zztTileAt(brd->bigboard, x, y).param != NULL)
		zztBoardSetParamcount(world, zztBoardGetParamcount(world) - 1);

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
uint8_t _zzt_display_char_line(ZZTblock * block, int x, int y)
{
	uint8_t flags;

	flags = 0;

	uint8_t neighbortype;

	neighbortype = zztTileAt(block, x, y - 1).type;
	if (y == 0 || neighbortype == ZZT_LINE || neighbortype == ZZT_EDGE)
		flags |= 1;
           
	neighbortype = zztTileAt(block, x, y + 1).type;
	if (y == block->height - 1 || neighbortype == ZZT_LINE || neighbortype == ZZT_EDGE)
		flags |= 2;
           
	neighbortype = zztTileAt(block, x - 1, y).type;
	if (x == 0 || neighbortype == ZZT_LINE || neighbortype == ZZT_EDGE)
		flags |= 4;
           
	neighbortype = zztTileAt(block, x + 1, y).type;
	if (x == block->width - 1 || neighbortype == ZZT_LINE || neighbortype == ZZT_EDGE)
		flags |= 8;

	return _zzt_display_char_line_table[flags];
}

uint8_t zztLoneTileGetDisplayChar(ZZTtile tile)
{
	if (tile.type > ZZT_BWHITETEXT)
		return '?';
	if (tile.type >= ZZT_BLUETEXT)
		return tile.color;

	switch (tile.type) {
		case ZZT_TRANSPORTER:
			if (tile.param == NULL) break;
			if (tile.param->xstep == -1) return '<';
			if (tile.param->xstep == 1) return '>';
			if (tile.param->ystep == -1) return '^';
			return 'v';
		case ZZT_OBJECT:
			if (tile.param == NULL) break;
			return tile.param->data[0];
		case ZZT_PUSHER:
			if (tile.param == NULL) break;
			if (tile.param->xstep == -1) return 17;
			if (tile.param->xstep == 1) return 16;
			if (tile.param->ystep == -1) return 30;
			return 31;
	}

	/* Use lookup table by default */
	return _zzt_display_char_table[tile.type];
}

uint8_t zztLoneTileGetDisplayColor(ZZTtile tile)
{
	switch (tile.type) {
		case ZZT_EMPTY:  return 0x0F;
		case ZZT_PLAYER: return tile.param != NULL ? 0x1F : tile.color;
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

uint8_t zztTileGetDisplayChar(ZZTblock * block, int x, int y)
{
	ZZTtile tile = zztTileAt(block, x, y);

	/* Only the line type is dependant on position */
	if (tile.type == ZZT_LINE)
		return _zzt_display_char_line(block, x, y);

	return zztLoneTileGetDisplayChar(tile);
}

uint8_t zztTileGetDisplayColor(ZZTblock * block, int x, int y)
{
	ZZTtile tile = zztTileAt(block, x, y);

	return zztLoneTileGetDisplayColor(tile);
}

uint8_t zztGetDisplayChar(ZZTworld * world, int x, int y) {
	ZZTboard* brd = zztBoardGetCurPtr(world);

	/* Error if board cannot be decompressed */
	if (!zztBoardDecompress(brd))
		return '?';

	/* TODO: add options to control display behavior (also for color):
	 *   * Show char/color underneath
	 *   * Make objects stand out
	 *   * Empty character
	 *   * Invisible wall character
	 *   * Board edge
	 */

	return zztTileGetDisplayChar(brd->bigboard, x, y);
}

uint8_t zztGetDisplayColor(ZZTworld * world, int x, int y) {
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

const char * zztTileGetKind(ZZTtile tile)
{
	if (tile.type <= ZZT_WHITETEXT) {
		return _zzt_type_kind_table[tile.type];
	}
	/* Default to string at end of table */
	return _zzt_type_kind_table[ZZT_WHITETEXT + 1];
}

int zztTileIsText(ZZTtile tile)
{
	return tile.type >= ZZT_BLUETEXT && tile.type <= ZZT_WHITETEXT;
}