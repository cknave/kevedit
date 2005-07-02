/* display_sdl.c	-- SDL Textmode Emulation display method for KevEdit
 * $Id: display_sdl.c,v 1.5 2005/07/02 21:31:30 kvance Exp $
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
#include "SDL.h"

#include "display.h"
#include "display_sdl.h"

int xstart, ystart;	/* Where the viewport begins */
static SDL_TimerID timerId;	/* Timer ID */
static int timer, csoc;	/* Timer for cursor, current state of cursor */

/* Forward defines :( */
static Uint32 display_tick(Uint32 interval, void *blank);
void display_curse(int x, int y);

#define CURSOR_RATE 200

/*************************************
 *** BEGIN TEXTMODE EMULATION CODE ***
 *************************************/

/* Default charset (embedded for convenience) */
#define DEFAULT_CHARSET_SIZE 3584
const Uint8 default_charset[] = {
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x7E', '\x81', '\xA5', '\x81', '\x81', '\xBD', '\x99', '\x81', 
	'\x7E', '\x00', '\x00', '\x00', '\x00', '\x00', '\x7E', '\xFF', 
	'\xDB', '\xFF', '\xFF', '\xC3', '\xE7', '\xFF', '\x7E', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x6C', '\xFE', '\xFE', 
	'\xFE', '\xFE', '\x7C', '\x38', '\x10', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x10', '\x38', '\x7C', '\xFE', '\x7C', 
	'\x38', '\x10', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x18', '\x3C', '\x3C', '\xE7', '\xE7', '\xE7', '\x18', '\x18', 
	'\x3C', '\x00', '\x00', '\x00', '\x00', '\x00', '\x18', '\x3C', 
	'\x7E', '\xFF', '\xFF', '\x7E', '\x18', '\x18', '\x3C', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x18', 
	'\x3C', '\x3C', '\x18', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xE7', '\xC3', '\xC3', 
	'\xE7', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\x00', '\x00', 
	'\x00', '\x00', '\x3C', '\x66', '\x42', '\x42', '\x66', '\x3C', 
	'\x00', '\x00', '\x00', '\x00', '\xFF', '\xFF', '\xFF', '\xFF', 
	'\xC3', '\x99', '\xBD', '\xBD', '\x99', '\xC3', '\xFF', '\xFF', 
	'\xFF', '\xFF', '\x00', '\x00', '\x1E', '\x0E', '\x1A', '\x32', 
	'\x78', '\xCC', '\xCC', '\xCC', '\x78', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x3C', '\x66', '\x66', '\x66', '\x3C', '\x18', 
	'\x7E', '\x18', '\x18', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x3F', '\x33', '\x3F', '\x30', '\x30', '\x30', '\x70', '\xF0', 
	'\xE0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x7F', '\x63', 
	'\x7F', '\x63', '\x63', '\x63', '\x67', '\xE7', '\xE6', '\xC0', 
	'\x00', '\x00', '\x00', '\x00', '\x18', '\x18', '\xDB', '\x3C', 
	'\xE7', '\x3C', '\xDB', '\x18', '\x18', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x80', '\xC0', '\xE0', '\xF8', '\xFE', '\xF8', 
	'\xE0', '\xC0', '\x80', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x02', '\x06', '\x0E', '\x3E', '\xFE', '\x3E', '\x0E', '\x06', 
	'\x02', '\x00', '\x00', '\x00', '\x00', '\x00', '\x18', '\x3C', 
	'\x7E', '\x18', '\x18', '\x18', '\x7E', '\x3C', '\x18', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x66', '\x66', '\x66', '\x66', 
	'\x66', '\x66', '\x00', '\x66', '\x66', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x7F', '\xDB', '\xDB', '\xDB', '\x7B', '\x1B', 
	'\x1B', '\x1B', '\x1B', '\x00', '\x00', '\x00', '\x00', '\x7C', 
	'\xC6', '\x60', '\x38', '\x6C', '\xC6', '\xC6', '\x6C', '\x38', 
	'\x0C', '\xC6', '\x7C', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\xFE', '\xFE', '\xFE', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x18', '\x3C', '\x7E', '\x18', 
	'\x18', '\x18', '\x7E', '\x3C', '\x18', '\x7E', '\x00', '\x00', 
	'\x00', '\x00', '\x18', '\x3C', '\x7E', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x7E', '\x3C', 
	'\x18', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x18', '\x0C', '\xFE', '\x0C', '\x18', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x30', '\x60', 
	'\xFE', '\x60', '\x30', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\xC0', '\xC0', '\xC0', 
	'\xFE', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x28', '\x6C', '\xFE', '\x6C', '\x28', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x10', 
	'\x38', '\x38', '\x7C', '\x7C', '\xFE', '\xFE', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\xFE', '\xFE', '\x7C', 
	'\x7C', '\x38', '\x38', '\x10', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x18', '\x3C', '\x3C', '\x3C', '\x18', '\x18', '\x00', '\x18', 
	'\x18', '\x00', '\x00', '\x00', '\x00', '\x66', '\x66', '\x66', 
	'\x24', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x6C', '\x6C', '\xFE', '\x6C', 
	'\x6C', '\x6C', '\xFE', '\x6C', '\x6C', '\x00', '\x00', '\x00', 
	'\x18', '\x18', '\x7C', '\xC6', '\xC2', '\xC0', '\x7C', '\x06', 
	'\x86', '\xC6', '\x7C', '\x18', '\x18', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\xC2', '\xC6', '\x0C', '\x18', '\x30', '\x66', 
	'\xC6', '\x00', '\x00', '\x00', '\x00', '\x00', '\x38', '\x6C', 
	'\x6C', '\x38', '\x76', '\xDC', '\xCC', '\xCC', '\x76', '\x00', 
	'\x00', '\x00', '\x00', '\x30', '\x30', '\x30', '\x60', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x0C', '\x18', '\x30', '\x30', '\x30', '\x30', 
	'\x30', '\x18', '\x0C', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x30', '\x18', '\x0C', '\x0C', '\x0C', '\x0C', '\x0C', '\x18', 
	'\x30', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x66', '\x3C', '\xFF', '\x3C', '\x66', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x18', '\x18', 
	'\x7E', '\x18', '\x18', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x18', '\x18', '\x18', '\x30', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\xFE', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x18', '\x18', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x02', '\x06', '\x0C', '\x18', 
	'\x30', '\x60', '\xC0', '\x80', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x7C', '\xC6', '\xCE', '\xDE', '\xF6', '\xE6', 
	'\xC6', '\xC6', '\x7C', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x18', '\x38', '\x78', '\x18', '\x18', '\x18', '\x18', '\x18', 
	'\x7E', '\x00', '\x00', '\x00', '\x00', '\x00', '\x7C', '\xC6', 
	'\x06', '\x0C', '\x18', '\x30', '\x60', '\xC6', '\xFE', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x7C', '\xC6', '\x06', '\x06', 
	'\x3C', '\x06', '\x06', '\xC6', '\x7C', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x0C', '\x1C', '\x3C', '\x6C', '\xCC', '\xFE', 
	'\x0C', '\x0C', '\x1E', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\xFE', '\xC0', '\xC0', '\xC0', '\xFC', '\x06', '\x06', '\xC6', 
	'\x7C', '\x00', '\x00', '\x00', '\x00', '\x00', '\x38', '\x60', 
	'\xC0', '\xC0', '\xFC', '\xC6', '\xC6', '\xC6', '\x7C', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\xFE', '\xC6', '\x06', '\x0C', 
	'\x18', '\x30', '\x30', '\x30', '\x30', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x7C', '\xC6', '\xC6', '\xC6', '\x7C', '\xC6', 
	'\xC6', '\xC6', '\x7C', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x7C', '\xC6', '\xC6', '\xC6', '\x7E', '\x06', '\x06', '\x0C', 
	'\x78', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x18', 
	'\x18', '\x00', '\x00', '\x00', '\x18', '\x18', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x18', '\x18', '\x00', 
	'\x00', '\x00', '\x18', '\x18', '\x30', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x06', '\x0C', '\x18', '\x30', '\x60', '\x30', 
	'\x18', '\x0C', '\x06', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x7E', '\x00', '\x00', '\x7E', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x60', '\x30', 
	'\x18', '\x0C', '\x06', '\x0C', '\x18', '\x30', '\x60', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x7C', '\xC6', '\xC6', '\x0C', 
	'\x18', '\x18', '\x00', '\x18', '\x18', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x7C', '\xC6', '\xC6', '\xDE', '\xDE', '\xDE', 
	'\xDC', '\xC0', '\x7C', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x10', '\x38', '\x6C', '\xC6', '\xC6', '\xFE', '\xC6', '\xC6', 
	'\xC6', '\x00', '\x00', '\x00', '\x00', '\x00', '\xFC', '\x66', 
	'\x66', '\x66', '\x7C', '\x66', '\x66', '\x66', '\xFC', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x3C', '\x66', '\xC2', '\xC0', 
	'\xC0', '\xC0', '\xC2', '\x66', '\x3C', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\xF8', '\x6C', '\x66', '\x66', '\x66', '\x66', 
	'\x66', '\x6C', '\xF8', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\xFE', '\x66', '\x62', '\x68', '\x78', '\x68', '\x62', '\x66', 
	'\xFE', '\x00', '\x00', '\x00', '\x00', '\x00', '\xFE', '\x66', 
	'\x62', '\x68', '\x78', '\x68', '\x60', '\x60', '\xF0', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x3C', '\x66', '\xC2', '\xC0', 
	'\xC0', '\xDE', '\xC6', '\x66', '\x3A', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\xC6', '\xC6', '\xC6', '\xC6', '\xFE', '\xC6', 
	'\xC6', '\xC6', '\xC6', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x3C', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', 
	'\x3C', '\x00', '\x00', '\x00', '\x00', '\x00', '\x1E', '\x0C', 
	'\x0C', '\x0C', '\x0C', '\x0C', '\xCC', '\xCC', '\x78', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\xE6', '\x66', '\x6C', '\x6C', 
	'\x78', '\x6C', '\x6C', '\x66', '\xE6', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\xF0', '\x60', '\x60', '\x60', '\x60', '\x60', 
	'\x62', '\x66', '\xFE', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\xC6', '\xEE', '\xFE', '\xFE', '\xD6', '\xC6', '\xC6', '\xC6', 
	'\xC6', '\x00', '\x00', '\x00', '\x00', '\x00', '\xC6', '\xE6', 
	'\xF6', '\xFE', '\xDE', '\xCE', '\xC6', '\xC6', '\xC6', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x38', '\x6C', '\xC6', '\xC6', 
	'\xC6', '\xC6', '\xC6', '\x6C', '\x38', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\xFC', '\x66', '\x66', '\x66', '\x7C', '\x60', 
	'\x60', '\x60', '\xF0', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x7C', '\xC6', '\xC6', '\xC6', '\xC6', '\xD6', '\xDE', '\x7C', 
	'\x0C', '\x0E', '\x00', '\x00', '\x00', '\x00', '\xFC', '\x66', 
	'\x66', '\x66', '\x7C', '\x6C', '\x66', '\x66', '\xE6', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x7C', '\xC6', '\xC6', '\x60', 
	'\x38', '\x0C', '\xC6', '\xC6', '\x7C', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x7E', '\x7E', '\x5A', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\x3C', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\xC6', '\xC6', '\xC6', '\xC6', '\xC6', '\xC6', '\xC6', '\xC6', 
	'\x7C', '\x00', '\x00', '\x00', '\x00', '\x00', '\xC6', '\xC6', 
	'\xC6', '\xC6', '\xC6', '\xC6', '\x6C', '\x38', '\x10', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\xC6', '\xC6', '\xC6', '\xC6', 
	'\xD6', '\xD6', '\xFE', '\x7C', '\x6C', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\xC6', '\xC6', '\x6C', '\x38', '\x38', '\x38', 
	'\x6C', '\xC6', '\xC6', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x66', '\x66', '\x66', '\x66', '\x3C', '\x18', '\x18', '\x18', 
	'\x3C', '\x00', '\x00', '\x00', '\x00', '\x00', '\xFE', '\xC6', 
	'\x8C', '\x18', '\x30', '\x60', '\xC2', '\xC6', '\xFE', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x3C', '\x30', '\x30', '\x30', 
	'\x30', '\x30', '\x30', '\x30', '\x3C', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x80', '\xC0', '\xE0', '\x70', '\x38', '\x1C', 
	'\x0E', '\x06', '\x02', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x3C', '\x0C', '\x0C', '\x0C', '\x0C', '\x0C', '\x0C', '\x0C', 
	'\x3C', '\x00', '\x00', '\x00', '\x10', '\x38', '\x6C', '\xC6', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xFF', '\x00', 
	'\x30', '\x30', '\x18', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x78', '\x0C', '\x7C', '\xCC', '\xCC', 
	'\x76', '\x00', '\x00', '\x00', '\x00', '\x00', '\xE0', '\x60', 
	'\x60', '\x78', '\x6C', '\x66', '\x66', '\x66', '\x7C', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x7C', 
	'\xC6', '\xC0', '\xC0', '\xC6', '\x7C', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x1C', '\x0C', '\x0C', '\x3C', '\x6C', '\xCC', 
	'\xCC', '\xCC', '\x76', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x7C', '\xC6', '\xFE', '\xC0', '\xC6', 
	'\x7C', '\x00', '\x00', '\x00', '\x00', '\x00', '\x38', '\x6C', 
	'\x64', '\x60', '\xF0', '\x60', '\x60', '\x60', '\xF0', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x76', 
	'\xCC', '\xCC', '\xCC', '\x7C', '\x0C', '\xCC', '\x78', '\x00', 
	'\x00', '\x00', '\xE0', '\x60', '\x60', '\x6C', '\x76', '\x66', 
	'\x66', '\x66', '\xE6', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x18', '\x18', '\x00', '\x38', '\x18', '\x18', '\x18', '\x18', 
	'\x3C', '\x00', '\x00', '\x00', '\x00', '\x00', '\x06', '\x06', 
	'\x00', '\x0E', '\x06', '\x06', '\x06', '\x06', '\x66', '\x66', 
	'\x3C', '\x00', '\x00', '\x00', '\xE0', '\x60', '\x60', '\x66', 
	'\x6C', '\x78', '\x6C', '\x66', '\xE6', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x38', '\x18', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\x3C', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\xEC', '\xFE', '\xD6', '\xD6', '\xD6', 
	'\xC6', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\xDC', '\x66', '\x66', '\x66', '\x66', '\x66', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x7C', 
	'\xC6', '\xC6', '\xC6', '\xC6', '\x7C', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\xDC', '\x66', '\x66', 
	'\x66', '\x7C', '\x60', '\x60', '\xF0', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x76', '\xCC', '\xCC', '\xCC', '\x7C', 
	'\x0C', '\x0C', '\x1E', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\xDC', '\x76', '\x66', '\x60', '\x60', '\xF0', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x7C', 
	'\xC6', '\x70', '\x1C', '\xC6', '\x7C', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x10', '\x30', '\x30', '\xFC', '\x30', '\x30', 
	'\x30', '\x36', '\x1C', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\xCC', '\xCC', '\xCC', '\xCC', '\xCC', 
	'\x76', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x66', '\x66', '\x66', '\x66', '\x3C', '\x18', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xC6', 
	'\xC6', '\xD6', '\xD6', '\xFE', '\x6C', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\xC6', '\x6C', '\x38', 
	'\x38', '\x6C', '\xC6', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\xC6', '\xC6', '\xC6', '\xC6', '\x7E', 
	'\x06', '\x0C', '\xF8', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\xFE', '\xCC', '\x18', '\x30', '\x66', '\xFE', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x0E', '\x18', '\x18', '\x18', 
	'\x70', '\x18', '\x18', '\x18', '\x0E', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x18', '\x18', '\x18', '\x18', '\x00', '\x18', 
	'\x18', '\x18', '\x18', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x70', '\x18', '\x18', '\x18', '\x0E', '\x18', '\x18', '\x18', 
	'\x70', '\x00', '\x00', '\x00', '\x00', '\x00', '\x76', '\xDC', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x10', '\x38', 
	'\x6C', '\xC6', '\xC6', '\xFE', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x3C', '\x66', '\xC2', '\xC0', '\xC0', '\xC2', 
	'\x66', '\x3C', '\x0C', '\x06', '\x7C', '\x00', '\x00', '\x00', 
	'\xCC', '\xCC', '\x00', '\xCC', '\xCC', '\xCC', '\xCC', '\xCC', 
	'\x76', '\x00', '\x00', '\x00', '\x00', '\x0C', '\x18', '\x30', 
	'\x00', '\x7C', '\xC6', '\xFE', '\xC0', '\xC6', '\x7C', '\x00', 
	'\x00', '\x00', '\x00', '\x10', '\x38', '\x6C', '\x00', '\x78', 
	'\x0C', '\x7C', '\xCC', '\xCC', '\x76', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\xCC', '\xCC', '\x00', '\x78', '\x0C', '\x7C', 
	'\xCC', '\xCC', '\x76', '\x00', '\x00', '\x00', '\x00', '\x60', 
	'\x30', '\x18', '\x00', '\x78', '\x0C', '\x7C', '\xCC', '\xCC', 
	'\x76', '\x00', '\x00', '\x00', '\x00', '\x38', '\x6C', '\x38', 
	'\x00', '\x78', '\x0C', '\x7C', '\xCC', '\xCC', '\x76', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x3C', '\x66', 
	'\x60', '\x66', '\x3C', '\x0C', '\x06', '\x3C', '\x00', '\x00', 
	'\x00', '\x10', '\x38', '\x6C', '\x00', '\x7C', '\xC6', '\xFE', 
	'\xC0', '\xC6', '\x7C', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\xCC', '\xCC', '\x00', '\x7C', '\xC6', '\xFE', '\xC0', '\xC6', 
	'\x7C', '\x00', '\x00', '\x00', '\x00', '\x60', '\x30', '\x18', 
	'\x00', '\x7C', '\xC6', '\xFE', '\xC0', '\xC6', '\x7C', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x66', '\x66', '\x00', '\x38', 
	'\x18', '\x18', '\x18', '\x18', '\x3C', '\x00', '\x00', '\x00', 
	'\x00', '\x18', '\x3C', '\x66', '\x00', '\x38', '\x18', '\x18', 
	'\x18', '\x18', '\x3C', '\x00', '\x00', '\x00', '\x00', '\x60', 
	'\x30', '\x18', '\x00', '\x38', '\x18', '\x18', '\x18', '\x18', 
	'\x3C', '\x00', '\x00', '\x00', '\x00', '\xC6', '\xC6', '\x10', 
	'\x38', '\x6C', '\xC6', '\xC6', '\xFE', '\xC6', '\xC6', '\x00', 
	'\x00', '\x00', '\x38', '\x6C', '\x38', '\x00', '\x38', '\x6C', 
	'\xC6', '\xC6', '\xFE', '\xC6', '\xC6', '\x00', '\x00', '\x00', 
	'\x18', '\x30', '\x60', '\x00', '\xFE', '\x66', '\x60', '\x7C', 
	'\x60', '\x66', '\xFE', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\xCC', '\x76', '\x36', '\x7E', '\xD8', '\xD8', 
	'\x6E', '\x00', '\x00', '\x00', '\x00', '\x00', '\x3E', '\x6C', 
	'\xCC', '\xCC', '\xFE', '\xCC', '\xCC', '\xCC', '\xCE', '\x00', 
	'\x00', '\x00', '\x00', '\x10', '\x38', '\x6C', '\x00', '\x7C', 
	'\xC6', '\xC6', '\xC6', '\xC6', '\x7C', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\xC6', '\xC6', '\x00', '\x7C', '\xC6', '\xC6', 
	'\xC6', '\xC6', '\x7C', '\x00', '\x00', '\x00', '\x00', '\x60', 
	'\x30', '\x18', '\x00', '\x7C', '\xC6', '\xC6', '\xC6', '\xC6', 
	'\x7C', '\x00', '\x00', '\x00', '\x00', '\x30', '\x78', '\xCC', 
	'\x00', '\xCC', '\xCC', '\xCC', '\xCC', '\xCC', '\x76', '\x00', 
	'\x00', '\x00', '\x00', '\x60', '\x30', '\x18', '\x00', '\xCC', 
	'\xCC', '\xCC', '\xCC', '\xCC', '\x76', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\xC6', '\xC6', '\x00', '\xC6', '\xC6', '\xC6', 
	'\xC6', '\x7E', '\x06', '\x0C', '\x78', '\x00', '\x00', '\xC6', 
	'\xC6', '\x38', '\x6C', '\xC6', '\xC6', '\xC6', '\xC6', '\x6C', 
	'\x38', '\x00', '\x00', '\x00', '\x00', '\xC6', '\xC6', '\x00', 
	'\xC6', '\xC6', '\xC6', '\xC6', '\xC6', '\xC6', '\x7C', '\x00', 
	'\x00', '\x00', '\x00', '\x18', '\x18', '\x3C', '\x66', '\x60', 
	'\x60', '\x66', '\x3C', '\x18', '\x18', '\x00', '\x00', '\x00', 
	'\x00', '\x38', '\x6C', '\x64', '\x60', '\xF0', '\x60', '\x60', 
	'\x60', '\xE6', '\xFC', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x66', '\x66', '\x3C', '\x18', '\x7E', '\x18', '\x7E', '\x18', 
	'\x18', '\x00', '\x00', '\x00', '\x00', '\xF8', '\xCC', '\xCC', 
	'\xF8', '\xC4', '\xCC', '\xDE', '\xCC', '\xCC', '\xC6', '\x00', 
	'\x00', '\x00', '\x00', '\x0E', '\x1B', '\x18', '\x18', '\x18', 
	'\x7E', '\x18', '\x18', '\x18', '\x18', '\xD8', '\x70', '\x00', 
	'\x00', '\x18', '\x30', '\x60', '\x00', '\x78', '\x0C', '\x7C', 
	'\xCC', '\xCC', '\x76', '\x00', '\x00', '\x00', '\x00', '\x0C', 
	'\x18', '\x30', '\x00', '\x38', '\x18', '\x18', '\x18', '\x18', 
	'\x3C', '\x00', '\x00', '\x00', '\x00', '\x18', '\x30', '\x60', 
	'\x00', '\x7C', '\xC6', '\xC6', '\xC6', '\xC6', '\x7C', '\x00', 
	'\x00', '\x00', '\x00', '\x18', '\x30', '\x60', '\x00', '\xCC', 
	'\xCC', '\xCC', '\xCC', '\xCC', '\x76', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x76', '\xDC', '\x00', '\xDC', '\x66', '\x66', 
	'\x66', '\x66', '\x66', '\x00', '\x00', '\x00', '\x76', '\xDC', 
	'\x00', '\xC6', '\xE6', '\xF6', '\xFE', '\xDE', '\xCE', '\xC6', 
	'\xC6', '\x00', '\x00', '\x00', '\x00', '\x3C', '\x6C', '\x6C', 
	'\x3E', '\x00', '\x7E', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x38', '\x6C', '\x6C', '\x38', '\x00', 
	'\x7C', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x30', '\x30', '\x00', '\x30', '\x30', '\x60', 
	'\xC6', '\xC6', '\x7C', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\xFE', '\xC0', '\xC0', '\xC0', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\xFE', '\x06', '\x06', '\x06', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\xC0', '\xC0', '\xC6', '\xCC', '\xD8', 
	'\x30', '\x60', '\xDC', '\x86', '\x0C', '\x18', '\x3E', '\x00', 
	'\x00', '\xC0', '\xC0', '\xC6', '\xCC', '\xD8', '\x30', '\x66', 
	'\xCE', '\x9E', '\x3E', '\x06', '\x06', '\x00', '\x00', '\x00', 
	'\x18', '\x18', '\x00', '\x18', '\x18', '\x3C', '\x3C', '\x3C', 
	'\x18', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x36', '\x6C', '\xD8', '\x6C', '\x36', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xD8', '\x6C', 
	'\x36', '\x6C', '\xD8', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x11', '\x44', '\x11', '\x44', '\x11', '\x44', '\x11', '\x44', 
	'\x11', '\x44', '\x11', '\x44', '\x11', '\x44', '\x55', '\xAA', 
	'\x55', '\xAA', '\x55', '\xAA', '\x55', '\xAA', '\x55', '\xAA', 
	'\x55', '\xAA', '\x55', '\xAA', '\xDD', '\x77', '\xDD', '\x77', 
	'\xDD', '\x77', '\xDD', '\x77', '\xDD', '\x77', '\xDD', '\x77', 
	'\xDD', '\x77', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\xF8', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\xF8', '\x18', '\xF8', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\x36', '\x36', '\xF6', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\x36', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\xFE', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\xF8', '\x18', '\xF8', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x36', '\x36', 
	'\x36', '\x36', '\x36', '\xF6', '\x06', '\xF6', '\x36', '\x36', 
	'\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\x36', '\x00', '\x00', '\x00', '\x00', '\x00', '\xFE', 
	'\x06', '\xF6', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\x36', '\x36', '\x36', '\x36', '\xF6', '\x06', '\xFE', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x36', '\x36', 
	'\x36', '\x36', '\x36', '\x36', '\x36', '\xFE', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\xF8', '\x18', '\xF8', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\xF8', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x1F', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\xFF', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\xFF', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\x1F', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xFF', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\xFF', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\x1F', '\x18', '\x1F', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\x37', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\x36', '\x36', '\x36', '\x36', '\x37', '\x30', '\x3F', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x3F', '\x30', '\x37', '\x36', '\x36', 
	'\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\xF7', '\x00', '\xFF', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xFF', 
	'\x00', '\xF7', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\x36', '\x36', '\x36', '\x36', '\x37', '\x30', '\x37', 
	'\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\xFF', '\x00', '\xFF', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\xF7', '\x00', '\xF7', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\x36', '\x18', '\x18', '\x18', '\x18', '\x18', '\xFF', 
	'\x00', '\xFF', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\xFF', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\xFF', '\x00', '\xFF', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\xFF', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\x3F', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x1F', '\x18', '\x1F', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x1F', '\x18', '\x1F', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x3F', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', 
	'\x36', '\xFF', '\x36', '\x36', '\x36', '\x36', '\x36', '\x36', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\xFF', '\x18', '\xFF', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\xF8', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x1F', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', 
	'\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xFF', 
	'\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xF0', '\xF0', 
	'\xF0', '\xF0', '\xF0', '\xF0', '\xF0', '\xF0', '\xF0', '\xF0', 
	'\xF0', '\xF0', '\xF0', '\xF0', '\x0F', '\x0F', '\x0F', '\x0F', 
	'\x0F', '\x0F', '\x0F', '\x0F', '\x0F', '\x0F', '\x0F', '\x0F', 
	'\x0F', '\x0F', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', 
	'\xFF', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x76', '\xDC', '\xD8', 
	'\xD8', '\xDC', '\x76', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x7C', '\xC6', '\xFC', '\xC6', '\xC6', '\xFC', 
	'\xC0', '\xC0', '\x40', '\x00', '\x00', '\x00', '\xFE', '\xC6', 
	'\xC6', '\xC0', '\xC0', '\xC0', '\xC0', '\xC0', '\xC0', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xFE', '\x6C', 
	'\x6C', '\x6C', '\x6C', '\x6C', '\x6C', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\xFE', '\xC6', '\x60', '\x30', '\x18', '\x30', 
	'\x60', '\xC6', '\xFE', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x7E', '\xD8', '\xD8', '\xD8', '\xD8', 
	'\x70', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x66', '\x66', '\x66', '\x66', '\x7C', '\x60', '\x60', '\xC0', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x76', '\xDC', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x7E', '\x18', '\x3C', '\x66', '\x66', '\x66', 
	'\x3C', '\x18', '\x7E', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x38', '\x6C', '\xC6', '\xC6', '\xFE', '\xC6', '\xC6', '\x6C', 
	'\x38', '\x00', '\x00', '\x00', '\x00', '\x00', '\x38', '\x6C', 
	'\xC6', '\xC6', '\xC6', '\x6C', '\x6C', '\x6C', '\xEE', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x1E', '\x30', '\x18', '\x0C', 
	'\x3E', '\x66', '\x66', '\x66', '\x3C', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x7E', '\xDB', '\xDB', 
	'\x7E', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x03', '\x06', '\x7E', '\xDB', '\xDB', '\xF3', '\x7E', '\x60', 
	'\xC0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x1C', '\x30', 
	'\x60', '\x60', '\x7C', '\x60', '\x60', '\x30', '\x1C', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x7C', '\xC6', '\xC6', 
	'\xC6', '\xC6', '\xC6', '\xC6', '\xC6', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\xFE', '\x00', '\x00', '\xFE', '\x00', 
	'\x00', '\xFE', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x18', '\x18', '\x7E', '\x18', '\x18', '\x00', '\x00', 
	'\xFF', '\x00', '\x00', '\x00', '\x00', '\x00', '\x30', '\x18', 
	'\x0C', '\x06', '\x0C', '\x18', '\x30', '\x00', '\x7E', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x0C', '\x18', '\x30', '\x60', 
	'\x30', '\x18', '\x0C', '\x00', '\x7E', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x0E', '\x1B', '\x1B', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\x18', 
	'\x18', '\x18', '\x18', '\x18', '\x18', '\x18', '\xD8', '\xD8', 
	'\x70', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x18', 
	'\x18', '\x00', '\x7E', '\x00', '\x18', '\x18', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x76', '\xDC', 
	'\x00', '\x76', '\xDC', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x38', '\x6C', '\x6C', '\x38', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x18', '\x18', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x18', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x0F', '\x0C', '\x0C', '\x0C', '\x0C', 
	'\x0C', '\xEC', '\x6C', '\x3C', '\x1C', '\x00', '\x00', '\x00', 
	'\x00', '\xD8', '\x6C', '\x6C', '\x6C', '\x6C', '\x6C', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x70', 
	'\xD8', '\x30', '\x60', '\xC8', '\xF8', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x7C', '\x7C', '\x7C', '\x7C', '\x7C', '\x7C', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'
};

