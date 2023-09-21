/* display_sdl.c	-- SDL Textmode Emulation display method for KevEdit
 * $Id: display_sdl.c,v 1.6 2006/11/01 18:52:02 kvance Exp $
 * Copyright (C) 2002 Gilead Kutnick <exophase@earthlink.net>
 * Copyright (C) 2002 Kev Vance <kvance@kvance.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place Suite 330; Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "SDL2/SDL.h"

#include "display.h"
#include "display_sdl.h"
#include "unicode.h"

#ifdef MACOS
#include "../kevedit/macos.h"
#endif

static SDL_TimerID timer_id = -1;

enum cursor_state {
	CURSOR_HIDDEN,
	CURSOR_VISIBLE,
	CURSOR_INACTIVE
};
static enum cursor_state cursor = CURSOR_HIDDEN;

/* Forward defines :( */
static Uint32 sdl_tick(Uint32 interval, void *blank);
static void sdl_curse(int x, int y);
static void sdl_curse_inactive(int x, int y);
static void sdl_update(video_info *vdest, int x, int y, int width, int height);
static void sdl_update_and_present(video_info *vdest, int x, int y, int width, int height);


#define CURSOR_RATE 200
#define ZOOM_STEP 1.0f

/* No MIN/MAX on mingw32 */
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

/*************************************
 *** BEGIN TEXTMODE EMULATION CODE ***
 *************************************/


static void sdl_expand_palette(Uint32 *dest, const Uint8 *src)
{
	Uint8 *dest_palette = (Uint8 *)dest;
	Uint32 i2, i3, i4;

	i3 = 0;
	i4 = 0;
	/* Convert the palette from 6 bits per component to 8 bits per component */
	for(i2 = 0; i2 < 16; i2++)
	{
		dest_palette[i3]   = (src[i4+2] * 255) / 63;
		dest_palette[i3+1] = (src[i4+1] * 255) / 63;
		dest_palette[i3+2] = (src[i4]   * 255) / 63;
		i3 += 4;
		i4 += 3;
	}
}

static void sdl_set_charset(video_info *vdest, const uint8_t *data)
{
        memcpy(vdest->char_set, data, CHARSET_SIZE);
}

static void sdl_init(video_info *vdest, Uint32 width, Uint32 height, Uint32 depth)
{
	Uint32 window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
	SDL_CreateWindowAndRenderer(width, height, window_flags,
		&vdest->window, &vdest->renderer);
	SDL_RenderSetLogicalSize(vdest->renderer, width, height);

	vdest->texture = SDL_CreateTexture(vdest->renderer,
		SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width,
		height);
	vdest->pixels = calloc(width * height, sizeof(Uint32));
	vdest->buffer = createTextBlock(80, 25);
	vdest->char_set = (Uint8 *)malloc(CHARSET_SIZE);
	vdest->palette = (Uint32 *)malloc(4 * 16);
	vdest->write_x = 0;
	vdest->write_y = 0;

	vdest->width  = width;
	vdest->height = height;
	vdest->depth  = depth;
	vdest->is_fullscreen = false;
	vdest->is_dirty = false;
        vdest->context = undefined;

    /* Initialize Unicode conversion and table */
    init_unicode_conversion();

#ifdef MACOS
	installTouchBar(vdest->window);
#endif
}

static void sdl_end(video_info *vdest)
{
	SDL_DestroyTexture(vdest->texture);

	deleteTextBlock(vdest->buffer);
	free(vdest->char_set);
	free(vdest->palette);
	free(vdest->pixels);

	if(display.dropped_file) {
		free(display.dropped_file);
		display.dropped_file = NULL;
	}

	/* SDL should restore everything okay.. just use SDL_quit() when ready */
}

static void sdl_putch(video_info *vdest, Uint32 x, Uint32 y, Uint8 ch, Uint8 co)
{
	textBlockPutch(vdest->buffer, x, y, ch, co);
}

static void sdl_update_cursor(video_info *vdest) {
	switch(cursor) {
		case CURSOR_HIDDEN:
			sdl_update_and_present(vdest, vdest->write_x, vdest->write_y, 1, 1);
			break;
		case CURSOR_VISIBLE:
			sdl_curse(vdest->write_x, vdest->write_y);
			break;
		case CURSOR_INACTIVE:
			sdl_update(vdest, vdest->write_x, vdest->write_y, 1, 1);
			sdl_curse_inactive(vdest->write_x, vdest->write_y);
			break;
	}
}

