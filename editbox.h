/* editbox.h  -- text editor/viewer in kevedit
 * $Id: editbox.h,v 1.4 2000/08/21 20:06:22 bitman Exp $
 * Copyright (C) 2000 Ryan Phillips <bitman@scn.org>
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

#ifndef __EDITBOX_H
#define __EDITBOX_H 1

#include "zzt.h"
#include "display.h"
#include "svector.h"

/* editmoredata - edits p->moredata in a scroll box */
void editmoredata(displaymethod * d, param * p);

/* editbox - display/edit a string vector in a scroll box, starting
 * at sv->cur. editwidth tells how long a line can be & how much memory to use
 * on new lines. If editwidth is zero, editsvector acts as a listbox and sets
 * sv->cur to selected node.  If zochighlight is nonzero, it does ZZT Object
 * Code (ZOC) highlighting. */
void editbox(displaymethod * d, char* title, stringvector * sv, int editwidth, int zocformatting);

/* displayzoc - display a string with zoc highlighting. If firstline is true,
 * "@" will be allowed to denote object name. */
void displayzoc(displaymethod * d, int x, int y, unsigned char *s, int firstline);

/* wordwrap - wrap text in sv */
int wordwrap(stringvector * sv, unsigned char *str, int inspos, int pos, int wrapwidth, int editwidth);


/* REFERENCE -- editbox key actions
 * - standard keys -
 * up       : moves cursor up
 * down     : moves cursor down
 * pageup   : moves up 8 lines
 * pagedown : moves down 8 lines
 * escape   : exit editbox
 * 
 * - view only keys -
 * enter    : exit editbox
 * 
 * - edit only keys -
 * left     : moves cursor left
 * right    : moves cursor right
 * insert   : toggle insert/replace modes
 * delete   : removes char under cursor
 * home     : moves to beginning of line
 * end      : moves to end of line
 * tab      : inserts 4 spaces
 * enter    : inserts newline
 * backspace: deletes space before cursor, or blank lines
 * -        : decrease wordwrap width
 * +        : increase wordwrap width
 * ctrl-y   : deletes the current line
 * ctrl-a   : inserts an ascii character (or a number on #char statements)
 * 
 * - coming soon from the desk of bitman -
 * alt-s   : save zoc to file
 * alt-o   : open zoc file (erases buffer)
 * alt-i   : insert zoc from file
 * alt-m   : insert zzm song from file
 * shift   : highlighting
 * alt-x   : cut
 * alt-c   : copy
 * alt-v   : paste
 * 
 */

#endif
