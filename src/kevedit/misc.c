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

int paste(keveditor * myeditor)
{
	ZZTworld * myworld = myeditor->myworld;
	int done = 0;
	int x = 0, y = 0;
	selection pasteselection;

	if (myeditor->copyBlock == NULL)
		return 0;

	/* Initialize valid pasting region selection */
	initselection(&pasteselection, ZZT_BOARD_X_SIZE, ZZT_BOARD_Y_SIZE);

	if (myeditor->selectmode) {
		/* Only paste within the selected region */
		copyselection(pasteselection, myeditor->selCurrent);
	} else {
		/* Paste everywhere */
		setselection(pasteselection);
	}

	/* Don't paste over the player */
	unselectpos(pasteselection, myworld->boards[zztBoardGetCurrent(myworld)].plx,
							myworld->boards[zztBoardGetCurrent(myworld)].ply);

	int key;
	while (!done) {
		ZZTblock * previewBlock;

		/* Merge the current board and copyBlock onto the previewBlock */
		previewBlock = zztBlockDuplicate(zztBoardGetBlock(myeditor->myworld));
		pasteblock(previewBlock, myeditor->copyBlock, pasteselection, myeditor->copySelection, x, y);

		/* Draw the preview */
		drawblock(myeditor->mydisplay, previewBlock, 0, 0);

		key = myeditor->mydisplay->getch();

		movebykeystroke(key, &x, &y,
		                -myeditor->copyBlock->width, -myeditor->copyBlock->height,
		                 myeditor->copyBlock->width,  myeditor->copyBlock->height,
		                myeditor->mydisplay);

		if (key == ' ' || key == DKEY_ENTER) {
			pasteblock(zztBoardGetBlock(myeditor->myworld),
								 myeditor->copyBlock, pasteselection, myeditor->copySelection, x, y);
			/* Set the paramcount for the board */
			zztBoardSetParamcount(myeditor->myworld, zztBoardGetBlock(myeditor->myworld)->paramcount);
			done = 1;
		}

		if (key == DKEY_ESC || key == DKEY_QUIT)
			done = 1;

		zztBlockFree(previewBlock);
	}

	deleteselection(&pasteselection);
	myeditor->clearselectflag = 1;
	myeditor->updateflags |= UD_BOARD | UD_OBJCOUNT;
	return key;
}

/* --------------------------- COMPLEX PASTING -------------------- */

/* TODO: For copying pre-bound stuff, the following logic needs to be
 * implemented:
 *	- If our selection only contains bound objects, and the destination
 *		board contains the source, link the bound objects to the source.
 *	- If our selection only contains bound objects, and the destination
 *		doesn't contain the source, unlink the bound objects before
 *		pasting.
 *	- If our selection contains some bound objects and their source,
 *		relink the bound objects to the source before pasting.
 *	- If we're pasting over the source to some bound objects, and that
 *		source exists in our selection, link the bound objects to the
 *		new source.
 *	- If we're pasting over the source to some bound objects, and that
 *		source doesn't exist in our selection, make one of the bound
 *		objects the new source.
 *
 * Some of this logic might be applicable to leader and follower as well,
 * but that's not my main priority. This is going to require some new
 * data structures, mainly for what objects link to what, and also a
 * way to determine if the source of a bound object exists when we don't
 * know its index. */ 



/* The following functions involve pasting a section into a board
	so that the relative order of params is preserved, and updating
	bind indices so no dangling indices occur.

Define:
	- object: any tile with a param.
	- destination board: The board we're pasting into.
	- source board: The board we're pasting from.
	- source object: the object a bound object is bound to.
	- destination area: The area of the destination board we're
		pasting over.
	- source area: The area of the source board we're copying from.
	- dangling index: A bind index with no destination.
*/

/* The source selection gives the shape of the selection. This is
 * offset by xofs and yofs, so to test if something will be pasted
 * over destination coordinates (destx, desty), we have to subtract
 * the offset. I'm doing it like this so I don't get my signs mixed
 * up. */

int isselected_dest(selection src_sel, int destx, int desty,
	int xofs, int yofs)
{

	return isselected(src_sel, destx - xofs, desty - yofs);
}

/* This function moves every object inside the selection that's
	the source of bound objects outside, outside. This ensures that
	when we copy something over a region, existing bind indices
	in the destination board do not become dangling indices. */