static void stop_cursor_timer() {
	if(timer_id != -1) {
		SDL_RemoveTimer(timer_id);
	}
	timer_id = -1;
}

static void start_cursor_timer() {
	if(timer_id != -1) {
		return;  /* Already started. */
	}
	timer_id = SDL_AddTimer(CURSOR_RATE, sdl_tick, NULL);
}

static void sdl_gotoxy(video_info *vdest, Uint32 x, Uint32 y)
{
	/* Redraw old position. */
	sdl_update_and_present(vdest, vdest->write_x, vdest->write_y, 1, 1);

	vdest->write_x = x;
	vdest->write_y = y;

	/* Here's a nice usability fix.  When we reposition the cursor, make
	 * it visible and reset the timer. */
	stop_cursor_timer();
	cursor = CURSOR_VISIBLE;
	sdl_update_cursor(vdest);
	start_cursor_timer();
}

static void sdl_present(video_info *vdest, const SDL_Rect *rect)
{
	Uint32 *pixels = vdest->pixels;
	if(rect) {
		pixels += (rect->y * vdest->width) + rect->x;
	}
	SDL_UpdateTexture(vdest->texture, rect, pixels,
			vdest->width * sizeof(Uint32));
	SDL_RenderClear(vdest->renderer);
	SDL_RenderCopy(vdest->renderer, vdest->texture, NULL, NULL);
	SDL_RenderPresent(vdest->renderer);

	if(rect == NULL) {
		vdest->is_dirty = false;
	}
}

void display_redraw(video_info *vdest)
{
	/* Updates the screen; call at the end of a cycle */

	SDL_Rect blit_rect;

	Uint32 *video_pointer = vdest->pixels;
	Uint32 *last_pointer, *end_pointer;
	Uint8 *char_pointer;
	Uint8 *color_pointer;
	Uint8 *charset_pointer = vdest->char_set;
	Uint32 *palette_pointer = vdest->palette;
	Uint8 *current_char_pointer;
	Uint8 char_row;
	Uint8 char_mask;
	Uint32 fg, bg;
	Uint32 i, i2, i3, i4;

	char_pointer = vdest->buffer->data;
	color_pointer = vdest->buffer->data + 1;

	i = 25;
	while(i)
	{
		i2 = 80;
		while(i2)
		{
			current_char_pointer = charset_pointer + (*(char_pointer) * 14);
			last_pointer = video_pointer;
			bg = *(palette_pointer + (*(color_pointer) >> 4));
			fg = *(palette_pointer + (*(color_pointer) & 15));

			i3 = 14;
			while(i3)
			{
				/* Draw an entire char row at a time.. */
				char_row = *(current_char_pointer);
				i4 = 8;
				while(i4)
				{
					char_mask = (1 << (i4 - 1));
					if((char_mask & char_row))
					{
						 /* Draw fg color */
						*(video_pointer) = fg;
					}
					else
					{
						/* Draw bg color */
						*(video_pointer) = bg;
					}
					i4--;
					video_pointer++;
				}
				end_pointer = video_pointer;
				video_pointer += 632;
				current_char_pointer++;
				i3--;
			}
			char_pointer += 2;
			color_pointer += 2;
			/* Jump to the next char */
			i2--;
			video_pointer = last_pointer + 8;
		}
		video_pointer = end_pointer;
		i--;
	}

	/* Update the buffer surface to the real thing.. */
	sdl_present(vdest, NULL);
}

