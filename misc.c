/* misc.c       -- General routines for everyday KevEditing
 * $Id: misc.c,v 1.13 2001/11/09 01:15:09 bitman Exp $
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
#include "display.h"
#include "svector.h"
#include "editbox.h"
#include "screen.h"
#include "patbuffer.h"
#include "hypertxt.h"
#include "misc.h"
#include "selection.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>


displaymethod * pickdisplay(displaymethod * rootdisplay)
{
	int x, i;
	displaymethod* mydisplay = rootdisplay;
	char *string = (char *) malloc(sizeof(char) * 256);

	/* How many display methods? */
	for (x = 1; x < 9; x++) {
		if (mydisplay->next == NULL) {
			break;
		}
		mydisplay = mydisplay->next;
	}

	if (x > 1) {
		/* More than 1 display method available, user must choose */
		printf("Hi.  This seems to be your first time running KevEdit.  What display method\n"
		       "works best on your platform?\n\n");

		mydisplay = rootdisplay;
		for (i = 0; i < x; i++) {
			printf("[%d] %s\n", i + 1, mydisplay->name);
			if (mydisplay->next != NULL)
				mydisplay = mydisplay->next;
		}
		do {
			printf("\nSelect [1-%i]: ", x);
			fgets(string, 255, stdin);
			i = string[0];
		}
		while (i > 49 && i < 51);
		i -= '1';
		printf("\n%d SELECT\n", i);
		mydisplay = rootdisplay;
		for (x = 0; x < i; x++) {
			mydisplay = mydisplay->next;
		}
	} else {
		mydisplay = rootdisplay;
	}

	free(string);
	return mydisplay;
}


void initeditorinfo(editorinfo * myinfo)
{
	/* Clear info to default values */

	myinfo->cursorx = myinfo->playerx = 0;
	myinfo->cursory = myinfo->playery = 0;
	myinfo->drawmode = 0;
	myinfo->gradmode = 0;
	myinfo->aqumode = 0;
	myinfo->blinkmode = 0;
	myinfo->textentrymode = 0;
	myinfo->defc = 1;
	myinfo->forec = 0x0f;
	myinfo->backc = 0x00;
	myinfo->standard_patterns = patbuffer_create(6);
	myinfo->backbuffer = patbuffer_create(10);
	myinfo->pbuf = myinfo->standard_patterns;

	myinfo->currenttitle = (char *) malloc(21);
	strcpy(myinfo->currenttitle, "UNTITLED");
	myinfo->currentfile = (char *) malloc(14);
	strcpy(myinfo->currentfile, "untitled.zzt");

	myinfo->curboard = 0;

	/* Initialize pattern definitions */
	myinfo->standard_patterns->patterns[0].type = Z_SOLID;
	myinfo->standard_patterns->patterns[1].type = Z_NORMAL;
	myinfo->standard_patterns->patterns[2].type = Z_BREAKABLE;
	myinfo->standard_patterns->patterns[3].type = Z_WATER;
	myinfo->standard_patterns->patterns[4].type = Z_EMPTY;
	myinfo->standard_patterns->patterns[5].type = Z_LINE;
	pat_applycolordata(myinfo->standard_patterns, myinfo);
}


void showParamData(param * p, int paramNumber, displaymethod * d)
{
	char buffer[50];
	stringvector data;
	initstringvector(&data);

	pushstring(&data, str_dup("$Param Data"));
	pushstring(&data, str_dup(""));

	sprintf(buffer, "param#:      %d / 150", paramNumber);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "x:           %d", p->x);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "y:           %d", p->y);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "xstep:       %d", p->xstep);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "ystep:       %d", p->ystep);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "cycle:       %d", p->cycle);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "data1:       %d", p->data1);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "data2:       %d", p->data2);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "data3:       %d", p->data3);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "magic:       %d", p->magic);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "undert:      0x%X", p->undert);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "underc:      0x%X", p->underc);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "instruction: %d", p->instruction);
	pushstring(&data, str_dup(buffer));
	sprintf(buffer, "length:      %d", p->length);
	pushstring(&data, str_dup(buffer));

	editbox("Param Data", &data, 0, 1, d);

	deletestringvector(&data);
}


