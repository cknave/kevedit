/* board.c	-- Board functions
 * $Id: board.c,v 1.1 2002/01/30 07:20:57 kvance Exp $
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

int _zzt_rle_decode(u_int8_t *packed, u_int8_t *unpacked)
{
	int ofs = 0, count = 0;
	u_int8_t i;

	do {
		for(i = 0; i < packed[ofs]; i++) {
			unpacked[count++] = packed[ofs+1];
			if(count == ZZT_BOARD_MAX_SIZE*2)
				return 1;
			unpacked[count++] = packed[ofs+2];
			if(count == ZZT_BOARD_MAX_SIZE*2)
				return 1;
		}
		ofs += 3;
	} while(count < ZZT_BOARD_MAX_SIZE*2);
	return 0;
}

u_int8_t *_zzt_rle_encode(u_int8_t *unpacked)
{
	int size = 0, ofs = 0, ofs2 = 0;
	u_int8_t blocks = 1;
	u_int8_t type;
	u_int8_t color;

	u_int8_t *packed;

	/* Get size of RLE data */
	do {
		type = unpacked[ofs++];
		color = unpacked[ofs++];
		if(ofs < ZZT_BOARD_MAX_SIZE*2) {
			while(type == unpacked[ofs] && color == unpacked[ofs+1] && blocks < 255) {
				blocks++;
				ofs += 2;
				if(ofs >= ZZT_BOARD_MAX_SIZE*2)
					break;
			}
		}
		size++;
		blocks = 1;
	} while(ofs < ZZT_BOARD_MAX_SIZE*2);

	/* Reallocate data */
	packed = malloc(size*3);
	/* RLE encode from unpacked */
	ofs = ofs2 = 0; blocks = 1;
	do {
		type = unpacked[ofs++];
		color = unpacked[ofs++];
		if(ofs < ZZT_BOARD_MAX_SIZE*2) {
			while(type == unpacked[ofs] && color == unpacked[ofs+1] && blocks < 255) {
				blocks++;
				ofs += 2;
				if(ofs >= ZZT_BOARD_MAX_SIZE*2)
					break;
			}
		}
		packed[ofs2++] = blocks;
		packed[ofs2++] = type;
		packed[ofs2++] = color;
		blocks = 1;
	} while (ofs < ZZT_BOARD_MAX_SIZE*2);
	return packed;
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
	int i, j, k;
	int x, y;

	u_int8_t *bigboard;

	/* Check that it's in range */
	if(number < 0 || number >= bcount)
		return 0;


	/* Delete the board */
	free(world->boards[number].packed);
	/* Delete all params */
	if(world->boards[number].params != NULL) {
		for(i = 0; i < world->boards[number].info.paramcount; i++) {
			if(world->boards[number].params[i].length != 0)
				free(world->boards[number].params[i].program);
		}
		free(world->boards[number].params);
	}
	if(bcount != 1) {
		/* Make backup array */
		backup = malloc(sizeof(ZZTboard)*bcount);
		memcpy(backup, world->boards, sizeof(ZZTboard)*bcount);
		free(world->boards);
		/* Resize new array */
		zztWorldSetBoardcount(world, --bcount);
		world->boards = malloc(sizeof(ZZTboard)*bcount);
		/* Copy boards before deleted one */
		for(i = 0; i < number; i++)
			memcpy(&world->boards[i], &backup[i], sizeof(ZZTboard));
		/* Copy boards after deleted one */
		for(i = number+1; i < bcount+1; i++)
			memcpy(&world->boards[i-1], &backup[i], sizeof(ZZTboard));
		free(backup);
		/* Relink boards */
		if(relink) {
			bigboard = malloc(ZZT_BOARD_MAX_SIZE*2);
			for(i = 0; i < bcount; i++) {
				/* If linked to deleted board, no more link */
				if(world->boards[i].info.board_n == number)
					world->boards[i].info.board_n = 0;
				if(world->boards[i].info.board_s == number)
					world->boards[i].info.board_s = 0;
				if(world->boards[i].info.board_e == number)
					world->boards[i].info.board_e = 0;
				if(world->boards[i].info.board_w == number)
					world->boards[i].info.board_w = 0;
				/* If after deleted board, shift down one */
				if(world->boards[i].info.board_n > number)
					world->boards[i].info.board_n--;
				if(world->boards[i].info.board_s > number)
					world->boards[i].info.board_s--;
				if(world->boards[i].info.board_e > number)
					world->boards[i].info.board_e--;
				if(world->boards[i].info.board_w > number)
					world->boards[i].info.board_w--;
				/* Do the same for passages */
				_zzt_rle_decode(world->boards[i].packed, bigboard);
				for(j = 0; j < ZZT_BOARD_MAX_SIZE; j++) {
					if(bigboard[j*2] == ZZT_PASSAGE) {
						y = j / ZZT_BOARD_X_SIZE;
						x = j % ZZT_BOARD_X_SIZE;
						/* Find its param */
						for(k = 0; k < world->boards[i].info.paramcount; k++) {
							if(world->boards[i].params[k].x == x &&
							   world->boards[i].params[k].y == y) {
								if(world->boards[i].params[k].data[2] == number)
									world->boards[i].params[k].data[2] = 0;
								else if(world->boards[i].params[k].data[2] > number)
									world->boards[i].params[k].data[2]--;
							}
						}
					}
				}
			}
			if(world->header->startboard == number)
				world->header->startboard = 0;
			if(world->header->startboard > number)
				world->header->startboard--;
			free(bigboard);
		}
		/* Move back one if we're over it */
		if(world->cur_board > number) {
			world->cur_board--;
		}
	} else {
		/* Delete the last board */
		free(world->boards);
		world->boards = NULL;
		zztWorldSetBoardcount(world, 0);
	}
	return 1;
}

