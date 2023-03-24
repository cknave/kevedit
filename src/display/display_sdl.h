/* display_sdl.h    -- SDL display
 * $Id: display_sdl.h,v 1.3 2005/06/29 03:20:34 kvance Exp $
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

#include "SDL2/SDL.h"

#include "notebuf.h"
#include "textblock.h"

#include <stdio.h>
#include <stdbool.h>

#ifndef DISPLAY_SDL
#define DISPLAY_SDL 1

#define MAX_AUDIO_CHANNELS (8)

typedef union {
    uint8_t u8;
    int8_t s8;
    int16_t s16;
    float f32;
} audio_sample_t;

/* This structure has pointers to all important video information */
typedef struct
{
	SDL_Window *window;
	SDL_Renderer *renderer;
        SDL_Texture *texture;          /* GPU texture to be updated           */
        Uint32 *pixels;                /* Screen pixels, each element an ARGB */
	textBlock *buffer;             /* Emualted VRAM                       */
	Uint8 *char_set;               /* Pointer to character set            */
	Uint32 *palette;               /* Pointer to expanded palette         */
	Uint32 write_x;                /* Current x write position in VRAM    */
	Uint32 write_y;                /* Current y write position in VRAM    */

	/* Info for reinitializing the screen  */
	Uint32 width, height, depth, vflags;

        /* Audio support */
        SDL_AudioDeviceID audio_device_id;
        SDL_AudioSpec audio_spec;
        audio_sample_t low_and_high_frames[2][MAX_AUDIO_CHANNELS];
        size_t audio_frame_size;
        notebuf_t audio_notebuf;
        double audio_curr_note_time;

        bool is_fullscreen;
        bool is_dirty;

	enum displaycontext context;
} video_info;

/* For RegisterDisplays() */
extern displaymethod display_sdl;

#endif
