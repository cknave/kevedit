/* zzm.c  -- zzm music format
 * $Id: zzm.c,v 1.2 2005/05/28 03:17:45 bitman Exp $
 * Copyright (C) 2000 Ryan Phillips <bitman@scn.org>
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

#include "zzm.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* Translation table for easy sharpening/flattening.
 * C is at the bottom of an octave, B at the top. */
static short xlat[7]=
{
	9,    /* A */
	11,   /* B */
	0,    /* C */
	2,    /* D */
	4,    /* E */
	5,    /* F */
	7     /* G */
};

musicalNote zzmGetDefaultNote(void)
{
	musicalNote note;

	note.type   = NOTETYPE_NONE;
	note.length = NOTELEN_THIRTYSECOND;
	note.dots   = 0;
	note.index  = 0;
	note.octave = 0;
	note.slur   = 1;

	note.src_pos = 0;
	note.next = NULL;
	return note;
}

musicSettings zzmGetDefaultSettings(void)
{
	musicSettings settings;
	settings.basePitch = ZZM_BASE_PITCH;
	settings.wholeDuration = 1760;
	settings.noteSpacing = 8;

	return settings;
}

musicalNote zzmGetNote(char* tune, musicalNote previousNote)
{
	musicalNote note = previousNote;

	while (tune[note.src_pos] != '\x0') {
		char curCh = tune[note.src_pos++];

		/* Uppercase any characters */
		curCh = toupper(curCh);

		/* If curCh is a duration character, reset the dots counter */
		if (strchr("TSIQHW", curCh))
			note.dots = 0;

		switch (curCh) {
			/* alter the current duration if we encounter one of these */
			case 'T': note.length  = NOTELEN_THIRTYSECOND; break; /* (default duration) */
			case 'S': note.length  = NOTELEN_SIXTEENTH; break;
			case 'I': note.length  = NOTELEN_EIGHTH; break;
			case 'Q': note.length  = NOTELEN_QUARTER; break;
			case 'H': note.length  = NOTELEN_HALF; break;
			case 'W': note.length  = NOTELEN_WHOLE; break;
			case '3': note.length |= NOTELEN_TRIPLET; break;
			case '.': note.dots++; break;   /* Note is dotted */
			/* octave modifiers */
			case '+': if (note.octave < ZZM_MAXOCTAVE) note.octave++; break;
			case '-': if (note.octave > ZZM_MINOCTAVE) note.octave--; break;
		}

		/* If we finally reached a note */
		if ((curCh >= 'A') && (curCh <= 'G')) {
			note.type = NOTETYPE_NOTE;

			/* Get the index for this note */
			note.index = xlat[(curCh) - 'A'];

			/* TODO: I think there is a bug in ZZT such that C# on the
			 * highest octave or C! on the lowest produces a high
			 * pitched squeel. Emulate. */

			/* check for sharpness and flatness */
			if (tune[note.src_pos] == '#') {
				/* Increase the frequency index, moving to the next octave
				 * FOR THIS NOTE ONLY if we go over the top */
				if (++note.index > 11) {
					note.index = 0;

					/* we shouldn't go higher than MAXOCTAVE */
					if ((++note.octave) > ZZM_MAXOCTAVE) note.octave = ZZM_MAXOCTAVE;
				}

				/* advance the position to the next character */
				note.src_pos++;
			}
			else if (tune[note.src_pos] == '!') {
				/* Decrease the frequency index, moving to the previous octave
				 * if we go too low */
				if (--note.index < 0) {
					note.index = 11;

					/* no going beneath MINOCTAVE */
					if ((--note.octave) < ZZM_MINOCTAVE) note.octave = ZZM_MINOCTAVE;
				}

				/* advance the position to the next character */
				note.src_pos++;
			}

			return note;
		}

		/* If we have a rest */
		if (curCh == 'X') {
			note.type = NOTETYPE_REST;
			note.index = 0;
			return note;
		}

		/* In case of percusion */
		if ((curCh >= '0') && (curCh <= '9') && curCh != '3')
		{
			note.type = NOTETYPE_DRUM;
			note.index = curCh - 0x30;
			return note;
		}
	}
	
	/* We reached the end of the string -- no note to generate */
	note.type = NOTETYPE_NONE;
	note.index = 0;
	return note;
}

musicalNote* zzmGetNoteChain(char* tune, musicalNote defNote)
{
	musicalNote* first, * last;

	last = first = (musicalNote*) malloc(sizeof(musicalNote));

	if (first == NULL)
		return NULL;

	/* The first note is actually a copy of the defNote */
	(*first) = defNote;

	do {
		musicalNote *cur = (musicalNote*) malloc(sizeof(musicalNote));

		/* Copy the returned note */
		(*cur) = zzmGetNote(tune, *last);

		last->next = cur;
		last = cur;
	} while (last->type != NOTETYPE_NONE);

	/* End of the line */
	last->next = NULL;

	return first;
}
