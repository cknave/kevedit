/* textblock.h   -- A block of coloured display text
 * $Id: textblock.h,v 1.2 2003/11/02 22:20:01 bitman Exp $
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

#ifndef DISPLAY_TEXTBLOCK_H
#define DISPLAY_TEXTBLOCK_H 1

#ifdef HAVE_U_INT8_T
#include <sys/types.h>
typedef u_int8_t textDatum;
#else
typedef unsigned char textDatum;
#endif

/* A block of text with given width and height */
typedef struct textBlock {
	int width;
	int height;

	/* Char and colour data */
	textDatum *data;
} textBlock;


#define textBlockChar(block, x, y)   (block->data[(y * block->width + x) * 2])
#define textBlockColour(block, x, y) (block->data[(y * block->width + x) * 2 + 1])

/* Create an empty textBlock */
textBlock * createTextBlock(int width, int height);

/* Destroy a textBlock */
void deleteTextBlock(textBlock * block);

/* Write a character to a text block */
void textBlockPutch(textBlock * block, int x, int y,
                    textDatum ch, textDatum co);

/* Copy one text block onto another */
/* TODO:
void textBlockBlit(textBlock * dest, textBlock * src, int x, int y);
*/


#endif
