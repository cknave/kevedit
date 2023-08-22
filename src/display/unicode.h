/* unicode.h    -- Unicode to CP437 lookup table
 * $Id: unicode.h,v 1.0 2023/08/30 18:58:49 kristomu Exp $
 * Copyright (C) 2023 Kristofer Munsterhjelm <kristofer@munsterhjelm.no>
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

#ifndef UNICODE_DEFS_H
#define UNICODE_DEFS_H 1

#include "stddef.h"

extern char unicode_to_CP437[65536];
extern int unicode_conversion_inited;

void init_unicode_conversion();

/* Returns 0 if either the code point is out of range
   or there's no CP437 character for it. */
char get_CP437_from_UTF8(char * utf8_str);

#endif