/* patbuffer.h    -- Pattern buffer (backbuffer) utilities
 * $Id: patbuffer.h,v 1.1 2001/04/21 03:06:48 bitman Exp $
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

#ifndef _PATBUFFER_H
#define _PATBUFFER_H 1

#include "zzt.h"
#include "kevedit.h"
#include "display.h"


patbuffer* patbuffer_create(int size);
void patbuffer_resize(patbuffer * pbuf, int delta);

param* param_duplicate(param* p);
void param_remove(board* b, unsigned char paramlist[60][25], int x, int y);

void pat_applycolordata(patbuffer * pbuf, editorinfo * myinfo);

int pat_plot(board* b, patdef pattern, int x, int y, u_int8_t * bigboard, unsigned char paramlist[60][25]);

void push(patbuffer* pbuf, int type, int color, param * p);
void plot(world * myworld, editorinfo * myinfo, displaymethod * mydisplay, u_int8_t * bigboard, unsigned char paramlist[60][25]);
void floodfill(world * myworld, editorinfo * myinfo, displaymethod * mydisplay, u_int8_t * bigboard, unsigned char paramlist[60][25], int xpos, int ypos, char code, u_int8_t colour, int fillmethod);


#endif				/* _PATBUFFER_H */
