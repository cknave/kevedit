/* misc.c       -- General routines for everyday KevEditing
 * $Id: misc.c,v 1.20 2001/12/12 22:08:02 bitman Exp $
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

#include "misc.h"

#include "kevedit.h"
#include "editbox.h"
#include "screen.h"

#include "svector.h"
#include "hypertxt.h"
#include "selection.h"
#include "gradient.h"
#include "zlaunch.h"

#include "patbuffer.h"

#include "panel_g1.h"
#include "panel_g2.h"

#include "display.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>

void runzzt(char* path, char* world)
{
	stringvector actions, files;
	stringvector info;
	zlaunchinfo zli;

	/* Initialize the vectors */
	initstringvector(&actions);
	initstringvector(&files);

	/* Add the world as the first option */
	pushstring(&files, str_dup(world));
	pushstring(&actions, str_dup("play"));

	info = loadinfo(path, world);
	zli = loadzlinfofromsvector(info);

	zli.datadir = str_dup(path);
	zli.bindir  = str_dup(path);
	zli.paramlist = files;
	stringvectorcat(&(zli.actionstoperform), &actions);

	zlaunchact(&zli);

	/* Cleanup */
	zlaunchcleanup(&zli);
	deletezlinfo(&zli);

	deletestringvector(&info);
	/* DO NOT delete actions and files. deletezlinfo() did this already */
}

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
	if (myinfo->pbuf == myinfo->standard_patterns) {
		/* TODO: use drawing patterns (later) */
		myinfo->pbuf = myinfo->backbuffer;
		myinfo->pbuf->pos = myinfo->pbuf->size - 1;
	}

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

patbuffer* createfillpatterns(editorinfo* myinfo)
{
	patbuffer* fillpatterns;

	fillpatterns = patbuffer_create(5);
	fillpatterns->patterns[0].type = Z_SOLID;
	fillpatterns->patterns[1].type = Z_NORMAL;
	fillpatterns->patterns[2].type = Z_BREAKABLE;
	fillpatterns->patterns[3].type = Z_WATER;
	fillpatterns->patterns[4].type = Z_SOLID;

	pat_applycolordata(fillpatterns, myinfo);

	/* Last pattern is an inverted-coloured solid */
	fillpatterns->patterns[4].color = myinfo->backc |
	                                  ((myinfo->forec & 0x07) << 4);

	return fillpatterns;
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
	int x = -1, y = 0;
	patdef pattern = pbuf.patterns[pbuf.pos];

	if (randomflag)
		srand(time(0));

	/* Plot the patterns */
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
	patbuffer* fillbuffer;

	/* Set up the fill buffer */
	fillbuffer = myinfo->pbuf;
	if (randomflag && myinfo->pbuf == myinfo->standard_patterns)
		fillbuffer = createfillpatterns(myinfo);

	/* Don't floodfill onto the player! It's not nice! */
	if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
		return;

	/* New selection as large as the board */
	initselection(&fillsel, 60, 25);

	/* Flood select then fill using the selection */
	floodselect(fillsel, myinfo->cursorx, myinfo->cursory, bigboard, 60, 25);
	fillbyselection(fillsel, *fillbuffer,
									randomflag, myworld->board[myinfo->curboard], bigboard, paramlist);

	/* Delete the fill buffer if we created it above */
	if (randomflag && myinfo->pbuf == myinfo->standard_patterns) {
		deletepatternbuffer(fillbuffer);
		free(fillbuffer);
	}

	/* Cleanup */
	deleteselection(&fillsel);
}


/*********** Gradient fill code *****************/
void movebykeystroke(int key, int* x, int* y, int minx, int miny,
										 int maxx, int maxy, displaymethod * mydisplay)
{
	switch (key) {
		case DKEY_LEFT:      if (*x > minx) (*x)--; break;
		case DKEY_RIGHT:     if (*x < maxx) (*x)++; break;
		case DKEY_UP:        if (*y > miny) (*y)--; break;
		case DKEY_DOWN:      if (*y < maxy) (*y)++; break;
		case DKEY_ALT_LEFT:  (*x) -= 10; if (*x < minx) *x = minx; break;
		case DKEY_ALT_RIGHT: (*x) += 10; if (*x > maxx) *x = maxx; break;
		case DKEY_ALT_UP:    (*y) -= 10; if (*y < miny) *y = miny; break;
		case DKEY_ALT_DOWN:  (*y) += 10; if (*y > maxy) *y = maxy; break;
	}
}


