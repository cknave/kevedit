/* patbuffer.h    -- Pattern buffer (backbuffer) utilities
 * $Id: patbuffer.h,v 1.2 2005/06/29 03:20:34 kvance Exp $
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

#ifndef _PATBUFFER_H
#define _PATBUFFER_H 1

#include "libzzt2/zzt.h"
#include "display/display.h"
#include "display/colours.h"


#define MAX_BACKBUF 1024

/* Various levels of patbuffer locking */
#define PATBUF_UNLOCK    0x00
#define PATBUF_NOPUSH    0x01
#define PATBUF_NOREPLACE 0x02
#define PATBUF_LOCK      0xFF

typedef struct patbuffer {
	ZZTtile* patterns;
	int size;
	int pos;

	int lock;
} patbuffer;

typedef struct backbuffers {
	/* Pattern buffers */
	patbuffer* pbuf;
	patbuffer* standard_patterns;
	patbuffer* backbuffer;
} backbuffers;


patbuffer* patbuffer_create(int size);
void deletepatternbuffer(patbuffer* pbuf);
void patbuffer_resize(patbuffer * pbuf, int delta);

void pat_applycolordata(patbuffer * pbuf, textcolor color);

void push(patbuffer* pbuf, ZZTtile tile);
void patreplace(patbuffer * pbuf, ZZTtile pattern);

#endif				/* _PATBUFFER_H */
