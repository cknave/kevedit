/* display.h    -- Defines for modular display
 * $Id: display.h,v 1.18 2003/10/02 01:16:55 bitman Exp $
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
 * Foundation, Inc., 59 Temple Place Suite 330; Boston, MA 02111-1307, USA.
 */

#ifndef _DISPLAY_H
#define _DISPLAY_H 1

#ifdef SDL
/* OS X port requires this in the file containing main() */
#include "SDL.h"
#endif

typedef struct displaymethod {
	struct displaymethod *next;
	char *name;		/* Name of display driver */

	char *version;		/* Version of driver */

	int (*init) (void);	/* Start the driver */

	void (*end) (void);	/* Close the driver */

	void (*putch) (int x, int y, int ch, int co);	/* Put ch[ar] of co[lour] at x,y */

	int (*getch) (void);	/* DOS style getch */

	int (*getkey) (void); /* Non-blocking getch (returns DKEY_NONE on empty buffer) */

	void (*cursorgo) (int x, int y);	/* Move the cursor */

	void (*print) (int x, int y, int c, char *ch);	/* Simple print function */

	void (*titlebar) (char *);	/* Set the titlebar if in MS-Windows or X11 */

	int (*shift) (void);	/* Is shift pressed? */

	void (*putch_discrete)(int x, int y, int ch, int co);	/* Draw a character without necessarily updating the screen */

	void (*print_discrete) (int x, int y, int c, char *ch);	/* Print without necessarily updating the screen */

	void (*update)(int x, int y, int w, int h);	/* Update the screen after a putch_discrete() */

} displaymethod;

extern displaymethod display;

extern void RegisterDisplays();

/* Display Keys */
#define DKEY_NONE       0x00

/* Flags */
/* TODO: actually used these */
#define DKEY_SHIFT      0x100
#define DKEY_ALT        0x200
#define DKEY_CTRL       0x400
#define DKEY_NUMPAD     0x800

/* DOS extended key */
#define DDOSKEY_EXT     0x1000

/* Standard action keys */
#define DKEY_ENTER      13
#define DKEY_ESC        27
#define DKEY_BACKSPACE  '\b'
#define DKEY_TAB        '\t'

#define DKEY_SHIFT_TAB  (15 | DDOSKEY_EXT)

/* Arrow keys */
#define DKEY_UP         (0x48 | DDOSKEY_EXT)
#define DKEY_DOWN       (0x50 | DDOSKEY_EXT)
#define DKEY_LEFT       (0x4B | DDOSKEY_EXT)
#define DKEY_RIGHT      (0x4D | DDOSKEY_EXT)

/* Navigation keys */
#define DKEY_INSERT     (0x52 | DDOSKEY_EXT)
#define DKEY_DELETE     (0x53 | DDOSKEY_EXT)
#define DKEY_HOME       (0x47 | DDOSKEY_EXT)
#define DKEY_END        (0x4F | DDOSKEY_EXT)
#define DKEY_PAGEUP     (0x49 | DDOSKEY_EXT)
#define DKEY_PAGEDOWN   (0x51 | DDOSKEY_EXT)

/* Function keys */
#define DKEY_F1         (0x3B | DDOSKEY_EXT)
#define DKEY_F2         (0x3C | DDOSKEY_EXT)
#define DKEY_F3         (0x3D | DDOSKEY_EXT)
#define DKEY_F4         (0x3E | DDOSKEY_EXT)
#define DKEY_F5         (0x3F | DDOSKEY_EXT)
#define DKEY_F6         (0x40 | DDOSKEY_EXT)
#define DKEY_F7         (0x41 | DDOSKEY_EXT)
#define DKEY_F8         (0x42 | DDOSKEY_EXT)
#define DKEY_F9         (0x43 | DDOSKEY_EXT)
#define DKEY_F10        (0x44 | DDOSKEY_EXT)
#define DKEY_F11        (0x85 | DDOSKEY_EXT)
#define DKEY_F12        (0x86 | DDOSKEY_EXT)

/* Alt-arrow keys */
#define DKEY_ALT_LEFT   (155 | DDOSKEY_EXT)
#define DKEY_ALT_RIGHT  (157 | DDOSKEY_EXT)
#define DKEY_ALT_UP     (152 | DDOSKEY_EXT)
#define DKEY_ALT_DOWN   (160 | DDOSKEY_EXT)

/* Ctrl-letter keys */
/* TODO: replace with 'a' | DKEY_CTRL, etc */
#define DKEY_CTRL_A     ('a' - 0x60)
#define DKEY_CTRL_B     ('b' - 0x60)
#define DKEY_CTRL_C     ('c' - 0x60)
#define DKEY_CTRL_D     ('d' - 0x60)
#define DKEY_CTRL_E     ('e' - 0x60)
#define DKEY_CTRL_F     ('f' - 0x60)
#define DKEY_CTRL_G     ('g' - 0x60)
#define DKEY_CTRL_H     ('h' - 0x60)  /* Same as BS */
#define DKEY_CTRL_I     ('i' - 0x60)  /* Same as TAB */
#define DKEY_CTRL_J     ('j' - 0x60)
#define DKEY_CTRL_K     ('k' - 0x60)
#define DKEY_CTRL_L     ('l' - 0x60)
#define DKEY_CTRL_M     ('m' - 0x60)  /* Same as ENTER */
#define DKEY_CTRL_N     ('n' - 0x60)
#define DKEY_CTRL_O     ('o' - 0x60)
#define DKEY_CTRL_P     ('p' - 0x60)
#define DKEY_CTRL_Q     ('q' - 0x60)
#define DKEY_CTRL_R     ('r' - 0x60)
#define DKEY_CTRL_S     ('s' - 0x60)
#define DKEY_CTRL_T     ('t' - 0x60)
#define DKEY_CTRL_U     ('u' - 0x60)
#define DKEY_CTRL_V     ('v' - 0x60)
#define DKEY_CTRL_W     ('w' - 0x60)
#define DKEY_CTRL_X     ('x' - 0x60)
#define DKEY_CTRL_Y     ('y' - 0x60)
#define DKEY_CTRL_Z     ('z' - 0x60)

#define DKEY_CTRL_DELETE  (147 | DDOSKEY_EXT)

/* Alt-letter keys */
/* TODO: replace with 'a' | DKEY_ALT, etc */
#define DKEY_ALT_I      (23 | DDOSKEY_EXT)
#define DKEY_ALT_M      (50 | DDOSKEY_EXT)
#define DKEY_ALT_O      (24 | DDOSKEY_EXT)
#define DKEY_ALT_S      (31 | DDOSKEY_EXT)
#define DKEY_ALT_T      (20 | DDOSKEY_EXT)
#define DKEY_ALT_Z      (44 | DDOSKEY_EXT)

#define DKEY_ALT_MINUS  (130 | DDOSKEY_EXT) 
/* TODO: make alt + different from alt = */
#define DKEY_ALT_PLUS   (131 | DDOSKEY_EXT)
#define DKEY_ALT_EQUALS (131 | DDOSKEY_EXT)

#endif				/* _DISPLAY_H */
