/* libzzt.c     -- ZZT functions
 * $Id: libzzt.c,v 1.4 2000/09/02 04:33:23 kvance Exp $
 * Copyright (C) 1998-2000 Kev Vance <kvance@zeux.org>
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
#include <stdio.h>

#include "zzt.h"

#include <signal.h>

void z_delete(world * w)
{
	int i, t;

	for (i = 0; i < w->zhead->boardcount + 1; i++) {
		free(w->board[i]->title);
		for (t = 0; t < w->board[i]->info->objectcount + 1; t++) {
			if (w->board[i]->params[t]->length != 0)
				free(w->board[i]->params[t]->moredata);
			free(w->board[i]->params[t]);
		}
		free(w->board[i]->info);
		free(w->board[i]->data);
		free(w->board[i]);
	}
	free(w->zhead);
	free(w);
}

world *
 z_newworld()
{
	world *w;

	/* Allocate base world stuff */
	w = (world *) malloc(sizeof(world));
	w->zhead = (zztheader *) malloc(sizeof(zztheader));

	/* Set up ZZT header */
	memset(w->zhead, 0, sizeof(zztheader));
	w->zhead->magicnumber = 0xFFFF;
	w->zhead->boardcount = 0;
	w->zhead->health = 100;
	w->zhead->titlelength = 8;
	strcpy(w->zhead->title, "UNTITLED");

	return w;
}

board *
 z_newboard(char *title)
{
	int i;
	board *b;
	u_int8_t *bigboard;

	/* Allocate the board */
	b = (board *) malloc(sizeof(board));

	/* Create title & info */
	b->title = (u_int8_t *) malloc(strlen(title));
	strcpy(b->title, title);
	b->info = (boardinfo *) malloc(sizeof(boardinfo));
	memset(b->info, 0, sizeof(boardinfo));
	b->info->maxshots = 255;

	/* Make a blank board with the player in the upper-left corner */
	bigboard = (u_int8_t *) malloc(BOARD_MAX * 2);
	memset(bigboard, 0, BOARD_MAX * 2);
	bigboard[0] = Z_PLAYER;
	bigboard[1] = 0x1f;

	/* And RLE compress... */
	b->data = rle_encode(bigboard);

	/* Finally, create the parameter record for the player */
	b->params[0] = (param *) malloc(sizeof(param));
	memset(b->params[0], 0, sizeof(param));
	b->params[0]->x = b->params[0]->y = 1;
	b->params[0]->cycle = 1;
	b->params[0]->magic = 0xFFFFFFFF;

	/* There are no other params, let's make that clear */
	for (i = 1; i < 151; i++)
		b->params[i] = NULL;

	/* All done */
	return b;
}

param *z_newparam_bomb(int x, int y)
{
	param *p = (param *) malloc(sizeof(param));
	p->x = x;
	p->y = y;
	p->xstep = 0;
	p->ystep = 0;
	p->cycle = 6;
	p->data1 = 0;
	p->data2 = 0;
	p->data3 = 0;
	p->magic = 0xFFFFFFFF;
	p->undert = Z_EMPTY;
	p->underc = 0x07;
	p->unused = 0;
	p->instruction = 0;
	p->length = 0;
	memset(p->pad, 0, sizeof(p->pad));
	p->moredata = NULL;

	return p;
}

param *z_newparam_passage(int x, int y, int b)
{
	param *p = (param *) malloc(sizeof(param));
	p->x = x;
	p->y = y;
	p->xstep = 0;
	p->ystep = 0;
	p->cycle = 0;
	p->data1 = 0;
	p->data2 = 0;
	p->data3 = b;
	p->magic = 0xFFFFFFFF;
	p->undert = Z_EMPTY;
	p->underc = 0x07;
	p->unused = 0;
	p->instruction = 0;
	p->length = 0;
	memset(p->pad, 0, sizeof(p->pad));
	p->moredata = NULL;

	return p;
}

