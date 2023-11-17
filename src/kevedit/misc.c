/* misc.c       -- General routines for everyday KevEditing
 * $Id: misc.c,v 1.6 2005/07/02 21:31:30 kvance Exp $
 * Copyright (C) 2000 Kev Vance <kvance@kvance.com>
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

#include "misc.h"

#include "kevedit.h"
#include "texteditor/editbox.h"
#include "texteditor/texteditor.h"
#include "screen.h"

#include "hash.h"

#include "structures/svector.h"
#include "structures/selection.h"
#include "structures/gradient.h"
#include "help/hypertxt.h"
#include "zlaunch/zlaunch.h"

#include "patbuffer.h"
#include "texteditor/register.h"

#include "themes/theme.h"

#include "display/display.h"
#include "display/colours.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#ifdef DOSBOX
/* Need PATH_MAX */
#include <sys/param.h>
#include "zlaunch/dosbox.h"
#endif

static char * save_backup_begin(char * filename) {
	char * bkp_filename;
#ifdef DOS
	char * bkp_filename_start;
#endif
	int pos;

	if (filename[0] == '\0') {
		return str_dup(filename);
	}

#ifdef DOS
	/* Do not assume LFN on DOS. */
	bkp_filename = str_dup(filename);
	bkp_filename_start = strrchr(bkp_filename, '/');
	if (bkp_filename_start != NULL)
		bkp_filename_start++;
	else
		bkp_filename_start = bkp_filename;
	bkp_filename_start[0] = '~';
#else
	bkp_filename = str_dupadd(filename, 4);
	strcat(bkp_filename, ".BAK");
	if (fileexists(bkp_filename)) {
		pos = strlen(bkp_filename) - 1;
		bkp_filename[pos] = '1';
		while (fileexists(bkp_filename) && bkp_filename[pos] < '9') {
			bkp_filename[pos]++;
		}
	}
#endif

	if (!fileexists(bkp_filename))
		if (!fileexists(filename) || (rename(filename, bkp_filename) == 0))
			return bkp_filename;

	free(bkp_filename);
	return NULL;
}

static void save_backup_end(char * filename, char * bkp_filename, bool success) {
	if (bkp_filename == NULL) {
		return;
	}

	if (success) {
		remove(bkp_filename);
	} else {
		remove(filename);
		rename(bkp_filename, filename);
	}

	free(bkp_filename);
}

void copy(keveditor * myeditor)
{
	/* Only copy if a block is selected */
	if (myeditor->selectmode) {
		if (myeditor->copyBlock != NULL)
			zztBlockFree(myeditor->copyBlock);

		myeditor->copyBlock = zztBlockDuplicate(zztBoardGetBlock(myeditor->myworld));
		copyselection(myeditor->copySelection, myeditor->selCurrent);

		myeditor->clearselectflag = 1;
	}
}

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