int promptforselection(selection sel, gradline * grad, editorinfo* myinfo, char* bigboard, displaymethod * mydisplay)
{
	int i, j;   /* Counters */
	int key;

	do {
		mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
		key = mydisplay->getch();

		movebykeystroke(key, &(myinfo->cursorx), &(myinfo->cursory),
										0, 0, 59, 24, mydisplay);
		if (key == DKEY_ESC) return 1;
		/* Check for flood selection */
		if (key == 'f' || key == 'F' || key == 'm') {
			floodselect(sel, myinfo->cursorx, myinfo->cursory, bigboard, 60, 25);
			/* Set the gradient endpoints to the current position */
			grad->x1 = grad->x2 = myinfo->cursorx;
			grad->y1 = grad->y2 = myinfo->cursory;
			if (key != 'm')
				return 0;
		}
	} while (key != DKEY_ENTER && key != ' ');
	grad->x1 = myinfo->cursorx; grad->y1 = myinfo->cursory;
	mydisplay->putch(myinfo->cursorx, myinfo->cursory, '+', 0x0F);
	
	do {
		mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
		key = mydisplay->getch();

		movebykeystroke(key, &(myinfo->cursorx), &(myinfo->cursory),
										0, 0, 59, 24, mydisplay);
		if (key == DKEY_ESC) return 1;
		/* Check for flood selection */
		if (key == 'f' || key == 'F' || key == 'm') {
			floodselect(sel, myinfo->cursorx, myinfo->cursory, bigboard, 60, 25);
			/* Set the gradient endpoints to the current position */
			grad->x2 = myinfo->cursorx;
			grad->y2 = myinfo->cursory;
			if (key != 'm')
				return 0;
		}
	} while (key != DKEY_ENTER && key != ' ');
	grad->x2 = myinfo->cursorx; grad->y2 = myinfo->cursory;

	/* just select everything */
	for (i = min(grad->x1, grad->x2); i <= max(grad->x1, grad->x2); i++)
		for (j = min(grad->y1, grad->y2); j <= max(grad->y1, grad->y2); j++)
			selectpos(sel, i, j);

	return 0;
}


void gradientfillbyselection(selection fillsel, patbuffer pbuf, gradline grad, int randomseed, int preview, board* destbrd, char* bigboard, unsigned char paramlist[60][25], displaymethod* mydisplay);

int pickgradientpoint(int* x, int* y, selection fillsel, patbuffer pbuf, gradline * grad, int randomseed, board* destbrd, world* myworld, editorinfo* myinfo, char* bigboard, unsigned char paramlist[60][25], displaymethod* mydisplay)
{
	int key;

	do {
		mydisplay->cursorgo(*x, *y);

		/* Preview the gradient */
		gradientfillbyselection(fillsel, pbuf, *grad, randomseed, 1,
									destbrd, bigboard, paramlist, mydisplay);
		mydisplay->putch(*x, *y, '*', 0x0F);

		key = mydisplay->getch();

		/* Refress the cursor location */
		myinfo->cursorx = *x; myinfo->cursory = *y;
		drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);

		movebykeystroke(key, x, y, 0, 0, 59, 24, mydisplay);

		/* Check for change of gradient type */
		switch (key) {
			case 'l':
			case 'L': grad->type = GRAD_LINEAR; break;
			case 'b':
			case 'B': grad->type = GRAD_BILINEAR; break;
			case 'r':
			case 'R': grad->type = GRAD_SCALEDRADIAL; break;
			case 'u':
			case 'U': grad->type = GRAD_RADIAL; break;
			case '=': /* (lowercase plus sign) */
			case '+': if (grad->randomness < 256) grad->randomness++; break;
			case '-': if (grad->randomness > 0)  grad->randomness--; break;
		}
	} while (key != DKEY_ESC && key != DKEY_ENTER &&
					 key != DKEY_TAB && key != ' ');

	return key;
}

