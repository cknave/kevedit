/* board.c	-- Board functions
 * $Id: board.c,v 1.3 2005/06/29 03:20:34 kvance Exp $
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

/* zztParamCopyPtr(dest, src)
 * Copies param onto pre-reserved memory
 * Found in params.c
 */
int zztParamCopyPtr(ZZTparam *dest, ZZTparam *src);

/* Helper functions native to board.c */

int _zzt_rle_decode(uint8_t *packed, ZZTblock *block)
{
	int ofs = 0, count = 0;
	int maxcount = block->width * block->height;
	uint8_t i;

	/* NOTE: we assume that the decompressed rle string is not smaller than the
	 * size of the block. A larger rle string will safely generate an error */

	do {
		for(i = 0; i < packed[ofs]; i++) {
			if (count >= maxcount)   /* Do not exceed the block size */
				return 0;
			block->tiles[count].type = packed[ofs+1];
			block->tiles[count++].color = packed[ofs+2];
		}
		ofs += 3;
	} while(count < maxcount);
	return 1;
}

int _zzt_rle_encoded_size(ZZTblock *block)
{
	int size = 0, ofs = 0;
	int maxcount = block->width * block->height;
	uint8_t blocks;
	uint8_t type;
	uint8_t color;

	/* Get size of RLE data */
	do {
		/* Start with one block */
		blocks = 1;
		type = block->tiles[ofs].type;
		color = block->tiles[ofs++].color;

		while(ofs < maxcount && type == block->tiles[ofs].type && color == block->tiles[ofs].color && blocks < 255) {
			blocks++;
			ofs++;
		}
		size++;
	} while(ofs < maxcount);

	return size;
}

uint8_t *_zzt_rle_encode(ZZTblock *block)
{
	int size = 0, ofs = 0, ofs2 = 0;
	int maxcount = block->width * block->height;
	uint8_t blocks = 1;
	uint8_t type;
	uint8_t color;

	uint8_t *packed;

	/* NOTE: the compressed string will only represent as many tiles as
	 * are in the given block. If the block is not ZZT size, the encoded
	 * string will not be ZZT size either. */

	size = _zzt_rle_encoded_size(block);

	/* Reallocate data */
	packed = malloc(size*3);
	/* RLE encode from unpacked */
	ofs = ofs2 = 0;
	do {
		blocks = 1;
		type = block->tiles[ofs].type;
		color = block->tiles[ofs++].color;
		if(ofs < maxcount) {
			while(type == block->tiles[ofs].type && color == block->tiles[ofs].color && blocks < 255) {
				blocks++;
				ofs++;
				if(ofs >= maxcount)
					break;
			}
		}
		packed[ofs2++] = blocks;
		packed[ofs2++] = type;
		packed[ofs2++] = color;
	} while (ofs < maxcount);
	return packed;
}

int _zzt_param_decode(ZZTparam* params, int paramcount, ZZTblock *block)
{
	int i;

	/* Store the number of params */
	block->paramcount = paramcount;
	/* Allocate an array of pointers */
	block->params = (ZZTparam **) malloc(sizeof(ZZTparam*) * paramcount);

	for (i = 0; i < paramcount; i++) {
		/* Duplicate the compressed param */
		ZZTparam* param = zztParamDuplicate(params + i);

		/* Store the param pointer in the ordered array */
		block->params[i] = param;
		param->index = i;

		/* Store the param pointer in the tile array */
		if (params[i].x < block->width && params[i].y < block->height)
			zztTileAt(block, params[i].x, params[i].y).param = param;
	}
	return 1;
}

ZZTparam *_zzt_param_encode(int *paramsize, int playerx, int playery, ZZTblock *block)
{
	ZZTparam *params;
	int i;

	if (block->paramcount == 0)
		return NULL;

	/* Make room for the compressed list of params */
	params = (ZZTparam *) malloc(sizeof(ZZTparam) * block->paramcount);

	/* Ensure that the first param points to the player */
	block->params[0]->x = playerx;
	block->params[0]->y = playery;

	/* Copy each param onto the params array */
	for (i = 0; i < block->paramcount; i++)
	{
		zztParamCopyPtr(params + i, block->params[i]);
	}

	*paramsize = block->paramcount;
	return params;
}