/* EGA palette (6 bits per component) */
#define EGA_PALETTE_SIZE 48
const Uint8 ega_palette[] = {
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x2A', '\x00', '\x2A', 
	'\x00', '\x00', '\x2A', '\x2A', '\x2A', '\x00', '\x00', '\x2A', 
	'\x00', '\x2A', '\x2A', '\x15', '\x00', '\x2A', '\x2A', '\x2A', 
	'\x15', '\x15', '\x15', '\x15', '\x15', '\x3F', '\x15', '\x3F', 
	'\x15', '\x15', '\x3F', '\x3F', '\x3F', '\x15', '\x15', '\x3F', 
	'\x15', '\x3F', '\x3F', '\x3F', '\x15', '\x3F', '\x3F', '\x3F'
};

void display_load_charset(Uint8 *dest, Uint8 *name)
{
	FILE *fp;
	fp = fopen(name, "rb");
	fread(dest, 256 * 14, 1, fp);
	fclose(fp);
}

void expand_palette(Uint32 *dest, Uint8 *src)
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

void display_load_palette(Uint32 *dest, Uint8 *name)
{
	FILE *fp;
	Uint8 palette[3 * 16];

	fp = fopen(name, "rb");
	fread(palette, 3 * 16, 1, fp);

	expand_palette(dest, palette);
	fclose(fp);
}