static void sdl_update(video_info *vdest, int x, int y, int width, int height)
{
	/* Updates a block */

	SDL_Rect src_rect, dest_rect;
	Uint32 rowsleft, colsleft;

	Uint8 *char_pointer  = vdest->buffer->data + ((y*80+x)<<1);
	Uint8 *color_pointer = vdest->buffer->data + ((y*80+x)<<1) + 1;

	/* Draw each row onto video memory */
	for (rowsleft = height; rowsleft; rowsleft--) {

		/* Determine the video pointer for this row */
		Uint32 *root = vdest->pixels;
		Uint32 *video_pointer = root + ((height-rowsleft+y)*14)*(640)+(x<<3);

		/* Draw each column in row onto video memory */
		for (colsleft = width; colsleft; colsleft--) {
			/* Find the current character in the charset */
			Uint8* current_char_pointer = vdest->char_set + (*(char_pointer) * 14);

			/* Find the fg and bg colors using the palette */
			Uint32 bg = *(vdest->palette + (*(color_pointer) >> 4));
			Uint32 fg = *(vdest->palette + (*(color_pointer) & 15));

			/* Remember the video pointer for the first line of this character */
			Uint32 *last_pointer = video_pointer;

			/* Consider making this portion a seperate function */

			int linesleft, pixelsleft;

			/* Draw the current character to video memory */
			for (linesleft = 14; linesleft; linesleft--) {
				/* Draw an entire char row at a time.. */
				Uint8 char_mask = 0x7f;
				Uint8 char_row = *(current_char_pointer);

				for (pixelsleft = 8; pixelsleft; pixelsleft--) {
					char_mask = (1 << (pixelsleft - 1));
					if((char_mask & char_row))
					{
						 /* Draw fg color */
						*(video_pointer) = fg;
					}
					else
					{
						/* Draw bg color */
						*(video_pointer) = bg;
					}
					video_pointer++;
				}
				video_pointer += 632;
				current_char_pointer++;
			}

			/* Advance to next char/color pair */
			char_pointer += 2;
			color_pointer += 2;

			/* Jump to the location of next char in video memory */
			video_pointer = last_pointer + 8;

			/* Do not dereference video_pointer, char_pointer, or color_pointer here! */
		}

		/* Move char/color pointers to the next line of the block */
		char_pointer += (80-width)<<1;
		color_pointer += (80-width)<<1;

		/* Do not dereference video_pointer, char_pointer, or color_pointer here! */
	}
	vdest->is_dirty = true;
}

static void sdl_update_and_present(video_info *vdest, int x, int y, int width,
		int height)
{
	bool was_dirty = vdest->is_dirty;
	sdl_update(vdest, x, y, width, height);
	vdest->is_dirty = was_dirty;

	SDL_Rect rect;
	rect.x = x * 8;
	rect.y = y * 14;
	rect.w = width * 8;
	rect.h = height * 14;
	sdl_present(vdest, &rect);
}

static float get_window_zoom_level(SDL_Window *window) {
	int width = 0;
	int height = 0;
	SDL_GetWindowSize(window, &width, &height);

	float zoom_x = width / 640.0f;
	float zoom_y = height / 350.0f;
	return MIN(zoom_x, zoom_y);
}

static float get_window_max_zoom_level(SDL_Window *window) {
	int index = SDL_GetWindowDisplayIndex(window);
	SDL_Rect bounds;
	SDL_GetDisplayBounds(index, &bounds);

	float max_zoom_x = bounds.w / 640.0f;
	float max_zoom_y = bounds.h / 350.0f;
	return MIN(max_zoom_x, max_zoom_y);
}

static void get_pixel_size(video_info *vdest, float *size_x, float *size_y) {
	int window_w, window_h;
	SDL_GetWindowSize(vdest->window, &window_w, &window_h);

	int pixel_w, pixel_h;
	SDL_GetRendererOutputSize(vdest->renderer, &pixel_w, &pixel_h);

	*size_x = (float)pixel_w / window_w;
	*size_y = (float)pixel_h / window_h;
}

static inline float round_to_nearest_half(float x) {
	return roundf(2.0f * x) / 2.0f;
}

static inline float floor_to_nearest_half(float x) {
	return floorf(2.0f * x) / 2.0f;
}

static void shrink_window(video_info *vdest) {
	float pixel_w, pixel_h;
	get_pixel_size(vdest, &pixel_w, &pixel_h);
	float pixel_size = MAX(pixel_w, pixel_h);

	float zoom = get_window_zoom_level(vdest->window) * pixel_size;
	zoom = round_to_nearest_half(zoom - ZOOM_STEP);
	zoom = MAX(pixel_size, zoom);

	SDL_SetWindowSize(vdest->window, 640 * zoom / pixel_w, 350 * zoom / pixel_h);
}

