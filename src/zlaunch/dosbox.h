/* dosbox.h		-- Routines for calling DOSBox to run ZZT
 * $Id: dosbox.h,v 1.1 2005/06/29 03:22:57 kvance Exp $
 * Copyright (C) 2002 Kev Vance <kvance@kvance.com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* Only do this if we want DOSBox support */
#ifdef DOSBOX

#ifndef _DOSBOX_H
#define _DOSBOX_H 1

/* DOSBox launch function */
int dosbox_launch(char *datapath, char *worldpath, char *world);

#endif	/* _DOSBOX_H */

#endif /* DOSBOX */
