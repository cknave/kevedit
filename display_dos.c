/* display_dos.c        -- Functions for the DOS display method
 * $Id: display_dos.c,v 1.4 2000/09/02 04:33:23 kvance Exp $
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

/* God, I love this DOS stuff.  If you can't tell from my coding style (or
 * lack thereof), I'm an assembly programmer at heart :)
 */

#include <stdlib.h>
#include <conio.h>

#include <sys/nearptr.h>
#include <dpmi.h>
#include <crt0.h>
#include <go32.h>

#include "display.h"
#include "display_dos.h"

short videomem;
int windows;

void release_time_slice()
{
/* INT 2F - MS Windows, DPMI, various - RELEASE CURRENT VIRTUAL MACHINE TIME-SLICE
   AX = 1680h
   Return: AL = status
   00h if the call is supported
   80h (unchanged) if the call is not supported
   Notes:  programs can use this function in idle loops to enhance performance
   under multitaskers; this call is supported by MS Windows 3+, DOS 5+,
   DPMI 1.0+, and in OS/2 2.0+ for multitasking DOS applications
   does not block the program; it just gives up the remainder of the time
   slice
   should not be used by Windows-specific programs
   when called very often without intermediate screen output under
   MS Windows 3.x, the VM will go into an idle-state and will not
   receive the next slice before 8 seconds have elapsed. This time can
   be changed in SYSTEM.INI through "IdleVMWakeUpTime=<seconds>".
   Setting it to zero results in a long wait.
   this function has no effect under OS/2 2.10-4.0 if the DOS box has an
   "Idle Sensitivity" setting of 100 */
	__dpmi_regs r;
	r.x.ax = 0x1680;
	__dpmi_int(0x2f, &r);
}

int display_dos_init()
{
	__dpmi_regs r;
	// Set char-smashed-together mode
	r.x.ax = 0x1201;
	r.h.bl = 0x30;
	__dpmi_int(0x10, &r);
	r.x.ax = 0x0003;
	__dpmi_int(0x10, &r);

	// Pointer to video memory
	videomem = __dpmi_segment_to_descriptor(0xb800);
	// Block cursor
	_setcursortype(_SOLIDCURSOR);

	// No windows by default
	windows = 0;

	// Check for Win95/Win98
	if (getenv("winbootdir") != NULL)
		windows = 1;
	// Check for WinNT/Win2k
	if ((getenv("OS") != NULL) && !strcmp(getenv("OS"), "Windows_NT"))
		windows = 2;

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
	/* We can always release a time slice because we're always in some
	   kind of DPMI */
	release_time_slice();
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


void display_dos_titlebar(char *title)
{
	int i;
	__dpmi_regs r;
	char buffer[81];

/* INT 2F - Windows95 - TITLE - SET APPLICATION TITLE
 * AX = 168Eh
 * DX = 0000h
 * ES:DI -> ASCIZ application title (max 79 chars+NUL)
 * Return: AX = status
 * 0000h failed
 * 0001h successful
 * Note:   if ES:DI is 0000h:0000h or points at an empty string, the current
 * title is removed
 * BUG:    this function can return a successful status even though the title was
 * not changed; reportedly, waiting for two clock ticks after program
 * startup solves this problem
 */

	if (windows == 1) {
		/* Put the title in the MS-Windows window.  Max 79 chars+NULL */
		if (strlen(title) > 79) {
			strncpy(&buffer, title, 79);
			buffer[79] = '\0';
		} else {
			strcpy(buffer, title);
		}
		/* Copy the title to the transfer buffer */
		dosmemput(&buffer, 80, __tb);
		r.x.ax = 0x168E;
		r.x.dx = 0x0000;
		r.x.di = __tb & 0x0f;
		r.x.es = __tb >> 4;
		__dpmi_int(0x2f, &r);
	}
	if (windows == 2) {
		/* FIXME -- Anyone know how to do this in NT? */
	}
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
	display_dos_print,
	display_dos_titlebar
};