void _zztBoardFree(ZZTboard board)
{
	int i;

	/* Delete the bigboard */
	if (board.bigboard != NULL)
		zztBlockFree(board.bigboard);
	/* Delete the packed board */
	if (board.packed != NULL)
		free(board.packed);
	/* Delete all params */
	if(board.params != NULL) {
		for(i = 0; i < board.info.paramcount; i++) {
			if(board.params[i].length != 0)
				free(board.params[i].program);
		}
		free(board.params);
	}
	board.bigboard = NULL;
	board.packed = NULL;
	board.params = NULL;
}

void zztBoardCopyPtr(ZZTboard *dest, ZZTboard *src)
{
	int tiles = 0, ofs = 0;
	int i;

	/* Base board junk */
	memcpy(dest, src, sizeof(ZZTboard));
	/* Packed board */
	if (src->packed != NULL) {
		do {
			tiles += src->packed[ofs];
			ofs += 3;
		} while(tiles < ZZT_BOARD_MAX_SIZE);
		dest->packed = malloc(ofs);
		memcpy(dest->packed, src->packed, ofs);
	}
	/* Parameters */
	if(dest->params != NULL) {
		dest->params = malloc(sizeof(ZZTparam)*dest->info.paramcount);
		memcpy(dest->params, src->params, dest->info.paramcount*sizeof(ZZTparam));
		for(i = 0; i < src->info.paramcount; i++) {
			if(dest->params[i].length != 0) {
				dest->params[i].program = malloc(dest->params[i].length);
				memcpy(dest->params[i].program, src->params[i].program, dest->params[i].length);
			}
		}
	}
	/* Bigboard */
	if (src->bigboard != NULL) {
		dest->bigboard = zztBlockDuplicate(src->bigboard);
	}
}

/* _zzt_board_relink(brd, offset, start, end movefrom, moveto)
 * Add offset to all links in a board between start and end, except
 * links to "movefrom" which become "moveto" instead
 */
int _zzt_board_relink(ZZTboard *brd, int offset, int start, int end, int movefrom, int moveto)
{
	int j, board_length;

	/* Cool macro to save major space */
#define relink(link) if ((link) == movefrom) (link) = moveto; else if ((link) >= start && (link) <= end) (link) += offset;

	/* Relink board connections */
	relink(brd->info.board_n);
	relink(brd->info.board_s);
	relink(brd->info.board_e);
	relink(brd->info.board_w);

	/* Do the same for passages */
	if (!zztBoardDecompress(brd)) {
		fprintf(stderr, "Error decompressing board\n");
		return 0;
	}
	board_length = brd->bigboard->width * brd->bigboard->height;
	for (j = 0; j < board_length; j++) {
		if (brd->bigboard->tiles[j].type == ZZT_PASSAGE && brd->bigboard->tiles[j].param != NULL) {
			relink(brd->bigboard->tiles[j].param->data[2]);
		}
	}

	return 1;
}

int _zzt_board_limit_links(ZZTboard *brd, int max)
{
	int j, board_length;

	/* Fix link macro */
#define fixlink(link) if ((link) > max) (link) = 0;

	/* Fix board connections */
	fixlink(brd->info.board_n);
	fixlink(brd->info.board_s);
	fixlink(brd->info.board_e);
	fixlink(brd->info.board_w);

	/* Do the same for passages */
	if (!zztBoardDecompress(brd)) {
		fprintf(stderr, "Error decompressing board\n");
		return 0;
	}

	board_length = brd->bigboard->width * brd->bigboard->height;
	for (j = 0; j < board_length; j++) {
		if (brd->bigboard->tiles[j].type == ZZT_PASSAGE && brd->bigboard->tiles[j].param != NULL) {
			fixlink(brd->bigboard->tiles[j].param->data[2]);
		}
	}

	return 1;
}

/* Functions present in zzt.h */