void texteditor(displaymethod * mydisplay)
{
	/* open text file for edit */
	char buffer[14];
	stringvector editvector;

	strcpy(buffer, "");
	initstringvector(&editvector);
	editbox("Text Editor", &editvector, 42, 0, mydisplay);
#if 0
	/* this code has fallen out of bitman's favour */
	if (filenamedialog(buffer, "Save As", "", 1, mydisplay) != NULL)
		svectortofile(&editvector, buffer);
#endif
	deletestringvector(&editvector);
}


void clearboard(world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	int i, x;
	char * buffer;
	board * curboard;

	curboard = myworld->board[myinfo->curboard];
	for (i = 0; i < curboard->info->objectcount + 1; i++) {
		if (curboard->params[i]->moredata != NULL)
			free(curboard->params[i]->moredata);
		free(curboard->params[i]);
	}
	free(curboard->data);
	free(curboard->info);
	buffer = (char *) malloc(sizeof(char) * (strlen(curboard->title) + 2));
	strcpy(buffer, curboard->title);
	free(curboard->title);
	free(curboard);

	curboard = z_newboard(buffer);
	myworld->board[myinfo->curboard] = curboard;

	free(buffer);
	rle_decode(curboard->data, bigboard);
	for (i = 0; i < 25; i++)
		for (x = 0; x < 60; x++)
			paramlist[x][i] = 0;

	myinfo->playerx = curboard->params[0]->x - 1;
	myinfo->playery = curboard->params[0]->y - 1;
}


world * clearworld(world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	int i, x;

	z_delete(myworld);
	myworld = z_newworld();
	myworld->board[0] = z_newboard("KevEdit World");
	rle_decode(myworld->board[0]->data, bigboard);
	for (i = 0; i < 25; i++)
		for (x = 0; x < 60; x++)
			paramlist[x][i] = 0;
	strcpy(myinfo->currenttitle, "UNTITLED");
	strcpy(myinfo->currentfile,  "untitled.zzt");
	myinfo->curboard = 0;

	myinfo->playerx = myworld->board[0]->params[0]->x - 1;
	myinfo->playery = myworld->board[0]->params[0]->y - 1;

	return myworld;
}


int toggledrawmode(editorinfo * myinfo)
{
	if (myinfo->gradmode != 0) {
		/* If grad mode is on, turn it and drawmode off */
		myinfo->gradmode = myinfo->drawmode = 0;
	} else {
		/* Otherwise toggle draw mode */
		myinfo->drawmode ^= 1;
	}
	/* Get mode should go off either way */
	myinfo->aqumode = 0;

	return myinfo->drawmode;
}


int togglegradientmode(editorinfo * myinfo)
{
	/* Toggle gradient mode - pattern changes with each cursor
	 * movement & drawmode is turned on. */
	if (myinfo->pbuf == myinfo->standard_patterns)
		return 0;     /* Cycling through standard_patterns not yet supported */

	myinfo->aqumode = 0;
	if (myinfo->gradmode != 0) {
		/* Gradmode is already on, advance once and reverse it */
		if (myinfo->gradmode < 0) {
			if (--myinfo->pbuf->pos < 0)
				myinfo->pbuf->pos = myinfo->pbuf->size - 1;
		} else {
			if (++myinfo->pbuf->pos >= myinfo->pbuf->size)
				myinfo->pbuf->pos = 0;
		}
		myinfo->gradmode = -(myinfo->gradmode);

		return 0;    /* No reason to start plotting yet */
	} else {
		/* Turn gradmode & drawmode on */
		myinfo->drawmode = 1;
		/* Gradmode cycles backward by default */
		myinfo->gradmode = -1;
		return 1;    /* Gradmode went on -- let the plotting begin! */
	}
}


