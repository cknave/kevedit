/* notes.h	-- Generate musical notes in chromatic scale
 * $Id: notes.h,v 1.2 2002/04/05 01:57:51 kvance Exp $
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

/* Pitch of A in octave 0 (where middle C is) */
/* In real music, this is 440. */
/* In ZZT, it seems to be 432. */
//#define BASE_PITCH	432
#define BASE_PITCH	440

/* On/off value for various states of signedness and bits */
#define U8_0		112
#define U8_1		142
#define S8_0		-1
#define S8_1		1
#define U16_0		0
#define U16_1		65535
#define S16_0		-32768
#define S16_1		32767

float NoteFreq(int note, int octave);	/* Return the frequency of a given note
					   the given octaves away from middle */
void AddToBuffer(SDL_AudioSpec spec, float freq, float seconds);
void AudioCallback(SDL_AudioSpec *spec, Uint8 *stream, int len);

#endif	/* _NOTES_H */