ZZTboard *zztBoardCreate(char *title)
{
	int blocks, remainder, ofs;
	int i;

	/* Create new board */
	ZZTboard *board = malloc(sizeof(ZZTboard));
	memset(&board->info, 0, sizeof(ZZTboardinfo));
	strncpy((char *)board->title, title, ZZT_BOARD_TITLE_SIZE);
	board->title[ZZT_BOARD_TITLE_SIZE] = '\0';
	board->info.maxshots = 255;
	board->bigboard = NULL;
	/* Make packed blank board */
	blocks = (ZZT_BOARD_MAX_SIZE-1)/255;
	remainder = (ZZT_BOARD_MAX_SIZE-1)%255;
	board->packed = malloc((blocks+2)*3);
	ofs = 0;
	board->packed[ofs++] = 1;
	board->packed[ofs++] = ZZT_PLAYER;
	board->packed[ofs++] = 0x1F;
	for(i = 0; i < blocks+1; i++) {
		board->packed[ofs++] = 255;
		board->packed[ofs++] = ZZT_EMPTY;
		board->packed[ofs++] = 0x0F;
	}
	board->packed[ofs-3] = remainder;
	/* Make player param */
	board->params = malloc(sizeof(ZZTparam));
	memset(board->params, 0, sizeof(ZZTparam));
	board->params->cycle = 1;
	board->info.paramcount = 1;
	/* Player position: (0, 0) */
	board->plx = board->ply = 0;

	return board;
}

void zztBoardFree(ZZTboard *board)
{
	_zztBoardFree(*board);
	/* Delete the board pointer */
	free(board);
}

ZZTboard *zztBoardLoad(char *filename)
{
	ZZTboard *board;
	FILE *fp;
       
	/* Open file */
	fp = fopen(filename, "rb");
	if(fp == NULL)
		return NULL;

	/* Read from file */
	board = zztBoardRead(fp);
	fclose(fp);

	/* Done */
	return board;
}

int zztBoardSave(ZZTboard *board, char *filename)
{
	int result;
	FILE *fp;
	
	/* Open file */
	fp = fopen(filename, "wb");
	if(fp == NULL)
		return 0;
	
	/* Compress the board before writing */
	zztBoardCompress(board);

	/* Write to file */
	result = zztBoardWrite(board, fp);
	fclose(fp);

	/* Done */
	return result;
}

ZZTboard *zztBoardCopy(ZZTboard *board)
{
	ZZTboard *new = malloc(sizeof(ZZTboard));
	zztBoardCopyPtr(new, board);
	return new;
}

int zztBoardDecompress(ZZTboard *board)
{
	int i;

	/* Do not decompress if already decompressed */
	if (board->bigboard != NULL || board->packed == NULL || board->params == NULL)
		/* Consider giving an error if one but not all of the above are true */
		return 1;

	board->bigboard = zztBlockCreate(ZZT_BOARD_X_SIZE, ZZT_BOARD_Y_SIZE);
	
	if (board->bigboard == NULL)
		return 0;

	/* Decode packed & params onto bigboard */
	_zzt_rle_decode(board->packed, board->bigboard);
	_zzt_param_decode(board->params, board->info.paramcount, board->bigboard);

	/* Free packed & params */
	free(board->packed);
	for(i = 0; i < board->info.paramcount; i++) {
		if(board->params[i].length != 0)
			free(board->params[i].program);
	}
	free(board->params);

	board->packed = NULL;
	board->params = NULL;

	return 1;
}

int zztBoardCompress(ZZTboard *board)
{
	int paramcount;

	/* Do not compress if alread compressed */
	if (board->packed != NULL || board->params != NULL || board->bigboard == NULL)
		/* Consider giving an error if one but not all of the above are true */
		return 1;

	/* Pack the bigboard back into packed and params */
	board->packed = _zzt_rle_encode(board->bigboard);
	if (board->packed == NULL)
		return 0;
	board->params = _zzt_param_encode(&paramcount, board->plx, board->ply,
																		board->bigboard);
	if (board->params == NULL)
		return 0;
	board->info.paramcount = paramcount;

	zztBlockFree(board->bigboard);
	board->bigboard = NULL;

	return 1;
}

uint16_t zztBoardGetSize(ZZTboard *board)
{
	uint16_t size = 0;
	int i;

	size += 0x34; /* Header */
	size += 0x58; /* Info */
	size += 1;    /* ? */

	/* Size the bigboard */
	if (board->bigboard != NULL) {
		size += _zzt_rle_encoded_size(board->bigboard) * 3;
		size += board->bigboard->paramcount * 0x21;
		/* Find size of program data */
		for (i = 0; i < board->bigboard->width * board->bigboard->height; i++)
			if (board->bigboard->tiles[i].param != NULL)
				size += board->bigboard->tiles[i].param->length;
	} else if (board->packed != NULL) {
		/* TODO: find size of packed & params */
	}

	return size;
}

