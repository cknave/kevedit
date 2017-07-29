/* screen.h    -- Functions for drawing
 * $Id: screen.h,v 1.2 2005/06/29 03:20:34 kvance Exp $
 * Copyright (C) 2000 Kev Vance <kvance@kvance.com>
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

#ifndef _SCREEN_H
#define _SCREEN_H 1

#include <stdbool.h>

#include "display/display.h"
#include "display/colours.h"

#include "libzzt2/zzt.h"
#include "structures/svector.h"
#include "dialogs/files.h"

#include "kevedit.h"

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
#define LINED_QUIT   2
#define LINED_CANCEL 1
#define LINED_OK     0

/* confirmprompt() return values */
#define CONFIRM_YES    0
#define CONFIRM_NO     1
#define CONFIRM_CANCEL 2
#define CONFIRM_QUIT   3

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

/* Component drawing functions */
void drawsidepanel(displaymethod * d, unsigned char panel[]);
void drawscrollbox(displaymethod * mydisplay, int yoffset, int yendoffset, int updateflag);
void drawscrolltitle(displaymethod * d, char * title);
void drawpanel(displaymethod * d);

/* Keveditor drawing functions */
void updatepanel(keveditor * e);
void drawscreen(keveditor * e);
void cursorspace(keveditor * e);
void drawspot(keveditor * e);

/* Block drawing functions */
void drawblocktile(displaymethod * d, ZZTblock * b, int x, int y, int offx, int offy, int invertflag);
void drawblock(displaymethod * d, ZZTblock * b, int offx, int offy);
void drawblockshowselection(displaymethod * d, ZZTblock * b, selection sel, int offx, int offy);
void drawblockcursorspace(displaymethod * d, ZZTblock * b, int x, int y, int offx, int offy);
void drawblockspot(displaymethod * d, ZZTblock * b, selection sel, int x, int y, int offx, int offy);

/* file dialogs */
char * filedialog(char * dir, char * extension, char * title, int filetypes,
		displaymethod * mydisplay, bool *quit);
char* filenamedialog(char* initname, char* extension, char* prompt,
		int askoverwrite, displaymethod * mydisplay, bool *quit);


/* board dialogs */
int boarddialog(ZZTworld * w, int curboard, char * title, int firstnone, displaymethod * mydisplay);
int switchboard(ZZTworld * w, displaymethod * mydisplay);
char *titledialog(char* prompt, displaymethod * d, bool *quit);

/* panels */
int dothepanel_f1(keveditor * e);
int dothepanel_f2(keveditor * e);
int dothepanel_f3(keveditor * e);

/* Prompts the user to select a char */
int charselect(displaymethod * d, int c);

/* Prompts the user to select a color */
int colorselector(displaymethod * d, textcolor * color);

/* confirmprompt() - Asks a yes/no question */
int confirmprompt(displaymethod * mydisplay, char * prompt);

#endif				/* _SCREEN_H */
