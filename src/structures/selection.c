/* selection.h  -- selected area
 * $Id: selection.c,v 1.2 2005/05/28 03:17:45 bitman Exp $
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "selection.h"

#include <stdlib.h>
#include <string.h>

/* The all-powerful min/max macros */
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define swap(a, b, t) { (t) = (a); (a) = (b); (b) = (t); }


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

	clearselection(*sel);
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
void clearselection(selection sel)
{
	int i;

	/* Run through each element of the field and clear it */
	for (i = 0; i < sel.fieldlength; i++)
		sel.field[i] = 0;
}

/* setselection() - sets all values */
void setselection(selection sel)
{
	int i;

	/* Run through each element of the field and set it */
	for (i = 0; i < sel.fieldlength; i++)
		sel.field[i] = -1;
}


/* copyselection() - copy one selection onto another of the same size.
 *                   Returns zero on success. */
int copyselection(selection dest, selection src)
{
	if (dest.fieldlength != src.fieldlength ||
			dest.width       != src.width ||
			dest.height      != src.height)
		return 1;

	memcpy(dest.field, src.field, fieldunitsize * dest.fieldlength);

	return 0;
}

int mergeselection(selection dest, selection src)
{
	int i;

	if (dest.fieldlength != src.fieldlength ||
			dest.width       != src.width ||
			dest.height      != src.height)
		return 1;

	for (i = 0; i < dest.fieldlength; i++)
		dest.field[i] &= src.field[i];

	return 0;
}


/* selectpos() - sets a given position to selected */
void selectpos   (selection sel, int x, int y)
{
	if (x < 0 || y < 0 || x >= sel.width || y >= sel.height) return;
	sel.field[(x / fieldunitsize) + (y * sel.fieldwidth)]
		|= (1 << x % fieldunitsize);
}

/* unselectpos() - unsets a given position */
void unselectpos (selection sel, int x, int y)
{
	if (x < 0 || y < 0 || x >= sel.width || y >= sel.height) return;
	sel.field[(x / fieldunitsize) + (y * sel.fieldwidth)]
		&= ~(1 << x % fieldunitsize);
}

/* isselected() - determines whether a given position is set as selected */
int  isselected  (selection sel, int x, int y)
{
	if (x < 0 || y < 0 || x >= sel.width || y >= sel.height) return 0;
	return sel.field[(x / fieldunitsize) + (y * sel.fieldwidth)]
		& (1 << x % fieldunitsize);
}


/* selectblock() - select a block of positions */
void selectblock(selection sel, int x1, int y1, int x2, int y2)
{
	int x, y, temp;

	/* Make sure that the lowest values come first */
	if (x1 > x2) swap(x1, x2, temp);
	if (y1 > y2) swap(y1, y2, temp);

	/* Make sure the values are in bounds */
	x1 = min(x1, sel.width);
	x2 = min(x2, sel.width);
	y1 = min(y1, sel.height);
	y2 = min(y2, sel.height);

	/* This could be more efficient, but who cares? */
	for (x = x1; x <= x2; x++)
		for (y = y1; y <= y2; y++)
			selectpos(sel, x, y);
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