int showObjects(keveditor * myeditor)
{
	displaymethod * mydisplay = myeditor->mydisplay;
	
	int x, y;
	ZZTboard * board = zztBoardGetCurPtr(myeditor->myworld);

	for (x = 0; x < board->bigboard->width; x++) {
		for (y = 0; y < board->bigboard->height; y++) {
			uint8_t ch, color;
			ZZTtile tile = zztTileGet(myeditor->myworld, x, y);
			if (tile.type != ZZT_OBJECT)
				continue;

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
	return mydisplay->getch();
}

void runzzt(char* path, char* world)
{
#ifndef DOSBOX

	/* The DOS/WINDOWS way */
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

#else

	/* The LINUX way.  Really, Windows could be using this too. */
	char cwd[PATH_MAX];

	/* ZZT world is always in current dir */
	getcwd(cwd, PATH_MAX);

	/* And run DOSBox */
	if(!dosbox_launch(path, cwd, world)) {
            dosbox_launch(DATAPATH, cwd, world);
        }

#endif /* DOSBOX */
}


int texteditordialog(displaymethod * mydisplay)
{
	/* open text file for edit */
	texteditor * editor;

	editor = createtexteditor("Text Editor", NULL, mydisplay);
	
	/* Load the ! register for editing */
	regput('!', editor->text, 0, EDITBOX_ZZTWIDTH, EDITBOX_ZZTWIDTH);

	/* Edit */
	int result = textedit(editor);

	/* Store result to the ! register */
	regstore('!', *(editor->text));

	/* Free the text editor and text. */
	deletetexteditortext(editor);
	deletetexteditor(editor);

	return result;
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

int saveworld(displaymethod * mydisplay, ZZTworld * myworld)
{
	/* Save World after prompting user for filename */
	char* filename;
	char* bkp_filename;
	char* path, * file;
	char* oldfilenamebase;    /* Old filename without extension */
	char* dotptr;             /* General pointer */
	char* suggestext;         /* Suggested extension */
	int result;

	if (zztWorldGetFilename(myworld) != NULL) {
		suggestext = strrchr(zztWorldGetFilename(myworld), '.');
		if (suggestext == NULL)
			suggestext = "";
	} else {
		suggestext = ".zzt";
	}
	
	bool quit = false;
	filename =
		filenamedialog(zztWorldGetFilename(myworld), suggestext, "Save World As", 1, mydisplay, &quit);
	if (quit)
		return DKEY_QUIT;

	if (filename == NULL)
		return 0;

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

	if ((!str_equ(zztWorldGetFilename(myworld), file, STREQU_UNCASE) &&
	     str_equ(oldfilenamebase, (char *)zztWorldGetTitle(myworld), STREQU_UNCASE)) ||
	    str_equ("UNTITLED", (char *)zztWorldGetTitle(myworld), 0)) {
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

	result = 0;
	bkp_filename = save_backup_begin(filename);
	if (bkp_filename != NULL) {
		result = zztWorldSave(myworld);
		save_backup_end(filename, bkp_filename, result > 0);
	}

	free(oldfilenamebase);
	oldfilenamebase = NULL;

	free(filename);
	free(path);
	free(file);

	mydisplay->print(61, 5, result > 0 ? 0x1f : 0x1c, result > 0 ? "Written." : "Write error!");
	mydisplay->cursorgo(result > 0 ? 69 : 73, 5);
	return mydisplay->getch();
}

ZZTworld * loadworld(displaymethod * mydisplay, ZZTworld * myworld, char *filename, bool *quit)
{
	if(filename) {
		filename = str_dup(filename);
	} else {
		filename = filedialog(".", "zzt", "Load World", FTYPE_ALL, mydisplay, quit);
		if (quit && *quit)
			return myworld;
	}
	
	if (filename == NULL)
		return myworld;

	ZZTworld* newworld;
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

int boardtransfer(displaymethod * mydisplay, ZZTworld * myworld)
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
			case DKEY_QUIT:
				return DKEY_QUIT;
		}
	} while (key != DKEY_ENTER && key != DKEY_ESC);

	if (key == DKEY_ESC)
		return 0;

	/* Act on choice */
	switch (choice) {
		case 0: /* Import from ZZT world */
			return importfromworld(mydisplay, myworld);
		case 1: /* Import from Board */
			return importfromboard(mydisplay, myworld);
		case 2: /* Export to Board */
			return exporttoboard(mydisplay, myworld);
	}
	return 0;
}

int importfromworld(displaymethod * mydisplay, ZZTworld * myworld)
{
	bool quit = false;
	char* filename = filedialog(".", "zzt", "Load World", FTYPE_ALL, mydisplay, &quit);
	if (quit)
		return DKEY_QUIT;
	if (filename == NULL)
		return 0;

	ZZTworld* inworld = zztWorldLoad(filename);

	if (inworld != NULL) {
		ZZTboard* brd;

		/* Select a board from the new world */
		int result = switchboard(inworld, mydisplay);
        if(result == DKEY_QUIT) {
            return DKEY_QUIT;
        }

		brd = zztBoardGetCurPtr(inworld);

		/* Insert after current board and advance */
		if (zztWorldInsertBoard(myworld, brd, zztBoardGetCurrent(myworld) + 1, 1))
			zztBoardSelect(myworld, zztBoardGetCurrent(myworld) + 1);

		/* Fix links over the top */
		zztBoardValidateLinks(myworld);

		zztWorldFree(inworld);
	}

	free(filename);
	return 0;
}

int importfromboard(displaymethod * mydisplay, ZZTworld * myworld)
{
	bool quit = false;
	char* filename = filedialog(".", "brd", "Import ZZT Board", FTYPE_ALL, mydisplay, &quit);
	if (quit)
		return DKEY_QUIT;

	if (filename == NULL)
		return 0;

	ZZTboard* brd = zztBoardLoad(filename);

	if (brd == NULL)
		/* TODO: report the error to the user */
		return 0;

	/* Insert after current board and advance */
	if (zztWorldInsertBoard(myworld, brd, zztBoardGetCurrent(myworld) + 1, 1))
		zztBoardSelect(myworld, zztBoardGetCurrent(myworld) + 1);

	/* Fix links over the top */
	zztBoardValidateLinks(myworld);

	/* Free the free board and filename */
	zztBoardFree(brd);
	free(filename);
	return 0;
}

int exporttoboard(displaymethod * mydisplay, ZZTworld * myworld)
{
	char* filename;
	char* bkp_filename;
	ZZTboard* brd;
	bool quit = false;
	int result;

	/* Prompt for a filename */
	filename = filenamedialog("", ".brd", "Export to Board", 1, mydisplay, &quit);
	if (quit)
		return DKEY_QUIT;

	if (filename == NULL)
		return 0;

	/* Grab the current board by the horns */
	brd = zztBoardGetCurPtr(myworld);

	/* Export */
	result = 0;
	bkp_filename = save_backup_begin(filename);
	if (bkp_filename != NULL) {
		result = zztBoardSave(brd, filename);
		save_backup_end(filename, bkp_filename, result > 0);
	}

	if (result <= 0) {
		mydisplay->print(61, 5, 0x1c, "Write error!");
		mydisplay->cursorgo(73, 5);
		mydisplay->getch();
	}

	/* Decompress; it is the current board, after all */
	zztBoardDecompress(brd);
	free(filename);
	return 0;
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
	standard_patterns->patterns[0].type = ZZT_SOLID;
	standard_patterns->patterns[1].type = ZZT_NORMAL;
	standard_patterns->patterns[2].type = ZZT_BREAKABLE;
	standard_patterns->patterns[3].type = ZZT_WATER;
	standard_patterns->patterns[4].type = ZZT_EMPTY;
	standard_patterns->patterns[5].type = ZZT_LINE;

	return standard_patterns;
}

/* Determine whether two tiles are equivalent
 * (for the purposes of selecting) */
static bool tileEquivalent(ZZTtile t1, ZZTtile t2)
{
	if (t1.type != t2.type)
		return false;
	if ((t1.type != ZZT_EMPTY) && (t1.color != t2.color))
		return false;
	if (t1.type == ZZT_OBJECT)
	{
		uint8_t t1_chr = t1.param == NULL ? 0 : zztParamGetProperty(t1.param, ZZT_DATAUSE_CHAR);
		uint8_t t2_chr = t2.param == NULL ? 0 : zztParamGetProperty(t2.param, ZZT_DATAUSE_CHAR);
		if (t1_chr != t2_chr)
			return false;
	}

	return true;
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
		if (tileEquivalent(zztTileAt(block, x, y), zztTileAt(block, x - 1, y)))
			floodselect(block, fillsel, x - 1, y);
	}

	/* A little to the right */
	if (x < block->width - 1) {
		if (tileEquivalent(zztTileAt(block, x, y), zztTileAt(block, x + 1, y)))
			floodselect(block, fillsel, x + 1, y);
	}

	/* A little to the north */
	if (y > 0) {
		if (tileEquivalent(zztTileAt(block, x, y), zztTileAt(block, x, y - 1)))
			floodselect(block, fillsel, x, y - 1);
	}

	/* A little to the south */
	if (y < block->height - 1) {
		if (tileEquivalent(zztTileAt(block, x, y), zztTileAt(block, x, y + 1)))
			floodselect(block, fillsel, x, y + 1);
	}
}

void tileselect(ZZTblock* block, selection fillsel, ZZTtile tile)
{
	int x, y;

	for (x = 0; x < block->width; x++)
		for (y = 0; y < block->height; y++) {
			if (tileEquivalent(tile, zztTileAt(block, x, y)))
				selectpos(fillsel, x, y);
		}
}

void fillbyselection(keveditor *myeditor, ZZTworld* world, selection fillsel, patbuffer pbuf,
                     int randomflag)
{
	int x = -1, y = 0;
	ZZTtile pattern = pbuf.patterns[pbuf.pos];
	if (myeditor->defcmode == 0) {
		pattern.color = encodecolor(myeditor->color);
	}

	if (randomflag)
		srand(time(0));

	/* Plot the patterns */
	while (!nextselected(fillsel, &x, &y)) {
		if (randomflag) {
			pattern = pbuf.patterns[rand() % pbuf.size];
			if (myeditor->defcmode == 0) {
				pattern.color = encodecolor(myeditor->color);
			}
		}

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

	/* New selection as large as the board */
	initselection(&fillsel, ZZT_BOARD_X_SIZE, ZZT_BOARD_Y_SIZE);

	if (myeditor->selectmode) {
		copyselection(fillsel, myeditor->selCurrent);
		myeditor->clearselectflag = 1;
	} else {
		/* Flood select */
		floodselect(block, fillsel, myeditor->cursorx, myeditor->cursory);
	}

	/* Unselect the player */
	unselectpos(fillsel, myworld->boards[zztBoardGetCurrent(myworld)].plx,
							myworld->boards[zztBoardGetCurrent(myworld)].ply);

	/* Fill using the selected area */
	fillbyselection(myeditor, myworld, fillsel, *fillbuffer, randomflag);

	/* Delete the fill buffer if we created it above */
	if (randomflag && myeditor->buffers.pbuf == myeditor->buffers.standard_patterns) {
		deletepatternbuffer(fillbuffer);
	}

	/* Cleanup */
	deleteselection(&fillsel);
}


/*********** Gradient fill and paste code *****************/
/* TODO, move into keyboard??? */
void movebykeystroke(int key, int* x, int* y, int minx, int miny,
	int maxx, int maxy)
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

		cursorspace(myeditor);
		movebykeystroke(key, &(myeditor->cursorx), &(myeditor->cursory),
				0, 0, 59, 24);

		if (key == DKEY_ESC) return 1;
		if (key == DKEY_QUIT) return DKEY_QUIT;
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
	mydisplay->putch(grad->x1, grad->y1, '+', 0x0F);
	
	do {
		mydisplay->cursorgo(myeditor->cursorx, myeditor->cursory);
		key = mydisplay->getch();

		cursorspace(myeditor);
		movebykeystroke(key, &(myeditor->cursorx), &(myeditor->cursory),
				0, 0, 59, 24);
		mydisplay->putch(grad->x1, grad->y1, '+', 0x0F);

		if (key == DKEY_ESC) return 1;
		if (key == DKEY_QUIT) return DKEY_QUIT;
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

		drawblocktile(mydisplay, zztBoardGetCurPtr(myworld)->bigboard, *x, *y, 0, 0, 0);

		movebykeystroke(key, x, y, 0, 0, 59, 24);

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
					 key != DKEY_TAB && key != ' ' && key != DKEY_QUIT);

	return key;
}

void gradientfillbyselection(ZZTworld * myworld, selection fillsel, patbuffer pbuf, gradline grad, int randomseed, int preview, displaymethod * mydisplay)
{
	int x = -1, y = 0;
	ZZTtile pattern;
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
			zztTilePlot(prevBlock, x, y, pattern);
		}
	}

	if (preview) {
		drawblock(mydisplay, prevBlock, 0, 0);
		zztBlockFree(prevBlock);
	}
}

