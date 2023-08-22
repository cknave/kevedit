/* file.c	-- File routines
 * $Id: file.c,v 1.3 2005/06/29 03:20:34 kvance Exp $
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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "zzt.h"

/* File macros */
/* ----------- */
/* ZZT files are on a 16-bit little-endian architecture */
#define _zzt_outb(a, fp)	fwrite(a, 1, 1, fp)
size_t _zzt_outw(uint16_t *a, FILE *fp)
{
	size_t result; uint8_t b = (*a & 0x00FF);
	result = fwrite(&b, 1, 1, fp);
	b = (*a & 0xFF00) >> 8;
	result += fwrite(&b, 1, 1, fp);
	return result;
}
#define _zzt_inb(a, fp)		fread(a, 1, 1, fp)
size_t _zzt_inw(uint16_t *a, FILE *fp)
{
        size_t result; uint8_t b = 0;
        result = fread(&b, 1, 1, fp);
        *a = b;
        result += fread(&b, 1, 1, fp);
        *a += b << 8;
        return result;
}
size_t _zzt_outs(char *s, int len, FILE *fp)
{
	size_t result = 0;
	if(len != 0)
		result = fwrite(s, 1, len, fp);
	return result;
}
size_t _zzt_outspad(char *s, int len, int total, FILE *fp)
{
	size_t result = 0; int i; uint8_t b = 0;
	if(len > total)
		return -1;
	result += _zzt_outs(s, len, fp);
	for(i = 0; i < total-len; i++)
		result += _zzt_outb(&b, fp);
	return result;
}
size_t _zzt_ins(char *s, int len, FILE *fp)
{
	size_t result = 0;
	if(len != 0) {
		result = fread(s, 1, len, fp);
		s[len] = '\0';
	}
	return result;
}
size_t _zzt_inspad(char *s, int len, int total, FILE *fp)
{
	size_t result = 0; int i; uint8_t b = 0;
	if(len > total)
		return -1;
	result += _zzt_ins(s, len, fp);
	for(i = 0; i < total-len; i++)
		result += _zzt_inb(&b, fp);
	return result;
}

#define _zzt_inb_or(a, fp)		if(_zzt_inb(a, fp) != 1)
#define _zzt_inw_or(a, fp)		if(_zzt_inw(a, fp) != 2)
#define _zzt_outb_ordie(a, fp)		if(_zzt_outb(a, fp) != 1) return 0
#define _zzt_outw_ordie(a, fp)		if(_zzt_outw(a, fp) != 2) return 0
#define _zzt_ins_or(s, len, fp)		if(_zzt_ins(s, len, fp) != len)
#define _zzt_inspad_or(s, len, total, fp)	if(_zzt_inspad(s, len, total, fp) != total)
#define _zzt_outs_ordie(s, len, fp)	if(_zzt_outs(s, len, fp) != len) return 0
#define _zzt_outspad_ordie(s, len, total, fp)	if(_zzt_outspad(s, len, total, fp) != total) return 0

