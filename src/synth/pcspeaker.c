/* pcspeaker.c	-- PC speaker music synthesizer
 * $Id: pcspeaker.c,v 1.3 2005/06/29 03:20:34 kvance Exp $
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

#include "pcspeaker.h"

#include <dos.h>

void pcSpeakerPlayNote(musicalNote note, musicSettings settings)
{
	int frequency = noteFrequency(note, settings);
	int wait = noteDuration(note, settings);
	int spacing = noteSpacing(note, settings);

	if (note.type == NOTETYPE_NOTE) {
		/* Play the sound at the frequency for a duration */
		sound(frequency);
		delay(wait);
	}

	/* Rests are simple */
	if (note.type == NOTETYPE_REST) {
		nosound();
		delay(wait);
	}

	/* Drums */
	if (note.type == NOTETYPE_DRUM) {
		int i;

		/* Loop through each drum cycle */
		for (i = 0; i < DRUMCYCLES; i++) {
			sound(drums[note.index][i]);
			delay(DRUMBREAK);
		}
		nosound();

		/* Add a break based on the current duration */
		delay(wait - DRUMBREAK * DRUMCYCLES);
	}

	if (spacing != 0.0) {
		nosound();
		delay(spacing);
	}
}

void pcSpeakerFinish(void)
{
	nosound();
}