static void grow_window(video_info *vdest) {
	float pixel_w, pixel_h;
	get_pixel_size(vdest, &pixel_w, &pixel_h);
	float pixel_size = MAX(pixel_w, pixel_h);

	float zoom = get_window_zoom_level(vdest->window) * pixel_size;
	float max_zoom = get_window_max_zoom_level(vdest->window) * pixel_size;
	zoom = MIN(max_zoom, zoom + ZOOM_STEP);
	zoom = floor_to_nearest_half(zoom);

	SDL_SetWindowSize(vdest->window, 640 * zoom / pixel_w, 350 * zoom / pixel_h);
}

/********************************
 *** BEGIN KEVEDIT GLUE LAYER ***
 ********************************/
static video_info info;	/* Display info */
static int shift;	/* Shift state */

/* Nice timer update callback thing */
static Uint32 sdl_tick(Uint32 interval, void *blank)
{
	SDL_Event e;
	e.type = SDL_USEREVENT;
	SDL_PushEvent(&e);
	return interval;
}

static void sdl_curse(int x, int y)
{
	SDL_Rect rect;
	Uint8 color;
	Uint32 *video_pointer = info.pixels;
	Uint32 fg;
	int i1, i2;

	/* Find out the color */
	color = textBlockColour(info.buffer, x, y);
	fg = info.palette[color & 15];

	/* Draw the cursor */
	video_pointer += (x<<3)+(y*14)*640;
	for(i1 = 0; i1 < 14; i1++) {
		for(i2 = 0; i2 < 8; i2++) {
			*(video_pointer) = fg;
			video_pointer++;
		}
		video_pointer += 632;
	}

	/* Command SDL to update this char */
	rect.x = x * 8;
	rect.y = y * 14;
	rect.w = 8;
	rect.h = 14;
	sdl_present(&info, &rect);
}

static void sdl_curse_inactive(int x, int y)
{
	SDL_Rect rect;
	Uint8 color;
	Uint32 *video_pointer = info.pixels;
	Uint32 fg;
	int i;

	/* Find out the color */
	color = textBlockColour(info.buffer, x, y);
	fg = info.palette[color & 15];

	/* Draw the cursor */
	video_pointer += (x<<3)+(y*14)*640;
	/* Top part of box */
	for(i = 0; i < 8; i++) {
		*(video_pointer) = fg;
		video_pointer++;
	}
	video_pointer += 632;
	/* Sides of box */
	for(i = 0; i < 12; i++) {
		*(video_pointer) = fg;
		video_pointer += 7;
		*(video_pointer) = fg;
		video_pointer += 633;
	}
	/* Bottom part of box */
	for(i = 0; i < 8; i++) {
		*(video_pointer) = fg;
		video_pointer++;
	}

	/* Command SDL to update this char */
	rect.x = x * 8;
	rect.y = y * 14;
	rect.w = 8;
	rect.h = 14;
	sdl_present(&info, &rect);
}

// Display method wrapper for sdl_init
static int display_sdl_init()
{
	/* Start up SDL */
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		return 0;
	}
	SDL_StartTextInput();

	/* Fire up the textmode emulator */
	sdl_init(&info, 640, 350, 32);
        sdl_set_charset(&info, default_charset.data);
        sdl_expand_palette(info.palette, default_palette.data);

	shift = 0;
	cursor = CURSOR_VISIBLE;

	start_cursor_timer();

	return 1;
}

// Display method wrapper for sdl_end
static void display_sdl_end()
{
	/* Terminate SDL stuff */
	sdl_end(&info);
	SDL_StopTextInput();
	SDL_Quit();
}

// Display method wrapper for sdl_putch
static void display_sdl_putch(int x, int y, int ch, int co)
{
	/* Call textmode emulator putch */
	sdl_putch(&info, x, y, ch, co);
	sdl_update(&info, x, y, 1, 1);
}

static void display_sdl_fullscreen()
{
	/* Toggle fullscreen */
	info.is_fullscreen = !info.is_fullscreen;

	Uint32 flags = 0;
	if(info.is_fullscreen)
		flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
	SDL_SetWindowFullscreen(info.window, flags);

	/* If in fullscreen mode, don't show the mouse */
	if(info.is_fullscreen)
		SDL_ShowCursor(SDL_DISABLE);
	else
		SDL_ShowCursor(SDL_ENABLE);
}

/* Check if there is a Unicode TextInput event further down
   the SDL queue. This is the only way we can distinguish AltGr
   from plain right Alt, it appears; by the text the former
   produces. */