param *z_newparam_conveyer(int x, int y)
{
	/* All that's important here is cycle 3 */
	param *p = (param *) malloc(sizeof(param));
	p->x = x;
	p->y = y;
	p->xstep = 0;
	p->ystep = 0;
	p->cycle = 3;
	p->data1 = 0;
	p->data2 = 0;
	p->data3 = 0;
	p->magic = 0xFFFFFFFF;
	p->undert = Z_EMPTY;
	p->underc = 0x07;
	p->unused = 0;
	p->instruction = 0;
	p->length = 0;
	memset(p->pad, 0, sizeof(p->pad));
	p->moredata = NULL;

	return p;
}

param *z_newparam_object(int x, int y, int ch, int undert, int underc)
{
	param *p = (param *) malloc(sizeof(param));
	p->x = x;
	p->y = y;
	p->xstep = 0;
	p->ystep = 0;
	p->cycle = 3;
	p->data1 = ch;
	p->data2 = 0;
	p->data3 = 0;
	p->magic = 0xFFFFFFFF;
	p->undert = undert;
	p->underc = underc;
	p->unused = 0;
	p->instruction = 0;
	p->length = 0;
	memset(p->pad, 0, sizeof(p->pad));
	p->moredata = NULL;

	return p;
}

u_int8_t *
 rle_encode(u_int8_t * unclean)
{				/* rle encodes as per ZZT specs */
	u_int8_t *clean;
	unsigned char ccol, ctyp, col, typ;
	int cycles, i, uoff, coff, bytes, target;

	ctyp = typ = unclean[0];	/* exceptions           */
	ccol = col = unclean[1];	/* schmexceptions       */

	bytes = 512;		/* we malloc in chunks of 512 */
	target = 3;		/* we start with 3 bytes */

	clean = (u_int8_t *) malloc(bytes);

	for (i = uoff = coff = 0; i < BOARD_MAX; i += cycles) {
		cycles = -1;	/* duh, me am stupid too */
		while (ccol == col && ctyp == typ && cycles < 255) {
			typ = unclean[uoff++];
			col = unclean[uoff++];
			cycles++;
			if (i + cycles >= BOARD_MAX)
				break;	/* eof */
		}
		clean[coff++] = cycles;
		clean[coff++] = ctyp;
		clean[coff++] = ccol;	/* structure filling fun */
		target += 3;
		if (target >= bytes) {	/* we need more power, capn'! */
			bytes += 512;
			clean = (u_int8_t *) realloc(clean, bytes);
		}
		ctyp = typ;
		ccol = col;
		uoff -= 2;
	}
	return clean;		/* hooray, we are finished */
}

int rle_decode(u_int8_t * clean, u_int8_t * unclean)
{				/* and the decoder as well */
	int i, x, off;

	if (unclean == NULL)
		return 0;

	off = 0;		/* let's start at zero, eh? */

	for (i = 0; i < rle_lengthof(clean); i += 3) {
		for (x = 0; x < clean[i]; x++) {	/* lather  */
			unclean[off++] = clean[i + 1];	/* rinse   */
			unclean[off++] = clean[i + 2];	/* repeaat */
		}
	}

	return -1;		/* that wasn't so bad, now was it? */
}

long rle_lengthof(u_int8_t * clean)
{				/* how many bytes is an rle encoded *//* block? */
	int i;
	long bytes;
	i = bytes = 0;

	do {
		i += clean[bytes];
		bytes += 3;	/* lalala, follow the bytes */
	}
	while (i < BOARD_MAX);

	return bytes;		/* that was mildly entertaining */
}

