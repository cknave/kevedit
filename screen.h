/* screen.h    -- Functions for drawing
 * $Id: screen.h,v 1.24 2002/02/19 09:41:36 bitman Exp $
 * Copyright (C) 2000 Kev Vance <kev@kvance.com>
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

#ifndef _SCREEN_H
#define _SCREEN_H 1

#include "display.h"
#include "libzzt2/zzt.h"
#include "kevedit.h"
#include "svector.h"
#include "files.h"
#include "selection.h"

/* line_editor flags */
#define LINED_NORMAL   0x00
#define LINED_NOUPPER  0x01
#define LINED_NOLOWER  0x02
#define LINED_NOALPHA  LINED_NOUPPER | LINED_NOLOWER
#define LINED_NODIGITS 0x04
#define LINED_NOPUNCT  0x08
#define LINED_NOSPACES 0x10
#define LINED_NOPERIOD 0x20

#define LINED_FILENAME 0x40
#define LINED_NOPATH   0x80

#define LINED_NUMBER   LINED_NOALPHA | LINED_NOPUNCT | LINED_NOSPACES
#define LINED_SNUMBER  LINED_NOALPHA | LINED_NOSPACES | LINED_NOPERIOD

/* line_editor responses */
#define LINED_CANCEL 1
#define LINED_OK     0

/* confirmprompt() return values */
#define CONFIRM_YES    0
#define CONFIRM_NO     1
#define CONFIRM_CANCEL 2

/* line_editor() - edit a string of characters on a single line 
 * 	str:       buffer to be edited
 * 	editwidth: maximum length string in buffer
 * 	flags:     select which classes of characters may not be used
 * 	return:    LINED_OK on enter, LINED_CANCEL on escape */
int line_editor(int x, int y, int color,
								char* str, int editwidth, int flags, displaymethod* d);

/* line_editnumber() - uses line_editor to edit the given number */
int line_editnumber(int x, int y, int color, int * number, int maxval,
                    displaymethod* d);

/* line_editor_raw() - even more powerful line editor, requires careful
 *                     handling
 * 	position: position in the string of the cursor
 * 	return:   whenever a control or non-ascii keypress occurs, it's value is
 * 	          returned */
int line_editor_raw(int x, int y, int color, char* str, int editwidth,
										int* position, int flags, displaymethod* d);

/* Drawing functions */
void drawsidepanel(displaymethod * d, unsigned char panel[]);
void drawscrollbox(int yoffset, int yendoffset, displaymethod * mydisplay);
void drawscrolltitle(displaymethod * d, char * title);
void drawpanel(displaymethod * d);
void updatepanel(displaymethod * d, editorinfo * e, ZZTworld * w);
void drawscreen(displaymethod * d, ZZTworld * w);
void cursorspace(displaymethod * d, ZZTworld * w, editorinfo * e);
void drawspot(displaymethod * d, ZZTworld * w, editorinfo * e);

/* Block drawing functions */
void drawblocktile(displaymethod * d, ZZTblock * b, int x, int y, int offx, int offy);
void drawblock(displaymethod * d, ZZTblock * b, selection alpha, int offx, int offy);
void cursorspaceblock(displaymethod * d, ZZTblock * b, int x, int y, int offx, int offy);
void drawblockspot(displaymethod * d, ZZTblock * b, int x, int y, int offx, int offy);

/* file dialogs */
char * filedialog(char * dir, char * extension, char * title, int filetypes,
									displaymethod * mydisplay);
char* filenamedialog(char* initname, char* extension, char* prompt,
										 int askoverwrite, displaymethod * mydisplay);


/* board dialogs */
int boarddialog(ZZTworld * w, int curboard, char * title, int firstnone, displaymethod * mydisplay);
int switchboard(ZZTworld * w, displaymethod * mydisplay);
char *titledialog(char* prompt, displaymethod * d);

/* panels */
int dothepanel_f1(displaymethod * d, editorinfo * e);
int dothepanel_f2(displaymethod * d, editorinfo * e);
int dothepanel_f3(displaymethod * d, editorinfo * e);

/* Prompts the user to select a char */
int charselect(displaymethod * d, int c);

/* Prompts the user to select a color */
int colorselector(displaymethod * d, int * bg, int * fg, int * blink);

/* confirmprompt() - Asks a yes/no question */
int confirmprompt(displaymethod * mydisplay, char * prompt);

#endif				/* _SCREEN_H */
