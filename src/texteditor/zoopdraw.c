/* zoopdraw - draw zzt oop to a text display */
/* $Id: zoopdraw.c,v 1.2 2005/05/28 03:17:46 bitman Exp $ */
/* Copyright (C) 2002 Ryan Phillips <bitman@users.sourceforge.net>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "zoopdraw.h"

#include "libzzt2/zztoop.h"
#include "display/colours.h"

#include <string.h>
#include <ctype.h>

/* Get the colour for a component */
int zztoopDrawGetColour(ZZTOOPdrawer drawer, ZZTOOPcomponent * component);

/* Draw a music component */
void zztoopDrawMusic(ZZTOOPdrawer drawer, ZZTOOPcomponent * component);

void zztoopInitDrawer(ZZTOOPdrawer * drawer)
{
	drawer->display = NULL;

	/* Put the cursor somewhere safe */
	drawer->x = drawer->y = 0;
	drawer->length = 80;

	/* Don't help format by default */
	drawer->helpformatting = 0;

	/* Use default colour data */
	drawer->colours      = zztoopdefaultcolours;
	drawer->textcolours  = zztoopdefaulttextcolours;
	drawer->musiccolours = zztoopdefaultmusiccolours;
}

void zztoopDraw(ZZTOOPdrawer drawer, ZZTOOPcomponent * components)
{
	ZZTOOPcomponent * current = components;
	int textonly = 0;

	if (drawer.display == NULL || components == NULL)
		return;

	if (drawer.helpformatting && components->type == ZOOPTYPE_SYMBOL &&
			strchr("!:$", (char) components->value))
		textonly = 1;

	/* Loop through each component */
	while (current != NULL) {
		if (current->text != NULL && current->text[0] != '\x0') {
			int colour = zztoopDrawGetColour(drawer, current);
			int x = drawer.x + current->pos;

			/* Draw musical notation */
			if (current->type == ZOOPTYPE_MUSIC) {
				zztoopDrawMusic(drawer, current);
				x = -1;
			}

			/* In textonly mode, only display the text type */
			if (textonly) {
				if (current->type != ZOOPTYPE_TEXT) {
					x = -1;
				} else {
					/* Most text types require special processing */
					switch (current->value) {
						case ZOOPTEXT_HEADING:
							/* Headings are centered */
							x = drawer.x + (drawer.length - strlen(current->text)) / 2;
							break;

						case ZOOPTEXT_HYPERTEXT:
							/* Hypertext is indented with an arrow pointing at it */
							x = drawer.x + 3;
							drawer.display->putch_discrete(drawer.x, drawer.y, 0x10, drawer.colours[ZOOPTYPE_MESSAGE]);
							break;

						case ZOOPTEXT_NORMAL:
						case ZOOPTEXT_LABEL:
						default:
							/* These all flush left */
							x = drawer.x;
							break;
					}
				}
			}

			if (x >= 0)
				/* TODO: don't write past drawer.length */
				drawer.display->print_discrete(x, drawer.y, colour, current->text);
		}

		/* Advance to next component */
		current = current->next;
	}
}

int zztoopDrawGetColour(ZZTOOPdrawer drawer, ZZTOOPcomponent * component)
{
	int colour;

	if (component->type <= ZOOPTYPE_MAX)
		colour = drawer.colours[component->type];
	else
		colour = drawer.colours[ZOOPTYPE_NONE];

	if (component->type == ZOOPTYPE_TEXT) {
		/* Refer to textcolours when displaying these */
		if (component->value <= ZOOPTEXT_MAX)
			colour = drawer.textcolours[component->value];
	} else if (component->type >= ZOOPTYPE_ITEM) {
		/* Display these as ZOOPTYPE_NONE when value is -1: */
		if (component->value == -1)
			colour = drawer.colours[ZOOPTYPE_NONE];
	} else if (component->type >= ZOOPTYPE_LABEL) {
		/* Toggle brightness of these when value is not -1: */
		if (component->value != -1)
			colour ^= BRIGHT_F;
	}

	return colour;
}

