/* sdl_synth.c	-- SDL music synthesizer
 * $Id: sdl_synth.c,v 1.3 2005/06/29 03:20:34 kvance Exp $
 * Copyright (C) 2001 Kev Vance <kvance@kvance.com>
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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SDL.h"

#include "notes.h"
#include "sdl_synth.h"

Uint8 *masterplaybuffer = NULL;
static size_t playbuffersize = 0, playbufferloc = 0, playbuffermax = 0;

int OpenSynth(SDL_AudioDeviceID * id, SDL_AudioSpec * spec)
{
	SDL_AudioSpec desired, obtained;

	if (!SDL_WasInit(SDL_INIT_AUDIO)) {
		/* If the audio subsystem isn't ready, initialize it */
		if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
			fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
			return 1;
		}
	}

	/* Set desired sound opts */
	memset(&desired, 0, sizeof(SDL_AudioSpec));
	desired.freq = 44100;
	desired.format = AUDIO_U16SYS;
	desired.channels = 1;
	desired.samples = 4096;
	desired.callback = AudioCallback;
	desired.userdata = spec;

	/* Open audio device */
	*id = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
	if (*id == 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		return 1;
	}
	SDL_PauseAudioDevice(*id, 0);

	(*spec) = obtained;

	return 0;
}

void CloseSynth(SDL_AudioDeviceID * id)
{
	/* Silence, close the audio, and clean up the memory we used. */
	SDL_PauseAudioDevice(*id, 1);
	SDL_CloseAudioDevice(*id);

	AudioCleanUp();
}

int IsSynthBufferEmpty()
{
	return playbufferloc == playbuffermax;
}

void SynthPlayNote(SDL_AudioSpec audiospec, musicalNote note, musicSettings settings)
{
	/* Find the frequency and duration (wait) in seconds */
	float frequency = noteFilter(noteFrequency(note, settings), settings);
	float wait    = noteDuration(note, settings) / 1000;
	float spacing = noteSpacing(note, settings) / 1000;
	size_t j = 0;

	if (note.type == NOTETYPE_NOTE) {
		/* Add the sound to the buffer */
		AddToBuffer(audiospec, frequency, wait, &j);
	}

	/* Rests are simple */
	if (note.type == NOTETYPE_REST) {
		AddToBuffer(audiospec, 0, wait, &j);
	}

	/* Drums */
	if (note.type == NOTETYPE_DRUM) {
		/* Okay, these drums sound terrible, but they're better than nothing. */
		int i;
		float breaktime = wait - ((float)DRUMBREAK) * DRUMCYCLES / 1000;

		/* Loop through each drum cycle */
		for (i = 0; i < DRUMCYCLES; i++) {
			AddToBuffer(audiospec, noteFilter(drums[note.index][i], settings), ((float)DRUMBREAK) / 1000, &j);
		}

		/* Add a break based on the current duration */
		if( breaktime > 0 ) {
			AddToBuffer(audiospec, 0, breaktime, &j);
		}
	}

	if (spacing != 0.0)
		AddToBuffer(audiospec, 0, spacing, &j);
}

void AddToBuffer(SDL_AudioSpec spec, float freq, float seconds, size_t *j_global)
{
	size_t notesize = seconds * spec.freq; /* Bytes of sound */
	size_t wordsize;
	size_t i, j;

	int osc;

	Uint16 uon = U16_1, uoff = U16_0;
	Sint16 son = S16_1, soff = S16_0;

	/* Don't let the callback function access the playbuffer while we're editing it! */
	SDL_LockAudio();

	if(spec.format == AUDIO_U8 || spec.format == AUDIO_S8)
		wordsize = 1;
	else
		wordsize = 2;

	if(playbuffersize != 0 && playbufferloc != 0) {
		/* Shift buffer back to zero */
		memmove(masterplaybuffer,
			&masterplaybuffer[playbufferloc],
			playbuffersize-playbufferloc);
		playbuffermax -= playbufferloc;
		playbufferloc = 0;
	}
	if(playbuffersize == 0) {
		/* Create buffer */
		masterplaybuffer = malloc(notesize*wordsize);
		playbuffersize   = notesize*wordsize;
	}
	if((notesize*wordsize) > (playbuffersize-playbuffermax)) {
		/* Make bigger buffer */
		masterplaybuffer = realloc(masterplaybuffer,
				playbuffersize+notesize*wordsize);

		playbuffersize += notesize*wordsize;
	}

	if(freq == 0) {
		/* Rest */
		memset(&masterplaybuffer[playbuffermax],
				spec.silence,
				notesize*wordsize);
		playbuffermax += notesize*wordsize;
	} else {
		/* Tone */
		float ffreq = (spec.freq/freq);
		float hfreq = (ffreq/2.0);
		for(i = 0, j = (j_global == NULL ? 0 : *j_global); i < notesize; i++, j++) {
			while (j >= ffreq) {
				j -= ffreq;
			}
			osc = j >= hfreq;
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
			} else if(spec.format == AUDIO_U16) {
				if(osc)
					memcpy(&masterplaybuffer[playbuffermax], &uon, 2);
				else
					memcpy(&masterplaybuffer[playbuffermax], &uoff, 2);
			} else if(spec.format == AUDIO_S16) {
				if(osc)
					memcpy(&masterplaybuffer[playbuffermax], &son, 2);
				else
					memcpy(&masterplaybuffer[playbuffermax], &soff, 2);
			}
			playbuffermax += wordsize;
		}
		if (j_global != NULL)
			*j_global = j;
	}

	/* Now let AudioCallback do its work */
	SDL_UnlockAudio();
}

void AudioCallback(void *userdata, Uint8 *stream, int len)
{
	int i;
	for(i = 0; i < len && playbufferloc < playbuffermax; i++) {
		stream[i] = masterplaybuffer[playbufferloc];
		playbufferloc++;
	}
	for(; i < len; i++)
		stream[i] = ((SDL_AudioSpec *)userdata)->silence;
}

void AudioCleanUp()
{
	if(playbuffersize != 0) {
		free(masterplaybuffer);
		masterplaybuffer = NULL;
		playbuffersize = 0;
		playbufferloc = 0;
		playbuffermax = 0;
	}
}
