/* misc.c       -- General routines for everyday KevEditing
 * $Id: misc.c,v 1.25 2002/02/17 07:26:03 bitman Exp $
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
#include "register.h"

#include "panel_g1.h"
#include "panel_g2.h"
#include "tdialog.h"

#include "display.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>

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

	myinfo->cursorx = 0;
	myinfo->cursory = 0;
	myinfo->drawmode = 0;
	myinfo->gradmode = 0;
	myinfo->aqumode = 0;
	myinfo->blinkmode = 0;
	myinfo->textentrymode = 0;
	myinfo->defc = 1;
	myinfo->forec = 0x0f;
	myinfo->backc = 0x00;
	myinfo->standard_patterns = createstandardpatterns();
	myinfo->backbuffer = patbuffer_create(10);
	myinfo->pbuf = myinfo->standard_patterns;

	pat_applycolordata(myinfo->standard_patterns, myinfo);
}


void texteditor(displaymethod * mydisplay)
{
	/* open text file for edit */
	char buffer[14];
	stringvector editvector;

	strcpy(buffer, "");
	initstringvector(&editvector);
	
	/* Load the ! register for editing */
	regput('!', &editvector, 0, EDITBOX_ZZTWIDTH, EDITBOX_ZZTWIDTH);

	/* Edit */
	editbox("Text Editor", &editvector, 42, 0, mydisplay);

	/* Store result to the ! register */
	regstore('!', editvector);
	deletestringvector(&editvector);
}

void clearboard(ZZTworld * myworld)
{
	zztBoardClear(myworld);
}

ZZTworld * clearworld(ZZTworld * myworld)
{
	ZZTworld * newworld;

	/* Create the new world */
	newworld = zztWorldCreate(NULL, NULL);
	if (newworld == NULL)
		return myworld;

	zztWorldFree(myworld);

	return newworld;
}

void entergradientmode(editorinfo * myinfo)
{
	if (myinfo->gradmode != 0)
		return;  /* Don't enter grad mode if we're already there */

	if (myinfo->pbuf == myinfo->standard_patterns) {
		int oldpos = myinfo->standard_patterns->pos;

		/* Substitute fill patterns for standard patterns */
		deletepatternbuffer(myinfo->standard_patterns);
		free(myinfo->standard_patterns);
		myinfo->standard_patterns = createfillpatterns(myinfo);
		myinfo->pbuf = myinfo->standard_patterns; /* Very important */

		/* Use the previous position if not too big */
		if (oldpos >= myinfo->standard_patterns->size)
			oldpos = myinfo->standard_patterns->size - 1;
		myinfo->standard_patterns->pos = oldpos;

		/* Turn gradmode on going forward for standard patterns */
		myinfo->gradmode = 1;
	} else {
		/* Turn gradmode on going backward for backbuffer */
		myinfo->gradmode = -1;
	}
	/* Drawmode goes on no matter what */
	myinfo->drawmode = 1;
}

void exitgradientmode(editorinfo * myinfo)
{
	if (myinfo->gradmode == 0)
		return;  /* Can't leave a state we're not in */

	if (myinfo->pbuf == myinfo->standard_patterns) {
		int oldpos = myinfo->standard_patterns->pos;

		/* Restore the regular standard patterns */
		deletepatternbuffer(myinfo->standard_patterns);
		free(myinfo->standard_patterns);
		myinfo->standard_patterns = createstandardpatterns();
		myinfo->pbuf = myinfo->standard_patterns; /* Very important */

		/* Apply the current colors */
		pat_applycolordata(myinfo->standard_patterns, myinfo);

		/* Use the previous position if not too big */
		if (oldpos >= myinfo->standard_patterns->size)
			oldpos = myinfo->standard_patterns->size - 1;
		myinfo->standard_patterns->pos = oldpos;
	}

	/* Turn gradmode and drawmode off */
	myinfo->gradmode = myinfo->drawmode = 0;
}

