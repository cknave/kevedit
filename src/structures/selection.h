/* selection.h  -- selected area
 * $Id: selection.h,v 1.1 2003/11/01 23:45:57 bitman Exp $
 * Copyright (C) 2000 Ryan Phillips <bitman@users.sf.net>
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

#ifndef __SELECTION_H
#define __SELECTION_H 1

typedef unsigned int bitfieldtype;

typedef struct selection {
	/* Width and height of the selection */
	int width, height;

	/* Bitfield and its descriptors */
	bitfieldtype* field;
	int fieldlength;
	int fieldwidth;
} selection;

/* initselection() - allocates the bit field and clears the selection */
void initselection(selection * sel, int width, int height);

/* deleteselection() - free()s any memory used by the selection */
void deleteselection(selection * sel);

/* clearselection() - unsets all values */
void clearselection(selection sel);

/* setselection() - sets all values */
void setselection(selection sel);


/* copyselection() - copy one selection onto another of the same size.
 *                   Returns zero on success. */
int copyselection(selection dest, selection src);

/* mergeselection() - merges selected areas in src onto dest */
int mergeselection(selection dest, selection src);


/* selectpos() - sets a given position to selected */
void selectpos   (selection sel, int x, int y);

/* unselectpos() - unsets a given position */
void unselectpos (selection sel, int x, int y);

/* isselected() - determines whether a given position is set as selected */
int  isselected  (selection sel, int x, int y);


/* selectblock() - select a block of positions */
void selectblock(selection sel, int x1, int y1, int x2, int y2);


/* nextselected() - advances x and y to the next set position, returning false
 *                  on success and true on failure (end of bitfield) */
int nextselected(selection sel, int * x, int * y);

/* firstselected() - advances x and y to the first set position, returning
 *                   false on success and true on failure */
int firstselected(selection sel, int * x, int * y);

#endif