int zztWorldWrite(ZZTworld *world, FILE *fp)
{
	char *magicnumber = "\xFF\xFF";
	uint16_t w;
	uint8_t b;
	int i;

	/* Header */
	_zzt_outs_ordie(magicnumber, 2, fp);
	w = world->header->boardcount-1;
	_zzt_outw_ordie(&w, fp);
	_zzt_outw_ordie(&world->header->ammo, fp);
	_zzt_outw_ordie(&world->header->gems, fp);
	for(i = 0; i < 7; i++)
		_zzt_outb_ordie(&world->header->keys[i], fp);
	_zzt_outw_ordie(&world->header->health, fp);
	_zzt_outw_ordie(&world->header->startboard, fp);
	_zzt_outw_ordie(&world->header->torches, fp);
	_zzt_outw_ordie(&world->header->torchcycles, fp);
	_zzt_outw_ordie(&world->header->energizercycles, fp);
	w = 0; /* Unused */
	_zzt_outw_ordie(&w, fp);
	_zzt_outw_ordie(&world->header->score, fp);
	b = strlen((char *)world->header->title);
	_zzt_outb_ordie(&b, fp);
	_zzt_outspad_ordie((char *)world->header->title,
			strlen((char *)world->header->title),
			ZZT_WORLD_TITLE_SIZE, fp);
	/* Flags */
	for(i = 0; i < ZZT_MAX_FLAGS; i++) {
		b = strlen((char *)world->header->flags[i]);
		_zzt_outb_ordie(&b, fp);
		_zzt_outspad_ordie((char *)world->header->flags[i],
				strlen((char *)world->header->flags[i]),
				ZZT_FLAG_SIZE, fp);
	}
	/* More header */
	_zzt_outw_ordie(&world->header->timepassed, fp);
	_zzt_outw_ordie(&world->header->timepassedhsec, fp);
	_zzt_outb_ordie(&world->header->savegame, fp);
	_zzt_outspad_ordie(NULL, 0, 247, fp);
	/* Write boards */
	for(i = 0; i < world->header->boardcount; i++) {
		if(!zztBoardWrite(&world->boards[i], fp))
			return 0;
	}
	
	return 1;
}

ZZTworld *zztWorldRead(FILE *fp)
{
	ZZTworld *world;
	ZZTboard *board;
	unsigned char magicnumber[2];
	int i;

	uint8_t len;
	uint16_t bcount, unusedw;

	/* Read & check header */
	_zzt_inb_or(&magicnumber[0], fp) return NULL;
	_zzt_inb_or(&magicnumber[1], fp) return NULL;
	if(magicnumber[0] != 0xFF || magicnumber[1] != 0xFF)
		return NULL;

	/* Allocate memory for world */
	world = malloc(sizeof(ZZTworld));
	world->header = malloc(sizeof(ZZTworldinfo));
	world->boards = NULL;
	world->filename = NULL;
#define freeworld { free(world->header); free(world); return NULL; }

	/* Load header */
	_zzt_inw_or(&bcount, fp) freeworld;
	bcount++;
	_zzt_inw_or(&world->header->ammo, fp) freeworld;
	_zzt_inw_or(&world->header->gems, fp) freeworld;
	for(i = 0; i < 7; i++)
		_zzt_inb_or(&world->header->keys[i], fp) freeworld;
	_zzt_inw_or(&world->header->health, fp) freeworld;
	_zzt_inw_or(&world->header->startboard, fp) freeworld;
	_zzt_inw_or(&world->header->torches, fp) freeworld;
	_zzt_inw_or(&world->header->torchcycles, fp) freeworld;
	_zzt_inw_or(&world->header->energizercycles, fp) freeworld;
	/* Unused */
	_zzt_inw_or(&unusedw, fp) freeworld;
	_zzt_inw_or(&world->header->score, fp) freeworld;
	_zzt_inb_or(&len, fp) freeworld;
	if(len > ZZT_WORLD_TITLE_SIZE)
		freeworld;
	_zzt_inspad_or((char *)world->header->title, len, ZZT_WORLD_TITLE_SIZE, fp) freeworld;
	/* Flags */
	for(i = 0; i < ZZT_MAX_FLAGS; i++) {
		world->header->flags[i][0] = '\0';
		_zzt_inb_or(&len, fp) freeworld;
		if(len > ZZT_FLAG_SIZE)
			freeworld;
		_zzt_inspad_or((char *)world->header->flags[i], len, ZZT_FLAG_SIZE, fp) freeworld;
	}
	/* More header */
	_zzt_inw_or(&world->header->timepassed, fp) freeworld;
	_zzt_inw_or(&world->header->timepassedhsec, fp) freeworld;
	_zzt_inb_or(&world->header->savegame, fp) freeworld;
	_zzt_inspad_or(NULL, 0, 247, fp) freeworld;
	/* Read boards */
	world->boards = zztBoardRead(fp);
	world->header->boardcount = 1;
	world->cur_board = 0;
	for(i = 1; i < bcount; i++) {
		board = zztBoardRead(fp);
		if (board == NULL) {
			board = zztBoardCreate("Error Loading Board");
		}
		zztWorldInsertBoard(world, board, i, 0);
		zztBoardFree(board);
	}

	/* Don't know the filename, something else must set it */
	world->filename = malloc(2);
	strcpy(world->filename, "-");
	return world;
}

