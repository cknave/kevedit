/* kevedit.h    -- Editor definitions
 * $Id: kevedit.h,v 1.1.1.1 2000/06/15 03:58:11 kvance Exp $
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

typedef struct editorinfo {
	int cursorx, cursory;
	int drawmode;
	int blinkmode;
	int defc;
	int forec, backc;
	int pattern;

	int playerx, playery;

	char *currentfile;
	char *currenttitle;

	int curboard;
} editorinfo;

typedef struct patdef {
	int type;
	int color;
} patdef;

#endif				/* _KEVEDIT_H */
