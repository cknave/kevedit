/* paramed.c  -- Parameter editor
 * $Id: paramed.c,v 1.1 2002/02/18 08:04:40 bitman Exp $
 * Copyright (C) 2000 Ryan Phillips <bitman@scn.org>
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

#include "paramed.h"

#include "screen.h"
#include "editbox.h"

#include "libzzt2/zzt.h"
#include "help.h"

#include "display.h"

#include <stdlib.h>
#include <string.h>

void modifyparam(displaymethod * d, ZZTworld * w, int x, int y)
{
	ZZTtile tile = zztTileGet(w, x, y);
	if (tile.param == NULL)
		return;

	/* We have params; have at 'em! */
	if (tile.type == ZZT_OBJECT) {
		/* Modify object char */
		int csel = charselect(d, tile.param->data[0]);
		if (csel != -1)
			tile.param->data[0] = csel;
	}

	if (tile.type == ZZT_OBJECT || tile.type == ZZT_SCROLL) {
		/* Edit the program/text */
		editprogram(d, tile.param);
	}

	if (tile.type == ZZT_PASSAGE) {
		/* Change the passage's destination */
		tile.param->data[2] = boarddialog(w, tile.param->data[2], "Passage Destination", 0, d);
	}
	/* TODO: modify other params */
}

stringvector programtosvector(ZZTparam * p, int editwidth)
{
	stringvector sv;    /* list of strings */
	char *str = NULL;   /* temporary string */
	int strpos = 0;     /* position in str */
	int i;

	initstringvector(&sv);

	/* load the vector */
	if ((p->program == NULL) | (p->length <= 0)) {
		/* No data! We need to create an empty node */
		pushstring(&sv, str_dupmin("", editwidth + 1));
		return sv;
	}

	/* Let's fill the node from program! */
	strpos = 0;
	str = (char *) malloc(sizeof(char) * (editwidth + 1));

	for (i = 0; i < p->length; i++) {
		if (p->program[i] == 0x0d) {
			/* end of the line (heh); push the string and start over */
			str[strpos] = 0;
			pushstring(&sv, str);
			strpos = 0;
			str = (char *) malloc(sizeof(char) * (editwidth + 1));
		} else if (strpos > editwidth) {
			/* hmmm... really long line; must not have been made in ZZT... */
			/* let's truncate! */
			str[strpos] = 0;
			pushstring(&sv, str);
			strpos = 0;
			str = (char *) malloc(sizeof(char) * (editwidth + 1));
			/* move to next 0x0d */
			do i++; while (i < p->length && p->program[i] != 0x0d);
		} else {
			/* just your everyday copying... */
			str[strpos++] = p->program[i];
		}
	}

	if (strpos > 0) {
		/* strange... we seem to have an extra line with no CR at the end... */
		str[strpos] = 0;
		pushstring(&sv, str);
	} else {
		/* we grabbed all that RAM for nothing. Darn! */
		free(str);
	}

	return sv;
}

ZZTparam svectortoprogram(stringvector sv)
{
	ZZTparam p;
	int pos;

	/* find out how much space we need */
	p.length = 0;
	/* and now for a wierdo for loop... */
	for (sv.cur = sv.first; sv.cur != NULL; sv.cur = sv.cur->next)
		p.length += strlen(sv.cur->s) + 1;		/* + 1 for CR */

	if (p.length <= 1) {
		/* sv holds one empty string (it can happen) */
		p.program = NULL;
		p.length = 0;
		return p;
	}

	/* lets make room for all that program */
	pos = 0;
	p.program = (char *) malloc(sizeof(char) * p.length);

	for (sv.cur = sv.first; sv.cur != NULL; sv.cur = sv.cur->next) {
		int i;
		int linelen = strlen(sv.cur->s);	/* I feel efficient today */
		for (i = 0; i < linelen; i++) {
			p.program[pos++] = sv.cur->s[i];
		}
		p.program[pos++] = 0x0d;
	}

	return p;
}

void editprogram(displaymethod * d, ZZTparam * p)
{
	stringvector sv;
	ZZTparam newparam;

	sv = programtosvector(p, EDITBOX_ZZTWIDTH);

	/* Now that the node is full, we can edit it. */
	sv.cur = sv.first;	/* This is redundant, but hey. */
	editbox("Program Editor", &sv, EDITBOX_ZZTWIDTH, 1, d);

	/* Okay, let's put the vector back in program */
	newparam = svectortoprogram(sv);

	deletestringvector(&sv);
	if (p->program != NULL)
		free(p->program);

	p->length = newparam.length;
	p->program = newparam.program;
}

