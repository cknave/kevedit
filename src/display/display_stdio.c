/* display_stdio.c   -- Dummy display using stdio
 * $Id: display_stdio.c,v 1.1 2003/11/01 23:45:56 bitman Exp $
 * Copyright (C) 2003 Ryan Phillips <bitman@users.sourceforge.net>
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

#include "display_stdio.h"

#include <stdio.h>
#include <string.h>

#define INPUT_BUFFER_MAX 256

static int shift_state = 0;
static int brief = 0;

void display_stdio_print_help(void)
{
	/* A helpful message */
	char message[] =
		"This display driver is for testing purposes only.\n\n"
		"To provide input, simply type a string of characters and press ENTER.\n"
		"For special characters, use one of the following special commands:\n"
		"\t# number   uses the decimal value of number\n"
		"\tx hex      uses the hexadecimal value of hex\n"
		"\t\\symbol    escapes the given symbol (including newline!)\n"
		"\t: command  use a key name (e.g. up, left, esc, enter) or\n"
		"\t           one of (shift, SHIFT, brief, verbose).\n\n"
		;

	printf(message);
}

int display_stdio_init(void)
{
	shift_state = 0;
	printf("%s version %s initialized\n", display_stdio.name, display_stdio.version);
	display_stdio_print_help();
	printf("Enter a command or press enter to continue...\n");
	display_stdio_getch();

	return 1;
}

void display_stdio_end(void)
{
	printf("%s closed.\n\n", display_stdio.name);
}

void display_stdio_putch(int x, int y, int ch, int co)
{
	char show_char = '?';

	if (ch >= ' ' && ch <= '~')
		show_char = ch;

	if (brief)
		printf("%c", show_char);
	else
		printf("putch at\t(%02X, %02X), color: %02X, character %02X \'%c\'\n",
					 x, y, co, ch, show_char);
}

int display_stdio_getch(void)
{
	/* Keycode */
	int keycode = 0;

	/* Command buffer */
	char command[INPUT_BUFFER_MAX];

	/* Check for keycode with hex specifier */
	if (scanf("x %x", &keycode) > 0) {
		return keycode;
	} else if (scanf("# %d", &keycode) > 0) {
		return keycode;
	} else if (scanf(": %s", &command) > 0) {
		/* Handle special commands */

		/* Named keys */
		if (!strcmp(command, "left"))  return DKEY_LEFT;
		if (!strcmp(command, "right")) return DKEY_RIGHT;
		if (!strcmp(command, "up"))    return DKEY_UP;
		if (!strcmp(command, "down"))  return DKEY_DOWN;
		if (!strcmp(command, "esc"))   return DKEY_ESC;
		if (!strcmp(command, "enter"))   return DKEY_ENTER;

		/* Set shift state */
		if (!strcmp(command, "shift")) { shift_state = 0; return 0; }
		if (!strcmp(command, "SHIFT")) { shift_state = 1; return 0; }

		/* Brief vs. verbose display */
		if (!strcmp(command, "brief"))   { brief = 1; return 0; }
		if (!strcmp(command, "verbose")) { brief = 0; return 0; }

		/* Help */
		if (!strcmp(command, "help")) { display_stdio_print_help(); return 0; }

		/* Return first character */
		return command[0];
	} else if (scanf("\\%c", &keycode) > 0) {
		return keycode;
	} else if (scanf("%c", &keycode) > 0) {
		/* Ignore newlines */
		if (keycode == '\n')
			return display_stdio_getch();

		return keycode;
	}

	return 0;
}

int display_stdio_getkey(void)
{
	/* There really isn't any way to provide non-blocking input */
	return display_stdio_getch();
}

void display_stdio_cursorgo(int x, int y)
{
	if (brief)
		printf("\n[-> (%02X, %02X)] ", x, y);
	else
		printf("cursor moved to\t(%02X, %02X)\n", x, y);
}

void display_stdio_print(int x, int y, int c, char *s)
{
	if (brief)
		printf("\n[(%02X, %02X) %02X] %s\n", x, y, c, s);
	else
		printf("print at\t(%02X, %02X), color:\t%02X, string:\n%s\n\n", x, y, c, s);
}

void display_stdio_titlebar(char *s)
{
	if (brief)
		printf("\n[title] \"%s\"", s);
	else
		printf("title set:\t%s\n", s);
}

int display_stdio_shift(void)
{
	return shift_state;
}

void display_stdio_discrete_putch(int x, int y, int ch, int co)
{
	if (!brief)
		printf("discrete ");
	display_stdio_putch(x, y, ch, co);
}

void display_stdio_discrete_print(int x, int y, int c, char *s)
{
	if (brief)
		printf("*");
	else
	printf("discrete ");
	display_stdio_print(x, y, c, s);
}

void display_stdio_update(int x, int y, int w, int h)
{
	if (brief)
		printf("\n*[(%02X, %02X) (%02Xx%02X)] ", x, y, w, h);
	else
		printf("updating area from\t(%02X, %02X) of size (%02Xx%02X)\n", x, y, w, h);
}

displaymethod display_stdio = {
	NULL,
	"Stdio Display Method",
	"1.0",
	display_stdio_init,
	display_stdio_end,
	display_stdio_putch,
	display_stdio_getch,
	display_stdio_getkey,
	display_stdio_cursorgo,
	display_stdio_print,
	display_stdio_titlebar,
	display_stdio_shift,

	display_stdio_discrete_putch,
	display_stdio_discrete_print,
	display_stdio_update
};