void zztBoardCopyPtr(ZZTboard *dest, ZZTboard *src)
{
	int tiles = 0, ofs = 0;
	int i;

	/* Base board junk */
	memcpy(dest, src, sizeof(ZZTboard));
	/* Packed board */
	do {
		tiles += src->packed[ofs];
		ofs += 3;
	} while(tiles < ZZT_BOARD_MAX_SIZE);
	dest->packed = malloc(ofs);
	memcpy(dest->packed, src->packed, ofs);
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
}

ZZTboard *zztBoardCopy(ZZTboard *board)
{
	ZZTboard *new = malloc(sizeof(ZZTboard));
	zztBoardCopyPtr(new, board);
	return new;
}

int zztWorldInsertBoard(ZZTworld *world, ZZTboard *board, int number, int relink)
{
	ZZTboard *backup;
	int bcount = zztWorldGetBoardcount(world);
	int i, j, k;
	int x, y;

	u_int8_t *bigboard;

	/* Check that it's in range */
	if(number < 0 || number > bcount)
		return 0;

	/* Deleting the 0th board and relinking is impossible */
	if(relink && number == 0)
		return 0;

	/* Make backup array */
	backup = malloc(sizeof(ZZTboard)*bcount);
	memcpy(backup, world->boards, sizeof(ZZTboard)*bcount);
	free(world->boards);
	/* Resize new array */
	zztWorldSetBoardcount(world, ++bcount);
	world->boards = malloc(sizeof(ZZTboard)*bcount);

	/* Copy boards before new one */
	for(i = 0; i < number; i++)
		memcpy(&world->boards[i], &backup[i], sizeof(ZZTboard));
	/* Copy boards after new one */
	for(i = number+1; i < bcount; i++)
		memcpy(&world->boards[i], &backup[i-1], sizeof(ZZTboard));
	free(backup);

	/* Relink boards */
	if(relink) {
		bigboard = malloc(ZZT_BOARD_MAX_SIZE*2);
		for(i = 0; i < bcount; i++) {
			/* Don't do the number yet */
			if(i == number)
				continue;
			/* If after number, shift up one */
			if(world->boards[i].info.board_n >= number && world->boards[i].info.board_n != 0)
				world->boards[i].info.board_n++;
			if(world->boards[i].info.board_s >= number && world->boards[i].info.board_s != 0)
				world->boards[i].info.board_s++;
			if(world->boards[i].info.board_e >= number && world->boards[i].info.board_e != 0)
				world->boards[i].info.board_e++;
			if(world->boards[i].info.board_w >= number && world->boards[i].info.board_w != 0)
				world->boards[i].info.board_w++;
			/* Do the same for passages */
			_zzt_rle_decode(world->boards[i].packed, bigboard);
			for(j = 0; j < ZZT_BOARD_MAX_SIZE; j++) {
				if(bigboard[j*2] == ZZT_PASSAGE) {
					y = j / ZZT_BOARD_X_SIZE;
					x = j % ZZT_BOARD_X_SIZE;
					/* Find its param */
					for(k = 0; k < world->boards[i].info.paramcount; k++) {
						if(world->boards[i].params[k].x == x &&
						   world->boards[i].params[k].y == y) {
							if(world->boards[i].params[k].data[2] >= number && world->boards[i].params[k].data[2] != 0)
								world->boards[i].params[k].data[2]++;
						}
					}
				}
			}
		}
		if(world->header->startboard >= number)
			world->header->startboard++;
		free(bigboard);
	}
	
	/* Insert the new board (finally!) */
	zztBoardCopyPtr(&world->boards[number], board);

	/* Move up one if we're over it */
	if(world->cur_board >= number) {
		world->cur_board++;
	}
	return 1;
}

