/* paramed.h  -- Parameter editor
 * $Id: paramed.h,v 1.1 2002/02/18 08:04:40 bitman Exp $
 * Copyright (C) 2001 Ryan Phillips <bitman@users.sourceforge.net>
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

#ifndef __PARAMED_H
#define __PARAMED_H

#include "libzzt2/zzt.h"
#include "svector.h"

#include "display.h"

/* modifyparam(display, world, x, y)
 * Modify the parameter for the given tile
 */
void modifyparam(displaymethod * d, ZZTworld * w, int x, int y);

/* editprogram(display, param)
 * edit program in param p
 */
void editprogram(displaymethod * d, ZZTparam * p);

/* programtosvector(param, editwidth)
 * creates a string vector from the given param
 */
stringvector programtosvector(ZZTparam * p, int editwidth);

/* svectortoprogram(stringvector)
 * copies the contents of the given svector into a new
 * param. Only the program and length variables are used!
 */
ZZTparam svectortoprogram(stringvector sv);

#endif
