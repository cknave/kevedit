/* display_sdl.c	-- SDL Textmode Emulation display method for KevEdit
 * $Id: display_sdl.c,v 1.1 2002/03/19 03:11:06 kvance Exp $
 * Copyright (C) 2002 Gilead Kutnick <exophase@earthlink.net>
 * Copyright (C) 2002 Kev Vance <kev@kvance.com>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include "SDL.h"

#include "display.h"
#include "display_sdl.h"

int xstart, ystart;	/* Where the viewport begins */

/*************************************
 *** BEGIN TEXTMODE EMULATION CODE ***
 *************************************/

void display_load_charset(Uint8 *dest, Uint8 *name)
{
  FILE *fp;
  fp = fopen(name, "rb");
  fread(dest, 256 * 14, 1, fp);
  fclose(fp);
}

void display_load_palette(Uint32 *dest, Uint8 *name)
{
  FILE *fp;
  Uint8 palette[3 * 16];
  Uint8 *dest_palette = (Uint8 *)dest;
  Uint32 i2, i3, i4;

  fp = fopen(name, "rb");
  fread(palette, 3 * 16, 1, fp);

  i3 = 0;
  i4 = 0;
  /* Convert the palette from 6 bits per component to 8 bits per component */
  for(i2 = 0; i2 < 16; i2++)
  {
  	  dest_palette[i3]   = (palette[i4+2] * 255) / 63;
	  dest_palette[i3+1] = (palette[i4+1] * 255) / 63;
	  dest_palette[i3+2] = (palette[i4]   * 255) / 63;
	  i3 += 4;
	  i4 += 3;
  }
  fclose(fp);
}

void display_init(video_info *vdest, Uint32 width, Uint32 height, Uint32
 depth, Uint32 full_screen, Uint32 hw_surface)
{
  Uint32 vflags = 0;
  if(full_screen)
  {
    vflags |= SDL_FULLSCREEN;
  }
  if(hw_surface)
  {
    vflags |= SDL_HWSURFACE;
  }

  vdest->video = SDL_SetVideoMode(width, height, depth, vflags);
  vdest->buffer_surface = SDL_CreateRGBSurface(0, 640, 350, 32, 0, 0, 0, 0);
  vdest->buffer = (Uint8 *)malloc(TEXT_MODE_VRAM);
  vdest->char_set = (Uint8 *)malloc(256 * 14);
  vdest->palette = (Uint32 *)malloc(4 * 16);
  vdest->write_x = 0;
  vdest->write_y = 0;
}

void display_end(video_info *vdest)
{
  free(vdest->buffer);
  free(vdest->char_set);
  free(vdest->palette);

  /* SDL should restore everything okay.. just use SDL_quit() when ready */
}

void display_putch(video_info *vdest, Uint32 x, Uint32 y, Uint8 ch, Uint8 co)
{
  Uint8 *vram = vdest->buffer;
  *(vram + (((y * 80) + x) * 2)) = ch;
  *(vram + (((y * 80) + x) * 2) + 1) = co;
}

void display_gotoxy(video_info *vdest, Uint32 x, Uint32 y)
{
  vdest->write_x = x;
  vdest->write_y = y;
}