/* This is black magick */
int zztWorldMoveBoard(ZZTworld *world, int src, int dest)
{
	ZZTboard *copy, *ptr;
	int bcount = zztWorldGetBoardcount(world);
	int i, j, k, x, y, high, low, coefficient;

	int maxlinks = 50, links = 0;
	u_int8_t *bigboard;
	struct linkpacket {
		enum { END = 0, NORTH, SOUTH, EAST, WEST, PASSAGE, STARTBOARD };
		int type;
		int board;
		int param;
	} *linkpacket = malloc(sizeof(linkpacket)*50);

	int type, board, param;

	if(src == dest) {
		free(linkpacket);
		return 1; /* ;) */
	}
	if(src < 1 || src >= bcount) {
		free(linkpacket);
		return 0;
	}
	if(dest < 1 || dest >= bcount) {
		free(linkpacket);
		return 0;
	}

	if(src < dest) {
		high = dest;
		low = src;
		coefficient = -1;
	} else {
		high = src;
		low = dest;
		coefficient = 1;
	}

	/* Make a copy of the board-to-be-moved */
	if((copy = zztBoardCopy(&world->boards[src])) == NULL) {
		free(linkpacket);
		return 0;
	}

	/* Run through all links and record those that link to the board-to-be-moved */
	memset(linkpacket, 0, sizeof(linkpacket)*50);
	bigboard = malloc(ZZT_BOARD_MAX_SIZE*2);
	for(i = 0; i <= bcount; i++) {
		if(i == src)
			continue;
		/* Enough link space? */
		if(links >= maxlinks-5) {
			maxlinks += 50;
			linkpacket = realloc(linkpacket, sizeof(linkpacket)*maxlinks);
		}
		ptr = (i == bcount) ? copy : &world->boards[i];
		/* Board n/s/e/w links */
		if(ptr->info.board_n == src) {
			linkpacket[links].type = NORTH;
			linkpacket[links++].board = (i >= low && i <= high) ? i+coefficient : i;
		}
		if(ptr->info.board_s == src) {
			linkpacket[links].type = SOUTH;
			linkpacket[links++].board = (i >= low && i <= high) ? i+coefficient : i;
		}
		if(ptr->info.board_e == src) {
			linkpacket[links].type = EAST;
			linkpacket[links++].board = (i >= low && i <= high) ? i+coefficient : i;
		}
		if(ptr->info.board_w == src) {
			linkpacket[links].type = WEST;
			linkpacket[links++].board = (i >= low && i <= high) ? i+coefficient : i;
		}
		if(i == bcount) {
			if(ptr->info.board_n >= low && ptr->info.board_n <= high)
				ptr->info.board_n+=coefficient;
			if(ptr->info.board_s >= low && ptr->info.board_s <= high)
				ptr->info.board_s+=coefficient;
			if(ptr->info.board_e >= low && ptr->info.board_e <= high)
				ptr->info.board_e+=coefficient;
			if(ptr->info.board_w >= low && ptr->info.board_w <= high)
				ptr->info.board_w+=coefficient;
		}
		/* Passage links */
		_zzt_rle_decode(ptr->packed, bigboard);
		for(j = 0; j < ZZT_BOARD_MAX_SIZE; j++) {
			if(bigboard[j*2] == ZZT_PASSAGE) {
				y = j / ZZT_BOARD_X_SIZE;
				x = j % ZZT_BOARD_X_SIZE;
				/* Find its param */
				for(k = 0; k < ptr->info.paramcount; k++) {
					if(i != bcount && 
					   ptr->params[k].x == x &&
					   ptr->params[k].y == y &&
					   ptr->params[k].data[2] == src) {
						linkpacket[links].type = PASSAGE;
						linkpacket[links].board = (i >= low && i <= high) ? i+coefficient : i;
						linkpacket[links++].param = k;
						if(links >= maxlinks-5) {
							maxlinks += 50;
							linkpacket = realloc(linkpacket, sizeof(linkpacket)*maxlinks);
						}
					}
					else if(i == bcount &&
					  ptr->params[k].x == x &&
 					  ptr->params[k].y == y &&
					  ptr->params[k].data[2] >= low &&
					  ptr->params[k].data[2] <= high) {
						ptr->params[k].data[2]+=coefficient;
					}
				}
			}
		}
	}
	if(links >= maxlinks-1) {
		maxlinks += 2;
		linkpacket = realloc(linkpacket, sizeof(linkpacket)*maxlinks);
	}
	if(world->header->startboard == src) {
		linkpacket[links].type = STARTBOARD;
		links++;
	}
	linkpacket[links].type = END;
	free(bigboard);

	/* Delete and reinsert board */
	if(!zztWorldDeleteBoard(world, src, 1)) {
		zztBoardFree(copy);
		free(linkpacket);
		return 0;
	}
	if(!zztWorldInsertBoard(world, copy, dest, 1)) {
		/* shit! */
		fprintf(stderr, "\a\nNO NO NO!$((!$  aUGH!&*%% NO@!&*!@\nI'm sorry\nI'm so sorry!%%(@\n");
		zztBoardFree(copy);
		free(linkpacket);
		return 0;
	}

	/* Relink stuff pointing to the board we just moved */
	links = 0;
	while(linkpacket[links].type != END) {
		type = linkpacket[links].type;
		board = linkpacket[links].board;
		param = linkpacket[links].param;
		if(type == NORTH)
			world->boards[board].info.board_n = dest;
		else if(type == SOUTH)
			world->boards[board].info.board_s = dest;
		else if(type == EAST)
			world->boards[board].info.board_e = dest;
		else if(type == WEST)
			world->boards[board].info.board_w = dest;
		else if(type == PASSAGE)
			world->boards[board].params[param].data[2] = dest;
		else if(type == STARTBOARD)
			world->header->startboard = dest;
		links++;
	}

	zztBoardFree(copy);
	free(linkpacket);

	return 1;
}

