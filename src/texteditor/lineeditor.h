/**@file texteditor/lineeditor.h  Editor for a single line.
 * $Id: lineeditor.h,v 1.1 2004/02/01 06:36:44 bitman Exp $
 * @author Ryan Phillips
 *
 * Copyright (C) 2002 Ryan Phillips <bitman@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either listersion 2 of the License, or
 * (at your option) any later listersion.
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

#ifndef TEXTEDITOR_LINEEDITOR_H
#define TEXTEDITOR_LINEEDITOR_H

#include "display/display.h"

/* Valid input flags. */
#define LINED_NORMAL   0x00  /**< All normal input allowed. */
#define LINED_NOUPPER  0x01  /**< No uppercase letters used. */
#define LINED_NOLOWER  0x02  /**< No lowercase letters used. */
#define LINED_NOALPHA  LINED_NOUPPER | LINED_NOLOWER  /**< No letters allowed. */
#define LINED_NODIGITS 0x04  /**< No digits 0-9 allowed. */
#define LINED_NOPUNCT  0x08  /**< No punctuation allowed. */
#define LINED_NOSPACES 0x10  /**< No spaces allowed. */
#define LINED_NOPERIOD 0x20  /**< No period character allowed. */

#define LINED_FILENAME 0x40  /**< Input must be a valid filename. */
#define LINED_NOPATH   0x80  /**< Input must contain no path characters (e.g. /) */

/** Input is restricted to integers. */
#define LINED_NUMBER   LINED_NOALPHA | LINED_NOPUNCT | LINED_NOSPACES
/** Input is restricted to signed integers. */
#define LINED_SNUMBER  LINED_NOALPHA | LINED_NOSPACES | LINED_NOPERIOD

/* Return values. */
#define LINED_CANCEL 1  /**< User cancelled input, so disregard value of line. */
#define LINED_OK     0  /**< User confirmed input. */


/**
 * A powerful editor for a single line of text.
 **/
typedef struct {
	char * line;          /**< A line of text to edit. */
	int editwidth;        /**< Maximum length of the line. */
	int visiblewidth;     /**< Visible width of the line. */

	int x, y;             /**< Line screen coordinates. */
	int pos;              /**< Cursor position. */
	int colour;           /**< Line colour. */

	int validinputflags;  /**< Flags controlling valid input. */

	displaymethod * d;    /**< Display method to use. */

} lineeditor;

lineeditor * createlineeditor(int x, int y, char * line, int width, displaymethod * d);
void deletelineeditor(lineeditor * editor);
int editline(lineeditor * editor);

void lineeditorUpdateDisplay(lineeditor * editor);
void lineeditorHandleKey(lineeditor * editor, int key);

#endif