void display_redraw(video_info *vdest)
{
  /* Updates the screen; call at the end of a cycle */

  SDL_Rect blit_rect;

  Uint32 *video_pointer = vdest->buffer_surface->pixels;
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

  char_pointer = vdest->buffer;
  color_pointer = vdest->buffer + 1;
  current_char_pointer = charset_pointer + (*(char_pointer) * 14);

  i = 25;
  while(i)
  {
    i2 = 80;
    while(i2)
    {
      last_pointer = video_pointer;
      bg = *(palette_pointer + (*(color_pointer) >> 4));
      fg = *(palette_pointer + (*(color_pointer) & 15));

      i3 = 14;
      while(i3)
      {
        /* Draw an entire char row at a time.. */
        char_mask = 0x7f;
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
      i3 = 14;
      char_pointer += 2;
      color_pointer += 2;
      current_char_pointer = charset_pointer + (*(char_pointer) * 14);
      /* Jump to the next char */
      i2--;
      video_pointer = last_pointer + 8;
    }
    video_pointer = end_pointer;
    i--;
  }

  /* Update the buffer surface to the real thing.. */

  blit_rect.x = xstart;
  blit_rect.y = ystart;

  SDL_BlitSurface(vdest->buffer_surface, NULL, vdest->video, &blit_rect);
  SDL_UpdateRect(vdest->video, 0, 0, 0, 0);
}

void display_update(video_info *vdest, int x, int y, int width, int height)
{
  /* Updates a block */

  SDL_Rect src_rect, dest_rect;

  Uint32 *root = vdest->buffer_surface->pixels;

  Uint32 *video_pointer = vdest->buffer_surface->pixels;
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

  char_pointer = vdest->buffer + (y*80+x)*2;
  color_pointer = vdest->buffer + (y*80+x)*2 + 1;
  current_char_pointer = charset_pointer + (*(char_pointer) * 14);

  video_pointer += 640*(y*14);
  video_pointer += (x*8);

  i = height;
  while(i) {
    i2 = width;
    if(height != 1)
	    video_pointer = root + ((height-i+y)*14)*(640)+(x*8);
    while(i2) {
      last_pointer = video_pointer;
      bg = *(palette_pointer + (*(color_pointer) >> 4));
      fg = *(palette_pointer + (*(color_pointer) & 15));

      i3 = 14;
      while(i3)
      {
        /* Draw an entire char row at a time.. */
        char_mask = 0x7f;
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

      i3 = 14;
      char_pointer += 2;
      color_pointer += 2;
      current_char_pointer = charset_pointer + (*(char_pointer) * 14);
      /* Jump to the next char */
      i2--;
      video_pointer = last_pointer + 8;
    }
    /* Move char/color pointers to the next line of the block */
    video_pointer = end_pointer;
    char_pointer += (80-width)*2;
    color_pointer += (80-width)*2;
    current_char_pointer = charset_pointer + (*(char_pointer) * 14);
    i--;
  }

  /* Update the buffer surface to the real thing.. */

  src_rect.x = (x*8);
  src_rect.y = (y*14);
  dest_rect.x = src_rect.x+xstart;
  dest_rect.y = src_rect.y+ystart;
  src_rect.w = dest_rect.w = (width*8);
  src_rect.h = dest_rect.h = (height*14);

  SDL_BlitSurface(vdest->buffer_surface, &src_rect, vdest->video, &dest_rect);
  SDL_UpdateRect(vdest->video, dest_rect.x, dest_rect.y, dest_rect.w, dest_rect.h);
}

/********************************
 *** BEGIN KEVEDIT GLUE LAYER ***
 ********************************/
video_info info;	/* Display info */
static int shift;	/* Shift state */
static int timer, csoc;	/* Timer for cursor, current state of cursor */
static SDL_TimerID timerId;	/* Timer ID */

/* Nice timer update callback thing */
static Uint32 display_tick(Uint32 interval, void *blank)
{
	SDL_Event e;
	e.type = SDL_USEREVENT;
	SDL_PushEvent(&e);
	timer ^= 1;
	return interval;
}

void display_curse(int x, int y)
{
	SDL_Rect src_rect, dest_rect;
	u_int8_t color;
	Uint32 *video_pointer = info.buffer_surface->pixels;
	Uint32 fg;
	int i1, i2;

	/* Find out the color */
	color = info.buffer[(x+y*80)*2+1];
	fg = info.palette[color & 15];

	/* Draw the cursor */
	video_pointer += (x*8)+(y*14)*640;
	for(i1 = 0; i1 < 14; i1++) {
		for(i2 = 0; i2 < 8; i2++) {
			*(video_pointer) = fg;
			video_pointer++;
		}
		video_pointer += 632;
	}

	/* Command SDL to update this char */
	src_rect.x = (x*8);
	src_rect.y = (y*14);
	dest_rect.x = src_rect.x+xstart;
	dest_rect.y = src_rect.y+ystart;
	src_rect.w = dest_rect.w = 8;
	src_rect.h = dest_rect.h = 14;

	SDL_BlitSurface(info.buffer_surface, &src_rect, info.video, &dest_rect);
	SDL_UpdateRect(info.video, dest_rect.x, dest_rect.y, dest_rect.w, dest_rect.h);
}

void display_curse_inactive(int x, int y)
{
	SDL_Rect src_rect, dest_rect;
	u_int8_t color;
	Uint32 *video_pointer = info.buffer_surface->pixels;
	Uint32 fg;
	int i;

	/* Find out the color */
	color = info.buffer[(x+y*80)*2+1];
	fg = info.palette[color & 15];

	/* Draw the cursor */
	video_pointer += (x*8)+(y*14)*640;
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
	src_rect.x = (x*8);
	src_rect.y = (y*14);
	dest_rect.x = src_rect.x+xstart;
	dest_rect.y = src_rect.y+ystart;
	src_rect.w = dest_rect.w = 8;
	src_rect.h = dest_rect.h = 14;

	SDL_BlitSurface(info.buffer_surface, &src_rect, info.video, &dest_rect);
	SDL_UpdateRect(info.video, dest_rect.x, dest_rect.y, dest_rect.w, dest_rect.h);
}

int display_sdl_init()
{
	/* Start up SDL */
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		return 0;
	}

	/* Automatic keyboard repeat */
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
			SDL_DEFAULT_REPEAT_INTERVAL);

	/* Fire up the textmode emulator */
	display_init(&info, 640, 350, 32, 0, 1);
	display_load_charset(info.char_set, "default.chr");
	display_load_palette(info.palette, "ega.pal");

	xstart = ((info.video->w) - 640) / 2;
	ystart = ((info.video->h) - 350) / 2;

	shift = 0;
	timer = csoc = 0;

	timerId = SDL_AddTimer(100, display_tick, NULL);

	return 1;
}