ZZTboard *zztBoardCreate(char *title)
{
	int blocks, remainder, ofs;
	int i;

	/* Create new board */
	ZZTboard *board = malloc(sizeof(ZZTboard));
	memset(&board->info, 0, sizeof(ZZTboardinfo));
	strncpy(board->title, title, ZZT_BOARD_TITLE_SIZE);
	board->title[ZZT_BOARD_TITLE_SIZE] = '\0';
	board->info.maxshots = 255;
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

	return board;
}

void zztBoardFree(ZZTboard *board)
{
	int i;

	/* Delete the board */
	free(board->packed);
	/* Delete all params */
	if(board->params != NULL) {
		for(i = 0; i < board->info.paramcount; i++) {
			if(board->params[i].length != 0)
				free(board->params[i].program);
		}
		free(board->params);
	}
	/* Delete the rest */
	free(board);
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
	_zzt_rle_decode(world->boards[number].packed, world->bigboard);

	return 1;
}

void zztBoardCommit(ZZTworld *world)
{
	int curboard = zztBoardGetCurrent(world);

	/* Replaced packed data with new stuff from bigboard */
	free(world->boards[curboard].packed);
	world->boards[curboard].packed = _zzt_rle_encode(world->bigboard);
}

int zztBoardGetCurrent(ZZTworld *world)
{
	return world->cur_board;
}