int dogradient(keveditor * myeditor)
{
	displaymethod* mydisplay = myeditor->mydisplay;
	ZZTworld* myworld = myeditor->myworld;
	int key;
	int randomseed;
	selection sel;
	gradline grad;
	patbuffer* fillbuffer;

	initselection(&sel, ZZT_BOARD_X_SIZE, ZZT_BOARD_Y_SIZE);

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

	if (myeditor->selectmode) {
		copyselection(sel, myeditor->selCurrent);
		grad.x1 = myeditor->selx;
		grad.y1 = myeditor->sely;
		grad.x2 = myeditor->cursorx;
		grad.y2 = myeditor->cursory;
	} else {
		int result = promptforselection(sel, &grad, myeditor);
		if (result) {
			/* Escape was pressed */
			deleteselection(&sel);
			return result;
		}
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

		if (key == DKEY_ESC || key == DKEY_QUIT) { deleteselection(&sel); return key; }
		if (key != DKEY_TAB && key != ' ') break;

		/* Pick the starting point */
		key = 
		pickgradientpoint(myworld, &grad.x1, &grad.y1, sel, *fillbuffer, &grad, randomseed, mydisplay);
		myeditor->cursorx = grad.x1; myeditor->cursory = grad.y1;

		if (key == DKEY_ESC || key == DKEY_QUIT) { deleteselection(&sel); return key; }
	} while (key == DKEY_TAB || key == ' ');

	/* Fill the selection by the gradient line */
	gradientfillbyselection(myworld, sel, *fillbuffer, grad, randomseed, 0, mydisplay);

	/* Delete the fillbuffer if we createded it custom */
	if (myeditor->buffers.pbuf == myeditor->buffers.standard_patterns) {
		deletepatternbuffer(fillbuffer);
	}

	myeditor->clearselectflag = 1;
	deleteselection(&sel);
	return 0;
}

