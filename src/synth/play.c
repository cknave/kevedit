/* play.c	-- Play ZZM music
 * $Id: play.c,v 1.3 2005/06/29 03:20:34 kvance Exp $
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

#include "notes.h"
#include "zzm.h"
#include "synth.h"

#include <stdio.h>
#include <stdlib.h>

void printNoteData(musicalNote note, musicSettings settings)
{
#if 0
	printf("index: %d, len: %d, dots: %d, freq: %f, len: %f\n",
				 note.index, note.length, note.dots,
				 noteFrequency(note, settings),
				 noteDuration(note, settings));
#endif
}

#ifdef SDL
SDL_AudioSpec spec;
#endif

void start() {
#ifdef SDL
	SDL_Init(SDL_INIT_AUDIO);

	atexit(SDL_Quit);

	OpenSynth(&spec);
#endif
}

void end() {
#ifdef SDL
	while (!IsSynthBufferEmpty())
		;
	CloseSynth();
	SDL_Quit();
#elif defined DOS
	pcSpeakerFinish();
#endif
}

void playSong(musicalNote* song, musicSettings settings)
{
	if (song == NULL)
		return;

	printf("Playing note\n");
#ifdef DOS
	pcSpeakerPlayNote(*song, settings);
#elif defined SDL
	SynthPlayNote(spec, *song, settings);
#endif

	playSong(song->next, settings);
}

void play(char* tune)
{
	musicalNote* song = zzmGetNoteChain(tune, zzmGetDefaultNote());

	playSong(song, zzmGetDefaultSettings());

	deleteNoteChain(song);
}

int main(int argc, char* argv[])
{
	int i;

	start();

	/* Default tune */
	if (argc <= 1)
		play("icdefgab+c");

	/* Play all the arguments as seperate lines */
	for (i = 1; i < argc; i++)
		play(argv[i]);

	end();

	return 0;
}