void zztWorldAddBoard(ZZTworld *world, char *title)
{
	ZZTboard *backup, *new;
	int bcount = zztWorldGetBoardcount(world);

	if(bcount != 0) {
		/* Make backup of existing board list */
		backup = malloc(sizeof(ZZTboard)*bcount);
		memcpy(backup, world->boards, sizeof(ZZTboard)*bcount);
		free(world->boards);
	}
	/* Allocate bigger board list */
	zztWorldSetBoardcount(world, ++bcount);
	world->boards = malloc(sizeof(ZZTboard)*bcount);
	/* Restore existing board list */
	if(bcount != 1) {
		memcpy(world->boards, backup, sizeof(ZZTboard)*(bcount-1));
		free(backup);
	}

	/* Create new board */
	new = zztBoardCreate(title);
	memcpy(&world->boards[bcount-1], new, sizeof(ZZTboard));
	free(new);	/* Don't ever free a board like this */
}

int zztWorldDeleteBoard(ZZTworld *world, int number, int relink)
{
	ZZTboard *backup;
	int bcount = zztWorldGetBoardcount(world);
	int i;

	/* Check that it's in range */
	if(number < 0 || number >= bcount)
		return 0;

	/* Clear the current board from memory */
	_zztBoardFree(world->boards[number]);

	if(bcount != 1) {
		/* Remember where the old board array is */
		backup = world->boards;

		/* Resize new array */
		zztWorldSetBoardcount(world, --bcount);
		world->boards = malloc(sizeof(ZZTboard)*bcount);

		/* Copy boards before deleted one */
		for(i = 0; i < number; i++)
			memcpy(&world->boards[i], &backup[i], sizeof(ZZTboard));

		/* Copy boards after deleted one */
		for(i = number+1; i < bcount+1; i++)
			memcpy(&world->boards[i-1], &backup[i], sizeof(ZZTboard));

		/* Free the old board array */
		free(backup);

		/* Move back one if we're over it */
		/* Do this before relinking so we know which board not to compress */
		if(world->cur_board > number) {
			world->cur_board--;
		}

		/* Relink boards */
		if(relink) {
			for(i = 0; i < bcount; i++) {
				ZZTboard* brd = &(world->boards[i]);
				/* Relink links pointing to the next board up to
				 * where the boardcount ended _before_ we deleted this board!
				 * (that's one less that bcount + 1) */
				_zzt_board_relink(brd, -1, number + 1, bcount, number, 0);

				/* Recompress the board unless it is the current board */
				if (i != world->cur_board)
					zztBoardCompress(brd);
			}
			if(world->header->startboard == number)
				world->header->startboard = 0;
			if(world->header->startboard > number)
				world->header->startboard--;
		}
	} else {
		/* Delete the last board */
		free(world->boards);
		world->boards = NULL;
		zztWorldSetBoardcount(world, 0);
	}
	return 1;
}

int zztWorldInsertBoard(ZZTworld *world, ZZTboard *board, int number, int relink)
{
	ZZTboard *backup;
	int bcount = zztWorldGetBoardcount(world);
	int i;

	/* Check that it's in range */
	if(number < 0 || number > bcount)
		return 0;

	/* Deleting the 0th board and relinking is impossible */
	if(relink && number == 0)
		return 0;

	/* Remember where the original board array is */
	backup = world->boards;

	/* Resize new array */
	zztWorldSetBoardcount(world, ++bcount);
	world->boards = malloc(sizeof(ZZTboard)*bcount);

	/* Copy boards before new one */
	for(i = 0; i < number; i++)
		memcpy(&world->boards[i], &backup[i], sizeof(ZZTboard));
	/* Copy boards after new one */
	for(i = number+1; i < bcount; i++)
		memcpy(&world->boards[i], &backup[i-1], sizeof(ZZTboard));
	
	/* Free the old array */
	free(backup);
	backup = NULL;

	/* Move up one if we're over it */
	/* Must be done before relinking */
	if(world->cur_board >= number) {
		world->cur_board++;
	}

	/* Relink boards */
	if(relink) {
		for(i = 0; i < bcount; i++) {
			ZZTboard* brd = &(world->boards[i]);
			
			/* This board has yet to be inserted -- avoid */
			if(i == number)
				continue;
			
			_zzt_board_relink(brd, 1, number, zztWorldGetBoardcount(world), 0, 0);

			/* Recompress the board unless it is the current board */
			if (i != world->cur_board)
				zztBoardCompress(brd);
		}
		if(world->header->startboard >= number)
			world->header->startboard++;
	}

	/* Insert the new board (finally!) */
	zztBoardCopyPtr(&world->boards[number], board);

	/* Make sure curboard is decompressed */
	zztBoardDecompress(&world->boards[world->cur_board]);

	return 1;
}