u_int8_t
z_getchar(u_int8_t type, u_int8_t colour, param * p, u_int8_t * boardata, u_int8_t x, u_int8_t y)
{				/* what char? */
	switch (type) {
	case Z_EMPTY:
		return ' ';
	case Z_EDGE:
		return 'E';
	case Z_PLAYER:
		return 2;
	case Z_AMMO:
		return 132;
	case Z_TORCH:
		return 157;
	case Z_GEM:
		return 4;
	case Z_KEY:
		return 12;
	case Z_DOOR:
		return 10;
	case Z_SCROLL:
		return 232;
	case Z_PASSAGE:
		return 240;
	case Z_DUPLICATOR:
		return 250;
	case Z_BOMB:
		return 11;
	case Z_ENERGIZER:
		return 127;
	case Z_STAR:
		return '*';
	case Z_CWCONV:
		return 179;
	case Z_CCWCONV:
		return '\\';
	case Z_BULLET:
		return 248;
	case Z_WATER:
		return 176;
	case Z_FOREST:
		return 176;
	case Z_SOLID:
		return 219;
	case Z_NORMAL:
		return 178;
	case Z_BREAKABLE:
		return 177;
	case Z_BOULDER:
		return 254;
	case Z_NSSLIDER:
		return 18;
	case Z_EWSLIDER:
		return 29;
	case Z_FAKE:
		return 178;
	case Z_INVISIBLE:
		return 176;
	case Z_BLINK:
		return 206;
	case Z_TRANSPORTER:
		if (p->xstep == 0xFFFF)
			return '<';
		if (p->xstep == 0x0001)
			return '>';
		if (p->ystep == 0xFFFF)
			return '^';
		return 'v';
		break;
	case Z_LINE:
		return (boardata == NULL) ? 206 : linechar(boardata, x, y);
	case Z_RICOCHET:
		return '*';
	case Z_BLINKHORIZ:
		return 205;
	case Z_BEAR:
		return 153;
	case Z_RUFFIAN:
		return 5;
	case Z_OBJECT:
		return p->data1;
	case Z_SLIME:
		return '*';
	case Z_SHARK:
		return '^';
	case Z_SPINNINGGUN:
		return 24;
	case Z_PUSHER:
		if (p->xstep == 0xFFFF)
			return 17;
		if (p->xstep == 0x0001)
			return 16;
		if (p->ystep == 0xFFFF)
			return 30;
		return 31;
	case Z_LION:
		return 234;
	case Z_TIGER:
		return 227;
	case Z_BLINKVERT:
		return 186;
	case Z_CENTHEAD:
		return 233;
	case Z_CENTBODY:
		return 'O';
	case Z_BLUETEXT:
		return colour;
	case Z_GREENTEXT:
		return colour;
	case Z_CYANTEXT:
		return colour;
	case Z_REDTEXT:
		return colour;
	case Z_PURPLETEXT:
		return colour;
	case Z_YELLOWTEXT:
		return colour;
	case Z_WHITETEXT:
		return colour;
	case Z_BBLUETEXT:
		return colour;
	case Z_BGREENTEXT:
		return colour;
	case Z_BCYANTEXT:
		return colour;
	case Z_BREDTEXT:
		return colour;
	case Z_BPURPLETEXT:
		return colour;
	case Z_BYELLOWTEXT:
		return colour;
	case Z_BWHITETEXT:
		return colour;
	}
	return '?';
}


u_int8_t
linechar(u_int8_t * b, u_int8_t x, u_int8_t y)
{
	u_int8_t flags;

	flags = 0;

	if (y == 0)
		flags += 1;
	else if (b[(x + (y - 1) * 60) * 2] == Z_LINE)
		flags += 1;

	if (y == 24)
		flags += 2;
	else if (b[(x + (y + 1) * 60) * 2] == Z_LINE)
		flags += 2;

	if (x == 0)
		flags += 4;
	else if (b[(x - 1 + y * 60) * 2] == Z_LINE)
		flags += 4;
	if (x == 59)
		flags += 8;
	else if (b[(x + 1 + y * 60) * 2] == Z_LINE)
		flags += 8;

	if (flags == 0)		/*  (none) */
		return 249;
	if (flags == 1)		/* n       */
		return 208;
	if (flags == 2)		/*   s     */
		return 210;
	if (flags == 3)		/* n s     */
		return 186;
	if (flags == 4)		/*     w   */
		return 181;
	if (flags == 5)		/* n   w   */
		return 188;
	if (flags == 6)		/*   s w   */
		return 187;
	if (flags == 7)		/* n s w   */
		return 185;
	if (flags == 8)		/*       e */
		return 198;
	if (flags == 9)		/* n     e */
		return 200;
	if (flags == 10)	/*   s   e */
		return 201;
	if (flags == 11)	/* n s   e */
		return 204;
	if (flags == 12)	/*     w e */
		return 205;
	if (flags == 13)	/* n   w e */
		return 202;
	if (flags == 14)	/*   s w e */
		return 203;
	if (flags == 15)	/* n s w e */
		return 206;
	return 206;		/* ??? */
}

