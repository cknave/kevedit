/* zzl.c  -- ZZT Object Library file routines
 * $Id: zzl.h,v 1.2 2002/02/16 10:25:22 bitman Exp $
 * Copyright (C) 2000 Ryan Phillips <bitman@users.sf.net>
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

#ifndef __ZZL_H
#define __ZZL_H

#include "kevedit.h"
#include "svector.h"
#include "display.h"

/* zzlpickobject() - presents a list of objects in the zzl file. zzlv->cur is
 *                   set to the title line of the selected object */
int zzlpickobject(stringvector * zzlv, displaymethod * d);

/* zzlpullobject() - pulls the object who's zzl definition starts on the
 *                   currentt line. */
ZZTtile zzlpullobject(stringvector zzlv, int x, int y, int undert, int underc);

/* zzlappendobject() - appends an object to a zzl with given title */
int zzlappendobject(stringvector * zzlv, ZZTtile obj, char* title, int editwidth);

#endif