void zztoopDrawMusic(ZZTOOPdrawer drawer, ZZTOOPcomponent * component)
{
	int i;
	char * music = component->text;
	const int defaultcolour = drawer.colours[component->type];

	for (i = 0; i < strlen(music); i++) {
		int colour = defaultcolour;

		/* Don't draw past the boundary */
		if (component->pos + i > drawer.length)
			return;

		switch (toupper(music[i])) {
			/* Time determiners */
			case 'T':
			case 'S':
			case 'I':
			case 'Q':
			case 'H':
			case 'W': colour = drawer.musiccolours[ZOOPMUSIC_TIME]; break;
			/* Time modifiers */
			case '3':
			case '.': colour = drawer.musiccolours[ZOOPMUSIC_TIMEMOD]; break;
			/* Octave switch */
			case '+':
			case '-': colour = drawer.musiccolours[ZOOPMUSIC_OCTAVE]; break;
			/* Sharp/flat: pitch modifiers */
			case '!':
			case '#': colour = drawer.musiccolours[ZOOPMUSIC_PITCH]; break;
			/* Rest */
			case 'X': colour = drawer.musiccolours[ZOOPMUSIC_REST]; break;
			default:
				if (toupper(music[i]) >= 'A' && toupper(music[i]) <= 'G')
					colour = drawer.musiccolours[ZOOPMUSIC_NOTE];
				if (music[i] >= '0' && music[i] <= '9')
					colour = drawer.musiccolours[ZOOPMUSIC_DRUM];
		}

		drawer.display->putch_discrete(drawer.x + component->pos + i, drawer.y, music[i], colour);
	}
}


/* Default colour data */

int zztoopdefaultcolours[ZOOPTYPE_MAX + 1] = {
	/* ZOOPTYPE_NONE     */ WHITE_F,
	/* ZOOPTYPE_TEXT     */ GREEN_F  | BRIGHT_F,  /* May require more parsing */
	/* ZOOPTYPE_SYMBOL   */ YELLOW_F | BRIGHT_F,
	/* ZOOPTYPE_COMMENT  */ CYAN_F   | BRIGHT_F,
	/* ZOOPTYPE_COMMAND  */ GREEN_F,
	/* ZOOPTYPE_KEYWORD  */ GREEN_F,
	/* ZOOPTYPE_MUSIC    */ GREEN_F | BRIGHT_F,  /* Requires more parsing */
	/* ZOOPTYPE_OBJNAME  */ BLUE_F  | BRIGHT_F,
	/* ZOOPTYPE_NUMBER   */ WHITE_F,

	/* Toggle brightness of these when value is not -1: */
	/* ZOOPTYPE_LABEL    */ RED_F,
	/* ZOOPTYPE_MESSAGE  */ MAGENTA_F,
	/* ZOOPTYPE_FLAG     */ YELLOW_F,
	/* ZOOPTYPE_FLAGMOD  */ YELLOW_F,

	/* Display these as ZOOPTYPE_NONE when value is -1: */
	/* ZOOPTYPE_ITEM     */ WHITE_F | BRIGHT_F,
	/* ZOOPTYPE_KIND     */ CYAN_F  | BRIGHT_F,
	/* ZOOPTYPE_COLOR    */ CYAN_F  | BRIGHT_F,
	/* ZOOPTYPE_DIR      */ WHITE_F | BRIGHT_F,
	/* ZOOPTYPE_DIRMOD   */ WHITE_F | BRIGHT_F
};

int zztoopdefaulttextcolours[ZOOPTEXT_MAX + 1] = {
	/* ZOOPTEXT_NORMAL    */ GREEN_F | BRIGHT_F,
	/* ZOOPTEXT_HEADING   */ WHITE_F | BRIGHT_F,
	/* ZOOPTEXT_LABEL     */ WHITE_F | BRIGHT_F,
	/* ZOOPTEXT_HYPERTEXT */ WHITE_F | BRIGHT_F
};

int zztoopdefaultmusiccolours[ZOOPMUSIC_MAX + 1] = {
	/* ZOOPMUSIC_NOTE     */ GREEN_F | BRIGHT_F,
	/* ZOOPMUSIC_REST     */ GREEN_F,
	/* ZOOPMUSIC_DRUM     */ MAGENTA_F | BRIGHT_F,
	/* ZOOPMUSIC_TIME     */ CYAN_F,
	/* ZOOPMUSIC_TIMEMOD  */ CYAN_F | BRIGHT_F,
	/* ZOOPMUSIC_OCTAVE   */ YELLOW_F | BRIGHT_F,
	/* ZOOPMUSIC_PITCH    */ BROWN_F
};
