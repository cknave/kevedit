/**@file texteditor/texteditor.h  Text editor/viewer.
 * $Id: texteditor.h,v 1.2 2003/12/21 03:21:29 bitman Exp $
 * @author Ryan Phillips
 *
 * Copyright (C) 2003 Ryan Phillips <bitman@users.sf.net>
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

#ifndef TEXTEDITOR_TEXTEDITOR_H
#define TEXTEDITOR_TEXTEDITOR_H

#include "libzzt2/zzt.h"
#include "display/display.h"
#include "structures/svector.h"
#include "zoopdraw.h"


/* Text editor updateflags */
#define TUD_NONE      0x00
#define TUD_CENTER    0x01
#define TUD_BOTTOM    0x02
#define TUD_TOP       0x04
#define TUD_EDITAREA  0x07
#define TUD_TITLE     0x08
#define TUD_PANEL     0x10
#define TUD_ALL       0xFF

#define TEXTED_MAXWIDTH 42
#define TEXTED_PAGELENGTH 7

/** A powerful, extendable text editing environment.
 *
 * TEXTEDITOR REFERENCE -- texteditor key actions
 * - Basic movement keys
 *   - up       : moves cursor up
 *   - down     : moves cursor down
 *   - pageup   : moves up 8 lines
 *   - pagedown : moves down 8 lines
 * 
 * - Scroll/Browse Dialog
 *   - enter    : exit with SCROLL_OK
 *   - escape   : exit with SCROLL_CANCEL
 *
 * - Browse Dialog
 *   - right    : exit with BROWSE_FORWARD
 *   - left     : exit with BROWSE_BACKWARD
 *   - backspace: exit with BROWSE_BACK
 * 
 * - Text Editor
 *   - Basic
 *     - left     : moves cursor left
 *     - right    : moves cursor right
 *     - insert   : toggle insert/replace modes
 *     - delete   : removes char under cursor
 *     - home     : moves to beginning of line
 *     - end      : moves to end of line
 *     - tab      : inserts 4 spaces
 *     - enter    : inserts newline
 *     - escape   : exit
 *     - backspace: deletes space before cursor, or blank lines
 *     - alt+'-'  : decrease wordwrap width
 *     - alt+'+'  : increase wordwrap width
 *     - ctrl-y   : deletes the current line
 *     - ctrl-a   : inserts an ascii character (or a number on #char statements)
 *   - File
 *     - alt-s   : save zoc to file
 *     - alt-o   : open zoc file (erases buffer)
 *     - alt-i   : insert zoc from file
 *     - alt-m   : insert zzm song from file
 *   - Copy and Paste
 *     - shift   : highlighting
 *     - ctrl-x   : cut
 *     - ctrl-c   : copy
 *     - ctrl-v   : paste
 */
typedef struct {
	displaymethod * d;      /**< Display method. */
	ZZTOOPdrawer drawer;    /**< Object code drawer. */
	char * title;           /**< Dialog title. */

	/** The text to edit or view. */
	stringvector * text;
	stringnode * curline;   /**< Current line. */
	int pos;                /**< Cursor position on line. */

	/* Configuration flags. */
	int editflag;       /**< True when text is editable. */
	int highlightflag;  /**< True when syntax highlighting should be used. */
	int insertflag;     /**< True when insert mode is on. */

	/** Maximum line length. All strings in text are assumed to be
	 * able to hold this many characters. */
	int linewidth;
	/** Width at which a line should be wrapped if possible. */
	int wrapwidth;

	/* Control flags */
	int updateflags;    /**< Show what parts of the display need to be updated. */
	int exitflag;       /**< True when editing/viewing should end. */

	/** The current keypress */
	int key;

	/* Text selection information */
	int selectflag;        /**< True when an area is being selected. */
	int selectpos;         /**< Cursor position of selection start. */
	int selectlineoffset;  /**< Offset of selection start from current line. */

	/** @TODO: Include references to help system, registers, and themes. */

} texteditor;

texteditor * createtexteditor(char * title, stringvector * text, displaymethod * d);

void deletetexteditor(texteditor * editor);
void deletetexteditortext(texteditor * editor);

void textedit(texteditor * editor);

/* For now, just use the real EDITBOX */
#include "editbox.h"
#if 0
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
#endif



#endif
