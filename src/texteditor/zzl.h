/* zzl.c  -- ZZT Object Library file routines
 * $Id: zzl.h,v 1.1 2003/11/01 23:45:57 bitman Exp $
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
 * Foundation, Inc., 59 Temple Place Suite 330; Boston, MA 02111-1307, USA.
 */

#ifndef __ZZL_H
#define __ZZL_H

#include "kevedit/kevedit.h"
#include "structures/svector.h"
#include "display/display.h"

/* zzlpickobject() - presents a list of objects in the zzl file. zzlv->cur is
 *                   set to the title line of the selected object */
int zzlpickobject(stringvector * zzlv, displaymethod * d);

/* zzlpullobject() - pulls the object who's zzl definition starts on the
 *                   currentt line. */
ZZTtile zzlpullobject(stringvector zzlv, int x, int y, int undert, int underc);

/* zzlappendobject() - appends an object to a zzl with given title */
int zzlappendobject(stringvector * zzlv, ZZTtile obj, char* title, int editwidth);

#endif
