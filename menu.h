/* menu.h       -- Code for using the F1-3 panels
 * $Id: menu.h,v 1.4 2001/11/10 07:42:39 bitman Exp $
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

#ifndef _MENU_H
#define _MENU_H

#include "kevedit.h"

#include "display.h"

void itemmenu(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25]);
void creaturemenu(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25]);
void terrainmenu(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25]);
void objectlibrarymenu(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25]);


#endif
