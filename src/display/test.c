/* test.c  -- Test display routines
 * $Id: test.c,v 1.3 2005/05/28 03:17:45 bitman Exp $
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "display.h"
#include "textblock.h"

void
interactiveDisplayTest()
{
	displaymethod * mydisplay;
	int key;
	int x = 0, y = 4;

	RegisterDisplays();
	mydisplay = &display;

	/* Start the display */
	mydisplay->init();

	/* Draw some things */
	mydisplay->print_discrete(20, 0, 0x3F, "Display test program");
	mydisplay->putch_discrete(18, 0, '~', 0x09);
	mydisplay->putch_discrete(41, 0, '~', 0x09);

	mydisplay->print_discrete(4, 2, 0x40, "Press keys to test input (esp. SHIFT), \'q\' or ESC to quit");

	/* Update first line */
	mydisplay->update(0, 0, 80, 3);
	
	/* A simple i/o test loop */
	do {
		if (mydisplay->shift())
			mydisplay->print(10, 6, 0x4F, " SHIFT down ");
		else
			mydisplay->print(10, 6, 0x00, "            ");

		mydisplay->cursorgo(x, y);
		key = mydisplay->getch();
		mydisplay->putch(x, y, key, 0x0F);
		x = (x + 1) % 40;
	} while (key != 'q' && key != DKEY_ESC);

	/* End the display */
	mydisplay->end();
}

void
testTextBlock()
{
	textBlock * block = createTextBlock(80, 25);

	textBlockPutch(block, 5, 7, 'c', 0x03);

	printf("Char: %c, Color: %2X\n",
				 textBlockChar(block, 5, 7), textBlockColour(block, 5, 7));

	deleteTextBlock(block);
}

int
main(int argc, char ** argv)
{
	interactiveDisplayTest();
	/*
	testTextBlock();
	*/

	return 0;
}

