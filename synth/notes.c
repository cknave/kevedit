/* notes.c	-- Generate musical notes in chromatic scale
 * $Id: notes.c,v 1.3 2002/04/05 01:57:51 kvance Exp $
 * Copyright (C) 2001 Kev Vance <kev@kvance.com>
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

#include <math.h>
#include <stdlib.h>
#include "SDL.h"

#include "notes.h"

static Uint8 *masterplaybuffer = NULL;
static size_t playbuffersize = 0, playbufferloc = 0, playbuffermax = 0;

/* Return the frequency of the given note, "octave" octaves from middle */
float NoteFreq(int note, int octave)
{
	float retval;
	float fraction;

	/* Multiply or divide the base frequency to get to the correct octave
	   relative to A */
	if(octave > 0)
		retval = (float)BASE_PITCH*pow(2, octave);
	else
		retval = (float)BASE_PITCH/pow(2, octave*(-1));
	if(note < NOTE_A)
		retval /= 2;

	/* Find the size of a half step */
	fraction = (log(retval*2) - log(retval))/12;

	/* Move base freq to log */
	retval = log(retval);

	/* Add half-steps to reach the desired note)
	   desired note */
	if(note < NOTE_A)
		retval += (note*(-1))*fraction;
	else if(note > NOTE_A)
		retval += note*fraction;

	/* Get out of log */
	retval = exp(retval);

	/* Return the frequency */
	return retval;
}

void AddToBuffer(SDL_AudioSpec spec, float freq, float seconds)
{
	size_t notesize = seconds * spec.freq; /* Bytes of sound */
	size_t wordsize;
	size_t i;

	int hfreq = (spec.freq/freq)/2;
	int osc = 1;

	if(spec.format == AUDIO_U8 || spec.format == AUDIO_S8)
		wordsize = 1;
	else
		wordsize = 2;

	if(playbuffersize != 0 && playbufferloc != 0) {
		/* Shift buffer back to zero */
		memcpy(masterplaybuffer,
			&masterplaybuffer[wordsize*playbufferloc],
			wordsize*(playbuffersize-playbufferloc));
		playbuffermax -= playbufferloc;
		playbufferloc = 0;
	}
	if(notesize > (playbuffersize-playbuffermax)) {
		/* Make bigger buffer */
		masterplaybuffer = realloc(masterplaybuffer,
				(playbuffersize+notesize)*wordsize);
		playbuffersize += notesize;
	}

	for(i = 0; i < notesize; i += wordsize) {
		if( (i/wordsize) % hfreq == 0)
			osc ^= 1;
		if(spec.format == AUDIO_U8) {
			if(osc)
				masterplaybuffer[playbuffermax] = U8_1;
			else
				masterplaybuffer[playbuffermax] = U8_0;
		} else if(spec.format == AUDIO_S8) {
			if(osc)
				masterplaybuffer[playbuffermax] = S8_1;
			else
				masterplaybuffer[playbuffermax] = S8_0;
		}
		playbuffermax += wordsize;
	}
}

void AudioCallback(SDL_AudioSpec *spec, Uint8 *stream, int len)
{
	int i;
	for(i = 0; i < len && playbufferloc < playbuffermax; i++) {
		stream[i] = masterplaybuffer[playbufferloc];
		playbufferloc++;
	}
	for(; i < len; i++)
		stream[i] = spec->silence;
}