int toggledrawmode(editorinfo * myinfo)
{
	if (myinfo->gradmode != 0) {
		exitgradientmode(myinfo);
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

	myinfo->aqumode = 0;
	if (myinfo->gradmode != 0) {
		/* Gradmode is already on -- reverse direction */
		myinfo->gradmode = -(myinfo->gradmode);

		return 0;    /* No reason to start plotting yet */
	} else {
		entergradientmode(myinfo);
		return 1;    /* Gradmode went on -- let the plotting begin! */
	}
}

void saveworld(displaymethod * mydisplay, ZZTworld * myworld)
{
	/* Save World after prompting user for filename */
	char* filename;
	char* path, * file;
	char* oldfilenamebase;    /* Old filename without extension */
	char* dotptr;             /* General pointer */

	filename =
		filenamedialog(zztWorldGetFilename(myworld), "zzt", "Save World As", 1, mydisplay);

	if (filename == NULL)
		return;

	path = (char*) malloc(sizeof(char) * (strlen(filename) + 1));
	file = (char*) malloc(sizeof(char) * (strlen(filename) + 1));

	fileof(file, filename, strlen(filename) + 1);
	pathof(path, filename, strlen(filename) + 1);

	/* Change to the selected path */
	chdir(path);

	/* Update the title of the world to reflect the new filename if
	 * the filename and title were the same previously.
	 * That is, if they started out the same, keep them the same. */

	/* Grab the base part of the original filename */
	oldfilenamebase = str_dup(zztWorldGetFilename(myworld));
	dotptr = strrchr(oldfilenamebase, '.');
	if (dotptr != NULL)
		*dotptr = '\0';

	if (!str_equ(zztWorldGetFilename(myworld), file, STREQU_UNCASE) &&
			str_equ(oldfilenamebase, zztWorldGetTitle(myworld), STREQU_UNCASE)) {
		char* newtitle = str_dup(file);
		dotptr = strrchr(newtitle, '.');
		if (dotptr != NULL)
			*dotptr = '\0';
		zztWorldSetTitle(myworld, newtitle);
		free(newtitle);
	}

	/* Update the filename used by the world */
	zztWorldSetFilename(myworld, file);

	/* Set the current board as the starting board */
	zztWorldSetStartboard(myworld, zztBoardGetCurrent(myworld));

	zztWorldSave(myworld);

	free(oldfilenamebase);
	oldfilenamebase = NULL;

	free(filename);
	free(path);
	free(file);

	mydisplay->print(61, 5, 0x1f, "Written.");
	mydisplay->cursorgo(69, 5);
	mydisplay->getch();
}

ZZTworld * loadworld(displaymethod * mydisplay, ZZTworld * myworld)
{
	char* filename = filedialog(".", "zzt", "Load World", FTYPE_ALL, mydisplay);
	ZZTworld* newworld;
	
	if (filename == NULL)
		return myworld;

	newworld = zztWorldLoad(filename);

	if (newworld != NULL) {
		char* newpath = (char*) malloc(sizeof(char)*(strlen(filename)+1));
		char* newfile = (char*) malloc(sizeof(char)*(strlen(filename)+1));

		/* Out with the old and in with the new */
		zztWorldFree(myworld);
		myworld = newworld;

		/* Change directory */
		pathof(newpath, filename, strlen(filename) + 1);
		chdir(newpath);

		/* Change filename */
		fileof(newfile, filename, strlen(filename) + 1);
		zztWorldSetFilename(myworld, newfile);

		/* Select the starting board */
		zztBoardSelect(myworld, zztWorldGetStartboard(myworld));

		free(newpath);
		free(newfile);
	}

	free(filename);
	return myworld;
}

void boardtransfer(displaymethod * mydisplay, ZZTworld * myworld)
{
	int x, y, i = 0;
	int choice = 0;
	int key;

	/* Display the transfer dialog */
	for (y = 9; y < 9 + TRANSFER_DIALOG_DEPTH; y++) {
		for (x = 14; x < 14 + TRANSFER_DIALOG_WIDTH; x++) {
			mydisplay->putch(x, y, TRANSFER_DIALOG[i], TRANSFER_DIALOG[i + 1]);
			i += 2;
		}
	}

	/* Make a choice */
	do {
		mydisplay->putch(17, 11 + choice, 0xAF, 0x02);
		mydisplay->putch(43, 11 + choice, 0xAE, 0x02);

		key = mydisplay->getch();

		mydisplay->putch(17, 11 + choice, ' ', 0x07);
		mydisplay->putch(43, 11 + choice, ' ', 0x07);

		switch (key) {
			case DKEY_UP:   choice--; if (choice < 0) choice = 2; break;
			case DKEY_DOWN: choice++; if (choice > 2) choice = 0; break;
		}
	} while (key != DKEY_ENTER && key != DKEY_ESC);

	if (key == DKEY_ESC)
		return;

	/* Act on choice */
	switch (choice) {
		case 0: /* Import from ZZT world */
			importfromworld(mydisplay, myworld);
			break;
		case 1: /* Import from Board */
			importfromboard(mydisplay, myworld);
			break;
		case 2: /* Export to Board */
			exporttoboard(mydisplay, myworld);
			break;
	}
}

void importfromworld(displaymethod * mydisplay, ZZTworld * myworld)
{
	char* filename = filedialog(".", "zzt", "Load World", FTYPE_ALL, mydisplay);
	ZZTworld* inworld;
	
	if (filename == NULL)
		return;

	inworld = zztWorldLoad(filename);

	if (inworld != NULL) {
		ZZTboard* brd;

		/* Select a board from the new world */
		switchboard(inworld, mydisplay);

		brd = zztBoardGetCurPtr(inworld);

		/* Insert after current board and advance */
		if (zztWorldInsertBoard(myworld, brd, zztBoardGetCurrent(myworld) + 1, 1))
			zztBoardSelect(myworld, zztBoardGetCurrent(myworld) + 1);

		/* Fix links over the top */
		zztBoardValidateLinks(myworld);

		zztWorldFree(inworld);
	}

	free(filename);
}

void importfromboard(displaymethod * mydisplay, ZZTworld * myworld)
{
	char* filename = filedialog(".", "brd", "Import ZZT Board", FTYPE_ALL, mydisplay);
	ZZTboard* brd;

	if (filename == NULL)
		return;

	brd = zztBoardLoad(filename);

	/* Insert after current board and advance */
	if (zztWorldInsertBoard(myworld, brd, zztBoardGetCurrent(myworld) + 1, 1))
		zztBoardSelect(myworld, zztBoardGetCurrent(myworld) + 1);

	/* Fix links over the top */
	zztBoardValidateLinks(myworld);

	/* Free the free board and filename */
	zztBoardFree(brd);
	free(filename);
}

void exporttoboard(displaymethod * mydisplay, ZZTworld * myworld)
{
	char* filename;
	ZZTboard* brd;

	/* Prompt for a filename */
	filename = filenamedialog("", "brd", "Export to Board", 1, mydisplay);

	if (filename == NULL)
		return;

	/* Grab the current board by the horns */
	brd = zztBoardGetCurPtr(myworld);

	/* Export */
	zztBoardSave(brd, filename);

	/* Decompress; it is the current board, after all */
	zztBoardDecompress(brd);
	free(filename);
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
	fillpatterns->patterns[0].type = ZZT_SOLID;
	fillpatterns->patterns[1].type = ZZT_NORMAL;
	fillpatterns->patterns[2].type = ZZT_BREAKABLE;
	fillpatterns->patterns[3].type = ZZT_WATER;
	fillpatterns->patterns[4].type = ZZT_SOLID;

	pat_applycolordata(fillpatterns, myinfo);

	/* Last pattern is an inverted-coloured solid */
	fillpatterns->patterns[4].color = myinfo->backc |
	                                  ((myinfo->forec & 0x07) << 4);

	return fillpatterns;
}

patbuffer* createstandardpatterns(void)
{
	patbuffer* standard_patterns;

	standard_patterns = patbuffer_create(6);

	/* Initialize pattern definitions */
	standard_patterns = patbuffer_create(6);
	standard_patterns->patterns[0].type = ZZT_SOLID;
	standard_patterns->patterns[1].type = ZZT_NORMAL;
	standard_patterns->patterns[2].type = ZZT_BREAKABLE;
	standard_patterns->patterns[3].type = ZZT_WATER;
	standard_patterns->patterns[4].type = ZZT_EMPTY;
	standard_patterns->patterns[5].type = ZZT_LINE;

	return standard_patterns;
}

void floodselect(ZZTblock* block, selection fillsel, int x, int y)
{
	/* If we've already been selected, go back a level */
	if (isselected(fillsel, x, y))
		return;

	/* Select ourselves. That makes us special */
	selectpos(fillsel, x, y);

	/* A little to the left */
	if (x > 0) {
		if ( zztTileAt(block, x - 1, y).type  == zztTileAt(block, x, y).type &&
		    (zztTileAt(block, x - 1, y).color == zztTileAt(block, x, y).color ||
				 zztTileAt(block, x, y).type == ZZT_EMPTY))
			floodselect(block, fillsel, x - 1, y);
	}

	/* A little to the right */
	if (x < block->width - 1) {
		if ( zztTileAt(block, x + 1, y).type  == zztTileAt(block, x, y).type &&
		    (zztTileAt(block, x + 1, y).color == zztTileAt(block, x, y).color ||
				 zztTileAt(block, x, y).type == ZZT_EMPTY))
			floodselect(block, fillsel, x + 1, y);
	}

	/* A little to the north */
	if (y > 0) {
		if ( zztTileAt(block, x, y - 1).type  == zztTileAt(block, x, y).type &&
		    (zztTileAt(block, x, y - 1).color == zztTileAt(block, x, y).color ||
				 zztTileAt(block, x, y).type == ZZT_EMPTY))
			floodselect(block, fillsel, x, y - 1);
	}

	/* A little to the south */
	if (y < block->height - 1) {
		if ( zztTileAt(block, x, y + 1).type  == zztTileAt(block, x, y).type &&
		    (zztTileAt(block, x, y + 1).color == zztTileAt(block, x, y).color ||
				 zztTileAt(block, x, y).type == ZZT_EMPTY))
			floodselect(block, fillsel, x, y + 1);
	}
}

void fillblockbyselection(ZZTblock* block, selection fillsel, patbuffer pbuf, int randomflag)
{
	int x = -1, y = 0;
	ZZTtile pattern = pbuf.patterns[pbuf.pos];

	if (randomflag)
		srand(time(0));

	/* Plot the patterns */
	while (!nextselected(fillsel, &x, &y)) {
		if (randomflag)
			pattern = pbuf.patterns[rand() % pbuf.size];

		zztTilePlot(block, x, y, pattern);
	}
}

void fillbyselection(ZZTworld* world, selection fillsel, patbuffer pbuf, int randomflag)
{
	int x = -1, y = 0;
	ZZTtile pattern = pbuf.patterns[pbuf.pos];

	if (randomflag)
		srand(time(0));

	/* Plot the patterns */
	while (!nextselected(fillsel, &x, &y)) {
		if (randomflag)
			pattern = pbuf.patterns[rand() % pbuf.size];

		zztPlot(world, x, y, pattern);
	}
}

void dofloodfill(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo, int randomflag)
{
	selection fillsel;
	patbuffer* fillbuffer;
	ZZTblock* block = myworld->boards[zztBoardGetCurrent(myworld)].bigboard;

	/* Set up the fill buffer */
	fillbuffer = myinfo->pbuf;
	if (randomflag && myinfo->pbuf == myinfo->standard_patterns)
		fillbuffer = createfillpatterns(myinfo);

	/* Don't floodfill onto the player! It's not nice! */
	if (myinfo->cursorx == zztBoardGetCurPtr(myworld)->plx &&
			myinfo->cursory == zztBoardGetCurPtr(myworld)->ply)
		return;

	/* New selection as large as the board */
	initselection(&fillsel, ZZT_BOARD_X_SIZE, ZZT_BOARD_Y_SIZE);

	/* Flood select then fill using the selection */
	floodselect(block, fillsel, myinfo->cursorx, myinfo->cursory);
	fillbyselection(myworld, fillsel, *fillbuffer, randomflag);

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


int promptforselection(selection sel, gradline * grad, editorinfo* myinfo, ZZTworld * myworld, displaymethod * mydisplay)
{
	int i, j;   /* Counters */
	int key;
	ZZTblock* block = myworld->boards[zztBoardGetCurrent(myworld)].bigboard;

	do {
		mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
		key = mydisplay->getch();

		movebykeystroke(key, &(myinfo->cursorx), &(myinfo->cursory),
										0, 0, 59, 24, mydisplay);
		if (key == DKEY_ESC) return 1;
		/* Check for flood selection */
		if (key == 'f' || key == 'F' || key == 'm') {
			floodselect(block, sel, myinfo->cursorx, myinfo->cursory);
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
			floodselect(block, sel, myinfo->cursorx, myinfo->cursory);
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

int pickgradientpoint(ZZTworld * myworld, int* x, int* y, selection fillsel, patbuffer pbuf, gradline * grad, int randomseed, displaymethod* mydisplay)
{
	int key;

	do {
		mydisplay->cursorgo(*x, *y);

		/* Preview the gradient */
		gradientfillbyselection(myworld, fillsel, pbuf, *grad, randomseed, 1, mydisplay);
		mydisplay->putch(*x, *y, '*', 0x0F);

		key = mydisplay->getch();

		drawblocktile(mydisplay, zztBoardGetCurPtr(myworld)->bigboard, *x, *y, 0, 0);

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

//void gradientfillbyselection(selection fillsel, patbuffer pbuf, gradline grad, int randomseed, int preview, board* destbrd, char* bigboard, unsigned char paramlist[60][25], displaymethod* mydisplay)
void gradientfillbyselection(ZZTworld * myworld, selection fillsel, patbuffer pbuf, gradline grad, int randomseed, int preview, displaymethod * mydisplay)
{
	int x = -1, y = 0;
	ZZTtile pattern = pbuf.patterns[pbuf.pos];

	if (randomseed != 0)
		srand(randomseed);

	/* Plot the patterns */
	while (!nextselected(fillsel, &x, &y)) {
		pattern = pbuf.patterns[gradientscaledistance(grad, x, y, pbuf.size-1)];

		if (!preview) {
			zztPlot(myworld, x, y, pattern);
		} else {
			/* This is kinda sloppy for types which need to know who is next to
			 * them, such as line-walls */
			/* TODO: consider writing to a temporary area to achieve previews */
			mydisplay->putch(x, y, zztLoneTileGetDisplayChar(pattern), zztLoneTileGetDisplayColor(pattern));
		}
	}
}

void dogradient(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
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

	if (promptforselection(sel, &grad, myinfo, myworld, mydisplay)) {
		/* Escape was pressed */
		deleteselection(&sel);
		return;
	}
	unselectpos(sel, myworld->boards[zztBoardGetCurrent(myworld)].plx,
							myworld->boards[zztBoardGetCurrent(myworld)].ply);

	/************ Build the gradient **************/

	/* Draw the second panel */
	drawsidepanel(mydisplay, PANEL_GRADTOOL2);

	/* Choose ending point for the gradient, previewing as we go */
	do {
		/* Pick the ending point */
		key = 
		pickgradientpoint(myworld, &grad.x2, &grad.y2, sel, *fillbuffer, &grad, randomseed, mydisplay);
		myinfo->cursorx = grad.x2; myinfo->cursory = grad.y2;

		if (key == DKEY_ESC) { deleteselection(&sel); return; }
		if (key != DKEY_TAB && key != ' ') break;

		/* Pick the starting point */
		key = 
		pickgradientpoint(myworld, &grad.x1, &grad.y1, sel, *fillbuffer, &grad, randomseed, mydisplay);
		myinfo->cursorx = grad.x1; myinfo->cursory = grad.y1;

		if (key == DKEY_ESC) { deleteselection(&sel); return; }
	} while (key == DKEY_TAB || key == ' ');

	/* Fill the selection by the gradient line */
	gradientfillbyselection(myworld, sel, *fillbuffer, grad, randomseed, 0, mydisplay);

	/* Delete the fillbuffer if we createded it custom */
	if (myinfo->pbuf == myinfo->standard_patterns) {
		deletepatternbuffer(fillbuffer);
		free(fillbuffer);
	}

	deleteselection(&sel);
}

