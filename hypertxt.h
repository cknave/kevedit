/* hypertxt.h  -- hypertext link system
 * $Id: hypertxt.h,v 1.1 2001/10/09 01:14:36 bitman Exp $
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

#ifndef __HYPERTXT_H
#define __HYPERTXT_H 1

#include "svector.h"

/* Both following functions inspect the current line of sv for a !message;text,
 * the first returning true in this case, the second returning the message
 * part, loaded into buffer. !;text is considered no message */
int   ishypermessage(stringvector* sv);
char* gethypermessage(char* buffer, stringvector* sv, int buflen);

int   findhypermessage(char* msg, stringvector* sv);

#endif
