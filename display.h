/* display.h    -- Defines for modular display
 * $Id: display.h,v 1.10 2001/12/15 00:54:53 bitman Exp $
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

#ifndef _DISPLAY_H
#define _DISPLAY_H 1

typedef struct displaymethod {
	struct displaymethod *next;
	char *name;		/* Name of display driver */

	char *version;		/* Version of driver */

	int (*init) (void);	/* Start the driver */

	void (*end) (void);	/* Close the driver */

	void (*putch) (int x, int y, int ch, int co);	/* Put ch[ar] of co[lour] at x,y */

	int (*getch) (void);	/* DOS style getch */

	int (*kbhit) (void);  /* TRUE when key waits in buffer */

	void (*cursorgo) (int x, int y);	/* Move the cursor */

	void (*print) (int x, int y, int c, char *ch);	/* Simple print function */

	void (*titlebar) (char *);	/* Set the titlebar if in MS-Windows or X11 */
	int (*shift) (void);	/* Is shift pressed? */

} displaymethod;

extern displaymethod display;

extern void RegisterDisplays();

/* DOS keys */
/* TODO: move to another file */

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
#define DKEY_CTRL_T     ('t' - 0x60)
#define DKEY_CTRL_V     ('v' - 0x60)
#define DKEY_CTRL_X     ('x' - 0x60)
#define DKEY_CTRL_Y     ('y' - 0x60)

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


#endif				/* _DISPLAY_H */