/* It takes a lot of work to free stuff made while creating a board */
void _zzt_boardread_freestuff(ZZTboard *board, uint8_t *packed)
{
	int i;
	/* packed data */
	if(packed) {
		free(packed);
	}
	/* programs for params */
	for(i = 0; i < board->info.paramcount; i++) {
		if(board->params[i].length != 0)
			free(board->params[i].program);
	}
	/* params */
	if(board->params != NULL)
		free(board->params);
	/* board data */
	free(board);
}

ZZTboard *zztBoardRead(FILE *fp)
{
	ZZTboard *board = malloc(sizeof(ZZTboard));
	board->params = NULL;
	board->info.paramcount = 0;

	bool success = false;

	unsigned int packsize = 1000*3, packofs = 0, tiles = 0;
	uint8_t *packed = malloc(packsize);
	uint16_t w;
	uint8_t len;
	uint8_t number, code, color;
	int i;

	/* Boards read from a file are initially compressed */
	board->bigboard = NULL;

	uint16_t board_size;
	_zzt_inw_or(&board_size, fp) goto boardReadDone;
	long board_end_offset = ftell(fp) + board_size;

	/* Board header */
	_zzt_inb_or(&len, fp) goto boardReadDone;
	if(len > ZZT_BOARD_TITLE_SIZE)
		goto boardReadDone;
	_zzt_inspad_or((char *)board->title, len, ZZT_BOARD_TITLE_SIZE, fp) goto boardReadDone;
	/* Put a null-zero at the end of the title to make it a valid C-string */
	board->title[len] = '\0';
	/* Board packed tiles */
	do {
		_zzt_inb_or(&number, fp) goto boardReadDone;
		_zzt_inb_or(&code, fp) goto boardReadDone;
		_zzt_inb_or(&color, fp) goto boardReadDone;
		packed[packofs++] = number;
		packed[packofs++] = code;
		packed[packofs++] = color;
		if(packofs >= packsize) {
			packsize += 1000*3;
			packed = realloc(packed, packsize);
		}
		tiles += (number > 0) ? number : 256;
	} while(tiles < ZZT_BOARD_MAX_SIZE);
	/* Create structure w/ exactly right size */
	board->packed = malloc(packsize);
	memcpy(board->packed, packed, packsize);
	free(packed);
	packed = board->packed;

	/* Board info */
	_zzt_inb_or(&board->info.maxshots, fp) goto boardReadDone;
	_zzt_inb_or(&board->info.darkness, fp) goto boardReadDone;
	_zzt_inb_or(&board->info.board_n, fp) goto boardReadDone;
	_zzt_inb_or(&board->info.board_s, fp) goto boardReadDone;
	_zzt_inb_or(&board->info.board_w, fp) goto boardReadDone;
	_zzt_inb_or(&board->info.board_e, fp) goto boardReadDone;
	_zzt_inb_or(&board->info.reenter, fp) goto boardReadDone;
	_zzt_inb_or(&len, fp) goto boardReadDone;
	if(len > ZZT_MESSAGE_SIZE)
		goto boardReadDone;
	board->info.message[0] = '\0';
	_zzt_inspad_or((char *)board->info.message, len, ZZT_MESSAGE_SIZE, fp) goto boardReadDone;
	_zzt_inb_or(&board->info.reenter_x, fp) goto boardReadDone;
	_zzt_inb_or(&board->info.reenter_y, fp) goto boardReadDone;
	board->info.reenter_x--;
	board->info.reenter_y--;
	_zzt_inw_or(&board->info.timelimit, fp) goto boardReadDone;
	_zzt_inspad_or(NULL, 0, 16, fp) goto boardReadDone;
	_zzt_inw_or(&w, fp) goto boardReadDone;
	board->info.paramcount = w+1;

	/* All the parameter records */
	board->params = malloc(sizeof(ZZTparam)*board->info.paramcount);
	memset(board->params, 0, sizeof(ZZTparam)*board->info.paramcount);
	for(i = 0; i < board->info.paramcount; i++) {
		_zzt_inb_or(&board->params[i].x, fp) goto boardReadDone;
		_zzt_inb_or(&board->params[i].y, fp) goto boardReadDone;
		board->params[i].x--;
		board->params[i].y--;
		_zzt_inw_or(&board->params[i].xstep, fp) goto boardReadDone;
		_zzt_inw_or(&board->params[i].ystep, fp) goto boardReadDone;
		_zzt_inw_or(&board->params[i].cycle, fp) goto boardReadDone;
		_zzt_inb_or(&board->params[i].data[0], fp) goto boardReadDone;
		_zzt_inb_or(&board->params[i].data[1], fp) goto boardReadDone;
		_zzt_inb_or(&board->params[i].data[2], fp) goto boardReadDone;
		_zzt_inw_or(&board->params[i].followerindex, fp) goto boardReadDone;
		_zzt_inw_or(&board->params[i].leaderindex, fp) goto boardReadDone;
		_zzt_inb_or(&board->params[i].utype, fp) goto boardReadDone;
		_zzt_inb_or(&board->params[i].ucolor, fp) goto boardReadDone;
		_zzt_inb_or(&board->params[i].magic[0], fp) goto boardReadDone;
		_zzt_inb_or(&board->params[i].magic[1], fp) goto boardReadDone;
		_zzt_inb_or(&board->params[i].magic[2], fp) goto boardReadDone;
		_zzt_inb_or(&board->params[i].magic[3], fp) goto boardReadDone;
		_zzt_inw_or(&board->params[i].instruction, fp) goto boardReadDone;
		_zzt_inw_or(&w, fp) goto boardReadDone;
		/* An object bound to another with param index i
		 * will have a program length of -i, or 65536 - i.
		 * An object will never be bound to the player (where i == 0) */
		if(w < (65535 - ZZT_BOARD_MAX_PARAMS + 1)) {
			board->params[i].length = w;
			board->params[i].bindindex = 0;
		} else {
			board->params[i].bindindex = -w;
			board->params[i].length = 0;
			w = 0;  /* Important: don't look for a program */
		}
		_zzt_inspad_or(NULL, 0, 8, fp) goto boardReadDone;
		if(w != 0) {
			board->params[i].program = malloc(w+1);
			_zzt_ins_or((char *)board->params[i].program, w, fp) goto boardReadDone;
			board->params[i].program[w] = '\0';
		}
	}

	/* Read the player x and y positions */
	board->plx = board->params[0].x;
	board->ply = board->params[0].y;
	success = true;

boardReadDone:
	if(!success) {
		_zzt_boardread_freestuff(board, packed);
		board = NULL;
	}

	/*
	 * Seek to the next board.  In a normal world, we're already at this position
	 * but if a board was corrupted there could be extra bytes between here and
	 * the next board.
	 */
	fseek(fp, board_end_offset, SEEK_SET);

	return board;
}

