/* misc.h       -- General routines for everyday KevEditing
 * $Id: misc.h,v 1.12 2002/02/16 10:25:22 bitman Exp $
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

#include "kevedit.h"

#include "display.h"

#include "selection.h"
#include "gradient.h"
#include "svector.h"

/* Kevedit initialization routines */
displaymethod * pickdisplay(displaymethod * rootdisplay);
void initeditorinfo(editorinfo * myinfo);

/* TODO: Everything from this point on needs to be sorted
 * very thoroughly. Most should be moved to other files. */

/* editprogram() - edit program in param p */
void editprogram(displaymethod * d, ZZTparam * p);

/* programtosvector() - creates a string vector from the given param */
stringvector programtosvector(ZZTparam * p, int editwidth);

/* svectortoprogram() - copies the contents of the given svector into a new
 *              parameter. Only the moredata and length variables are used! */
ZZTparam svectortoprogram(stringvector sv);

/* Running zzt */
void runzzt(char* path, char* world);

void texteditor(displaymethod * mydisplay);

void clearboard(ZZTworld * myworld);
ZZTworld * clearworld(ZZTworld * myworld);

int toggledrawmode(editorinfo * myinfo);
int togglegradientmode(editorinfo * myinfo);

void saveworldprompt(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo);

void previouspattern(editorinfo * myinfo);
void nextpattern(editorinfo * myinfo);

void floodselect(ZZTblock* block, selection fillsel, int x, int y);
void fillblockbyselection(ZZTblock* block, selection fillsel, patbuffer pbuf, int randomflag);
void fillbyselection(ZZTworld* world, selection fillsel, patbuffer pbuf, int randomflag);

void dofloodfill(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo, int randomflag);

/* Gradient fill helpers */
void movebykeystroke(int key, int* x, int* y, int minx, int miny, int maxx, int maxy, displaymethod * mydisplay);
int promptforselection(selection sel, gradline * grad, editorinfo* myinfo, ZZTworld * myworld, displaymethod * mydisplay);
int pickgradientpoint(ZZTworld * myworld, int* x, int* y, selection fillsel, patbuffer pbuf, gradline * grad, int randomseed, displaymethod* mydisplay);

void gradientfillbyselection(ZZTworld * myworld, selection fillsel, patbuffer pbuf, gradline grad, int randomseed, int preview, displaymethod * mydisplay);

/* Do the gradient */
void dogradient(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo);

#endif