void display_sdl_end()
{
	/* Terminate SDL stuff */
	SDL_Quit();
}

void display_sdl_putch(int x, int y, int ch, int co)
{
	/* Call textmode emulator putch */
	display_putch(&info, x, y, ch, co);
	display_update(&info, x, y, 1, 1);
}

void display_fullscreen()
{
	static int mouse = 1;
	/* Toggle fullscreen */
	SDL_WM_ToggleFullScreen(info.video);

	/* If in fullscreen mode, don't show the mouse */
	mouse ^= 1;
	if(mouse)
		SDL_ShowCursor(SDL_DISABLE);
	else
		SDL_ShowCursor(SDL_ENABLE);
}

int display_sdl_getch()
{
	SDL_Event event;

	/* Wait for a KEYDOWN event */
	do {
		SDL_WaitEvent(&event);
		/* Preemptive stuff */
		if(event.type == SDL_KEYDOWN) {
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
				case SDLK_RETURN:
					/* Fullscreen toggle */
					if(event.key.keysym.mod & KMOD_ALT)
						display_fullscreen();
					break;
				default:
					break;
			}
		/* UserEvent means it's time to update the cursor */
		} else if(event.type == SDL_USEREVENT && timer != csoc) {
			if(timer)
				display_update(&info, info.write_x, info.write_y, 1, 1);
			else
				display_curse(info.write_x, info.write_y);
			csoc = timer;
		/* Focus change? */
		} else if(event.type == SDL_ACTIVEEVENT) {
			if(event.active.state & SDL_APPINPUTFOCUS) {
				if(event.active.gain && timer == 2) {
					/* Make cursor normal */
					csoc = timer = 1;
					timerId = SDL_AddTimer(100, display_tick, NULL);
					display_curse(info.write_x, info.write_y);
				} else {
					/* Inactive cursor */
					SDL_RemoveTimer(timerId);
					timer = 2;
					display_update(&info, info.write_x, info.write_y, 1, 1);
					display_curse_inactive(info.write_x, info.write_y);
				}
			}
		}
	} while(event.type != SDL_KEYDOWN);

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
		default:
			break;
	}

	/* Ctrl is down */
	if(event.key.keysym.mod & KMOD_CTRL) {
		/* If alpha key, return special ctrl+alpha */
		if(event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z) {
			event.key.keysym.sym -= 0x60;
		}
	}
	/* Alt is down */
	else if(event.key.keysym.mod & KMOD_ALT) {
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
			default:
				break;
		}
	}
	/* Shift is down */
	if(event.key.keysym.mod & KMOD_SHIFT) {
		/* If alpha key, shift means make capital */
		if(event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z) {
			event.key.keysym.sym ^= ' ';
		} else {
			/* Other shift conversions */
			switch(event.key.keysym.sym) {
				case '`':
					event.key.keysym.sym = '~';
					break;
				case '1':
					event.key.keysym.sym = '!';
					break;
				case '2':
					event.key.keysym.sym = '@';
					break;
				case '3':
					event.key.keysym.sym = '#';
					break;
				case '4':
					event.key.keysym.sym = '$';
					break;
				case '5':
					event.key.keysym.sym = '%';
					break;
				case '6':
					event.key.keysym.sym = '^';
					break;
				case '7':
					event.key.keysym.sym = '&';
					break;
				case '8':
					event.key.keysym.sym = '*';
					break;
				case '9':
					event.key.keysym.sym = '(';
					break;
				case '0':
					event.key.keysym.sym = ')';
					break;
				case '-':
					event.key.keysym.sym = '_';
					break;
				case '=':
					event.key.keysym.sym = '+';
					break;
				case '[':
					event.key.keysym.sym = '{';
					break;
				case ']':
					event.key.keysym.sym = '}';
					break;
				case '\\':
					event.key.keysym.sym = '|';
					break;
				case ',':
					event.key.keysym.sym = '<';
					break;
				case '.':
					event.key.keysym.sym = '>';
					break;
				case '/':
					event.key.keysym.sym = '?';
					break;
				case ';':
					event.key.keysym.sym = ':';
					break;
				case '\'':
					event.key.keysym.sym = '"';
					break;
				default:
					break;
			}
		}
		shift = 1;
	} else {
		shift = 0;
	}

	/* Return the key */
	return event.key.keysym.sym;
}

