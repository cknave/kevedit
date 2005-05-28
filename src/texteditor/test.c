/**@file texteditor/test.c  Test texteditor.
 * $Id: test.c,v 1.2 2005/05/28 03:17:46 bitman Exp $
 * @author Ryan Phillips
 *
 * Copyright (C) 2003 Ryan Phillips <bitman@users.sf.net>
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


#include "lineeditor.h"
#include "display/display.h"
#include "display/colours.h"

int main(int argc, char *argv[])
{
	displaymethod * d;
	lineeditor * editor;

	RegisterDisplays();
	d = &display;

	d->init();

	editor = createlineeditor(2, 5, "Foo bar", 16, d);
	editor->visiblewidth = 10;
	editor->colour = BLUE_B | WHITE_F | BRIGHT_F;
	editline(editor);
	deletelineeditor(editor);

	d->end();
	
	return 0;
}