void changeboard(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	int i, x;

	/* Switch boards */
	i = switchboard(myworld, myinfo, mydisplay);
	if (i == -1 || i == myinfo->curboard)
		return;

	/* It may become useful to put the following code in another function for
	 * greater flexibility in the future. Presently, such provides no gains. */
	free(myworld->board[myinfo->curboard]->data);
	myworld->board[myinfo->curboard]->data = rle_encode(bigboard);
	myinfo->curboard = i;
	if (myinfo->curboard > myworld->zhead->boardcount)
		myinfo->curboard = 0;
	rle_decode(myworld->board[myinfo->curboard]->data, bigboard);
	for (i = 0; i < 25; i++)
		for (x = 0; x < 60; x++)
			paramlist[x][i] = 0;
	for (i = 0; i < myworld->board[myinfo->curboard]->info->objectcount + 1; i++) {
		if (myworld->board[myinfo->curboard]->params[i]->x > 0 && myworld->board[myinfo->curboard]->params[i]->x < 61 && myworld->board[myinfo->curboard]->params[i]->y > 0 && myworld->board[myinfo->curboard]->params[i]->y < 26)
			paramlist[myworld->board[myinfo->curboard]->params[i]->x - 1][myworld->board[myinfo->curboard]->params[i]->y - 1] = i;
	}

	myinfo->playerx = myworld->board[myinfo->curboard]->params[0]->x - 1;
	myinfo->playery = myworld->board[myinfo->curboard]->params[0]->y - 1;
}


void saveworldprompt(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard)
{
	/* Save World after prompting user for filename */
	int i;
	char* filename;
	char* path, * file;

	filename =
		filenamedialog(myinfo->currentfile, "zzt", "Save World As", 1, mydisplay);

	if (filename == NULL)
		return;

	path = (char*) malloc(sizeof(char) * (strlen(filename) + 1));
	file = (char*) malloc(sizeof(char) * (strlen(filename) + 1));

	fileof(file, filename, strlen(filename) + 1);
	pathof(path, filename, strlen(filename) + 1);

	/* Update board data to reflect bigboard */
	free(myworld->board[myinfo->curboard]->data);
	myworld->board[myinfo->curboard]->data = rle_encode(bigboard);

	/* Copy filename w/o ext onto world's title */
	for (i = 0; i < 9 && file[i] != '.' && file[i] != '\0'; i++) {
		myinfo->currenttitle[i] = file[i];
	}
	myinfo->currenttitle[i] = '\0';

	/* Change internal name of world only on first use of save */
	if (!strcmp(myworld->zhead->title, "UNTITLED")) {
		strcpy(myworld->zhead->title, myinfo->currenttitle);
		myworld->zhead->titlelength = strlen(myworld->zhead->title);
	}

	strcpy(myinfo->currentfile, file);

	myworld->zhead->startboard = myinfo->curboard;
	saveworld(filename, myworld);

	/* Switch the current directory to the same location as the file */
	chdir(path);

	free(filename);
	free(path);
	free(file);

	mydisplay->print(61, 5, 0x1f, "Written.");
	mydisplay->cursorgo(69, 5);
	mydisplay->getch();
}


void updateparamlist(world * myworld, editorinfo * myinfo, unsigned char paramlist[60][25])
{
	int i, x;
	board* curboard = myworld->board[myinfo->curboard];

	for (i = 0; i < 25; i++)
		for (x = 0; x < 60; x++)
			paramlist[x][i] = 0;

	for (i = 0; i < curboard->info->objectcount + 1; i++) {
		if (curboard->params[i]->x > 0 && curboard->params[i]->x < 61 && curboard->params[i]->y > 0 && curboard->params[i]->y < 26)
			paramlist[curboard->params[i]->x - 1][curboard->params[i]->y - 1] = i;
	}

	myinfo->playerx = myworld->board[myinfo->curboard]->params[0]->x - 1;
	myinfo->playery = myworld->board[myinfo->curboard]->params[0]->y - 1;
}


void updateinfo(world * myworld, editorinfo * myinfo, char * bigboard)
{
	/* TODO: Should the current title be filename based or "title" based */
	strncpy(myinfo->currenttitle, myworld->zhead->title, 20);
	myinfo->currenttitle[myworld->zhead->titlelength] = '\0';
	myinfo->curboard = myworld->zhead->startboard;
	rle_decode(myworld->board[myworld->zhead->startboard]->data, bigboard);
}


void previouspattern(editorinfo * myinfo)
{
	myinfo->pbuf->pos--;
	if (myinfo->pbuf->pos == -1) {
		if (myinfo->pbuf == myinfo->standard_patterns)
			myinfo->pbuf = myinfo->backbuffer;
		else
			myinfo->pbuf = myinfo->standard_patterns;
		myinfo->pbuf->pos = myinfo->pbuf->size - 1;
	}
}


