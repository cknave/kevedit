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

#define MAX_CHANNELS 8
typedef union {
	uint8_t u8;
	int8_t s8;
	uint16_t u16;
	int16_t s16;
	float f32;
} Sample;
static Sample low_and_high_frames[2][MAX_CHANNELS];
static size_t frame_size;
static void init_low_and_high_frames(SDL_AudioSpec *spec);


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

	/* Open audio device, optimistically accepting all format changes */
	*id = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (*id == 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		return 1;
	}
	/* Make sure we can actually support whatever we got */
	bool spec_ok = true;
	switch(obtained.format) {
		case AUDIO_U8:
		case AUDIO_S8:
		case AUDIO_U16SYS:
		case AUDIO_S16SYS:
		case AUDIO_F32SYS:
			break;
		default:
			spec_ok = false;
	}
	if(obtained.channels > MAX_CHANNELS) {
		spec_ok = false;
	}
	if(!spec_ok) {
		/* Nope, close the device and let SDL do the conversion for us. */
		SDL_CloseAudioDevice(*id);
		*id = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
		if (*id == 0) {
			fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
			return 1;
		}
	}
	init_low_and_high_frames(&obtained);

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
	size_t notesize = seconds * spec.freq; /* Samples of sound */
	size_t i, j;

	int osc;

	/* Don't let the callback function access the playbuffer while we're editing it! */
	SDL_LockAudio();

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
		masterplaybuffer = malloc(notesize*frame_size);
		playbuffersize   = notesize*frame_size;
	}
	if((notesize*frame_size) > (playbuffersize-playbuffermax)) {
		/* Make bigger buffer */
		masterplaybuffer = realloc(masterplaybuffer,
				playbuffersize+notesize*frame_size);

		playbuffersize += notesize*frame_size;
	}

	if(freq == 0) {
		/* Rest */
		memset(&masterplaybuffer[playbuffermax],
				spec.silence,
				notesize*frame_size);
		playbuffermax += notesize*frame_size;
	} else {
		/* Tone */
		float ffreq = (spec.freq/freq);
		float hfreq = (ffreq/2.0);
		for(i = 0, j = (j_global == NULL ? 0 : *j_global); i < notesize; i++, j++) {
			while (j >= ffreq) {
				j -= ffreq;
			}
			osc = j >= hfreq;

			if(osc) {
				memcpy(&masterplaybuffer[playbuffermax], low_and_high_frames[1], frame_size);
			} else {
				memcpy(&masterplaybuffer[playbuffermax], low_and_high_frames[0], frame_size);
			}
			playbuffermax += frame_size;
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

void init_low_and_high_frames(SDL_AudioSpec *spec) {
	size_t sample_size = SDL_AUDIO_BITSIZE(spec->format) / 8;
	frame_size = sample_size * spec->channels;
	memset(low_and_high_frames, spec->silence, sizeof(low_and_high_frames));

	// Convert low and high amplitude to requested sample format
	const float amplitude = 0.117191f;
	Sample low, high;
	switch(spec->format) {
		case AUDIO_U8:
			low.u8 = (uint8_t)(255/2. - amplitude*255/2);
			high.u8 = (uint8_t)(255/2. + amplitude*255/2);
			break;
		case AUDIO_S8:
			low.s8 = (int8_t)(-amplitude*255);
			high.s8 = (int8_t)(amplitude*255);
			break;
		case AUDIO_U16SYS:
			low.u16 = (uint16_t)(65535/2. - amplitude*65535/2);
			high.u16 = (uint16_t)(65535/2. + amplitude*65535/2);
			break;
		case AUDIO_S16SYS:
			low.s16 = (int16_t)(-amplitude*65535);
			high.s16 = (int16_t)(amplitude*65535);
			break;
		case AUDIO_F32SYS:
			low.f32 = -amplitude;
			high.f32 = amplitude;
			break;
	}

	// Channel indexes defined by SDL.
	int channel1_idx, channel2_idx;
	if(spec->channels == 1) {
		channel1_idx = 0;
		channel2_idx = -1;
	} else if(spec->channels < 5) {
		// For stereo, 2.1, and quadraphonic, use the front left/right channels
		channel1_idx = 0;
		channel2_idx = 1;
	} else {
		// For systems with a center channel, use that only
		channel1_idx = 2;
		channel2_idx = -1;
	}
	memcpy((uint8_t *)low_and_high_frames[0] + channel1_idx * sample_size, &low.u8, sample_size);
	memcpy((uint8_t *)low_and_high_frames[1] + channel1_idx * sample_size, &high.u8, sample_size);
	if(channel2_idx != -1) {
		memcpy((uint8_t *)low_and_high_frames[0] + channel2_idx * sample_size, &low.u8, sample_size);
		memcpy((uint8_t *)low_and_high_frames[1] + channel2_idx * sample_size, &high.u8, sample_size);
	}
}