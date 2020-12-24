/* notes.c	-- Generate musical notes in chromatic scale
 * $Id: notes.c,v 1.4 2005/06/29 03:20:34 kvance Exp $
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
#include <stdlib.h>
#include <string.h>

#include "notes.h"

/* PIT frequency (used for rounding) */
#define PIT_FREQUENCY 1193182.0

short drums[DRUMCOUNT][DRUMCYCLES] = {
	{3200,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0},
    {1100,     1200,   1300,   1400,   1500,   1600,   1700,   1800,   1900,   2000,   2100,   2200,   2300,   2400},
    {4800,     4800,   8000,   1600,   4800,   4800,   8000,   1600,   4800,   4800,   8000,   1600,   4800,   4800},
    {0,        0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0},
    {500,      2556,   1929,   3776,   3386,   4517,   1385,   1103,   4895,   3396,   874,    1616,   5124,   606},
    {1600,     1514,   1600,   821,    1600,   1715,   1600,   911,    1600,   1968,   1600,   1490,   1600,   1722},
    {2200,     1760,   1760,   1320,   2640,   880,    2200,   1760,   1760,   1320,   2640,   880,    2200,   1760},
    {688,      676,    664,    652,    640,    628,    616,    604,    592,    580,    568,    556,    544,    5320},
    {1207,     1224,   1163,   1127,   1159,   1236,   1269,   1314,   1127,   1224,   1320,   1332,   1257,   1327},
    {378,      331,    316,    230,    224,    384,    480,    320,    358,    412,    376,    621,    554,    426}
};

/* Delete a chain of notes */
void deleteNoteChain(musicalNote* chain)
{
	/* Free each note in the chain recursively */
	if (chain == NULL)
		return;

	deleteNoteChain(chain->next);
	chain->next = NULL;  /* Why not? */
	free(chain);
}

float noteFilter(float freq, musicSettings settings) {
	if (freq <= 1.0) {
		return freq;
	}
	if (settings.pitRounding) {
		freq = (PIT_FREQUENCY / ((int)PIT_FREQUENCY / (int)(freq)));
	}
	return freq;
}

float noteFrequency(musicalNote mnote, musicSettings settings)
{
	double freq;
	float fraction;
	int i;

	if (mnote.type != NOTETYPE_NOTE)
		return 0.0;

	/* Note frequency calculation - matching ZZT */
	freq = 32 * (1 << (mnote.octave + 3)) * exp(log(2.0) * mnote.index / 12.0);

	/* Return the frequency */
	return freq;
}

float noteDuration(musicalNote note, musicSettings settings)
{
	float wholeDuration = settings.wholeDuration;
	float duration;
	int i;

	/* Transfer length to duration */
	if (note.length <= 0) return 0;
	duration = (wholeDuration / NOTELEN_WHOLE) * (note.length == 0 ? 256 : note.length);

	/* Leave room for note spacing (except rests) */
	if (!note.slur && note.type != NOTETYPE_REST)
		duration -= settings.noteSpacing;

	/* duration must be at least zero */
	if (duration < 0) duration = 0;

	return duration;
}

float noteSpacing(musicalNote note, musicSettings settings)
{
	/* No break when slurring or for rests */
	if (note.slur || note.type == NOTETYPE_REST)
		return 0.0;

	/* Otherwise return the noteSpacing setting. */
	return settings.noteSpacing;
}
