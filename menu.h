/* menu.h       -- Code for using the F1-3 panels
 * $Id: menu.h,v 1.6 2002/09/12 07:48:00 bitman Exp $
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

void itemmenu(keveditor * myeditor);
void creaturemenu(keveditor * myeditor);
void terrainmenu(keveditor * myeditor);
void objectlibrarymenu(keveditor * myeditor);


#endif
