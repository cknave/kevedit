/* kevedit.h    -- Editor definitions
 * $Id: kevedit.h,v 1.12 2002/02/16 23:42:28 bitman Exp $
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

#ifndef _KEVEDIT_H
#define _KEVEDIT_H 1

#include "libzzt2/zzt.h"

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

typedef struct editorinfo {
	int cursorx, cursory;
	int drawmode;
	int gradmode;
	int aqumode;
	int blinkmode;
	int textentrymode;
	int defc;
	int forec, backc;
	patbuffer* pbuf;
	patbuffer* standard_patterns;
	patbuffer* backbuffer;

} editorinfo;

/* The all-powerful min/max macros */
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#endif				/* _KEVEDIT_H */