int display_sdl_kbhit()
{
	SDL_Event event;
	int retval;

	/* Get number of KEYDOWN events in the queue */
	retval = SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_KEYDOWN);

	/* Non-zero means 1 */
	if(retval != 0)
		retval = 1;

	return retval;
}

void display_sdl_gotoxy(int x, int y)
{
	/* Undraw the cursor if it's on */
	if(csoc) {
		display_update(&info, info.write_x, info.write_y, 1, 1);
		display_curse(x, y);
	}
	display_gotoxy(&info, x, y);
}

void display_sdl_print(int x, int y, int c, char *ch)
{
	int i, len = strlen(ch);

	for(i = 0; i < len; i++)
		display_sdl_putch(x+i, y, ch[i], c);
	display_update(&info, x, y, len, 1);
}

void display_sdl_titlebar(char *title)
{
	SDL_WM_SetCaption(title, NULL);
}

int display_sdl_shift()
{
	return shift;
}

void display_sdl_putch_discrete(int x, int y, int ch, int co)
{
	display_putch(&info, x, y, ch, co);
}

void display_sdl_update(int x, int y, int w, int h)
{
	display_update(&info, x, y, w, h);
}

displaymethod display_sdl =
{
	NULL,
	"SDL Textmode Emulator",
	"1.0",
	display_sdl_init,
	display_sdl_end,
	display_sdl_putch,
	display_sdl_getch,
	display_sdl_kbhit,
	display_sdl_gotoxy,
	display_sdl_print,
	display_sdl_titlebar,
	display_sdl_shift,
	display_sdl_putch_discrete,
	display_sdl_update
};