void nextpattern(editorinfo * myinfo)
{
	myinfo->pbuf->pos++;
	if (myinfo->pbuf->pos == myinfo->pbuf->size) {
		if (myinfo->pbuf == myinfo->standard_patterns)
			myinfo->pbuf = myinfo->backbuffer;
		else
			myinfo->pbuf = myinfo->standard_patterns;
		myinfo->pbuf->pos = 0;
	}
}

void floodselect(selection fillsel, int x, int y, char* bigboard, int brdwidth, int brdheight)
{
	/* If we've already been selected, go back a level */
	if (isselected(fillsel, x, y))
		return;

	/* Select ourselves. That makes us special */
	selectpos(fillsel, x, y);

	/* A little to the left */
	if (x > 0) {
		if ( tiletype (bigboard, x - 1, y) == tiletype (bigboard, x, y) &&
				(tilecolor(bigboard, x - 1, y) == tilecolor(bigboard, x, y) ||
				 tiletype (bigboard, x, y) == Z_EMPTY))
			floodselect(fillsel, x - 1, y, bigboard, brdwidth, brdheight);
	}

	/* A little to the right */
	if (x < brdwidth - 1) {
		if ( tiletype (bigboard, x + 1, y) == tiletype (bigboard, x, y) &&
				(tilecolor(bigboard, x + 1, y) == tilecolor(bigboard, x, y) ||
				 tiletype (bigboard, x, y) == Z_EMPTY))
			floodselect(fillsel, x + 1, y, bigboard, brdwidth, brdheight);
	}

	/* A little to the north */
	if (y > 0) {
		if ( tiletype (bigboard, x, y - 1) == tiletype (bigboard, x, y) &&
				(tilecolor(bigboard, x, y - 1) == tilecolor(bigboard, x, y) ||
				 tiletype (bigboard, x, y) == Z_EMPTY))
			floodselect(fillsel, x, y - 1, bigboard, brdwidth, brdheight);
	}

	/* A little to the south */
	if (y < brdheight - 1) {
		if ( tiletype (bigboard, x, y + 1) == tiletype (bigboard, x, y) &&
				(tilecolor(bigboard, x, y + 1) == tilecolor(bigboard, x, y) ||
				 tiletype (bigboard, x, y) == Z_EMPTY))
			floodselect(fillsel, x, y + 1, bigboard, brdwidth, brdheight);
	}
}

void fillbyselection(selection fillsel, patbuffer pbuf, int randomflag, board* destbrd, char* bigboard, unsigned char paramlist[60][25])
{
	int x = 0, y = 0;
	patdef pattern = pbuf.patterns[pbuf.pos];

	if (randomflag)
		srand(time(0));

	/* Plot the first pattern */
	if (isselected(fillsel, x, y)) {
		if (randomflag)
			pattern = pbuf.patterns[rand() % pbuf.size];

		/* Check for object overflow if we have params & aren't overwriting some */
		if (destbrd->info->objectcount >= 150 && pattern.patparam != NULL &&
		    paramlist[x][y] == 0)
			return;

		pat_plot(destbrd, pattern, x, y, bigboard, paramlist);
	}

	/* Plot remaining patterns */
	while (!nextselected(fillsel, &x, &y)) {
		if (randomflag)
			pattern = pbuf.patterns[rand() % pbuf.size];

		/* Check for object overflow if we have params & aren't overwriting some */
		if (destbrd->info->objectcount >= 150 && pattern.patparam != NULL &&
		    paramlist[x][y] == 0)
			return;

		pat_plot(destbrd, pattern, x, y, bigboard, paramlist);
	}
}

void dofloodfill(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25], int randomflag)
{
	selection fillsel;

	/* Don't floodfill onto the player! It's not nice! */
	if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
		return;

	/* New selection as large as the board */
	initselection(&fillsel, 60, 25);

	/* Flood select then fill using the selection */
	floodselect(fillsel, myinfo->cursorx, myinfo->cursory, bigboard, 60, 25);
	fillbyselection(fillsel, (randomflag ? *myinfo->backbuffer : *myinfo->pbuf),
									randomflag, myworld->board[myinfo->curboard], bigboard, paramlist);

	/* Cleanup */
	deleteselection(&fillsel);
}


