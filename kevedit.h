/* kevedit.h    -- Editor definitions
 * $Id: kevedit.h,v 1.4 2001/04/21 03:06:48 bitman Exp $
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

#ifndef _KEVEDIT_H
#define _KEVEDIT_H 1

#include "zzt.h"

typedef struct patdef {
	int type;
	int color;
	param* patparam;
} patdef;


typedef struct patbuffer {
	patdef* patterns;
	int size;
	int pos;
} patbuffer;


typedef struct editorinfo {
	int cursorx, cursory;
	int drawmode;
	int gradmode;
	int getmode;
	int blinkmode;
	int textentrymode;
	int defc;
	int forec, backc;
//	int pattern;
	patbuffer* pbuf;
	patbuffer* standard_patterns;
	patbuffer* backbuffer;

	int playerx, playery;

	char *currentfile;
	char *currenttitle;

	int curboard;
} editorinfo;

#endif				/* _KEVEDIT_H */
