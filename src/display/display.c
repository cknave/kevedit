/* display.c    -- Functions for the modular display
 * $Id: display.c,v 1.3 2005/06/29 03:20:34 kvance Exp $
 * Copyright (C) 2000 Kev Vance <kvance@kvance.com>
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

#if defined HAVE_CONFIG_H
#include <config.h>
#endif

#include "display.h"
#include "display_stdio.h"

#if defined(SDL)
#include "display_sdl.h"
#endif
#if defined(VCSA)
#include "display_vcsa.h"
#endif
#if defined(DOS)
#include "display_dos.h"
#endif

displaymethod display;

void RegisterDisplays()
{
#if defined(DOS)
	display = display_dos;
#elif defined(SDL)
	display = display_sdl;
#if defined(VCSA)
	display.next = &display_vcsa;
#endif
#else
	display = display_stdio;
#endif
}
