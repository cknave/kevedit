/* display.h    -- Defines for modular display
 * $Id: display.h,v 1.2 2005/06/29 03:20:34 kvance Exp $
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

#ifdef SDL
/* The OS X port of SDL requires this to be included in the file containing
 * main(). Be sure to include display.h in said file whether you need
 * the display there or not. */
#include "SDL.h"
#endif

#include "keys.h"


#ifndef DISPLAY_H
#define DISPLAY_H 1

typedef struct displaymethod {
	/* Next display method, if more than one is available */
	struct displaymethod *next;

	/* Descriptive name of the display driver */
	char *name;

	/* Version number of the display driver */
	char *version;

	/* Initialize the display */
	int (*init) (void);

	/* Close the display. The display may be initialized again later. */
	void (*end) (void);

	/* Put ch[ar] of co[lour] at x,y */
	void (*putch) (int x, int y, int ch, int co);

	/* Wait for an input event and return the key value */
	int (*getch) (void);

	/* Non-blocking getch()
	 * Returns DKEY_NONE when there are no pending events */
	int (*getkey) (void);

	/* Move the cursor to a given position */
	void (*cursorgo) (int x, int y);

	/* Print a line of text */
	void (*print) (int x, int y, int c, char *s);

	/* Set a descriptive string for the titlebar if available */
	void (*titlebar) (char *);

	/* Check state of shift key
	 * Returns true if shift is depressed, otherwise false */
	int (*shift) (void);

	/* Put a character without necessarily updating the screen */
	void (*putch_discrete) (int x, int y, int ch, int co);

	/* Print a line without necessarily updating the screen */
	void (*print_discrete) (int x, int y, int c, char *s);

	/* Update a region of the screen after a discrete write */
	void (*update) (int x, int y, int w, int h);

} displaymethod;

/* The main display method. Set by RegisterDisplays() */
extern displaymethod display;

/* Find available displays */
extern void RegisterDisplays();


#endif				/* DISPLAY_H */