static int has_unicode_event_queued()
{
	const int MAX_EVENTS = 10;
	SDL_Event outevent[MAX_EVENTS];
	int pending_events = SDL_PeepEvents(outevent,
		MAX_EVENTS, SDL_PEEKEVENT, SDL_TEXTINPUT,
		SDL_TEXTINPUT);

	int i;
	for (i = 0; i < pending_events; i++) {
		if (!is_ascii_key(outevent[i].text.text[0])) {
			return true;
		}
	}

	return false;
}

/* Determine if there is a SDL_TEXTINPUT event waiting in the
   queue, and if so, delete it. This is used to keep the literal
   part of a hotkey from being passed through when dealing with
   hotkeys. (E.g. we don't want Alt+S to also return 's'.) */
static void clear_textinput_event()
{
	const int MAX_EVENTS = 10;
	SDL_Event outevent[MAX_EVENTS];
	int pending_events = SDL_PeepEvents(outevent,
		MAX_EVENTS, SDL_PEEKEVENT, SDL_TEXTINPUT,
		SDL_TEXTINPUT);

	if (pending_events == 0)
		return;

	SDL_PeepEvents(outevent, 1, SDL_GETEVENT,
		SDL_TEXTINPUT, SDL_TEXTINPUT);

	return;
}

static int display_sdl_getkey()
{
	SDL_Event event;

	/* Draw the cursor if necessary */
	sdl_update_cursor(&info);

	/* Check for a KEYDOWN event */
	if (SDL_PollEvent(&event) == 0)
		return DKEY_NONE;

	if(event.type == SDL_DROPFILE) {
		if(display.dropped_file) {
			free(display.dropped_file);
		}
		display.dropped_file = event.drop.file;
		return DKEY_DROPFILE;
	}

	/* Preemptive stuff */
	if(event.type == SDL_TEXTINPUT) {
		if (is_ascii_key(event.text.text[0])) {
			return event.text.text[0];
		} else {
			/* Try to convert Unicode to CP437 */
			unsigned char CP437_char = get_CP437_from_UTF8(event.text.text);

			if (CP437_char != 0) {
				return CP437_char;
			}
			return DKEY_NONE;
		}
	} else if(event.type == SDL_KEYDOWN) {
		/* Hack for windows: alt+tab will never show up */
		if((event.key.keysym.sym == SDLK_TAB) &&
				(event.key.keysym.mod & KMOD_ALT)) {
			return DKEY_NONE;
		}
		switch(event.key.keysym.sym) {
			case SDLK_RSHIFT:
			case SDLK_LSHIFT:
			case SDLK_RCTRL:
			case SDLK_LCTRL:
			case SDLK_RALT:
			case SDLK_LALT:
				/* Shift, ctrl, and alt don't count */
				event.type = SDL_KEYUP;
				break;
			case SDLK_MINUS:
			case SDLK_KP_MINUS:
				if(event.key.keysym.mod & (KMOD_CTRL | KMOD_GUI)) {
				    if(!info.is_fullscreen) {
					shrink_window(&info);
				    }
				}
				break;
			case SDLK_PLUS:
			case SDLK_EQUALS:
			case SDLK_KP_PLUS:
				if(event.key.keysym.mod & (KMOD_CTRL | KMOD_GUI)) {
				    if(!info.is_fullscreen) {
					grow_window(&info);
				    }
				}
				break;
			case SDLK_RETURN:
				/* Fullscreen toggle */
				if(event.key.keysym.mod & KMOD_ALT) {
					event.type = SDL_KEYUP;
					if(!event.key.repeat) {
					    display_sdl_fullscreen();
					}
				}
				break;
			default:
				break;
		}
	/* UserEvent means it's time to update the cursor */
	} else if(event.type == SDL_USEREVENT) {
		if(cursor == CURSOR_HIDDEN) {
			cursor = CURSOR_VISIBLE;
		} else if(cursor == CURSOR_VISIBLE) {
			cursor = CURSOR_HIDDEN;
		}
		sdl_update_cursor(&info);
	/* Focus change? */
	} else if(event.type == SDL_WINDOWEVENT) {
		if(event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
			display_redraw(&info);
			/* Make cursor normal */
			start_cursor_timer();
			cursor = CURSOR_VISIBLE;
			sdl_update_cursor(&info);
		} else if(event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
			/* Inactive cursor */
			stop_cursor_timer();
			cursor = CURSOR_INACTIVE;
			sdl_update_cursor(&info);
		}
	} else if(event.type == SDL_QUIT) {
		return DKEY_QUIT;
	}

	if (event.type != SDL_KEYDOWN)
		return DKEY_NONE;

	/* Map the weirder keysyms to KevEdit form */
	switch(event.key.keysym.sym) {
		case SDLK_UP:
			event.key.keysym.sym = DKEY_UP;
			break;
		case SDLK_DOWN:
			event.key.keysym.sym = DKEY_DOWN;
			break;
		case SDLK_LEFT:
			event.key.keysym.sym = DKEY_LEFT;
			break;
		case SDLK_RIGHT:
			event.key.keysym.sym = DKEY_RIGHT;
			break;
		case SDLK_INSERT:
			event.key.keysym.sym = DKEY_INSERT;
			break;
		case SDLK_DELETE:
			event.key.keysym.sym = DKEY_DELETE;
			break;
		case SDLK_HOME:
			event.key.keysym.sym = DKEY_HOME;
			break;
		case SDLK_END:
			event.key.keysym.sym = DKEY_END;
			break;
		case SDLK_PAGEUP:
			event.key.keysym.sym = DKEY_PAGEUP;
			break;
		case SDLK_PAGEDOWN:
			event.key.keysym.sym = DKEY_PAGEDOWN;
			break;
		case SDLK_F1:
			event.key.keysym.sym = DKEY_F1;
			break;
		case SDLK_F2:
			event.key.keysym.sym = DKEY_F2;
			break;
		case SDLK_F3:
			event.key.keysym.sym = DKEY_F3;
			break;
		case SDLK_F4:
			event.key.keysym.sym = DKEY_F4;
			break;
		case SDLK_F5:
			event.key.keysym.sym = DKEY_F5;
			break;
		case SDLK_F6:
			event.key.keysym.sym = DKEY_F6;
			break;
		case SDLK_F7:
			event.key.keysym.sym = DKEY_F7;
			break;
		case SDLK_F8:
			event.key.keysym.sym = DKEY_F8;
			break;
		case SDLK_F9:
			event.key.keysym.sym = DKEY_F9;
			break;
		case SDLK_F10:
			event.key.keysym.sym = DKEY_F10;
			break;
		case SDLK_F11:
			event.key.keysym.sym = DKEY_F11;
			break;
		case SDLK_F12:
			event.key.keysym.sym = DKEY_F12;
			break;
		default:
			break;
	}

	/* Ctrl is down */
	if(event.key.keysym.mod & KMOD_CTRL) {
		/* If alpha key, return special ctrl+alpha */
		if(event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z) {
			event.key.keysym.sym -= 0x60;
		} else {
			switch(event.key.keysym.sym) {
				case DKEY_PAGEUP:
					event.key.keysym.sym = DKEY_CTRL_PAGEUP;
					break;
				case DKEY_PAGEDOWN:
					event.key.keysym.sym = DKEY_CTRL_PAGEDOWN;
					break;
				case DKEY_LEFT:
					event.key.keysym.sym = DKEY_CTRL_LEFT;
					break;
				case DKEY_RIGHT:
					event.key.keysym.sym = DKEY_CTRL_RIGHT;
					break;
				case DKEY_UP:
					event.key.keysym.sym = DKEY_CTRL_UP;
					break;
				case DKEY_DOWN:
					event.key.keysym.sym = DKEY_CTRL_DOWN;
					break;
				default:
					break;
			}
		}
	}

	/* Alt is down */
	else if(event.key.keysym.mod & KMOD_ALT) {
		/* It turns out that SDL2 can't directly distinguish AltGr
		   from ordinary right Alt. As AltGr will produce a Unicode
		   text event downstream, though, we can check for that.
		   If there's such a text event, skip the hotkey check
		   below. */
		if (has_unicode_event_queued()) {
			return DKEY_NONE;
		}

		switch(event.key.keysym.sym) {
			case DKEY_LEFT:
				event.key.keysym.sym = DKEY_ALT_LEFT;
				break;
			case DKEY_RIGHT:
				event.key.keysym.sym = DKEY_ALT_RIGHT;
				break;
			case DKEY_UP:
				event.key.keysym.sym = DKEY_ALT_UP;
				break;
			case DKEY_DOWN:
				event.key.keysym.sym = DKEY_ALT_DOWN;
				break;
			case '-':
				event.key.keysym.sym = DKEY_ALT_MINUS;
				break;
			case '=':
				event.key.keysym.sym = DKEY_ALT_PLUS;
				break;
			case 'i':
				event.key.keysym.sym = DKEY_ALT_I;
				break;
			case 'm':
				event.key.keysym.sym = DKEY_ALT_M;
				break;
			case 'o':
				event.key.keysym.sym = DKEY_ALT_O;
				break;
			case 's':
				event.key.keysym.sym = DKEY_ALT_S;
				break;
			case 't':
				event.key.keysym.sym = DKEY_ALT_T;
				break;
			case 'z':
				event.key.keysym.sym = DKEY_ALT_Z;
				break;
			/* Add other letters of the alphabet as necessary */
			default:
				break;
		}
	}

	/* Shift is down */
	shift = event.key.keysym.mod & KMOD_SHIFT; /* set global variable */
	if(shift) {
		switch(event.key.keysym.sym) {
			case SDLK_TAB:
				event.key.keysym.sym = DKEY_SHIFT_TAB;
				break;
		}
	}

	/* If we're dealing with a literal key, then return DKEY_NONE because
		it will be handled as part of a later SDL_TEXTINPUT event. But if it
		is a hotkey (non-literal), then remove the SDL_TEXTINPUT event if
		there is one, which will prevent spurious text output. */

	if (is_literal_key(event.key.keysym.sym)) {
		return DKEY_NONE;
	} else {
		clear_textinput_event();
		return event.key.keysym.sym;
	}

}

