/* patbuffer.c    -- Pattern buffer (backbuffer) utilities
 * $Id: patbuffer.c,v 1.5 2001/11/11 06:38:07 bitman Exp $
 * Copyright (C) 2000 Kev Vance <kev@kvance.com>
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

#include "kevedit.h"
#include "zzt.h"
#include "display.h"
#include "screen.h"

#include <stdlib.h>


/* Create an empty patbuffer of given size */
patbuffer* patbuffer_create(int size)
{
	int i;
	patbuffer* pbuf = NULL;

	if (size <= 0)
		return NULL;

	pbuf = (patbuffer *) malloc(sizeof(patbuffer));

	pbuf->patterns = (patdef *) malloc(sizeof(patdef) * size);
	pbuf->size = size;
	pbuf->pos = 0;

	for (i = 0; i < pbuf->size; i++) {
		pbuf->patterns[i].type = Z_EMPTY;
		pbuf->patterns[i].color = 0x07;
		pbuf->patterns[i].patparam = NULL;
	}

	return pbuf;
}

int deletepatternbuffer(patbuffer* pbuf)
{
	int i;

	/* Free all the patterns with patparam data */
	for (i = 0; i < pbuf->size; i++) {
		if (pbuf->patterns[i].patparam != NULL) {
			if (pbuf->patterns[i].patparam->moredata != NULL)
				free(pbuf->patterns[i].patparam->moredata);
			free(pbuf->patterns[i].patparam);
		}
	}

	/* Free the patterns themselves */
	free(pbuf->patterns);
	pbuf->patterns = NULL;
}

void patbuffer_resize(patbuffer * pbuf, int delta)
{
	int i;
	int resize = pbuf->size + delta;
	patdef * repat = NULL;

	if (resize <= 0)
		return;

	repat = (patdef *) malloc(sizeof(patdef) * resize);

	/* Copy over all the patterns. To save time, we won't bother the param
	 * data for those patterns which remain in the new buffer. */
	for (i = 0; i < resize && i < pbuf->size; i++)
		repat[i] = pbuf->patterns[i];

	/* pbuf shrunk: clear all param data beyond the new size */
	for (; i < pbuf->size; i++) {
		if (pbuf->patterns[i].patparam != NULL) {
			if (pbuf->patterns[i].patparam->moredata != NULL)
				free(pbuf->patterns[i].patparam->moredata);
			free(pbuf->patterns[i].patparam);
		}
	}

	/* pbuf grew: fill the new slots with empties */
	for (; i < resize; i++) {
		repat[i].type = Z_EMPTY;
		repat[i].color = 0x07;
		repat[i].patparam = NULL;
	}

	/* Out with the old, in with the new */
	free(pbuf->patterns);
	pbuf->patterns = repat;
	pbuf->size = resize;

	if (pbuf->pos >= pbuf->size)
		pbuf->pos = pbuf->size - 1;
}


/* Duplicate the given param if not NULL */
param* param_duplicate(param* p)
{
	param* dup = NULL;

	/* don't duplicate null params */
	if (p == NULL)
		return NULL;

	dup = (param *) malloc(sizeof(param));
	memcpy(dup, p, sizeof(param));
	if (p->moredata != NULL) {
		/* dup. the data, too */
		dup->moredata = (char *) malloc(p->length);
		memcpy(dup->moredata, p->moredata, p->length);
	}
	return dup;
}


/* Remove a parameter from board b having paramlist at pos (x, y) */
void param_remove(board* b, unsigned char paramlist[60][25], int x, int y)
{
	int i, j, t;
	param* currentParam = b->params[paramlist[x][y]];

	if (paramlist[x][y] == 0 || currentParam == NULL)
		return;

	if (currentParam->moredata != NULL)
		free(currentParam->moredata);
	free(currentParam);
	for (t = i = paramlist[x][y]; i < b->info->objectcount + 1; i++) {
		b->params[i] = b->params[i + 1];
	}
	for (j = 0; j < 25; j++) {
		for (i = 0; i < 60; i++) {
			if (paramlist[i][j] > t)
				paramlist[i][j]--;
		}
	}
	b->info->objectcount--;
	paramlist[x][y] = 0;
}


void pat_applycolordata(patbuffer * pbuf, editorinfo * myinfo)
{
	int i, colour;
	colour = (myinfo->backc << 4) + myinfo->forec;
	if (myinfo->blinkmode == 1)
		colour += 0x80;

	for (i = 0; i < pbuf->size; i++)
		pbuf->patterns[i].color = colour;
}


/* Push the given pattern attributes into a pattern buffer */
void push(patbuffer* pbuf, int type, int color, param * p)
{
	int i;
	patdef lastpat = pbuf->patterns[pbuf->size-1];

	if (lastpat.patparam != NULL) {
		if (lastpat.patparam->moredata != NULL)
			free(lastpat.patparam->moredata);
		free(lastpat.patparam);
	}
	
	/* Slide everything forward */
	for (i = pbuf->size - 1; i > 0; i--) {
		pbuf->patterns[i] = pbuf->patterns[i - 1];
	}

	pbuf->patterns[0].type = type;
	pbuf->patterns[0].color = color;
	pbuf->patterns[0].patparam = param_duplicate(p);
}


/* plots pattern at (x, y) in board b having bigboard and paramlist. 
 * Returns true when number of objects on board changes (panel should be
 * updated). Will plot over player, so be careful. */