int zztBoardWrite(ZZTboard *board, FILE *fp)
{
	uint32_t size;
	uint16_t w;
	uint8_t b;
	int i, ofs;

	ZZTparam *p;

	/* First, determine size of board */
	size = 51;	/* Title length + Title + Pad */
	for(i = 0, ofs = 0; i < ZZT_BOARD_MAX_SIZE;) {
		i += board->packed[ofs];
		ofs += 3;
	}
	size += ofs;	/* Size of packed data */
	size += 0x58;	/* Board info */
	for(i = 0; i < board->info.paramcount; i++) {
		size += 0x21;			/* Object header */
		size += board->params[i].length;/* Object data */
	}

	/* Completely forbid saving boards which go over the 16-bit length */
	if (size > 65535) {
		return 0;
	}

	/* Write board header */
	w = size;
	_zzt_outw_ordie(&w, fp);
	b = strlen((char *)board->title);
	if (b > ZZT_BOARD_TITLE_SIZE)
		b = ZZT_BOARD_TITLE_SIZE;
	_zzt_outb_ordie(&b, fp);
	_zzt_outspad_ordie((char *)board->title, b, ZZT_BOARD_TITLE_SIZE, fp);
	/* Write board */
	_zzt_outs_ordie((char *)board->packed, ofs, fp);
	/* Write board info */
	_zzt_outb_ordie(&board->info.maxshots, fp);
	_zzt_outb_ordie(&board->info.darkness, fp);
	_zzt_outb_ordie(&board->info.board_n, fp);
	_zzt_outb_ordie(&board->info.board_s, fp);
	_zzt_outb_ordie(&board->info.board_w, fp);
	_zzt_outb_ordie(&board->info.board_e, fp);
	_zzt_outb_ordie(&board->info.reenter, fp);
	b = strlen((char *)board->info.message);
	_zzt_outb_ordie(&b, fp);
	_zzt_outspad_ordie((char *)board->info.message, b, ZZT_MESSAGE_SIZE, fp);
	/* Increment re-entry points by 1 before storing to file */
	board->info.reenter_x++;
	board->info.reenter_y++;
	_zzt_outb_ordie(&board->info.reenter_x, fp);
	_zzt_outb_ordie(&board->info.reenter_y, fp);
	/* Decrease again so that they are still usable */
	board->info.reenter_x--;
	board->info.reenter_y--;
	_zzt_outw_ordie(&board->info.timelimit, fp);
	_zzt_outspad_ordie(NULL, 0, 16, fp);
	w = (board->info.paramcount == 0) ? 0 : board->info.paramcount-1;
	_zzt_outw_ordie(&w, fp);
	/* Write parameter records */
	for(i = 0; i < board->info.paramcount; i++) {
		p = &board->params[i];
		b = p->x+1;
		_zzt_outb_ordie(&b, fp);
		b = p->y+1;
		_zzt_outb_ordie(&b, fp);
		_zzt_outw_ordie(&p->xstep, fp);
		_zzt_outw_ordie(&p->ystep, fp);
		_zzt_outw_ordie(&p->cycle, fp);
		_zzt_outb_ordie(&p->data[0], fp);
		_zzt_outb_ordie(&p->data[1], fp);
		_zzt_outb_ordie(&p->data[2], fp);
		_zzt_outw_ordie(&p->followerindex, fp);
		_zzt_outw_ordie(&p->leaderindex, fp);
		_zzt_outb_ordie(&p->utype, fp);
		_zzt_outb_ordie(&p->ucolor, fp);
		_zzt_outb_ordie(&p->magic[0], fp);
		_zzt_outb_ordie(&p->magic[1], fp);
		_zzt_outb_ordie(&p->magic[2], fp);
		_zzt_outb_ordie(&p->magic[3], fp);
		_zzt_outw_ordie(&p->instruction, fp);

		/* Ignore binding for anything with code */
		w = (p->length != 0 ? p->length : -p->bindindex);
		_zzt_outw_ordie(&w, fp);
		_zzt_outspad_ordie(NULL, 0, 8, fp);
		_zzt_outs_ordie((char *)p->program, p->length, fp);
	}
	return 1;
}
