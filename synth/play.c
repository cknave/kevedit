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

