
/* display.h    -- Defines for modular display
 * $Id: display.h,v 1.2 2000/08/08 01:57:38 kvance Exp $
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
	char *name;		// Name of display driver

	char *version;		// Version of driver

	int (*init) (void);	// Start the driver

	void (*end) (void);	// Close the driver

	void (*putch) (int x, int y, int ch, int co);	// Put ch[ar] of co[lour] at x,y

	int (*getch) (void);	// DOS style getch

	void (*cursorgo) (int x, int y);	// Move the cursor

	void (*print) (int x, int y, int c, char *ch);	// Simple print function

	void (*titlebar) (char *);	// Set the titlebar if in MS-Windows or X11

} displaymethod;

extern displaymethod display;

extern void RegisterDisplays();

#endif				/* _DISPLAY_H */
