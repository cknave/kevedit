/* zzm.h  -- zzm music format
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

#ifndef __SYNTH_ZZM_H
#define __SYNTH_ZZM_H

#include "notes.h"

#define ZZM_MAXOCTAVE  3
#define ZZM_MINOCTAVE -2

/* In real music, this is 440. */
/* In ZZT, it seems to be 432. */
#define ZZM_BASE_PITCH  432

/* Generate default ZZM note settings */
musicalNote zzmGetDefaultNote(void);

/* Generate default ZZM playback settings */
musicSettings zzmGetDefaultSettings(void);

/* Read a note from a tune, using the previous note for defaults */
musicalNote zzmGetNote(char* tune, musicalNote previousNote);

/* Read a chain of notes from a tune (use deleteNoteChain() when done) */
musicalNote* zzmGetNoteChain(char* tune, musicalNote defNote);

#endif
