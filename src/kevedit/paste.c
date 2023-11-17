/* paste.c       -- Paste functionality, including rebinding
 * $Id: paste.c,v 1.6 2023/17/11 13:58:30 kristomu Exp $
 * Copyright (C) 2023 Kristofer Munsterhjelm <kristofer@munsterhjelm.no>
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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "hash.h"
#include "misc.h"
#include "paste.h"
#include "screen.h"

#include "structures/svector.h"
#include "structures/selection.h"
#include "structures/gradient.h"

#include "kevedit.h"

/* --------------------------- COMPLEX PASTING -------------------- */

/* The following functions involve pasting a section into a board
 * so that the relative order of params is preserved, and updating
 * bind indices so no dangling indices occur.
 *
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
 * Define:
 *	- object: any tile with a param.
 *	- destination board: The board we're pasting into.
 *	- source board: The board we're pasting from.
 *  - source object: the object a bound object is bound to.
 *  - destination area: The area of the destination board we're
 *		pasting over.
 *	- source area: The area of the source board we're copying from.
 *  - dangling index: A bind index with no destination.
*/

/* The source selection gives the shape of the selection. This is
 * offset by xofs and yofs, so to test if something will be pasted
 * over destination coordinates (destx, desty), we have to subtract
 * the offset. I'm doing it like this so I don't get my signs mixed
 * up. */

int isselected_dest(selection srcsel, int destx, int desty,
	int xofs, int yofs)
{

	return isselected(srcsel, destx - xofs, desty - yofs);
}

/* TODO: Use this in the main paste function... but I need to decide
 * if the coordinates are source-based or destination-based... */
//bool isselected_both(selection destsel, selection srcsel)

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
		param->program_hash = source_backup.program_hash;
		param->bindindex = 0;

		zztParamRehash(source_obj);
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

/* Remove or restore uncopyable tiles from the destination selection.
 * If restore is true, then we add the positions, otherwise we remove
 * them. Restoring is used for cleanup. */
void remove_uncopyable_tiles(ZZTblock * dest, const ZZTblock * src,
	selection destsel, selection srcsel, int x, int y,
	int objects_to_add)
{
	int i, objects_counted = 0;

	for (i = 0; i < src->paramcount; ++i) {
		ZZTparam * param = src->params[i];

		/* If not inside the area, skip. */
		if (!isselected(srcsel, param->x, param->y)
			|| !isselected(destsel, param->x + x, param->y + y)) {
			continue;
		}

		/* If above the capacity threshold, toggle. */
		if (objects_counted >= objects_to_add) {
			unselectpos(destsel, param->x + x, param->y + y);
		}

		++objects_counted;
	}
}

void restore_uncopyable_tiles(ZZTblock * dest, const ZZTblock * src,
	selection destsel, selection srcsel, int x, int y,
	int objects_to_add)
{
	int i, objects_counted = 0;

	for (i = 0; i < src->paramcount; ++i) {
		ZZTparam * param = src->params[i];

		/* If not excluded, skip. */
		if (!isselected(srcsel, param->x, param->y)
			|| isselected(destsel, param->x + x, param->y + y)) {
			continue;
		}

		if (objects_counted >= objects_to_add) {
			/* Never add the destination player back in. */
			ZZTparam * player = dest->params[0];
			if (param->x + x != player->x
				&& param->y + y != player->y) {
				selectpos(destsel, param->x + x, param->y + y);
			}
		}

		++objects_counted;
	}
}


/* Copy the tiles themselves, but no associated params since we'll
 * deal with those later. */