/* This is blue magick */
int zztWorldMoveBoard(ZZTworld *world, int src, int dest)
{
	int bcount = zztWorldGetBoardcount(world);
	int high, low, offset;
	int i;
	ZZTboard* copy;

	if(src == dest) {
		return 1; /* ;) */
	}
	if(src < 1 || src >= bcount) {
		return 0;
	}
	if(dest < 1 || dest >= bcount) {
		return 0;
	}

	if(src < dest) {
		high = dest;
		low = src;
		offset = -1;
	} else {
		high = src;
		low = dest;
		offset = 1;
	}

	/* Consider the current board */
	if (world->cur_board == src)
		world->cur_board = dest;
	else if (world->cur_board >= low && world->cur_board <= high)
		world->cur_board += offset;

	/* Consider the starting board */
	if (world->header->startboard == src)
		world->header->startboard = dest;
	else if (world->header->startboard >= low && world->header->startboard <= high)
		world->header->startboard += offset;

	/* Run thourgh all boards and relink them */
	for (i = 0; i < bcount; i++) {
		ZZTboard* brd = &(world->boards[i]);
		_zzt_board_relink(brd, offset, low, high, src, dest);

		/* Recompress the board unless it is the current board */
		if (i != world->cur_board)
			zztBoardCompress(brd);
	}

	/* MOVE IT */

	/* Make a copy of the board-to-be-moved */
	if((copy = zztBoardCopy(&world->boards[src])) == NULL) {
		return 0;
	}

	/* Delete the src board from it's current position */
	zztWorldDeleteBoard(world, src, 0);

	/* Insert copy of src board at destination */
	zztWorldInsertBoard(world, copy, dest, 0);

	/* Free the copy! */
	zztBoardFree(copy);

	return 1;
}

int zztBoardSelect(ZZTworld *world, int number)
{
	/* Check range */
	if(number < 0 || number >= world->header->boardcount)
		return 0;

	/* Commit old current board */
	zztBoardCommit(world);

	/* Set new current board */
	world->cur_board = number;
	/* Uncompress to bigboard */
	zztBoardDecompress(&world->boards[number]);

	return 1;
}

void zztBoardCommit(ZZTworld *world)
{
	int curboard = zztBoardGetCurrent(world);

	/* Compress the current board */
	zztBoardCompress(&(world->boards[curboard]));
}

int zztBoardClear(ZZTworld *world)
{
	ZZTboard * brd = zztBoardGetCurPtr(world);
	ZZTboard * newbrd;

	/* Create a new board */
	newbrd = zztBoardCreate("");
	if (newbrd == NULL)
		return 0;

	/* Free the old board data (not the board itself) */
	_zztBoardFree(*brd);

	/* Who feels lazy? */
	memcpy(brd, newbrd, sizeof(ZZTboard));

	/* FREE ONLY THE LOWEST LEVEL of the new board!
	 * Pointers to other memory locations are now copied
	 * into the board array via brd */
	free(newbrd);

	/* Decompress the current board */
	zztBoardDecompress(brd);

	return 1;
}

int zztBoardValidateLinks(ZZTworld *world)
{
	return _zzt_board_limit_links(zztBoardGetCurPtr(world), zztWorldGetBoardcount(world) - 1);
}

int zztBoardGetCurrent(ZZTworld *world)
{
	return world->cur_board;
}

ZZTboard * zztBoardGetCurPtr(ZZTworld *world)
{
	return &(world->boards[world->cur_board]);
}

