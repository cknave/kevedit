/* selection.h  -- selected area
 * $Id: selection.c,v 1.3 2002/09/13 17:51:21 bitman Exp $
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

#include "selection.h"

#include <stdlib.h>

const int fieldunitsize = (int) sizeof(bitfieldtype) * 8;

/* initselection() - allocates the bit field and clears the selection */
void initselection(selection * sel, int width, int height)
{
	/* Increase width to multiple of the field type if necessary */
	sel->fieldwidth = width / fieldunitsize;
	if (width % fieldunitsize > 0)
		sel->fieldwidth++;

	sel->width = sel->fieldwidth * fieldunitsize;
	sel->height = height;

	sel->fieldlength = sel->fieldwidth * sel->height;
	sel->field = (bitfieldtype *) malloc(fieldunitsize *
																			 (sel->fieldlength));

	clearselection(sel);
}

/* deleteselection() - free()s any memory used by the selection */
void deleteselection(selection * sel)
{
	if (sel->field != NULL)
		free(sel->field);
	sel->field = NULL;

	sel->width = sel->height = sel->fieldwidth = sel->fieldlength = 0;
}

/* clearselection() - unsets all values */
void clearselection(selection * sel)
{
	int i;

	/* Run through each element of the field and clear it */
	for (i = 0; i < sel->fieldlength; i++)
		sel->field[i] = 0;
}

#define selectionindex(sel, x, y) ()

/* selectpos() - sets a given position to selected */
void selectpos   (selection sel, int x, int y)
{
	sel.field[(x / fieldunitsize) + (y * sel.fieldwidth)]
		|= (1 << x % fieldunitsize);
}

/* unselectpos() - unsets a given position */
void unselectpos (selection sel, int x, int y)
{
	sel.field[(x / fieldunitsize) + (y * sel.fieldwidth)]
		&= ~(1 << x % fieldunitsize);
}

/* isselected() - determines whether a given position is set as selected */
int  isselected  (selection sel, int x, int y)
{
	return sel.field[(x / fieldunitsize) + (y * sel.fieldwidth)]
		& (1 << x % fieldunitsize);
}

/* nextselected() - advances x and y to the next set position */
int nextselected(selection sel, int * x, int * y)
{
	int position = ((*x + 1) / fieldunitsize) + (*y * sel.fieldwidth);
	int offset = (*x + 1) % fieldunitsize;

	do {
		do {
			if (sel.field[position] & (1 << offset)) {
				*y = position / sel.fieldwidth;
				*x = (position % sel.fieldwidth) * fieldunitsize + offset;
				return 0;
			}
			offset++;
		} while (offset < fieldunitsize);
		
		/* Nothing found in this position, advance to the next. */
		offset = 0;
		position++;

		/* Skip through positions until we find a true one */
		while (position < sel.fieldlength && !sel.field[position])
			position++;

	} while (position < sel.fieldlength);

	/* Reached the end of the field */
	return 1;
}

int firstselected(selection sel, int * x, int * y)
{
	/* nextselected will advance from (-1, 0) to (0, 0) before it starts
	 * checking values, so this is a very easy trick to find the first
	 * selected value. */
	*x = -1; *y = 0;
	return nextselected(sel, x, y);
}


#if 0
/* temporary */
#include <stdio.h>

void printselection(selection sel)
{
	int x, y;

	printf("Width:      %d \tHeight:      %d\n", sel.width, sel.height);
	printf("Fieldwidth: %d \tFieldlength: %d\n\n", sel.fieldwidth, sel.fieldlength);

	printf("  ");
	for (x = 0; x < sel.width; x++)
		printf("%d", x % 10);
	printf("\n");

	for (y = 0; y < sel.height; y++) {
		printf("%d ", y % 10);
		for (x = 0; x < sel.width; x++) {
			if (isselected(sel, x, y))
				printf("X");
			else
				printf(".");
		}
		printf("\n");
	}
}

void listselected(selection sel)
{
	int x = 0, y = 0;

	if (isselected(sel, x, y))
		printf("%d, %d\n", x, y);

	while (!nextselected(sel, &x, &y))
		printf("%d, %d\n", x, y);
}

int
main()
{
	selection sel;

	initselection(&sel, 60, 10);

	selectpos(sel, 3, 0);
	selectpos(sel, 5, 0);
	selectpos(sel, 0, 2);
	selectpos(sel, 3, 2);
	selectpos(sel, 14, 1);
	selectpos(sel, 15, 3);
	selectpos(sel, 16, 7);

	printselection(sel);
	listselected(sel);

	deleteselection(&sel);

	return 0;
}

/* entemporary */
#endif