void move_bind_sources(ZZTblock * board, selection srcsel, int x, int y)
{

	/*	Set up a bind map array: an array of length equal to the number
	of params on the board, filled with zero.

	For every destination board bound object X outside the destination
	area:
		- If its source is also outside, everything is OK.
			Skip to the next.

		- If its source's bind map array value is not zero,
			point the object to the value in it instead, and
			continue.

		- If its source is inside the destination area, move its
			program to X. Set its bind map array value to the index
			of X.

	Once this is done, every source object inside the destination area
	will have been moved outside and the bind indices updated to reflect
	the change. */

	int * bind_map = malloc(board->paramcount * sizeof(int));
	memset(bind_map, 0, board->paramcount * sizeof(int));

	int i;

	for (i = 1; i < board->paramcount; ++i) {
		ZZTparam * param = board->params[i];

		/* Is the object inside the selection? If so, skip */
		if (isselected_dest(srcsel, param->x, param->y, x, y)) {
			continue;
		}

		/* Not bound? Skip. */
		if (param->bindindex == 0 
			|| param->bindindex > board->paramcount) {
			continue;
		}

		ZZTparam * source_obj = board->params[param->bindindex];

		/* Is the source outside? If so, skip. */
		if (!isselected_dest(srcsel, source_obj->x, source_obj->y, x, y)) {
			continue;
		}

		/* Does it already have a new bind index? If so, set it
		 * and skip. */
		if (bind_map[param->bindindex] != 0) {
			param->bindindex = bind_map[param->bindindex];
			continue;
		}

		/* Make the bound object into the source object and set
		 * the bind map to point to it instead. */

		ZZTparam source_backup;
		memcpy(&source_backup, source_obj, sizeof(ZZTparam));

		bind_map[param->bindindex] = i;

		source_obj->instruction = param->instruction;
		source_obj->length = param->length;
		source_obj->program = NULL;
		source_obj->bindindex = i;

		param->instruction = source_backup.instruction;
		param->length = source_backup.length;
		param->program = source_backup.program;
		param->bindindex = 0;
	}

	free(bind_map);
}

void remove_selection_params(ZZTblock * board, selection srcsel,
	int x, int y)
{
	/* Erase every tile with params in the destination area,
	 * so that what we paste will come after every existing
	 * object. */

	/* This is a simple O(n^2) algorithm; it's possible to do
	 * this in linear time, but the approach would be more
	 * complex. Only do it if optimization requires it. */

	int i = 1;

	while (i < board->paramcount) {
		ZZTparam * param = board->params[i];

		if (!isselected_dest(srcsel, param->x, param->y, x, y)) {
			++i;
			continue;
		}

		/* Delete this tile. This will shift every param one step
		 * up, so don't increment i.*/
		zztTileErase(board, param->x, param->y);
	}
}

/* Copy the tiles themselves, but no associated params since we'll
 * deal with those later. */
void copy_tiles(ZZTblock * dest, ZZTblock * src, selection destsel,
	selection srcsel, int x, int y) {

	int src_y, src_x;

	for (src_y = 0; src_y < src->height && src_y + y < dest->height; ++src_y) {
		for (src_x = 0; src_x < src->width && src_x + x < dest->width; ++src_x) {

			if (src_x + x < 0 || src_y + y < 0) {
				continue;
			}

			/* Only copy selected tiles */
			if (!isselected(destsel, src_x + x, src_y + y)
				|| !isselected(srcsel, src_x, src_y)) {
				continue;
			}

			ZZTtile src_tile = zztTileAt(src, src_x, src_y);

			zztTileAt(dest, src_x + x, src_y + y) = src_tile;
			zztTileAt(dest, src_x + x, src_y + y).param = NULL;
		}
	}
}

