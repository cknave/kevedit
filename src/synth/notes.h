/* notes.h	-- Generate musical notes in chromatic scale
 * $Id: notes.h,v 1.2 2005/06/29 03:20:34 kvance Exp $
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

#ifndef _NOTES_H
#define _NOTES_H 1

/* Note types */
#define NOTETYPE_NONE 0   /* Usually end-of-input */
#define NOTETYPE_NOTE 1   /* Just a note */
#define NOTETYPE_REST 2   /* Rest for a moment */
#define NOTETYPE_DRUM 3   /* Tick-tock */

/* Note lengths */
#define NOTELEN_WHOLE        0x01
#define NOTELEN_HALF         0x02
#define NOTELEN_QUARTER      0x04
#define NOTELEN_EIGHTH       0x08
#define NOTELEN_SIXTEENTH    0x10
#define NOTELEN_THIRTYSECOND 0x20
#define NOTELEN_TRIPLET      0x100  /* Flag: divide duration by 3 */

/* Note indexes */
#define NOTE_C   0
#define NOTE_Cs  1
#define NOTE_D   2
#define NOTE_Ds  3
#define NOTE_E   4
#define NOTE_F   5
#define NOTE_Fs  6
#define NOTE_G   7
#define NOTE_Gs  8
#define NOTE_A   9
#define NOTE_As 10
#define NOTE_B  11

/* Pitch of A in octave 0 (where middle C is) */
#define BASE_PITCH  440

/* Drum information */
#define DRUMBREAK  2  /* 2 millisecond delay between drum changes */ 
#define DRUMCOUNT  10 /* 10 drums in all */
#define DRUMCYCLES 10 /* 10 cycles per drum */
extern short drums[DRUMCOUNT][DRUMCYCLES];

typedef struct musicalNote {
	int type;       /* Type of note */
	int length;     /* Note length */
	int dots;       /* Number of times a note length is dotted */
	int index;      /* Frequency index or drum index */
	int octave;     /* Octave of note (0 is middle octave) */
	int slur;       /* TRUE if note is to slur/tie with the next */

	int src_pos;    /* Position of next note in source string */
	struct musicalNote* next;  /* Notes can be chained together */
} musicalNote;

typedef struct musicSettings {
	/* Frequency of A in octave 0 (where middle C is) */
	float basePitch;

	/* Duration of a whole note, in milliseconds */
	float wholeDuration;

	/* Spacing between notes, in milliseconds */
	float noteSpacing;
} musicSettings;

/* Delete a chain of notes */
void deleteNoteChain(musicalNote* chain);

/* Return the frequency of a given musical note. */
float noteFrequency(musicalNote mnote, musicSettings settings);

/* Return the duration of a note. */
float noteDuration(musicalNote note, musicSettings settings);

/* Return the spacing expected after a note (when not sluring) */
float noteSpacing(musicalNote note, musicSettings settings);

#endif	/* _NOTES_H */