void zztBoardSetTitle(ZZTworld *world, char *title)
{
	strncpy(world->boards[world->cur_board].title, title, ZZT_BOARD_TITLE_SIZE);
	world->boards[world->cur_board].title[ZZT_BOARD_TITLE_SIZE] = '\0';
}	
void zztBoardSetMaxshots(ZZTworld *world, u_int8_t maxshots)
{
	world->boards[world->cur_board].info.maxshots = maxshots;
}
void zztBoardSetDarkness(ZZTworld *world, u_int8_t darkness)
{
	world->boards[world->cur_board].info.darkness = darkness;
}
void zztBoardSetBoard_n(ZZTworld *world, u_int8_t board_n)
{
	world->boards[world->cur_board].info.board_n = board_n;
}
void zztBoardSetBoard_s(ZZTworld *world, u_int8_t board_s)
{
	world->boards[world->cur_board].info.board_s = board_s;
}
void zztBoardSetBoard_w(ZZTworld *world, u_int8_t board_w)
{
	world->boards[world->cur_board].info.board_w = board_w;
}
void zztBoardSetBoard_e(ZZTworld *world, u_int8_t board_e)
{
	world->boards[world->cur_board].info.board_e = board_e;
}
void zztBoardSetReenter(ZZTworld *world, u_int8_t reenter)
{
	world->boards[world->cur_board].info.reenter = reenter;
}
void zztBoardSetMessage(ZZTworld *world, char *message)
{
	strncpy(world->boards[world->cur_board].info.message, message, ZZT_MESSAGE_SIZE);
	world->boards[world->cur_board].info.message[ZZT_MESSAGE_SIZE] = '\0';
}
void zztBoardSetTimelimit(ZZTworld *world, u_int16_t timelimit)
{
	world->boards[world->cur_board].info.timelimit = timelimit;
}
void zztBoardSetParamcount(ZZTworld *world, u_int16_t paramcount)
{
	// XXX Don't ever use this, it's just here for completeness
	world->boards[world->cur_board].info.paramcount = paramcount;
}

u_int8_t *zztBoardGetTitle(ZZTworld *world)
{
	return world->boards[world->cur_board].title;
}
u_int8_t zztBoardGetMaxshots(ZZTworld *world)
{
	return world->boards[world->cur_board].info.maxshots;
}
u_int8_t zztBoardGetDarkness(ZZTworld *world)
{
	return world->boards[world->cur_board].info.darkness;
}
u_int8_t zztBoardGetBoard_n(ZZTworld *world)
{
	return world->boards[world->cur_board].info.board_n;
}
u_int8_t zztBoardGetBoard_s(ZZTworld *world)
{
	return world->boards[world->cur_board].info.board_s;
}
u_int8_t zztBoardGetBoard_w(ZZTworld *world)
{
	return world->boards[world->cur_board].info.board_w;
}
u_int8_t zztBoardGetBoard_e(ZZTworld *world)
{
	return world->boards[world->cur_board].info.board_e;
}
u_int8_t zztBoardGetReenter(ZZTworld *world)
{
	return world->boards[world->cur_board].info.reenter;
}
u_int8_t *zztBoardGetMessage(ZZTworld *world)
{
	return world->boards[world->cur_board].info.message;
}
u_int16_t zztBoardGetTimelimit(ZZTworld *world)
{
	return world->boards[world->cur_board].info.timelimit;
}
u_int16_t zztBoardGetParamcount(ZZTworld *world)
{
	return world->boards[world->cur_board].info.paramcount;
}
