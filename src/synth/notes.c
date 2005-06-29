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

short drums[DRUMCOUNT][DRUMCYCLES] = {
		{   0,   0, 175, 175, 100,  90,  80,  70,  60,  50},  /* 0 */
		{ 500, 300, 520, 320, 540, 340, 550, 350, 540, 340},  /* 1 */
		{1000,1200,1250,1400,1100,1150,1300,1000,1200, 500},  /* 2 */
		{   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  /* 3 (not a sound) */
		{ 950,1950, 750,1750, 550,1550, 350,1350, 150,1150},  /* 4 */
		{ 200, 210, 220, 230, 240, 250, 260, 270, 280, 600},  /* 5 */
		{ 900, 800, 700, 600, 500, 400, 300, 200, 100,   0},  /* 6 */
		{ 300, 200, 290, 190, 280, 180, 270, 170, 260, 160},  /* 7 */
		{ 400, 380, 360, 340, 320, 300, 280, 260, 250, 240},  /* 8 */
		{ 150, 100, 140,  90, 130,  80, 120,  70, 110,  60}   /* 9 */
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

/* Frequency notation (makes it easier to compute frequency):
	C   -3
	C#  -4
	D   -5
	D#  -6
	E   -7
	F   -8
	F#  -9
	G  -10
	G# -11
	A    0
	A#   1
	B    2
*/

/* Translate a note index to frequency notation (above) */
int frequencyNotation(int index)
{
	/* If the note is A or above */
	if (index >= 9)
		return index - 9;
	else
		return -(index + 3);
}

float noteFrequency(musicalNote mnote, musicSettings settings)
{
	double freq;
	float fraction;
	float basePitch = settings.basePitch;

	int note = frequencyNotation(mnote.index);
	int octave = mnote.octave;

	if (mnote.type != NOTETYPE_NOTE)
		return 0.0;

	/* Multiply or divide the base frequency to get to the correct octave
	   relative to A */
	if(octave > 0)
		freq = basePitch*pow(2, octave);
	else if (octave < 0)
		freq = basePitch/pow(2, octave*(-1));
	else
		freq = basePitch;

	if(note < 0)
		freq /= 2.0;

	/* Find the size of a half step */
	fraction = (log(freq * 2.0) - log(freq)) / 12.0;

	/* Move base freq to log */
	freq = log(freq);

	/* Add half-steps to reach the desired note)
	   desired note */
	if(note < 0)
		freq += (note*(-1))*fraction;
	else if(note > 0)
		freq += note*fraction;

	/* Get out of log */
	freq = exp(freq);

	/* Return the frequency */
	return freq;
}

float noteDuration(musicalNote note, musicSettings settings)
{
	float wholeDuration = settings.wholeDuration;
	float duration;

	/* Omit the triplet flag from the note length */
	int length = note.length & ~NOTELEN_TRIPLET;

	if (length <= 0) return 0;
 
	/* Divide the whole duration by the length */
	duration = wholeDuration / (float)length;

	/* Adjust duration for dotted notes */
	duration = duration * (2 - (1 / (float)(1 << note.dots)));

	/* Note is a triplet: divide by 3 */
	if (note.length & NOTELEN_TRIPLET)
		duration /= 3;

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
