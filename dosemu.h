/* dosemu.h		-- Routines for calling dosemu to run ZZT
 * $Id: dosemu.h,v 1.2 2002/09/24 01:05:37 kvance Exp $
 * Copyright (C) 2002 Kev Vance <kev@kvance.com>
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

/* Only need this if DOSEMU support is to be included */
#ifdef DOSEMU

#ifndef _DOSEMU_H
#define _DOSEMU_H 1

/* Location of the path in the DEXE */
#define DEXE_PATH_LOCATION 0x298A2
#define DEXE_PATH_LENGTH 56

/* Location of the world name in the DEXE */
#define DEXE_WORLD_LOCATION 0x2A89F
#define DEXE_WORLD_LENGTH 8

/* DOSEMU launch function */
int dosemu_launch(char *datapath, char *worldpath, char *world);

#endif	/* _DOSEMU_H */

#endif /* DOSEMU */