void merge_paste(ZZTblock *dest, ZZTblock *src,
	selection destsel, selection srcsel, int x, int y) {

	/* Move objects out of the way first. */
	move_bind_sources(dest, srcsel, x, y);

	/* Clear every param in the destination area, and update bind
	 * indices to reflect this. */
	remove_selection_params(dest, srcsel, x, y);

	/* Copy the tiles themselves. */
	copy_tiles(dest, src, destsel, srcsel, x, y);

	/*	Record the hashes of every board on the destination board outside
	 *  the destination area. Since we removed every object inside the
	 *  destination area, that's just every board on the destination.*/

	hash_table dest_board_ht = hashInit(dest->paramcount);
	addNodes(&dest_board_ht, dest);

	/* The remaining algorithm consists of two steps. First we add the
	 * unbound objects from the source to the destination in a way that
	 * preserves the relative order, noting their new indices in the bind
	 * map. Then we add the bound objects. Those who bind to objects whose
	 * indices are in the bound map can just be remapped; otherwise we
	 * need to look for some object with the same code in the destination
	 * and bind to that - or failing that, copy the code from the source
	 * board's object to the first bound object. */



	/* Count the number of objects in the selection area. */

	int i, objects_in_area = 0;

	for (i = 0; i < src->paramcount; ++i) {
		ZZTparam * param = src->params[i];
		if (isselected(srcsel, param->x, param->y)) {
			++objects_in_area;
		}
	}

	/* Determine how many objects we can add. */
	int objects_to_add = min(dest->maxparams - dest->paramcount,
		objects_in_area);

	int num_objects_after = dest->paramcount + objects_in_area,
		first_new_idx = dest->paramcount;

	if (objects_in_area == 0) {
		return;
	}
	
	int * bind_map = malloc(src->paramcount * sizeof(int));
	memset(bind_map, 0, src->paramcount * sizeof(int));

	/* Reallocate the destination param array accordingly. */
	dest->params = (ZZTparam **) realloc(dest->params,
		sizeof(ZZTparam*) * num_objects_after);
	dest->paramcount = num_objects_after;

	/* Copy unbound objects. */
	int object_idx = 0;
	for (i = 0; i < src->paramcount && object_idx < objects_to_add; ++i) {
		ZZTparam * param = src->params[i];

		if (!isselected(srcsel, param->x, param->y)) {
			continue;
		}

		/* Count objects bound to themselves or containing 
		 * code as unbound. */
		if (param->bindindex == i || param->program != NULL) {
			param->bindindex = 0;
		}

		if (param->bindindex != 0) {
			++object_idx;
			continue;
		}
		
		int dest_idx = first_new_idx + object_idx;

		ZZTparam * dest_param = zztParamDuplicate(param);
		dest_param->x += x;
		dest_param->y += y;

		dest->params[dest_idx] = dest_param;
		zztTileAt(dest, dest_param->x, dest_param->y).param = dest_param;

		bind_map[i] = dest_idx;
		addNode(&dest_board_ht, dest_param, first_new_idx + object_idx);
		++object_idx;
	}

	printf("dest param last: %d out of %d\n", object_idx + first_new_idx, num_objects_after);

	/* Copy bound objects. */
	object_idx = 0;
	for (i = 0; i < src->paramcount && object_idx < objects_to_add; ++i) {
		ZZTparam * param = src->params[i];

		if (!isselected(srcsel, param->x, param->y)) {
			continue;
		}

		if (param->bindindex == 0) {
			++object_idx;
			continue;
		}

		/* Do the actual copy here. */
		int dest_idx = first_new_idx + object_idx;
		ZZTparam * dest_param = zztParamDuplicate(param);
		dest_param->x += x;
		dest_param->y += y;

		dest->params[dest_idx] = dest_param;
		zztTileAt(dest, dest_param->x, dest_param->y).param = dest_param;

		/* Now there are three possibilities. Either the object is bound to
		 * something we copied earlier and thus have a bind index for;
		 * or it's bound to something that can be found on the destination
		 * board but isn't part of what we copied;
		 * or it's bound to something that isn't on the destination board. */

		/* If it's bound to something we copied earlier, just resolve. */
		if (bind_map[dest_param->bindindex] != 0) {
			dest_param->bindindex = bind_map[dest_param->bindindex];
			++object_idx;
			continue;
		}

		/* Search the destination board for an object with code matching
		 * the bound object we're looking for, using the hash table. */
		ZZTparam * bound_to_param = src->params[param->bindindex];

		const llnode * first_equal = getFirstEqual(&dest_board_ht,
			bound_to_param);

		if (first_equal != NULL) {
			bind_map[dest_param->bindindex] = first_equal->param_index;
			dest_param->bindindex = first_equal->param_index;
			continue;
		}

		/* It's not currently on the destination board. Copy the source
		 * to our object and set it as source for everything with the
		 * same bind index. */

		/* Copy the program to dest_param. Include hash. */
		dest_param->length = bound_to_param->length;
		dest_param->program = (uint8_t *) malloc(
			dest_param->length);
		memcpy(dest_param->program, bound_to_param->program,
			dest_param->length);
		zztParamRehash(dest_param);

		/* And update bind map and index. */
		bind_map[dest_param->bindindex] = object_idx;
		dest_param->bindindex = 0;
	}

	/* TODO: Scrub the tiles that are associated with objects at the
	 * source but were not copied to dest */

	/* Cleanup. */
	freeTable(&dest_board_ht);
	free(bind_map);
}

/* TODO: make a new type "alphablock" containing a block and a selection */
/* TODO: src should be const. */
int pasteblock(ZZTblock *dest, ZZTblock *src,
	selection destsel, selection srcsel, int x, int y)
{
	int srcpos;     /* Current index in source */
	int row, col;   /* Current row and col in dest */

	/* Paste */

	/* Merge source params into the destination. */
	merge_paste(dest, src, destsel, srcsel, x, y);
	return 1;

	srcpos = 0;     /* Start at beginning of source object */
	for (row = y; row < src->height + y && row < dest->height; row++) {
		for (col = x; col < src->width + x && col < dest->width; col++, srcpos++) {
			/* Paste the currently indexed tile from source to (row, col) in dest */

			if (row < 0 || col < 0)
				continue;

			/* Only copy selected tiles */
			if (!isselected(destsel, col, row) || !isselected(srcsel, col - x, row - y))
				continue;

			/* Can't use plot because we want to maintain terrain under creatures
			 * from the source block, not the destination block */
			zztTileSet(dest, col, row, src->tiles[srcpos]);
		}
		/* If the loop stopped short of using every column in src, advance
		 * the srcpos index to ignore these columns */
		srcpos += (src->width + x) - col;
	}

	/* Success! */
	return 1;
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

		cursorspace(myeditor);
		movebykeystroke(key, &(myeditor->cursorx), &(myeditor->cursory),
				0, 0, 59, 24, mydisplay);

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
				0, 0, 59, 24, mydisplay);
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

