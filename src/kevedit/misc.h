/* misc.h       -- General routines for everyday KevEditing
 * $Id: misc.h,v 1.3 2005/06/29 03:20:34 kvance Exp $
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

#ifndef _MISC_H
#define _MISC_H 1

#include <stdbool.h>

#include "kevedit.h"

#include "display/display.h"

#include "structures/selection.h"
#include "structures/gradient.h"
#include "structures/svector.h"

/* TODO: Everything in this file needs to be sorted
 * very thoroughly. Most should be moved to other files. */

/* Copy and paste */
void copy(keveditor * myeditor);
int paste(keveditor * myeditor);
int countparams(ZZTblock *block);
int pasteblock(ZZTblock *dest, ZZTblock *src, selection destsel, selection srcsel, int x, int y);

/* Change a tile's color, taking text into account */
void encodetilecolor(ZZTtile * tile, textcolor color);

/* Plot from the backbuffer to the cursor */
void plot(keveditor * myeditor);

/* Make objects more obvious */
int showObjects(keveditor * myeditor);

/* Running zzt */
void runzzt(char* path, char* world);

int texteditordialog(displaymethod * mydisplay);

void clearboard(ZZTworld * myworld);
ZZTworld * clearworld(ZZTworld * myworld);

void entergradientmode(keveditor * myeditor);
void exitgradientmode(keveditor * myeditor);
int toggledrawmode(keveditor * myeditor);
int togglegradientmode(keveditor * myeditor);

int saveworld(displaymethod * mydisplay, ZZTworld * myworld);
ZZTworld * loadworld(displaymethod * mydisplay, ZZTworld * myworld, char *filename, bool *quit);

int boardtransfer(displaymethod * mydisplay, ZZTworld * myworld);
int importfromworld(displaymethod * mydisplay, ZZTworld * myworld);
int importfromboard(displaymethod * mydisplay, ZZTworld * myworld);
int exporttoboard(displaymethod * mydisplay, ZZTworld * myworld);

void previouspattern(keveditor * myeditor);
void nextpattern(keveditor * myeditor);

patbuffer* createfillpatterns(keveditor* myeditor);
patbuffer* createstandardpatterns(void);

void floodselect(ZZTblock* block, selection fillsel, int x, int y);
void tileselect (ZZTblock* block, selection fillsel, ZZTtile tile);

void fillbyselection(keveditor *myeditor, ZZTworld* world, selection fillsel, patbuffer pbuf, int randomflag);

void dofloodfill(keveditor * myeditor, int randomflag);

/* Gradient fill helpers */
void movebykeystroke(int key, int* x, int* y, int minx, int miny, int maxx, int maxy, displaymethod * mydisplay);
int promptforselection(selection sel, gradline * grad, keveditor* myeditor);
int pickgradientpoint(ZZTworld * myworld, int* x, int* y, selection fillsel, patbuffer pbuf, gradline * grad, int randomseed, displaymethod* mydisplay);

void gradientfillbyselection(ZZTworld * myworld, selection fillsel, patbuffer pbuf, gradline grad, int randomseed, int preview, displaymethod * mydisplay);

/* Do the gradient */
int dogradient(keveditor * myeditor);

#endif
