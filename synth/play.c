/* play.c	-- Play ZZT music from the commandline
 * $Id: play.c,v 1.4 2002/04/05 04:44:38 kvance Exp $
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

#include <stdlib.h>
#include <ctype.h>
#include "SDL.h"

#include "notes.h"

int main(int argc, char **argv)
{
	if(argc != 2) {
		printf("USAGE: play <zzt music string>\n");
		return 1;
	}
	PlayZZTMusic(argv[1]);
	return 0;
}

int PlayZZTMusic(char *string)
{
	SDL_AudioSpec desired, obtained;
	void *masterbufer;
	int pos = 0, len = strlen(string);
	float time = LEN_T;
	int octave = 0;
	int delaytime = 0;

	if(SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	/* Set desired sound opts */
	desired.freq = 44100;
	desired.format = AUDIO_U16SYS;
	desired.channels = 1;
	desired.samples = 4096;
	desired.callback = AudioCallback;
	desired.userdata = &obtained;

	/* Open audio device */
	if(SDL_OpenAudio(&desired, &obtained) < 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		exit(1);
	}
	printf("Sound device:\n"\
	       "Freq = %i\n"\
	       "Channels = %i\n"\
	       "Samples = %i\n"\
	       "Format = ",
	       obtained.freq,
	       obtained.channels,
	       obtained.samples);

	switch(obtained.format) {
		case AUDIO_U8:
			printf("Unsigned 8-bit\n");
			break;
		case AUDIO_S8:
			printf("Signed 8-bit\n");
			break;
		case AUDIO_U16:
			printf("Unsigned 16-bit LSB\n");
			break;
		case AUDIO_S16:
			printf("Signed 16-bit LSB\n");
			break;
		case AUDIO_U16MSB:
			printf("Unsigned 16-bit MSB\n");
			break;
		case AUDIO_S16MSB:
			printf("Signed 16-bit MSB\n");
			break;
		default:
			printf("UNKNOWN!!!\n");
			break;
	}

	SDL_PauseAudio(0);
	while(pos < len) {
		char c = string[pos];
		if(isalpha(c))
			c = tolower(c);
		if(c == 'x' || (c >= '0' && c <= '9' && c != '3')) {
			// XXX TODO Implement strings
			AddToBuffer(obtained, 0, time);
			delaytime += time * 1000;
		} else if(c == '+') {
			octave++;
		} else if(c == '-') {
			octave--;
		} else if(c == 't') {
			time = LEN_T;
		} else if(c == 's') {
			time = LEN_S;
		} else if(c == 'i') {
			time = LEN_I;
		} else if(c == 'q') {
			time = LEN_Q;
		} else if(c == 'h') {
			time = LEN_H;
		} else if(c == 'w') {
			time = LEN_W;
		} else if(c == '3') {
			time /= 3;
		} else if(c == '.') {
			time += (time/2);
		} else if(pos+1 < len && string[pos+1] == '#') {
			if(c == 'a') {
				AddToBuffer(obtained, NoteFreq(NOTE_As, octave), time);
				delaytime += time * 1000;
			} else if(c == 'b') {
				AddToBuffer(obtained, NoteFreq(NOTE_C, octave+1), time);
				delaytime += time * 1000;
			} else if(c == 'c') {
				AddToBuffer(obtained, NoteFreq(NOTE_Cs, octave), time);
				delaytime += time * 1000;
			} else if(c == 'd') {
				AddToBuffer(obtained, NoteFreq(NOTE_Ds, octave), time);
				delaytime += time * 1000;
			} else if(c == 'e') {
				AddToBuffer(obtained, NoteFreq(NOTE_F, octave), time);
				delaytime += time * 1000;
			} else if(c == 'f') {
				AddToBuffer(obtained, NoteFreq(NOTE_Fs, octave), time);
				delaytime += time * 1000;
			} else if(c == 'g') {
				AddToBuffer(obtained, NoteFreq(NOTE_Gs, octave), time);
				delaytime += time * 1000;
			}
		} else {
			if(c == 'a') {
				AddToBuffer(obtained, NoteFreq(NOTE_A, octave), time);
				delaytime += time * 1000;
			} else if(c == 'b') {
				AddToBuffer(obtained, NoteFreq(NOTE_B, octave), time);
				delaytime += time * 1000;
			} else if(c == 'c') {
				AddToBuffer(obtained, NoteFreq(NOTE_C, octave), time);
				delaytime += time * 1000;
			} else if(c == 'd') {
				AddToBuffer(obtained, NoteFreq(NOTE_D, octave), time);
				delaytime += time * 1000;
			} else if(c == 'e') {
				AddToBuffer(obtained, NoteFreq(NOTE_E, octave), time);
				delaytime += time * 1000;
			} else if(c == 'f') {
				AddToBuffer(obtained, NoteFreq(NOTE_F, octave), time);
				delaytime += time * 1000;
			} else if(c == 'g') {
				AddToBuffer(obtained, NoteFreq(NOTE_G, octave), time);
				delaytime += time * 1000;
			}
		}
		pos++;
	}

	SDL_Delay(delaytime);

	SDL_Quit();
}