void display_load_default_charset(Uint8 *dest)
{
	memcpy(dest, default_charset, DEFAULT_CHARSET_SIZE);
}

void display_load_default_palette(Uint32 *dest)
{
	expand_palette(dest, (Uint8 *) ega_palette);
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
	vdest->buffer = createTextBlock(80, 25);
	vdest->char_set = (Uint8 *)malloc(256 * 14);
	vdest->palette = (Uint32 *)malloc(4 * 16);
	vdest->write_x = 0;
	vdest->write_y = 0;

	vdest->width  = width;
	vdest->height = height;
	vdest->depth  = depth;
	vdest->vflags = vflags;
}

void display_restart(video_info *vdest)
{
	vdest->video = SDL_SetVideoMode(vdest->width, vdest->height, vdest->depth, vdest->vflags);
	display_redraw(vdest);
}

void display_end(video_info *vdest)
{
	SDL_FreeSurface(vdest->buffer_surface);

	deleteTextBlock(vdest->buffer);
	free(vdest->char_set);
	free(vdest->palette);

	/* SDL should restore everything okay.. just use SDL_quit() when ready */
}

void display_putch(video_info *vdest, Uint32 x, Uint32 y, Uint8 ch, Uint8 co)
{
	textBlockPutch(vdest->buffer, x, y, ch, co);
}

