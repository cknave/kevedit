/* display_dos.c        -- Functions for the DOS display method
 * $Id: display_dos.c,v 1.1.1.1 2000/06/15 03:58:10 kvance Exp $
 * Copyright (C) 2000 Kev Vance <kvance@tekktonik.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <conio.h>

#include <sys/nearptr.h>
#include <dpmi.h>
#include <crt0.h>

#include "display.h"
#include "display_dos.h"

short videomem;

int display_dos_init()
{
	__dpmi_regs r;
	r.x.ax = 0x1201;
	r.h.bl = 0x30;
	__dpmi_int(0x10, &r);
	r.x.ax = 0x0003;
	__dpmi_int(0x10, &r);
	videomem = __dpmi_segment_to_descriptor(0xb800);
	_setcursortype(_SOLIDCURSOR);
	return -1;
}

void display_dos_end()
{
	__dpmi_regs r;
	r.x.ax = 0x1202;
	r.h.bl = 0x30;
	__dpmi_int(0x10, &r);
	r.x.ax = 0x0003;
	__dpmi_int(0x10, &r);
	_setcursortype(_NORMALCURSOR);
}

void display_dos_putch(int x, int y, int ch, int co)
{
	_farpokeb(videomem, (y * 80 + x) * 2, ch);
	_farpokeb(videomem, (y * 80 + x) * 2 + 1, co);
}

int display_dos_getch()
{
	return getch();
}

void display_dos_gotoxy(int x, int y)
{
	gotoxy(x + 1, y + 1);
}

void display_dos_print(int x, int y, int c, char *ch)
{
	int i;
	for (i = 0; i < strlen(ch); i++)
		display_dos_putch(x + i, y, ch[i], c);
}

displaymethod display_dos =
{
	NULL,
	"DOS Display Method",
	"1.0",
	display_dos_init,
	display_dos_end,
	display_dos_putch,
	display_dos_getch,
	display_dos_gotoxy,
	display_dos_print
};