int pat_plot(board* b, patdef pattern, int x, int y, u_int8_t * bigboard, unsigned char paramlist[60][25])
{
	int u = 0;

	if (b->info->objectcount >= 150 && pattern.patparam != NULL &&
	    paramlist[x][y] == 0) {
		return 0;
	}
	
	/* Clear existing parameter */
	if (paramlist[x][y] != 0) {
		/* We're overwriting a parameter */
		param_remove(b, paramlist, x, y);
		u = 1;
	}

	/* Plot the type & colour */
	bigboard[(x + y * 60) * 2] = pattern.type;
	bigboard[(x + y * 60) * 2 + 1] = pattern.color;

	/* Plot the parameter if applicable */
	if (pattern.patparam != NULL) {
		b->info->objectcount++;
		if (b->info->objectcount < 151) {
			param* newparam = param_duplicate(pattern.patparam);
			/* link param onto the current board's list */
			b->params[b->info->objectcount] = newparam;

			paramlist[x][y] = b->info->objectcount;
			newparam->x = x + 1;
			newparam->y = y + 1;
			u = 1;
		}
	}

	/* Oops, too many */
	if (b->info->objectcount >= 151) {
		bigboard[(x + y * 60) * 2] = Z_EMPTY;
		bigboard[(x + y * 60) * 2 + 1] = 0x07;
		b->info->objectcount--;
		u = 1;
	}
	return u;
}


void plot(world * myworld, editorinfo * myinfo, displaymethod * mydisplay, u_int8_t * bigboard, unsigned char paramlist[60][25])
{
	patbuffer* pbuf = myinfo->pbuf;
	patdef pattern = pbuf->patterns[pbuf->pos];

	/* No fair plotting over the player */
	if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
		return;

	/* Change the color to reflect state of default color mode */
	if (myinfo->defc == 0) {
		pattern.color = (myinfo->backc << 4) + myinfo->forec;
		if (myinfo->blinkmode == 1)
			pattern.color += 0x80;
	}

	if (pat_plot(myworld->board[myinfo->curboard], pattern, myinfo->cursorx, myinfo->cursory, bigboard, paramlist))
		updatepanel(mydisplay, myinfo, myworld);
}


void floodfill(world * myworld, editorinfo * myinfo, displaymethod * mydisplay, u_int8_t * bigboard, unsigned char paramlist[60][25], int xpos, int ypos, char code, u_int8_t colour, int fillmethod)
{
	patbuffer* pbuf = myinfo->pbuf;
	int whichPattern = pbuf->pos;
	patdef pattern;
	board* curboard = myworld->board[myinfo->curboard];

	if (xpos == myinfo->playerx && ypos == myinfo->playery)
		return;

	/* If fillmethod is negative, cycle through patterns backward.
	 * This produces a bizzare effect simply for the amusement of
	 * bitman. Practical applications: none. */
	if (fillmethod < 0) {
		fillmethod += 1;
		if (fillmethod >= 0)
			fillmethod = -pbuf->size;
		whichPattern = -fillmethod - 1;
	} else
	/* If fillmethod is positive, select a random pattern less than
	 * or equal to pbuf->size (random floodfill!) */
	if (fillmethod > 0) {
		whichPattern = rand() % pbuf->size;
	}

	/* Now that we know which pattern to use, copy its info so we
	 * can mess with the color. */
	pattern = pbuf->patterns[whichPattern];

	/* Find the target colour */
	if (myinfo->defc == 0) {
		pattern.color = (myinfo->backc << 4) + myinfo->forec;
		if (myinfo->blinkmode == 1)
			pattern.color += 0x80;
	}
	
	/* Make sure there's object space so we don't run into infinite recursion
	 * and crash bitman's computer again! */
	if (curboard->info->objectcount >= 150 && pattern.patparam != NULL &&
	    paramlist[xpos][ypos] == 0) {
		return;
	}

	/* Plot the pattern we chose at the proper position */
	pat_plot(curboard, pattern, xpos, ypos, bigboard, paramlist);

	if (xpos != 0) {
		if (bigboard[((xpos - 1) + ypos * 60) * 2] == code && (bigboard[((xpos - 1) + ypos * 60) * 2 + 1] == colour || bigboard[((xpos - 1) + ypos * 60) * 2] == Z_EMPTY))
			floodfill(myworld, myinfo, mydisplay, bigboard, paramlist, xpos - 1, ypos, code, colour, fillmethod);
	}
	if (xpos != 59) {
		if (bigboard[((xpos + 1) + ypos * 60) * 2] == code && (bigboard[((xpos + 1) + ypos * 60) * 2 + 1] == colour || bigboard[((xpos + 1) + ypos * 60) * 2] == Z_EMPTY))
			floodfill(myworld, myinfo, mydisplay, bigboard, paramlist, xpos + 1, ypos, code, colour, fillmethod);
	}
	if (ypos != 0) {
		if (bigboard[(xpos + (ypos - 1) * 60) * 2] == code && (bigboard[(xpos + (ypos - 1) * 60) * 2 + 1] == colour || bigboard[(xpos + (ypos - 1) * 60) * 2] == Z_EMPTY))
			floodfill(myworld, myinfo, mydisplay, bigboard, paramlist, xpos, ypos - 1, code, colour, fillmethod);
	}
	if (ypos != 24) {
		if (bigboard[(xpos + (ypos + 1) * 60) * 2] == code && (bigboard[(xpos + (ypos + 1) * 60) * 2 + 1] == colour || bigboard[(xpos + (ypos + 1) * 60) * 2] == Z_EMPTY))
			floodfill(myworld, myinfo, mydisplay, bigboard, paramlist, xpos, ypos + 1, code, colour, fillmethod);
	}
}
