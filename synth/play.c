/* play.c	-- Play ZZT music from the commandline
 * $Id: play.c,v 1.2 2002/04/04 21:13:25 kvance Exp $
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

	SDL_Quit();
}
