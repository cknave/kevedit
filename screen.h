/* kevedit.h    -- Editor definitions
 * $Id: screen.h,v 1.3 2000/08/18 04:39:47 bitman Exp $
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

extern void drawscrollbox(int yoffset, int yendoffset, displaymethod * mydisplay);
extern void drawpanel(displaymethod * d);
extern void updatepanel(displaymethod * d, editorinfo * e, world * w);
extern void drawscreen(displaymethod * d, world * w, editorinfo * e, char *bigboard, unsigned char paramlist[60][25]);
extern void cursorspace(displaymethod * d, world * w, editorinfo * e, char *bigboard, unsigned char paramlist[60][25]);

extern void drawspot(displaymethod * d, world * w, editorinfo * e, char *bigboard, unsigned char paramlist[60][25]);

extern int filedialog(char *extention, displaymethod * mydisplay);
extern int boarddialog(world * w, editorinfo * e, displaymethod * mydisplay);


extern int dothepanel_f1(displaymethod * d, editorinfo * e);
extern int dothepanel_f2(displaymethod * d, editorinfo * e);
extern int dothepanel_f3(displaymethod * d, editorinfo * e);

extern char charselect(displaymethod * d);

#endif				/* _SCREEN_H */
