/* sdl_synth.h	-- SDL music synthesizer
 * $Id: synth.h,v 1.1 2002/08/23 21:34:15 bitman Exp $
 * Copyright (C) 2002 bitman <bitman@users.sf.net>
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

#ifndef SYNTH_H
#define SYNTH_H 1

#ifdef SDL
#include "sdl_synth.h"
#elif defined DOS
#include "pcspeaker.h"
#endif

#endif