u_int8_t
z_getcolour(u_int8_t type, u_int8_t colour, param * p)
{				/* what colour? */
	switch (type) {
	case Z_EMPTY:
		return 0x07;
	case Z_EDGE:
		return 0x4C;
	case Z_PLAYER:
		return 0x1f;
	case Z_AMMO:
		return colour;
	case Z_TORCH:
		return colour;
	case Z_GEM:
		return colour;
	case Z_KEY:
		return colour;
	case Z_DOOR:
		return colour;
	case Z_SCROLL:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_PASSAGE:
		return colour;
	case Z_DUPLICATOR:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_BOMB:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_ENERGIZER:
		return colour;
	case Z_STAR:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_CWCONV:
		return colour;
	case Z_CCWCONV:
		return colour;
	case Z_BULLET:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_WATER:
		return colour;
	case Z_FOREST:
		return colour;
	case Z_SOLID:
		return colour;
	case Z_NORMAL:
		return colour;
	case Z_BREAKABLE:
		return colour;
	case Z_BOULDER:
		return colour;
	case Z_NSSLIDER:
		return colour;
	case Z_EWSLIDER:
		return colour;
	case Z_FAKE:
		return colour;
	case Z_INVISIBLE:
		return colour;
	case Z_BLINK:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_TRANSPORTER:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_LINE:
		return colour;
	case Z_RICOCHET:
		return colour;
	case Z_BLINKHORIZ:
		return colour;
	case Z_BEAR:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_RUFFIAN:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_OBJECT:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_SLIME:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_SHARK:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_SPINNINGGUN:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_PUSHER:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_LION:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_TIGER:
		return p->undert != Z_EMPTY ?
		    (p->underc & 0xf0) + (colour & 0x0f) : colour;
	case Z_BLINKVERT:
		return colour;
	case Z_CENTHEAD:
		return p->undert != Z_EMPTY ?
		    (p->underc * 0xf0) + (colour * 0x0f) : colour;
	case Z_CENTBODY:
		return p->undert != Z_EMPTY ?
		    (p->underc * 0xf0) + (colour * 0x0f) : colour;
	case Z_BLUETEXT:
		return 0x1f;
	case Z_GREENTEXT:
		return 0x2f;
	case Z_CYANTEXT:
		return 0x3f;
	case Z_REDTEXT:
		return 0x4f;
	case Z_PURPLETEXT:
		return 0x5f;
	case Z_YELLOWTEXT:
		return 0x6f;
	case Z_WHITETEXT:
		return 0x0f;
	case Z_BBLUETEXT:
		return 0x9f;
	case Z_BGREENTEXT:
		return 0xaf;
	case Z_BCYANTEXT:
		return 0xbf;
	case Z_BREDTEXT:
		return 0xcf;
	case Z_BPURPLETEXT:
		return 0xdf;
	case Z_BYELLOWTEXT:
		return 0xef;
	case Z_BWHITETEXT:
		return 0xff;
	}
	return 0xcc;
}