void copy_tiles(ZZTblock * dest, const ZZTblock * src,
	selection destsel, selection srcsel, int x, int y)
{
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

bool pasteblock(ZZTblock *dest, const ZZTblock *src,
	selection destsel, selection srcsel, int x, int y) {

	/* Move objects out of the way first. */
	move_bind_sources(dest, srcsel, x, y);

	/* Clear every param in the destination area, and update bind
	 * indices to reflect this. */
	remove_selection_params(dest, srcsel, x, y);

	/*	Record the hashes of every board on the destination board outside
	 *  the destination area. Since we removed every object inside the
	 *  destination area, that's just every board on the destination.*/

	hash_table dest_board_ht = hashInit(dest->paramcount);
	addNodes(&dest_board_ht, dest);

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

	/* If we can't add every object to the destination, we have to
	 * change the destination selection to exclude the objects
	 * that we aren't going to copy, because we don't want their
	 * tiles copied. */

	if (objects_in_area > dest->maxparams - dest->paramcount) {
		remove_uncopyable_tiles(dest, src,
			destsel, srcsel, x, y, objects_to_add);
	}

	/* Copy the tiles themselves. */
	copy_tiles(dest, src, destsel, srcsel, x, y);

	/* The remaining algorithm consists of two steps. First we add the
	 * unbound objects from the source to the destination in a way that
	 * preserves the relative order, noting their new indices in the bind
	 * map. Then we add the bound objects. Those who bind to objects whose
	 * indices are in the bound map can just be remapped; otherwise we
	 * need to look for some object with the same code in the destination
	 * and bind to that - or failing that, copy the code from the source
	 * board's object to the first bound object. */


	/* If there are no objects to add, there's no reason to bother
	 * with any of this. */
	if (objects_in_area == 0) {
		/* Cleanup. */
		restore_uncopyable_tiles(dest, src,
			destsel, srcsel, x, y, objects_to_add);
		return true;
	}

	int num_objects_after = dest->paramcount + objects_to_add,
		first_new_idx = dest->paramcount;
	
	int * bind_map = malloc(src->paramcount * sizeof(int));
	memset(bind_map, 0, src->paramcount * sizeof(int));

	/* For leader/follower. object_map[i] is the destination
	 * index of the object with source index i. 0 if not copied. */
	int * object_map = malloc(src->paramcount * sizeof(int));
	memset(object_map, 0, src->paramcount * sizeof(int));

	/* Reallocate the destination param array accordingly. */
	dest->params = (ZZTparam **) realloc(dest->params,
		sizeof(ZZTparam*) * num_objects_after);
	dest->paramcount = num_objects_after;

	/* Copy unbound objects. */
	/* object_idx is the index of the source object if we only count
	 * objects inside the selection area. It's only used to calculate
	 * the destination index and shouldn't be touched directly. */
	int object_idx = 0;
	for (i = 0; i < src->paramcount && object_idx < objects_to_add; ++i) {
		ZZTparam * param = src->params[i];

		if (!isselected(srcsel, param->x, param->y)) {
			continue;
		}

		int dest_idx = first_new_idx + object_idx;
		++object_idx;

		/* Count objects bound to themselves or containing 
		 * code as unbound. */
		if (param->bindindex == i || param->program != NULL) {
			param->bindindex = 0;
		}

		if (param->bindindex != 0) {
			continue;
		}

		ZZTparam * dest_param = zztParamDuplicate(param);
		dest_param->index = dest_idx;
		dest_param->x += x;
		dest_param->y += y;

		dest->params[dest_idx] = dest_param;
		zztTileAt(dest, dest_param->x, dest_param->y).param = dest_param;

		bind_map[i] = dest_idx;
		object_map[i] = dest_idx;
		addNode(&dest_board_ht, dest_param);
	}

	/* Copy bound objects. */
	object_idx = 0;
	for (i = 0; i < src->paramcount && object_idx < objects_to_add; ++i) {
		ZZTparam * param = src->params[i];

		if (!isselected(srcsel, param->x, param->y)) {
			continue;
		}

		int dest_idx = first_new_idx + object_idx;
		++object_idx;

		if (param->bindindex == 0) {
			continue;
		}

		/* Do the actual copy here. */
		ZZTparam * dest_param = zztParamDuplicate(param);
		dest_param->index = dest_idx;
		dest_param->x += x;
		dest_param->y += y;

		dest->params[dest_idx] = dest_param;
		zztTileAt(dest, dest_param->x, dest_param->y).param = dest_param;

		object_map[i] = dest_idx;

		/* Now there are three possibilities. Either the object is bound to
		 * something we copied earlier and thus have a bind index for;
		 * or it's bound to something that can be found on the destination
		 * board but isn't part of what we copied;
		 * or it's bound to something that isn't on the destination board. */

		/* If it's bound to something we copied earlier, just resolve. */
		if (bind_map[dest_param->bindindex] != 0) {
			dest_param->bindindex = bind_map[dest_param->bindindex];
			continue;
		}

		/* Search the destination board for an object with code matching
		 * the bound object we're looking for, using the hash table. */
		ZZTparam * bound_to_param = src->params[param->bindindex];

		const llnode * first_equal = getFirstEqual(&dest_board_ht,
			bound_to_param);

		if (first_equal != NULL) {
			bind_map[dest_param->bindindex] = first_equal->param->index;
			dest_param->bindindex = first_equal->param->index;
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
		bind_map[dest_param->bindindex] = dest_param->index;
		dest_param->bindindex = 0;
	}

	/* Deal with leaders and followers. */
	for (i = first_new_idx; i < num_objects_after; ++i) {
		ZZTparam * param = dest->params[i];

		if (param->followerindex == -1
			|| param->followerindex >= src->paramcount
			|| object_map[param->followerindex] == 0) {
				param->followerindex = -1;
		} else {
			param->followerindex =
				object_map[param->followerindex];
		}

		if (param->leaderindex == -1
			|| param->leaderindex >= src->paramcount
			|| object_map[param->leaderindex] == 0) {
				param->leaderindex = -1;
		} else {
			param->leaderindex =
				object_map[param->leaderindex];
		}
	}

	/* Cleanup. */

	restore_uncopyable_tiles(dest, src,
		destsel, srcsel, x, y, objects_to_add);

	freeTable(&dest_board_ht);
	free(bind_map);
	return true; /* Success! */
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
		                 myeditor->copyBlock->width,  myeditor->copyBlock->height);

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