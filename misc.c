/* misc.c       -- General routines for everyday KevEditing
 * $Id: misc.c,v 1.34 2002/09/12 22:05:49 bitman Exp $
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

#include "colours.h"

#include "display.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>

void plot(keveditor * myeditor)
{
	patbuffer* pbuf = myeditor->buffers.pbuf;
	ZZTtile pattern = pbuf->patterns[pbuf->pos];

	/* Change the color to reflect state of default color mode */
	if (myeditor->defcmode == 0) {
		pattern.color = encodecolor(myeditor->color);
	}

	zztPlot(myeditor->myworld, myeditor->cursorx, myeditor->cursory, pattern);
}

void showObjects(keveditor * myeditor)
{
	displaymethod * mydisplay = myeditor->mydisplay;
	
	int x, y;
	ZZTboard * board = zztBoardGetCurPtr(myeditor->myworld);

	for (x = 0; x < board->bigboard->width; x++) {
		for (y = 0; y < board->bigboard->height; y++) {
			u_int8_t ch, color;
			ZZTtile tile = zztTileGet(myeditor->myworld, x, y);
			if (tile.type != ZZT_OBJECT)
				continue;

			ch    = zztLoneTileGetDisplayChar(tile);
			color = zztLoneTileGetDisplayColor(tile);

			/* Make invisible chars smile */
			ch = 0x02;

			/* Make same-colored tiles ugly */
			if (colorfg(color) == colorbg(color))
				color ^= 0x07;

			mydisplay->putch_discrete(x, y, ch, color);
		}
	}

	mydisplay->update(0, 0, board->bigboard->width, board->bigboard->height);
	mydisplay->getch();
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
	editbox("Text Editor", &editvector, 42, EDITBOX_ZOCMODE, mydisplay);

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

void entergradientmode(keveditor * myeditor)
{
	backbuffers * buffers = &(myeditor->buffers);

	if (myeditor->gradmode != 0)
		return;  /* Don't enter grad mode if we're already there */

	if (buffers->pbuf == buffers->standard_patterns) {
		int oldpos = buffers->standard_patterns->pos;

		/* Substitute fill patterns for standard patterns */
		deletepatternbuffer(buffers->standard_patterns);
		free(buffers->standard_patterns);
		buffers->standard_patterns = createfillpatterns(myeditor);
		buffers->pbuf = buffers->standard_patterns; /* Very important */

		/* Use the previous position if not too big */
		if (oldpos >= buffers->standard_patterns->size)
			oldpos = buffers->standard_patterns->size - 1;
		buffers->standard_patterns->pos = oldpos;

		/* Turn gradmode on going forward for standard patterns */
		myeditor->gradmode = 1;
	} else {
		/* Turn gradmode on going backward for backbuffer */
		myeditor->gradmode = -1;
	}
	/* Drawmode goes on no matter what */
	myeditor->drawmode = 1;
}

void exitgradientmode(keveditor * myeditor)
{
	backbuffers * buffers = &(myeditor->buffers);

	if (myeditor->gradmode == 0)
		return;  /* Can't leave a state we're not in */

	if (buffers->pbuf == buffers->standard_patterns) {
		int oldpos = buffers->standard_patterns->pos;

		/* Restore the regular standard patterns */
		deletepatternbuffer(buffers->standard_patterns);
		free(buffers->standard_patterns);
		buffers->standard_patterns = createstandardpatterns();
		buffers->pbuf = buffers->standard_patterns; /* Very important */

		/* Apply the current colors */
		pat_applycolordata(buffers->standard_patterns, myeditor->color);

		/* Use the previous position if not too big */
		if (oldpos >= buffers->standard_patterns->size)
			oldpos = buffers->standard_patterns->size - 1;
		buffers->standard_patterns->pos = oldpos;
	}

	/* Turn gradmode and drawmode off */
	myeditor->gradmode = myeditor->drawmode = 0;
}

int toggledrawmode(keveditor * myeditor)
{
	if (myeditor->gradmode != 0) {
		exitgradientmode(myeditor);
	} else {
		/* Otherwise toggle draw mode */
		myeditor->drawmode ^= 1;
	}
	/* Get mode should go off either way */
	myeditor->aqumode = 0;

	return myeditor->drawmode;
}

int togglegradientmode(keveditor * myeditor)
{
	/* Toggle gradient mode - pattern changes with each cursor
	 * movement & drawmode is turned on. */

	myeditor->aqumode = 0;
	if (myeditor->gradmode != 0) {
		/* Gradmode is already on -- reverse direction */
		myeditor->gradmode = -(myeditor->gradmode);

		return 0;    /* No reason to start plotting yet */
	} else {
		entergradientmode(myeditor);
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

	/* Get the cursor out of the way */
	mydisplay->cursorgo(0, 0);

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

	if (brd == NULL)
		/* TODO: report the error to the user */
		return;

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

void previouspattern(keveditor * myeditor)
{
	backbuffers * buffers = &(myeditor->buffers);

	buffers->pbuf->pos--;
	if (buffers->pbuf->pos == -1) {
		if (buffers->pbuf == buffers->standard_patterns)
			buffers->pbuf = buffers->backbuffer;
		else
			buffers->pbuf = buffers->standard_patterns;
		buffers->pbuf->pos = buffers->pbuf->size - 1;
	}
}

void nextpattern(keveditor * myeditor)
{
	backbuffers * buffers = &(myeditor->buffers);

	buffers->pbuf->pos++;
	if (buffers->pbuf->pos == buffers->pbuf->size) {
		if (buffers->pbuf == buffers->standard_patterns)
			buffers->pbuf = buffers->backbuffer;
		else
			buffers->pbuf = buffers->standard_patterns;
		buffers->pbuf->pos = 0;
	}
}

patbuffer* createfillpatterns(keveditor* myeditor)
{
	patbuffer* fillpatterns;

	fillpatterns = patbuffer_create(5);
	fillpatterns->patterns[0].type = ZZT_SOLID;
	fillpatterns->patterns[1].type = ZZT_NORMAL;
	fillpatterns->patterns[2].type = ZZT_BREAKABLE;
	fillpatterns->patterns[3].type = ZZT_WATER;
	fillpatterns->patterns[4].type = ZZT_SOLID;

	pat_applycolordata(fillpatterns, myeditor->color);

	/* Last pattern is an inverted-coloured solid */
	fillpatterns->patterns[4].color = myeditor->color.bg |
	                                  ((myeditor->color.fg & 0x07) << 4);

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

void dofloodfill(keveditor * myeditor, int randomflag)
{
	selection fillsel;
	patbuffer* fillbuffer;
	ZZTworld* myworld = myeditor->myworld;
	ZZTblock* block = myworld->boards[zztBoardGetCurrent(myworld)].bigboard;

	/* Set up the fill buffer */
	fillbuffer = myeditor->buffers.pbuf;
	if (randomflag && myeditor->buffers.pbuf == myeditor->buffers.standard_patterns)
		fillbuffer = createfillpatterns(myeditor);

	/* Don't floodfill onto the player! It's not nice! */
	if (myeditor->cursorx == zztBoardGetCurPtr(myworld)->plx &&
			myeditor->cursory == zztBoardGetCurPtr(myworld)->ply)
		return;

	/* New selection as large as the board */
	initselection(&fillsel, ZZT_BOARD_X_SIZE, ZZT_BOARD_Y_SIZE);

	/* Flood select then fill using the selection */
	floodselect(block, fillsel, myeditor->cursorx, myeditor->cursory);
	fillbyselection(myworld, fillsel, *fillbuffer, randomflag);

	/* Delete the fill buffer if we created it above */
	if (randomflag && myeditor->buffers.pbuf == myeditor->buffers.standard_patterns) {
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


int promptforselection(selection sel, gradline * grad, keveditor* myeditor)
{
	int i, j;   /* Counters */
	int key;
	displaymethod* mydisplay = myeditor->mydisplay;
	ZZTworld* myworld = myeditor->myworld;
	ZZTblock* block = myworld->boards[zztBoardGetCurrent(myworld)].bigboard;

	do {
		mydisplay->cursorgo(myeditor->cursorx, myeditor->cursory);
		key = mydisplay->getch();

		movebykeystroke(key, &(myeditor->cursorx), &(myeditor->cursory),
										0, 0, 59, 24, mydisplay);
		if (key == DKEY_ESC) return 1;
		/* Check for flood selection */
		if (key == 'f' || key == 'F' || key == 'm') {
			floodselect(block, sel, myeditor->cursorx, myeditor->cursory);
			/* Set the gradient endpoints to the current position */
			grad->x1 = grad->x2 = myeditor->cursorx;
			grad->y1 = grad->y2 = myeditor->cursory;
			if (key != 'm')
				return 0;
		}
	} while (key != DKEY_ENTER && key != ' ');
	grad->x1 = myeditor->cursorx; grad->y1 = myeditor->cursory;
	mydisplay->putch(myeditor->cursorx, myeditor->cursory, '+', 0x0F);
	
	do {
		mydisplay->cursorgo(myeditor->cursorx, myeditor->cursory);
		key = mydisplay->getch();

		movebykeystroke(key, &(myeditor->cursorx), &(myeditor->cursory),
										0, 0, 59, 24, mydisplay);
		if (key == DKEY_ESC) return 1;
		/* Check for flood selection */
		if (key == 'f' || key == 'F' || key == 'm') {
			floodselect(block, sel, myeditor->cursorx, myeditor->cursory);
			/* Set the gradient endpoints to the current position */
			grad->x2 = myeditor->cursorx;
			grad->y2 = myeditor->cursory;
			if (key != 'm')
				return 0;
		}
	} while (key != DKEY_ENTER && key != ' ');
	grad->x2 = myeditor->cursorx; grad->y2 = myeditor->cursory;

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

void gradientfillbyselection(ZZTworld * myworld, selection fillsel, patbuffer pbuf, gradline grad, int randomseed, int preview, displaymethod * mydisplay)
{
	int x = -1, y = 0;
	ZZTtile pattern = pbuf.patterns[pbuf.pos];
	ZZTblock * prevBlock = NULL;

	if (randomseed != 0)
		srand(randomseed);

	if (preview) {
		prevBlock = zztBlockDuplicate(zztBoardGetCurPtr(myworld)->bigboard);
		if (prevBlock == NULL)
			return;
	}

	/* Plot the patterns */
	while (!nextselected(fillsel, &x, &y)) {
		pattern = pbuf.patterns[gradientscaledistance(grad, x, y, pbuf.size-1)];

		if (!preview) {
			zztPlot(myworld, x, y, pattern);
		} else {
			/* TODO: don't plot if we have too many params already */
			/* (not a big deal, this is only the preview after all */
			zztTilePlot(prevBlock, x, y, pattern);
		}
	}

	if (preview) {
		drawblock(mydisplay, prevBlock, 0, 0);
		zztBlockFree(prevBlock);
	}
}

void dogradient(keveditor * myeditor)
{
	displaymethod* mydisplay = myeditor->mydisplay;
	ZZTworld* myworld = myeditor->myworld;
	int key;
	int randomseed;
	selection sel;
	gradline grad;
	patbuffer* fillbuffer;

	initselection(&sel, 60, 25);

	/* Set up the fill buffer */
	if (myeditor->buffers.pbuf == myeditor->buffers.standard_patterns)
		fillbuffer = createfillpatterns(myeditor);
	else
		fillbuffer = myeditor->buffers.backbuffer;

	/* Prepare for randomness */
	randomseed = time(0);
	grad.randomness = 0;
	grad.type = GRAD_LINEAR;

	/*********** Make the selection ***************/

	/* Draw the first panel */
	drawsidepanel(mydisplay, PANEL_GRADTOOL1);

	if (promptforselection(sel, &grad, myeditor)) {
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
		myeditor->cursorx = grad.x2; myeditor->cursory = grad.y2;

		if (key == DKEY_ESC) { deleteselection(&sel); return; }
		if (key != DKEY_TAB && key != ' ') break;

		/* Pick the starting point */
		key = 
		pickgradientpoint(myworld, &grad.x1, &grad.y1, sel, *fillbuffer, &grad, randomseed, mydisplay);
		myeditor->cursorx = grad.x1; myeditor->cursory = grad.y1;

		if (key == DKEY_ESC) { deleteselection(&sel); return; }
	} while (key == DKEY_TAB || key == ' ');

	/* Fill the selection by the gradient line */
	gradientfillbyselection(myworld, sel, *fillbuffer, grad, randomseed, 0, mydisplay);

	/* Delete the fillbuffer if we createded it custom */
	if (myeditor->buffers.pbuf == myeditor->buffers.standard_patterns) {
		deletepatternbuffer(fillbuffer);
		free(fillbuffer);
	}

	deleteselection(&sel);
}