int saveworld(char *fname, world * wo)
{
	FILE *fp;

	long ofs1, ofs2;

	int i, t;
	u_int8_t x;
	u_int16_t w;
	size_t s;

	param temp;

	fp = fopen(fname, "wb");
	if (fp == NULL)
		return 0;

	/* write ZZT file header */
	fwrite(&wo->zhead->magicnumber, sizeof(u_int16_t), 1, fp);
	fwrite(&wo->zhead->boardcount, sizeof(u_int16_t), 1, fp);
	fwrite(&wo->zhead->ammo, sizeof(u_int16_t), 1, fp);
	fwrite(&wo->zhead->gems, sizeof(u_int16_t), 1, fp);

	fwrite(&wo->zhead->bluekey, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->greenkey, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->cyankey, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->redkey, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->purplekey, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->yellowkey, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->whitekey, sizeof(u_int8_t), 1, fp);

	fwrite(&wo->zhead->health, sizeof(u_int16_t), 1, fp);
	fwrite(&wo->zhead->startboard, sizeof(u_int16_t), 1, fp);
	fwrite(&wo->zhead->torches, sizeof(u_int16_t), 1, fp);
	fwrite(&wo->zhead->torchcycles, sizeof(u_int16_t), 1, fp);
	fwrite(&wo->zhead->energizercycles, sizeof(u_int16_t), 1, fp);
	fwrite(&wo->zhead->unknown, sizeof(u_int16_t), 1, fp);
	fwrite(&wo->zhead->score, sizeof(u_int16_t), 1, fp);

	fwrite(&wo->zhead->titlelength, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->title, 20, 1, fp);

	fwrite(&wo->zhead->flag1len, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->flag1, 20, 1, fp);
	fwrite(&wo->zhead->flag2len, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->flag2, 20, 1, fp);
	fwrite(&wo->zhead->flag3len, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->flag3, 20, 1, fp);
	fwrite(&wo->zhead->flag4len, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->flag4, 20, 1, fp);
	fwrite(&wo->zhead->flag5len, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->flag5, 20, 1, fp);
	fwrite(&wo->zhead->flag6len, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->flag6, 20, 1, fp);
	fwrite(&wo->zhead->flag7len, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->flag7, 20, 1, fp);
	fwrite(&wo->zhead->flag8len, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->flag8, 20, 1, fp);
	fwrite(&wo->zhead->flag9len, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->flag9, 20, 1, fp);
	fwrite(&wo->zhead->flag10len, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->flag10, 20, 1, fp);

	fwrite(&wo->zhead->timepassed, sizeof(u_int16_t), 1, fp);
	fwrite(&wo->zhead->nothing, sizeof(u_int16_t), 1, fp);
	fwrite(&wo->zhead->savebyte, sizeof(u_int8_t), 1, fp);
	fwrite(&wo->zhead->pad, 247, 1, fp);

	for (i = 0; i < wo->zhead->boardcount + 1; i++) {
		/* board header */
		ofs1 = ftell(fp);	/* we need this later */
		fwrite(&wo->zhead->nothing, sizeof(u_int16_t), 1, fp);
		x = strlen(wo->board[i]->title);
		fwrite(&x, sizeof(u_int8_t), 1, fp);
		fwrite(wo->board[i]->title, 50, 1, fp);
		fwrite(wo->board[i]->data, rle_lengthof(wo->board[i]->data), 1, fp);

		/* board info */
		fwrite(&wo->board[i]->info->maxshots, sizeof(u_int8_t), 1, fp);
		fwrite(&wo->board[i]->info->darkness, sizeof(u_int8_t), 1, fp);
		fwrite(&wo->board[i]->info->board_n, sizeof(u_int8_t), 1, fp);
		fwrite(&wo->board[i]->info->board_s, sizeof(u_int8_t), 1, fp);
		fwrite(&wo->board[i]->info->board_w, sizeof(u_int8_t), 1, fp);
		fwrite(&wo->board[i]->info->board_e, sizeof(u_int8_t), 1, fp);
		fwrite(&wo->board[i]->info->reenter, sizeof(u_int8_t), 1, fp);
		fwrite(&wo->board[i]->info->messagelen, sizeof(u_int8_t), 1, fp);
		fwrite(&wo->board[i]->info->message, 58, 1, fp);
		fwrite(&wo->board[i]->info->unknown, sizeof(u_int16_t), 1, fp);
		fwrite(&wo->board[i]->info->timelimit, sizeof(u_int16_t), 1, fp);
		fwrite(&wo->board[i]->info->pad, 16, 1, fp);
		fwrite(&wo->board[i]->info->objectcount, sizeof(u_int16_t), 1, fp);

		/* parameters */
		for (t = 0; t < wo->board[i]->info->objectcount + 1; t++) {
			fwrite(&wo->board[i]->params[t]->x, sizeof(u_int8_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->y, sizeof(u_int8_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->xstep, sizeof(u_int16_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->ystep, sizeof(u_int16_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->cycle, sizeof(u_int16_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->data1, sizeof(u_int8_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->data2, sizeof(u_int8_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->data3, sizeof(u_int8_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->magic, sizeof(u_int32_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->undert, sizeof(u_int8_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->underc, sizeof(u_int8_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->unused, sizeof(u_int32_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->instruction, sizeof(u_int16_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->length, sizeof(u_int16_t), 1, fp);
			fwrite(&wo->board[i]->params[t]->pad, 8, 1, fp);
			if (wo->board[i]->params[t]->length != 0) {
				fwrite(wo->board[i]->params[t]->moredata, wo->board[i]->params[t]->length, 1, fp);
			}
		}

		ofs2 = ftell(fp);
		w = ofs2 - ofs1 - 2;
		fseek(fp, ofs1, SEEK_SET);
		fwrite(&w, sizeof(u_int16_t), 1, fp);	/* pesky board size */
		fseek(fp, ofs2, SEEK_SET);
	}

	/* that's enough of that */
	fclose(fp);

	return -1;
}

world *
 loadworld(char *fname)
{				/* load world */
	FILE *fp;
	long pos;
	int i, t;

	u_int16_t bsize;
	u_int8_t tsize;
	long realsize;

	char *tempbuffer, *pointer;
	world *wo;

	fp = fopen(fname, "rb");
	if (fp == NULL)
		return NULL;

	tempbuffer = (char *) malloc(64 * 1024);
	if (tempbuffer == NULL)
		return NULL;

	wo = (world *) malloc(sizeof(world));
	wo->zhead = (zztheader *) malloc(sizeof(zztheader));

	/* Read header */
	fread(&wo->zhead->magicnumber, sizeof(u_int16_t), 1, fp);
	if (wo->zhead->magicnumber != 0xffff) {
		fclose(fp);
		free(wo->zhead);
		free(wo);
		free(tempbuffer);
		return NULL;
	}
	fread(&wo->zhead->boardcount, sizeof(u_int16_t), 1, fp);
	fread(&wo->zhead->ammo, sizeof(u_int16_t), 1, fp);
	fread(&wo->zhead->gems, sizeof(u_int16_t), 1, fp);

	fread(&wo->zhead->bluekey, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->greenkey, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->cyankey, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->redkey, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->purplekey, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->yellowkey, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->whitekey, sizeof(u_int8_t), 1, fp);

	fread(&wo->zhead->health, sizeof(u_int16_t), 1, fp);
	fread(&wo->zhead->startboard, sizeof(u_int16_t), 1, fp);
	fread(&wo->zhead->torches, sizeof(u_int16_t), 1, fp);
	fread(&wo->zhead->torchcycles, sizeof(u_int16_t), 1, fp);
	fread(&wo->zhead->energizercycles, sizeof(u_int16_t), 1, fp);
	fread(&wo->zhead->unknown, sizeof(u_int16_t), 1, fp);
	fread(&wo->zhead->score, sizeof(u_int16_t), 1, fp);

	fread(&wo->zhead->titlelength, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->title, 20, 1, fp);
	if (wo->zhead->titlelength < 9) {
		wo->zhead->title[wo->zhead->titlelength] = '\0';
	} else {
		wo->zhead->title[8] = '\0';
	}

	fread(&wo->zhead->flag1len, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->flag1, 20, 1, fp);
	fread(&wo->zhead->flag2len, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->flag2, 20, 1, fp);
	fread(&wo->zhead->flag3len, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->flag3, 20, 1, fp);
	fread(&wo->zhead->flag4len, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->flag4, 20, 1, fp);
	fread(&wo->zhead->flag5len, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->flag5, 20, 1, fp);
	fread(&wo->zhead->flag6len, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->flag6, 20, 1, fp);
	fread(&wo->zhead->flag7len, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->flag7, 20, 1, fp);
	fread(&wo->zhead->flag8len, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->flag8, 20, 1, fp);
	fread(&wo->zhead->flag9len, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->flag9, 20, 1, fp);
	fread(&wo->zhead->flag10len, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->flag10, 20, 1, fp);

	fread(&wo->zhead->timepassed, sizeof(u_int16_t), 1, fp);
	fread(&wo->zhead->nothing, sizeof(u_int16_t), 1, fp);
	fread(&wo->zhead->savebyte, sizeof(u_int8_t), 1, fp);
	fread(&wo->zhead->pad, 247, 1, fp);

	for (i = 0; i < wo->zhead->boardcount + 1; i++) {
		wo->board[i] = (board *) malloc(sizeof(board));
		fread(&bsize, sizeof(u_int16_t), 1, fp);
		pos = ftell(fp);
		fread(&tsize, sizeof(u_int8_t), 1, fp);
		wo->board[i]->title = (u_int8_t *) malloc(50);
		wo->board[i]->info = (boardinfo *) malloc(sizeof(boardinfo));

		fread(wo->board[i]->title, 50, 1, fp);
		if (tsize < 51) {
			wo->board[i]->title[tsize] = '\0';
		} else {
			wo->board[i]->title[32] = '\0';
		}

		fread(tempbuffer, bsize, 1, fp);
		realsize = rle_lengthof(tempbuffer);
		wo->board[i]->data = (u_int8_t *) malloc(realsize);
		memcpy(wo->board[i]->data, tempbuffer, realsize);
		pointer = tempbuffer + realsize;

		/* board info */
		memcpy(&wo->board[i]->info->maxshots, pointer++, 1);
		memcpy(&wo->board[i]->info->darkness, pointer++, 1);
		memcpy(&wo->board[i]->info->board_n, pointer++, 1);
		memcpy(&wo->board[i]->info->board_s, pointer++, 1);
		memcpy(&wo->board[i]->info->board_w, pointer++, 1);
		memcpy(&wo->board[i]->info->board_e, pointer++, 1);
		memcpy(&wo->board[i]->info->reenter, pointer++, 1);
		memcpy(&wo->board[i]->info->messagelen, pointer++, 1);
		memcpy(&wo->board[i]->info->message, pointer, 58);
		pointer += 58;
		memcpy(&wo->board[i]->info->unknown, pointer, 2);
		pointer += 2;
		memcpy(&wo->board[i]->info->timelimit, pointer, 2);
		pointer += 2;
		memcpy(&wo->board[i]->info->pad, pointer, 16);
		pointer += 16;
		memcpy(&wo->board[i]->info->objectcount, pointer, 2);
		pointer += 2;

		/* parameters */
		for (t = 0; t < wo->board[i]->info->objectcount + 1; t++) {
			wo->board[i]->params[t] = (param *) malloc(sizeof(param));
			memcpy(&wo->board[i]->params[t]->x, pointer++, 1);
			memcpy(&wo->board[i]->params[t]->y, pointer++, 1);
			memcpy(&wo->board[i]->params[t]->xstep, pointer, 2);
			pointer += 2;
			memcpy(&wo->board[i]->params[t]->ystep, pointer, 2);
			pointer += 2;
			memcpy(&wo->board[i]->params[t]->cycle, pointer, 2);
			pointer += 2;
			memcpy(&wo->board[i]->params[t]->data1, pointer++, 1);
			memcpy(&wo->board[i]->params[t]->data2, pointer++, 1);
			memcpy(&wo->board[i]->params[t]->data3, pointer++, 1);
			memcpy(&wo->board[i]->params[t]->magic, pointer, 4);
			pointer += 4;
			memcpy(&wo->board[i]->params[t]->undert, pointer++, 1);
			memcpy(&wo->board[i]->params[t]->underc, pointer++, 1);
			memcpy(&wo->board[i]->params[t]->unused, pointer, 4);
			pointer += 4;
			memcpy(&wo->board[i]->params[t]->instruction, pointer, 2);
			pointer += 2;
			memcpy(&wo->board[i]->params[t]->length, pointer, 2);
			pointer += 2;
			memcpy(&wo->board[i]->params[t]->pad, pointer, 8);
			pointer += 8;
			if (wo->board[i]->params[t]->length != 0) {
				wo->board[i]->params[t]->moredata = malloc(wo->board[i]->params[t]->length);
				memcpy(wo->board[i]->params[t]->moredata, pointer, wo->board[i]->params[t]->length);
				pointer += wo->board[i]->params[t]->length;
			} else {
				wo->board[i]->params[t]->moredata = NULL;
			}
		}

		//      fseek(fp, pos, SEEK_SET);
		//      fseek(fp, bsize, SEEK_CUR);
		fseek(fp, -51, SEEK_CUR);
	}

	/* all done */
	fclose(fp);
	free(tempbuffer);

	return wo;
}
