/* screen.h    -- Functions for drawing
 * $Id: screen.h,v 1.14 2001/10/26 23:36:05 bitman Exp $
 * Copyright (C) 2000 Kev Vance <kvance@tekktonik.net>
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
#include "zzt.h"
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

/* line_editor responses */
#define LINED_CANCEL 0
#define LINED_OK     1

int line_editor(int x, int y, int color,
								char* str, int editwidth, int flags, displaymethod* d);
int line_editnumber(int x, int y, int color, int * number, int maxval,
                    displaymethod* d);

extern void drawscrollbox(int yoffset, int yendoffset, displaymethod * mydisplay);
extern void drawpanel(displaymethod * d);
extern void updatepanel(displaymethod * d, editorinfo * e, world * w);
extern void drawscreen(displaymethod * d, world * w, editorinfo * e, char *bigboard, unsigned char paramlist[60][25]);
extern void cursorspace(displaymethod * d, world * w, editorinfo * e, char *bigboard, unsigned char paramlist[60][25]);

extern void drawspot(displaymethod * d, world * w, editorinfo * e, char *bigboard, unsigned char paramlist[60][25]);

extern char * filedialog(char * buffer, char * extention, char * title, displaymethod * mydisplay);
extern int boarddialog(world * w, int curboard, int firstnone, char * title, displaymethod * mydisplay);
extern int switchboard(world * w, editorinfo * e, displaymethod * mydisplay);
extern char * filenamedialog(char * filename, char * prompt, char * ext, int askoverwrite, displaymethod * mydisplay);
extern char *titledialog(displaymethod * d);

extern int dothepanel_f1(displaymethod * d, editorinfo * e);
extern int dothepanel_f2(displaymethod * d, editorinfo * e);
extern int dothepanel_f3(displaymethod * d, editorinfo * e);

/* Prompts the user to select a char */
extern unsigned char charselect(displaymethod * d, int c);

/* Prompts the user to select a color */
int colorselector(displaymethod * d, int * bg, int * fg, int * blink);

/* confirmprompt() return values */
#define CONFIRM_YES    0
#define CONFIRM_NO     1
#define CONFIRM_CANCEL 2

/* confirmprompt() - Asks a yes/no question */
int confirmprompt(displaymethod * mydisplay, char * prompt);

#endif				/* _SCREEN_H */