void display_gotoxy(video_info *vdest, Uint32 x, Uint32 y)
{
	vdest->write_x = x;
	vdest->write_y = y;

	/* Here's a nice usability fix.  When we reposition the cursor, make
	 * it visible and reset the timer. */
	SDL_RemoveTimer(timerId);
	timerId = SDL_AddTimer(CURSOR_RATE, display_tick, NULL);
	if(timer != 2)
		timer = 0;
	display_curse(x, y);
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

	char_pointer = vdest->buffer->data;
	color_pointer = vdest->buffer->data + 1;
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
	Uint32 rowsleft, colsleft;

	Uint8 *char_pointer  = vdest->buffer->data + ((y*80+x)<<1);
	Uint8 *color_pointer = vdest->buffer->data + ((y*80+x)<<1) + 1;

	/* Draw each row onto video memory */
	for (rowsleft = height; rowsleft; rowsleft--) {

		/* Determine the video pointer for this row */
		Uint32 *root = vdest->buffer_surface->pixels;
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

	/* Update the buffer surface to the real thing.. */

	src_rect.x = (x<<3);
	src_rect.y = (y*14);
	dest_rect.x = src_rect.x+xstart;
	dest_rect.y = src_rect.y+ystart;
	src_rect.w = dest_rect.w = (width<<3);
	src_rect.h = dest_rect.h = (height*14);

	SDL_BlitSurface(vdest->buffer_surface, &src_rect, vdest->video, &dest_rect);
	SDL_UpdateRect(vdest->video, dest_rect.x, dest_rect.y, dest_rect.w, dest_rect.h);
}

/********************************
 *** BEGIN KEVEDIT GLUE LAYER ***
 ********************************/
video_info info;	/* Display info */
static int shift;	/* Shift state */

static int fullscreen = 0; /* Initial fullscreen setting */

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
	Uint8 color;
	Uint32 *video_pointer = info.buffer_surface->pixels;
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
	src_rect.x = (x<<3);
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
	Uint8 color;
	Uint32 *video_pointer = info.buffer_surface->pixels;
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
	src_rect.x = (x<<3);
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
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		return 0;
	}

	/* Automatic keyboard repeat */
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
			SDL_DEFAULT_REPEAT_INTERVAL);

	/* Fire up the textmode emulator */
	display_init(&info, 640, 350, 32, fullscreen, 1);
	display_load_default_charset(info.char_set);
	display_load_default_palette(info.palette);

	/* If in fullscreen mode, don't show the mouse */
	if(info.vflags & SDL_FULLSCREEN)
		SDL_ShowCursor(SDL_DISABLE);
	else
		SDL_ShowCursor(SDL_ENABLE);

	xstart = ((info.video->w) - 640) / 2;
	ystart = ((info.video->h) - 350) / 2;

	shift = 0;
	timer = csoc = 0;

	timerId = SDL_AddTimer(CURSOR_RATE, display_tick, NULL);

	return 1;
}

