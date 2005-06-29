/* sdl_synth.h	-- SDL music synthesizer
 * $Id: sdl_synth.h,v 1.2 2005/06/29 03:20:34 kvance Exp $
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

#ifndef _SYNTH_H
#define _SYNTH_H 1

#include "SDL.h"

#include "notes.h"

/* On/off value for various states of signedness and bits */
#define U8_0		112
#define U8_1		142
#define S8_0		-15
#define S8_1		15
#define U16_0		28927
#define U16_1		36607
#define S16_0		-3840
#define S16_1		3840

/* TODO: rename these functions */

/* Open the synthesizer and store audio spec in "spec" (true on error) */
int OpenSynth(SDL_AudioSpec * spec);

/* Close the synthesizer */
void CloseSynth(void);

/* Returns true if the buffer is empty */
int IsSynthBufferEmpty();

/* Play a note on to the SDL synthesizer */
void SynthPlayNote(SDL_AudioSpec audiospec, musicalNote note, musicSettings settings);

/* Add a frequency and duration to the SDL audio
 * buffer */
void AddToBuffer(SDL_AudioSpec spec, float freq, float seconds);

/* Internal audio callback function (don't call manually!) */
void AudioCallback(SDL_AudioSpec *spec, Uint8 *stream, int len);

/* Cleanup memory used by audio system */
void AudioCleanUp();

#endif
