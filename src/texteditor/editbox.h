/* editbox.h  -- text editor/viewer in kevedit
 * $Id: editbox.h,v 1.1 2003/11/01 23:45:57 bitman Exp $
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

#ifndef __EDITBOX_H
#define __EDITBOX_H 1

#include "libzzt2/zzt.h"
#include "display/display.h"
#include "structures/svector.h"

/* Return codes */
#define EDITBOX_OK       1     /* ENTER, ESC when editbox > EDITBOX_NOEDIT */
#define EDITBOX_CANCEL   2     /* ESC */
#define EDITBOX_FORWARD  3     /* RIGHT-ARROW */
#define EDITBOX_BACK     4     /* BACKSPACE */
#define EDITBOX_BACKWARD 5     /* LEFT-ARROW */
#define EDITBOX_HELP     6     /* F1 */

/* Flags */
#define EDITBOX_ZOCMODE  1     /* Use ZZT markup / syntax highlighting */
#define EDITBOX_MOVEMENT 2     /* Forward and backward exit dialog (only
																	effective with editwidth = EDITBOX_NOEDIT) */

/* Editwidth */
#define EDITBOX_ZZTWIDTH 42    /* ZZT's maximum text width */
#define EDITBOX_NOEDIT   0     /* Width used to specify browse/scroll dialog */

/* editbox() - edit/browse a string vector in a scroll box.
 *   Browsing starts at sv->cur.
 *   Editwidth determines maximum line width, zero for browse only.
 *   Flags and return codes are listed above. */
int editbox(char* title, stringvector * sv, int editwidth, int flags, displaymethod * d);

/* Special instances of editbox() */
#define scrolldialog(title, sv, d) editbox((title), (sv), EDITBOX_NOEDIT, EDITBOX_ZOCMODE, (d))
#define browsedialog(title, sv, d) editbox((title), (sv), EDITBOX_NOEDIT, EDITBOX_ZOCMODE | EDITBOX_MOVEMENT, (d))

/* displayzoc() - display a string with zoc highlighting. If firstline is true,
 * "@" will be allowed to denote object name. */
void displayzoc(int x, int y, char *s, int format, int firstline, displaymethod * d);

/* displaycommand() - displays highlighting for zzt #command arguments */
void displaycommand(int x, int y, char *command, char *args, displaymethod * d);

/* displayzzm() - displays zzm music highlighted */
void displayzzm(int x, int y, char *music, displaymethod * d);


/* EDITBOX REFERENCE -- editbox key actions
 * - standard keys -
 * up       : moves cursor up
 * down     : moves cursor down
 * pageup   : moves up 8 lines
 * pagedown : moves down 8 lines
 * escape   : exit editbox with EDITBOX_CANCEL or EDITBOX_OK
 * 
 * - EDITBOX_NOEDIT -
 * enter    : exit with EDITBOX_OK
 * - (with EDITBOX_MOVEMENT set) -
 * right    : exit with EDITBOX_FORWARD
 * left     : exit with EDITBOX_BACKWARD
 * backspace: exit with EDITBOX_BACK
 * 
 * - editwidth > EDITBOX_NOEDIT -
 * left     : moves cursor left
 * right    : moves cursor right
 * insert   : toggle insert/replace modes
 * delete   : removes char under cursor
 * home     : moves to beginning of line
 * end      : moves to end of line
 * tab      : inserts 4 spaces
 * enter    : inserts newline
 * backspace: deletes space before cursor, or blank lines
 *
 * alt+'-'  : decrease wordwrap width
 * alt+'+'  : increase wordwrap width
 * ctrl-y   : deletes the current line
 * ctrl-a   : inserts an ascii character (or a number on #char statements)
 *
 * alt-s   : save zoc to file
 * alt-o   : open zoc file (erases buffer)
 * alt-i   : insert zoc from file
 * alt-m   : insert zzm song from file
 *
 * shift   : highlighting
 * ctrl-x   : cut
 * ctrl-c   : copy
 * ctrl-v   : paste
 * 
 */

#endif