void display_sdl_end()
{
	/* Remember fullscreen setting */
	fullscreen = (info.vflags & SDL_FULLSCREEN) ? 1 : 0;

	/* Terminate SDL stuff */
	display_end(&info);
	SDL_Quit();
}

void display_sdl_putch(int x, int y, int ch, int co)
{
	/* Call textmode emulator putch */
	display_putch(&info, x, y, ch, co);
	display_update(&info, x, y, 1, 1);
}

void display_sdl_fullscreen()
{
	/* Toggle fullscreen */
	info.vflags ^= SDL_FULLSCREEN;

#ifdef WIN32
	/* Toggle fullscreen flag and restart the display */
	display_restart(&info);
#else
	/* This doesn't work in Windows */
	SDL_WM_ToggleFullScreen(info.video);
#endif

	/* If in fullscreen mode, don't show the mouse */
	if(info.vflags & SDL_FULLSCREEN)
		SDL_ShowCursor(SDL_DISABLE);
	else
		SDL_ShowCursor(SDL_ENABLE);
}

int display_sdl_getkey()
{
	SDL_Event event;

	/* Draw the cursor if necessary */
	if(timer)
		display_update(&info, info.write_x, info.write_y, 1, 1);
	else
		display_curse(info.write_x, info.write_y);

	/* Check for a KEYDOWN event */
	if (SDL_PollEvent(&event) == 0)
	return DKEY_NONE;

	/* Preemptive stuff */
	if(event.type == SDL_KEYDOWN) {
		/* Hack for windows: alt+tab will never show up */
		if((event.key.keysym.sym == SDLK_TAB) && 
				(event.key.keysym.mod & KMOD_ALT)) {
			printf("alt tab\n");
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
			case SDLK_RETURN:
				/* Fullscreen toggle */
				if(event.key.keysym.mod & KMOD_ALT) {
					event.type = SDL_KEYUP;
					display_sdl_fullscreen();
				}
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
		/* Repaint if the app becomes active again */
		if(event.active.state & SDL_APPACTIVE) {
			if(event.active.gain == 1)
				display_redraw(&info);
		}
		/* Change cursor type if focus changes */
		if(event.active.state & SDL_APPINPUTFOCUS) {
			if(event.active.gain && timer == 2) {
				/* Make cursor normal */
				csoc = timer = 1;
				timerId = SDL_AddTimer(CURSOR_RATE, display_tick, NULL);
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
			/* Add other letters of the alphabet as necessary */
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
				case SDLK_TAB:
					event.key.keysym.sym = DKEY_SHIFT_TAB;
					break;
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

int display_sdl_getch()
{
	int key;

	do {
		/* Draw the cursor if necessary */
		if(timer == 1)
			display_update(&info, info.write_x, info.write_y, 1, 1);
		else if(timer == 0)
			display_curse(info.write_x, info.write_y);

		if (SDL_WaitEvent(NULL) == 0)
			return DKEY_NONE;

		key = display_sdl_getkey();
	} while (key == DKEY_NONE);

	return key;
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

void display_sdl_print(int x, int y, int c, char *s)
{
	int i, len = strlen(s);

	for(i = 0; i < len; i++)
		display_sdl_putch(x+i, y, s[i], c);
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

void display_sdl_print_discrete(int x, int y, int c, char *s)
{
	int i, len = strlen(s);

	for(i = 0; i < len; i++)
		display_sdl_putch_discrete(x+i, y, s[i], c);
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
	display_sdl_getkey,
	display_sdl_gotoxy,
	display_sdl_print,
	display_sdl_titlebar,
	display_sdl_shift,
	display_sdl_putch_discrete,
	display_sdl_print_discrete,
	display_sdl_update
};
