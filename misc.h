/* misc.h       -- General routines for everyday KevEditing
 * $Id: misc.h,v 1.8 2001/10/27 19:30:42 kvance Exp $
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

#ifndef _MISC_H
#define _MISC_H 1

#include "display.h"

/* Running zzt */
int runzztforworld(char* zztpath, char* worldname);
int copyzztdatfile(char* srcdir, char* destdir);

/* Finds location of executable based on main()'s argv[0] and cwd() */
char* locateself(char* argv0);

/* Filename parsing */
char* fileof(char* buffer, char* fullpath, int buflen);
char* pathof(char* buffer, char* fullpath, int buflen);

/* Confirms that a file exists */
int fileexists(char* filename);

/* Kevedit initialization routines */
displaymethod * pickdisplay(displaymethod * rootdisplay);
void initeditorinfo(editorinfo * myinfo);

/* TODO: Everything from this point on needs to be sorted
 * very thoroughly. Most should be moved to other files. */

void runzzt(char* zztpath, char *world);
void showParamData(param * p, int paramNumber, displaymethod * d);
void texteditor(displaymethod * mydisplay);

void clearboard(world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25]);
world * clearworld(world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25]);
int toggledrawmode(editorinfo * myinfo);
int togglegradientmode(editorinfo * myinfo);
void saveworldprompt(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard);
void changeboard(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25]);

void updateparamlist(world * myworld, editorinfo * myinfo, unsigned char paramlist[60][25]);
void updateinfo(world * myworld, editorinfo * myinfo, char * bigboard);

void previouspattern(editorinfo * myinfo);
void nextpattern(editorinfo * myinfo);

void dofloodfill(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25], int randomflag);

#endif
