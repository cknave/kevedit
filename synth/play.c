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

void play(char* tune)
{
	musicalNote note = zzmGetDefaultNote();
	musicSettings settings = zzmGetDefaultSettings();

	do {
		note = zzmGetNote(tune, note);

		printNoteData(note, settings);

#ifdef DOS
		pcSpeakerPlayNote(note, settings);
#elif defined SDL
		SynthPlayNote(spec, note, settings);
#endif
	} while (note.type != NOTETYPE_NONE);
}

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

