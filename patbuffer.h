/* patbuffer.h    -- Pattern buffer (backbuffer) utilities
 * $Id: patbuffer.h,v 1.5 2002/02/16 23:42:28 bitman Exp $
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

#ifndef _PATBUFFER_H
#define _PATBUFFER_H 1

#include "libzzt2/zzt.h"
#include "kevedit.h"
#include "display.h"


patbuffer* patbuffer_create(int size);
void deletepatternbuffer(patbuffer* pbuf);
void patbuffer_resize(patbuffer * pbuf, int delta);

void pat_applycolordata(patbuffer * pbuf, editorinfo * myinfo);

void push(patbuffer* pbuf, ZZTtile tile);
void patreplace(patbuffer * pbuf, ZZTtile pattern);
void plot(ZZTworld * myworld, editorinfo * myinfo, displaymethod * mydisplay);

#endif				/* _PATBUFFER_H */
