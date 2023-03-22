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

#include <math.h>

#include "pcspeaker.h"

void pcSpeakerPlayNote(displaymethod *mydisplay, musicalNote note, musicSettings settings)
{
	float frequency = noteFrequency(note, settings);
	int wait = (int)noteDuration(note, settings);
	int spacing = noteSpacing(note, settings);
	int drumbreaktime = wait - DRUMBREAK * DRUMCYCLES;

	if (note.type == NOTETYPE_NOTE) {
		/* Play the sound at the frequency for a duration */
		mydisplay->audio_square(frequency, wait);
	}

	/* Rests are simple */
	if (note.type == NOTETYPE_REST) {
		mydisplay->audio_silence(wait);
	}

	/* Drums */
	if (note.type == NOTETYPE_DRUM) {
		int i;

		/* Loop through each drum cycle */
		for (i = 0; i < DRUMCYCLES; i++) {
                        mydisplay->audio_square(drums[note.index][i], DRUMBREAK);
		}

		/* Add a break based on the current duration */
                mydisplay->audio_silence(drumbreaktime);
	}

	if (spacing != 0.0) {
                mydisplay->audio_silence(spacing);
	}
}
