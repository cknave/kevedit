/* zzm.h  -- zzm file routines
 * $Id: zzm.h,v 1.1 2003/11/01 23:45:57 bitman Exp $
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
 * Foundation, Inc., 59 Temple Place Suite 330; Boston, MA 02111-1307, USA.
 */

#ifndef __ZZM_H
#define __ZZM_H

#include "structures/svector.h"
#include "display/display.h"


/* zzmpullsong() - pulls song #songnum out of zzmv and returns it */
stringvector zzmpullsong(stringvector * zzmv, int songnum);

/* zzmpicksong() - presents a dialog to choose a song based on title */
int zzmpicksong(stringvector * zzmv, displaymethod * d);

/* zzmripsong() - rip a song from zzt object code */
stringvector zzmripsong(stringvector * zoc, int maxseparation);

#endif
