/* zzm.h  -- zzm file routines
 * $Id: zzm.h,v 1.8 2002/08/23 22:34:49 bitman Exp $
 * Copyright (C) 2000 Ryan Phillips <bitman@scn.org>
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

#ifndef __ZZM_H
#define __ZZM_H

#include "svector.h"
#include "display.h"


/* zzmpullsong() - pulls song #songnum out of zzmv and returns it */
stringvector zzmpullsong(stringvector * zzmv, int songnum);

/* zzmpicksong() - presents a dialog to choose a song based on title */
int zzmpicksong(stringvector * zzmv, displaymethod * d);

#endif