static int display_sdl_getch_with_context(enum displaycontext context) {
	info.context = context;
#if MACOS
	switch(context) {
		case board_editor:
			enableTouchBarWithTextMode(false);
			break;
		case board_editor_text:
			enableTouchBarWithTextMode(true);
			break;
		default:
			disableTouchBar();
			break;
	}
#endif

	if(info.is_dirty) {
		sdl_present(&info, NULL);
		if(cursor != CURSOR_HIDDEN) {
			sdl_update_cursor(&info);
		}
	}

	int key;

	do {
		if (SDL_WaitEvent(NULL) == 0)
			return DKEY_NONE;

		key = display_sdl_getkey();
	} while (key == DKEY_NONE);

	return key;
}

static int display_sdl_getch()
{
	return display_sdl_getch_with_context(undefined);
}

static void display_sdl_gotoxy(int x, int y)
{
	sdl_gotoxy(&info, x, y);
}

static void display_sdl_print(int x, int y, int c, char *s)
{
	int i, len = strlen(s);

	for(i = 0; i < len; i++)
		display_sdl_putch(x+i, y, s[i], c);
	sdl_update(&info, x, y, len, 1);
}

static void display_sdl_titlebar(char *title)
{
	SDL_SetWindowTitle(info.window, title);
}

static int display_sdl_shift()
{
	return shift;
}

