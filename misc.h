/* misc.h       -- General routines for everyday KevEditing
 * $Id: misc.h,v 1.19 2002/09/12 22:05:49 bitman Exp $
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

/* TODO: Everything from this point on needs to be sorted
 * very thoroughly. Most should be moved to other files. */

/* Plot from the backbuffer to the cursor */
void plot(keveditor * myeditor);

/* Make objects more obvious */
void showObjects(keveditor * myeditor);

/* Running zzt */
void runzzt(char* path, char* world);

void texteditor(displaymethod * mydisplay);

void clearboard(ZZTworld * myworld);
ZZTworld * clearworld(ZZTworld * myworld);

void entergradientmode(keveditor * myeditor);
void exitgradientmode(keveditor * myeditor);
int toggledrawmode(keveditor * myeditor);
int togglegradientmode(keveditor * myeditor);

void saveworld(displaymethod * mydisplay, ZZTworld * myworld);
ZZTworld * loadworld(displaymethod * mydisplay, ZZTworld * myworld);

void boardtransfer(displaymethod * mydisplay, ZZTworld * myworld);
void importfromworld(displaymethod * mydisplay, ZZTworld * myworld);
void importfromboard(displaymethod * mydisplay, ZZTworld * myworld);
void exporttoboard(displaymethod * mydisplay, ZZTworld * myworld);

void previouspattern(keveditor * myeditor);
void nextpattern(keveditor * myeditor);

patbuffer* createfillpatterns(keveditor* myeditor);
patbuffer* createstandardpatterns(void);

void floodselect(ZZTblock* block, selection fillsel, int x, int y);
void fillblockbyselection(ZZTblock* block, selection fillsel, patbuffer pbuf, int randomflag);
void fillbyselection(ZZTworld* world, selection fillsel, patbuffer pbuf, int randomflag);

void dofloodfill(keveditor * myeditor, int randomflag);

/* Gradient fill helpers */
void movebykeystroke(int key, int* x, int* y, int minx, int miny, int maxx, int maxy, displaymethod * mydisplay);
int promptforselection(selection sel, gradline * grad, keveditor* myeditor);
int pickgradientpoint(ZZTworld * myworld, int* x, int* y, selection fillsel, patbuffer pbuf, gradline * grad, int randomseed, displaymethod* mydisplay);

void gradientfillbyselection(ZZTworld * myworld, selection fillsel, patbuffer pbuf, gradline grad, int randomseed, int preview, displaymethod * mydisplay);

/* Do the gradient */
void dogradient(keveditor * myeditor);

#endif
