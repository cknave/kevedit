/* selection.h  -- selected area
 * $Id: selection.h,v 1.1 2001/10/22 02:48:23 bitman Exp $
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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
void clearselection(selection * sel);


/* selectpos() - sets a given position to selected */
void selectpos   (selection sel, int x, int y);

/* unselectpos() - unsets a given position */
void unselectpos (selection sel, int x, int y);

/* isselected() - determines whether a given position is set as selected */
int  isselected  (selection sel, int x, int y);

/* nextselected() - advances x and y to the next set position, returning false
 *                  on success and true on failure (end of bitfield) */
int nextselected(selection sel, int * x, int * y);

#endif
