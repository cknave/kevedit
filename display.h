/* display.h    -- Defines for modular display
 * $Id: display.h,v 1.1.1.1 2000/06/15 03:58:04 kvance Exp $
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

#ifndef _DISPLAY_H
#define _DISPLAY_H 1

typedef struct displaymethod {
	struct displaymethod *next;
	char *name;
	char *version;
	int (*init) (void);
	void (*end) (void);
	void (*putch) (int x, int y, int ch, int co);
	int (*getch) (void);
	void (*cursorgo) (int x, int y);
	void (*print) (int x, int y, int c, char *ch);
} displaymethod;

extern displaymethod display;

extern void RegisterDisplays();

#endif				/* _DISPLAY_H */
