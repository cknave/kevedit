/* zzm.h  -- zzm file routines
 * $Id: zzm.h,v 1.7 2002/08/23 21:34:12 bitman Exp $
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

#if 0
typedef struct zzmplaystate {
	int octave;     /* Current octave */
	int duration;   /* Current note duration */
	int pos;        /* Position on zzm line */
	int slur;       /* Are we slurring presently? */
} zzmplaystate;

/* ZZM note types */
#define ZZM_ERROR 0   /* Usually end-of-string */
#define ZZM_NOTE  1   /* Just a note */
#define ZZM_REST  2   /* Rest for a moment */
#define ZZM_DRUM  3   /* Tick-tock */

/* ZZM octave bounds */
#define ZZM_MAXOCTAVE 6
#define ZZM_MINOCTAVE 1
#endif

/* zzmpullsong() - pulls song #songnum out of zzmv and returns it */
stringvector zzmpullsong(stringvector * zzmv, int songnum);

/* zzmpicksong() - presents a dialog to choose a song based on title */
int zzmpicksong(stringvector * zzmv, displaymethod * d);

#if 0
/* resetzzmplaystate() - clears state to default settings */
void resetzzmplaystate(zzmplaystate * s);

/* zzmplaynote() - plays a single note from a tune */
zzmnote zzmgetnote(char * tune, zzmplaystate * s);

/* zzmgetfrequency() - get the frequency of a zzmnote */
int zzmgetfrequency(zzmnote note);

/* zzmOpenaudio() - open the audio device */
int zzmOpenaudio();

/* zzmPlaynote() - play a note to the audio device */
void zzmPlaynote(zzmnote note);

/* zzmCloseaudio() - close the audio device */
void zzmCloseaudio();
#endif

#endif