void gradientfillbyselection(selection fillsel, patbuffer pbuf, gradline grad, int randomseed, int preview, board* destbrd, char* bigboard, unsigned char paramlist[60][25], displaymethod* mydisplay)
{
	int x = -1, y = 0;
	patdef pattern = pbuf.patterns[pbuf.pos];

	if (randomseed != 0)
		srand(randomseed);

	/* Plot the patterns */
	while (!nextselected(fillsel, &x, &y)) {
		pattern = pbuf.patterns[gradientscaledistance(grad, x, y, pbuf.size-1)];

		/* Check for object overflow if we have params & aren't overwriting some */
		if (destbrd->info->objectcount >= 150 && pattern.patparam != NULL &&
		    paramlist[x][y] == 0)
			return;

		if (!preview) {
			pat_plot(destbrd, pattern, x, y, bigboard, paramlist);
		} else {
			/* This is kinda sloppy for types which need to know who is next to
			 * them, such as line-walls */
			mydisplay->putch(x, y,
							 z_getchar(pattern.type, pattern.color, pattern.patparam, bigboard, x, y),
							 z_getcolour(pattern.type, pattern.color, pattern.patparam)
							 );
		}
	}
}

void dogradient(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	int key;
	int randomseed;
	selection sel;
	gradline grad;
	patbuffer* fillbuffer;

	initselection(&sel, 60, 25);

	/* Set up the fill buffer */
	if (myinfo->pbuf == myinfo->standard_patterns)
		fillbuffer = createfillpatterns(myinfo);
	else
		fillbuffer = myinfo->backbuffer;

	/* Prepare for randomness */
	randomseed = time(0);
	grad.randomness = 0;
	grad.type = GRAD_LINEAR;

	/*********** Make the selection ***************/

	/* Draw the first panel */
	drawsidepanel(mydisplay, PANEL_GRADTOOL1);

	if (promptforselection(sel, &grad, myinfo, bigboard, mydisplay)) {
		/* Escape was pressed */
		deleteselection(&sel);
		return;
	}
	unselectpos(sel, myinfo->playerx, myinfo->playery);

	/************ Build the gradient **************/

	/* Draw the second panel */
	drawsidepanel(mydisplay, PANEL_GRADTOOL2);

	/* Choose ending point for the gradient, previewing as we go */
	do {
		/* Pick the ending point */
		key = 
		pickgradientpoint(&grad.x2, &grad.y2, sel, *fillbuffer, &grad, randomseed,
						myworld->board[myinfo->curboard], myworld, myinfo, bigboard, paramlist, mydisplay);
		myinfo->cursorx = grad.x2; myinfo->cursory = grad.y2;

		if (key == DKEY_ESC) { deleteselection(&sel); return; }
		if (key != DKEY_TAB && key != ' ') break;

		/* Pick the starting point */
		key = 
		pickgradientpoint(&grad.x1, &grad.y1, sel, *fillbuffer, &grad, randomseed,
						myworld->board[myinfo->curboard], myworld, myinfo, bigboard, paramlist, mydisplay);
		myinfo->cursorx = grad.x1; myinfo->cursory = grad.y1;

		if (key == DKEY_ESC) { deleteselection(&sel); return; }
	} while (key == DKEY_TAB || key == ' ');

	/* Fill the selection by the gradient line */
	gradientfillbyselection(sel, *fillbuffer, grad, randomseed, 0,
						myworld->board[myinfo->curboard], bigboard, paramlist, mydisplay);

	/* Delete the fillbuffer if we createded it custom */
	if (myinfo->pbuf == myinfo->standard_patterns) {
		deletepatternbuffer(fillbuffer);
		free(fillbuffer);
	}

	deleteselection(&sel);
}