ZZTblock * zztBoardGetBlock(ZZTworld *world)
{
	return world->boards[world->cur_board].bigboard;
}

void zztBoardSetTitle(ZZTworld *world, char *title)
{
	strncpy((char *)world->boards[world->cur_board].title, title, ZZT_BOARD_TITLE_SIZE);
	world->boards[world->cur_board].title[ZZT_BOARD_TITLE_SIZE] = '\0';
}	
void zztBoardSetMaxshots(ZZTworld *world, uint8_t maxshots)
{
	world->boards[world->cur_board].info.maxshots = maxshots;
}
void zztBoardSetDarkness(ZZTworld *world, uint8_t darkness)
{
	world->boards[world->cur_board].info.darkness = darkness;
}
void zztBoardSetBoard_n(ZZTworld *world, uint8_t board_n)
{
	world->boards[world->cur_board].info.board_n = board_n;
}
void zztBoardSetBoard_s(ZZTworld *world, uint8_t board_s)
{
	world->boards[world->cur_board].info.board_s = board_s;
}
void zztBoardSetBoard_w(ZZTworld *world, uint8_t board_w)
{
	world->boards[world->cur_board].info.board_w = board_w;
}
void zztBoardSetBoard_e(ZZTworld *world, uint8_t board_e)
{
	world->boards[world->cur_board].info.board_e = board_e;
}
void zztBoardSetReenter(ZZTworld *world, uint8_t reenter)
{
	world->boards[world->cur_board].info.reenter = reenter;
}
void zztBoardSetReenter_x(ZZTworld *world, uint8_t reenter_x)
{
	world->boards[world->cur_board].info.reenter_x = reenter_x;
}
void zztBoardSetReenter_y(ZZTworld *world, uint8_t reenter_y)
{
	world->boards[world->cur_board].info.reenter_y = reenter_y;
}
void zztBoardSetMessage(ZZTworld *world, char *message)
{
	strncpy((char *)world->boards[world->cur_board].info.message, message, ZZT_MESSAGE_SIZE);
	world->boards[world->cur_board].info.message[ZZT_MESSAGE_SIZE] = '\0';
}
void zztBoardSetTimelimit(ZZTworld *world, uint16_t timelimit)
{
	world->boards[world->cur_board].info.timelimit = timelimit;
}
void zztBoardSetParamcount(ZZTworld *world, uint16_t paramcount)
{
	// XXX Don't ever use this, it's just here for completeness
	world->boards[world->cur_board].info.paramcount = paramcount;
}

uint8_t *zztBoardGetTitle(ZZTworld *world)
{
	return world->boards[world->cur_board].title;
}
uint8_t zztBoardGetMaxshots(ZZTworld *world)
{
	return world->boards[world->cur_board].info.maxshots;
}
uint8_t zztBoardGetDarkness(ZZTworld *world)
{
	return world->boards[world->cur_board].info.darkness;
}
uint8_t zztBoardGetBoard_n(ZZTworld *world)
{
	return world->boards[world->cur_board].info.board_n;
}
uint8_t zztBoardGetBoard_s(ZZTworld *world)
{
	return world->boards[world->cur_board].info.board_s;
}
uint8_t zztBoardGetBoard_w(ZZTworld *world)
{
	return world->boards[world->cur_board].info.board_w;
}
uint8_t zztBoardGetBoard_e(ZZTworld *world)
{
	return world->boards[world->cur_board].info.board_e;
}
uint8_t zztBoardGetReenter(ZZTworld *world)
{
	return world->boards[world->cur_board].info.reenter;
}
uint8_t zztBoardGetReenter_x(ZZTworld *world)
{
	return world->boards[world->cur_board].info.reenter_x;
}
uint8_t zztBoardGetReenter_y(ZZTworld *world)
{
	return world->boards[world->cur_board].info.reenter_y;
}
uint8_t *zztBoardGetMessage(ZZTworld *world)
{
	return world->boards[world->cur_board].info.message;
}
uint16_t zztBoardGetTimelimit(ZZTworld *world)
{
	return world->boards[world->cur_board].info.timelimit;
}
uint16_t zztBoardGetParamcount(ZZTworld *world)
{
	return world->boards[world->cur_board].info.paramcount;
}