static void display_sdl_putch_discrete(int x, int y, int ch, int co)
{
	sdl_putch(&info, x, y, ch, co);
}

static void display_sdl_print_discrete(int x, int y, int c, char *s)
{
	int i, len = strlen(s);

	for(i = 0; i < len; i++)
		display_sdl_putch_discrete(x+i, y, s[i], c);
}

static void display_sdl_update(int x, int y, int w, int h)
{
	sdl_update(&info, x, y, w, h);
}

static void display_sdl_set_charset(const charset *cs) {
        sdl_set_charset(&info, cs->data);
        sdl_update_and_present(&info, 0, 0, 80, 25);
}

static void display_sdl_set_palette(const palette *pal) {
        sdl_expand_palette(info.palette, pal->data);
        sdl_update_and_present(&info, 0, 0, 80, 25);
}

displaymethod display_sdl =
{
	NULL,
	"SDL Textmode Emulator",
	"2.0",
	display_sdl_init,
	display_sdl_end,
	display_sdl_putch,
	display_sdl_getch,
	display_sdl_getch_with_context,
	display_sdl_getkey,
	display_sdl_gotoxy,
	display_sdl_print,
	display_sdl_titlebar,
	display_sdl_shift,
	display_sdl_putch_discrete,
	display_sdl_print_discrete,
	display_sdl_update,
        display_sdl_set_charset,
        display_sdl_set_palette,
	NULL,
};
