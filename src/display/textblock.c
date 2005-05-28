/* textblock.c   -- A block of coloured display text
 * $Id: textblock.c,v 1.2 2005/05/28 03:17:45 bitman Exp $
 * Copyright (C) 2003 Ryan Phillips <bitman@users.sourceforge.net>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "textblock.h"

#include <stdlib.h>
#include <string.h>

/* Create an empty textBlock */
textBlock *
createTextBlock(int width, int height)
{
	textBlock * block = (textBlock *) malloc(sizeof(textBlock));
	int dataSize = width * height * 2;

	block->width = width;
	block->height = height;

	block->data = (textDatum *) malloc(sizeof(textDatum) * dataSize);
	memset(block->data, 0, sizeof(textDatum) * dataSize);

	return block;
}


/* Destroy a textBlock */
void
deleteTextBlock(textBlock * block)
{
	if (block == NULL)
		return;

	if (block->data != NULL) {
		free(block->data);
		block->data = NULL;
	}

	free(block);
}


/* Write a character to a text block */
void
textBlockPutch(textBlock * block, int x, int y,
               textDatum ch, textDatum co)
{
	textBlockChar(block, x, y)   = ch;
	textBlockColour(block, x, y) = co;
}


