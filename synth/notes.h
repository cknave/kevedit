/* notes.h	-- Generate musical notes in chromatic scale
 * $Id: notes.h,v 1.5 2002/06/07 02:03:12 bitman Exp $
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

#ifndef _NOTES_H
#define _NOTES_H 1

#include "SDL.h"

/* Notes in relation to A */
#define NOTE_C	-3
#define NOTE_Cs	-4
#define NOTE_D	-5
#define NOTE_Ds	-6
#define NOTE_E	-7
#define NOTE_F	-8
#define NOTE_Fs	-9
#define NOTE_G	-10
#define NOTE_Gs	-11
#define NOTE_A	 0
#define NOTE_As	 1
#define NOTE_B	 2

/* Note length in seconds */
#define LEN_T	0.05
#define LEN_S	0.11
#define LEN_I   0.22
#define LEN_Q	0.44
#define LEN_H	0.88
#define LEN_W	1.76

/* Pitch of A in octave 0 (where middle C is) */
/* In real music, this is 440. */
/* In ZZT, it seems to be 432. */
//#define BASE_PITCH	432
#define BASE_PITCH	440

/* On/off value for various states of signedness and bits */
#define U8_0		112
#define U8_1		142
#define S8_0		-15
#define S8_1		15
#define U16_0		28927
#define U16_1		36607
#define S16_0		-3840
#define S16_1		3840

/* Open the synthesizer and store audio spec in "spec" (true on error) */
int OpenSynth(SDL_AudioSpec * spec);

/* Close the synthesizer */
void CloseSynth(void);

/* Return the frequency of a given note the
 * given octaves away from middle */
float NoteFreq(int note, int octave);	

/* Add a frequency and duration to the SDL audio
 * buffer */
void AddToBuffer(SDL_AudioSpec spec, float freq, float seconds);

/* Internal audio callback function (don't call manually!) */
void AudioCallback(SDL_AudioSpec *spec, Uint8 *stream, int len);

/* Cleanup memory used by audio system */
void AudioCleanUp();

#endif	/* _NOTES_H */
