/* display.c    -- Functions for the modular display
 * $Id: display.c,v 1.2 2001/01/07 23:55:41 bitman Exp $
 * Copyright (C) 2000 Kev Vance <kvance@tekktonik.net>
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

#include "display.h"

#ifdef LONG_FILES

#ifdef GGI
#include "display_ggi.h"
#endif
#ifdef VCSA
#include "display_vcsa.h"
#endif
#ifdef DOS
#include "display_dos.h"
#endif

#else

#ifdef GGI
#include "d_ggi.h"
#endif
#ifdef VCSA
#include "d_vcsa.h"
#endif
#ifdef DOS
#include "d_dos.h"
#endif

#endif

displaymethod display;

void RegisterDisplays()
{
#ifdef DOS
	display = display_dos;
#else
	display = display_ggi;
#ifdef VCSA
	display.next = &display_vcsa;
#endif
#endif
}
