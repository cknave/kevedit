/* zzm.c  -- zzm file routines
 * $Id: zzm.c,v 1.6 2001/12/15 00:54:53 bitman Exp $
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "zzm.h"
#include "kevedit.h"
#include "editbox.h"
#include "svector.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef DOS
#include <dos.h>
#endif

/* Note lengths, given in millisecond delay */
#define NOTE_QUARTER      440
#define NOTE_HALF         (NOTE_QUARTER * 2)
#define NOTE_WHOLE        (NOTE_QUARTER * 4)
#define NOTE_EIGHTH       (NOTE_QUARTER / 2)
#define NOTE_SIXTEENTH    (NOTE_QUARTER / 4)
#define NOTE_THIRTYSECOND (NOTE_QUARTER / 8)

#define NOTE_DRUMBREAK   2 /* 2 millisecond delay between drum changes */ 
#define NOTE_NOSLURBREAK 8 /* Break of 8 if notes are not to be slurred */

#define MAXOCTAVE 6
#define MINOCTAVE 1

/* Scale, in octave 6 */
static short scale[12]=
{
	2032, /* 0=C   */
	2152, /* 1=C#  */
	2280, /* 2=D   */
	2416, /* 3=D#  */
	2560, /* 4=E   */
	2712, /* 5=F   */
	2880, /* 6=F#  */
	3048, /* 7=G   */
	3232, /* 8=G#  */
	3424, /* 9=A   */
	3624, /* 10=A# */
	3840  /* 11=B  */
};

/* Translation table for scale[] */
static short xlat[7]=
{
	9,    /* indexes A in Scale[] */
	11,   /* B */
	0,    /* C */
	2,    /* D */
	4,    /* E */
	5,    /* F */
	7     /* G */
};

/* Drum data */
#define DRUMCYCLES 10
static short drums[10][DRUMCYCLES]= {
		{   0,   0, 175, 175, 100,  90,  80,  70,  60,  50},  /* 0 */
		{ 500, 300, 520, 320, 540, 340, 550, 350, 540, 340},  /* 1 */
		{1000,1200,1250,1400,1100,1150,1300,1000,1200, 500},  /* 2 */
		{   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  /* 3 (not a sound) */
		{ 950,1950, 750,1750, 550,1550, 350,1350, 150,1150},  /* 4 */
		{ 200, 210, 220, 230, 240, 250, 260, 270, 280, 600},  /* 5 */
		{ 900, 800, 700, 600, 500, 400, 300, 200, 100,   0},  /* 6 */
		{ 300, 200, 290, 190, 280, 180, 270, 170, 260, 160},  /* 7 */
		{ 400, 380, 360, 340, 320, 300, 280, 260, 250, 240},  /* 8 */
		{ 150, 100, 140,  90, 130,  80, 120,  70, 110,  60}}; /* 9 */


stringvector zzmpullsong(stringvector * zzmv, int songnum)
{
	stringvector songlines;		/* List of zzm format music */
	stringnode * curline;		/* Current node */

	initstringvector(&songlines);

	if (songnum < 1)
		return songlines;

	for (curline = zzmv->first; curline != NULL; curline = curline->next) {
		if (curline->s != NULL &&
		    str_equ(curline->s, "; $SONG ", STREQU_UNCASE | STREQU_FRONT)) {
			int cursongnum = 0;
			sscanf(curline->s + 8, "%d", &cursongnum);
			if (cursongnum == songnum)
				for (curline = curline->next; curline != NULL && curline->s[0] != ';'; curline = curline->next)
					pushstring(&songlines, strcpy((char *) malloc(strlen(curline->s)+2), curline->s));
		}
	}
	
	return songlines;
}


int zzmpicksong(stringvector * zzmv, displaymethod * d)
{
	stringvector songtitles;	/* List of song names in zzmv */
	stringvector rawtitles;		/* vector sharing strings with zzmv */
	stringnode * curline;		/* Current node */
	char *newstr;			/* New strings for songtitles */
	int i = 0, k = 0;

	if (zzmv == NULL || zzmv->first == NULL)
		return -1;

	initstringvector(&songtitles);
	initstringvector(&rawtitles);


	pushstring(&songtitles, strcpy((char *) malloc(42), "@here"));
	/* put header info at top of songtitles */
	for (curline = zzmv->first; curline != NULL; curline = curline->next) {
		if (curline->s != NULL &&
		    str_equ(curline->s, "; $TITLE ", STREQU_UNCASE | STREQU_FRONT)) {
			newstr = (char *) malloc(43);
			strcpy(newstr, "$");
			strncat(newstr, curline->s + 9, 41);
			for (i = 0; i < strlen(newstr); i++)
				if (newstr[i] == '~')
					newstr[i] = ' ';

			pushstring(&songtitles, newstr);
			pushstring(&songtitles, strcpy((char *) malloc(2), ""));
		}
	}

	/* find all the song titles and put them songnames */
	for (curline = zzmv->first; curline != NULL; curline = curline->next) {
		if (curline->s != NULL &&
		    str_equ(curline->s, "; $SONG TITLE ", STREQU_UNCASE | STREQU_FRONT)) {

			/* put the song number and title in rawtitles */
			pushstring(&rawtitles, curline->s + 14);
		}
	}

	if (rawtitles.first == NULL)
		return -1;

	/* Sort rawtitles */
	/* Use insertion sort because the titles are most likely in order anyway. */
	if (rawtitles.first != NULL && rawtitles.first->next != NULL) {
		char *curstr = NULL;
		stringnode *nodepos = NULL;
		int valcur, valprev;

		for (rawtitles.cur = rawtitles.first->next; rawtitles.cur != NULL; rawtitles.cur = rawtitles.cur->next) {
			curstr = rawtitles.cur->s;
			sscanf(rawtitles.cur->s, "%d", &valcur);
			
			/* Find place to insert curstr */
			nodepos = rawtitles.cur;
			while (nodepos != rawtitles.first && (sscanf(nodepos->prev->s, "%d", &valprev), valprev) > valcur) {
				nodepos->s = nodepos->prev->s;
				nodepos = nodepos->prev;
			}
			nodepos->s = curstr;
		}
	}

	/* Copy titles into songtitles for browsing */

	for (rawtitles.cur = rawtitles.first; rawtitles.cur != NULL; rawtitles.cur = rawtitles.cur->next) {
		/* the title from rawtitles.cur can now be pushed onto songtitles */
		newstr = (char *) malloc(strlen(rawtitles.cur->s) + 3);
		strcpy(newstr, "!");
		strcat(newstr, rawtitles.cur->s);
		if (strstr(newstr, " ") != NULL)
			*strstr(newstr, " ") = ';';

		/* remove ~s from older files */
		for (k = 0; k < strlen(newstr); k++)
			if (newstr[k] == '~') newstr[k] = ' ';

		/* Cut the string off if wider than dialog */
		if (strlen(newstr) > 42)
			newstr[42] = 0;

		pushstring(&songtitles, newstr);
	}

	/* songtitles now contains all the info we need to offer a dialog */
	songtitles.cur = songtitles.first;
	do {
		if (editbox("Pick a ZZM Song", &songtitles, 0, 1, d) == 27) {
			deletestringvector(&songtitles);
			return -1;
		}
	} while (songtitles.cur->s[0] != '!');

	/* determine the number chosen */
	sscanf(songtitles.cur->s, "%*c%d", &i);

	deletestringvector(&songtitles);

	return i;
}

/* resetzzmplaystate() - clears state to default settings */
void resetzzmplaystate(zzmplaystate * s)
{
	s->octave = 3;                    /* Start in middle octave */
	s->duration = NOTE_THIRTYSECOND;  /* Default note length */
	s->pos = 0;
	s->slur = 1;                      /* ZZT slurs, so should we. */
}

/* zzmplaynote() - plays a single note from a tune */
zzmnote zzmgetnote(char * tune, zzmplaystate * s)
{
	while (tune[s->pos] != '\x0') {
		char curCh = tune[s->pos++];

		/* Uppercase any characters */
		curCh = toupper(curCh);

		switch (curCh) {
			/* alter the current duration if we encounter one of these */
			case 'T': s->duration = NOTE_THIRTYSECOND; break; /* (default duration) */
			case 'S': s->duration = NOTE_SIXTEENTH; break;
			case 'I': s->duration = NOTE_EIGHTH; break;
			case 'Q': s->duration = NOTE_QUARTER; break;
			case 'H': s->duration = NOTE_HALF; break;
			case 'W': s->duration = NOTE_WHOLE; break;
			case '3': s->duration /= 3; break; /* triplets, divide current by 3. */
			case '.': s->duration += (s->duration >> 1); break;
								/* increase current duration by half */
			/* octave modifiers */
			case '+': if (s->octave < MAXOCTAVE) s->octave++; break;
			case '-': if (s->octave > MINOCTAVE) s->octave--; break;
		}

		/* If we finally reached a note */
		if ((curCh >= 'A') && (curCh <= 'G')) {
			zzmnote note = { ZZM_NOTE, s->duration, 0, s->octave, s->slur };

			/* get the denomination for this note */
			note.index = xlat[(curCh) - 'A'];

			/* TODO: I think there is a bug in ZZT such that C# on the
			 * highest octave or C! on the lowest produces a high
			 * pitched squeel. Emulate. */

			/* check for sharpness and flatness */
			if (tune[s->pos] == '#') {
				/* Increase the frequency index, moving to the next octave
				 * FOR THIS NOTE ONLY if we go over the top */
				if (++note.index > 12) {
					note.index = 0;

					/* we shouldn't go higher than MAXOCTAVE */
					if ((++note.octave) > MAXOCTAVE) note.octave = MAXOCTAVE;
				}

				/* advance the position to the next character */
				s->pos++;
			}
			else if (tune[s->pos] == '!') {
				/* Decrease the frequency index, moving to the previous octave
				 * if we go too low */
				if (--note.index < 0) {
					note.index = 12;

					/* no going beneath MINOCTAVE */
					if ((--note.octave) < MINOCTAVE) note.octave = MINOCTAVE;
				}

				/* advance the position to the next character */
				s->pos++;
			}

			return note;
		}

		/* If we have a rest */
		if (curCh == 'X') {
			zzmnote note = { ZZM_REST, s->duration, 0, 0, s->slur };
			return note;
		}

		/* In case of percusion */
		if ((curCh >= '0') && (curCh <= '9') && curCh != '3')
		{
			zzmnote note = { ZZM_DRUM, s->duration, curCh - 0x30, 0, s->slur };
			return note;
		}
	}
	
	/* We reached the end of the string -- no note to generate */
	{
		zzmnote note = { ZZM_ERROR, 0, 0, 0 };
		return note;
	}
}

void zzmPCspeakerPlaynote(zzmnote note)
{
#ifdef DOS
	if (note.type == ZZM_NOTE) {
		int frequency = scale[note.index] >> (MAXOCTAVE - note.octave);

		/* Play the sound at the frequency for a duration */
		sound(frequency);
		delay(note.duration);

		/* If we aren't slurring, end the sound and insert a break */
		if (!note.slur) {
			nosound();
			delay(NOTE_NOSLURBREAK);
		}
	}

	/* Rests are simple */
	if (note.type == ZZM_REST) {
		nosound();
		delay(note.duration);
	}

	/* Drums */
	if (note.type == ZZM_DRUM) {
		int i;

		/* Loop through each drum cycle */
		for (i = 0; i < DRUMCYCLES; i++) {
			sound(drums[note.index][i]);
			delay(NOTE_DRUMBREAK);
		}
		nosound();

		/* Add a break based on the current duration */
		delay(note.duration - NOTE_DRUMBREAK * DRUMCYCLES);

		/* Insert a delay when not sluring. No sense in leaving the sound on
		 * for such a brief noise, though. */
		if (!note.slur)
			delay(NOTE_NOSLURBREAK);
	}

#endif
}

void zzmPCspeakerFinish(void)
{
#ifdef DOS
	nosound();
#endif
}


#if 0
void playline(char* tune)
{
	zzmplaystate s;

	resetzzmplaystate(&s);

	while (s.pos < strlen(tune)) {
		int oldpos = s.pos;
		char* strpart;

		zzmnote note = zzmgetnote(tune, &s);

		strpart = str_duplen(tune + oldpos, s.pos - oldpos);
		printf(strpart);
		free(strpart);

		zzmPCspeakerPlaynote(note);
	}
	printf("\n");
	zzmPCspeakerFinish();
}

int main() {
	playline("s-cg+cegec-gcg+cegec-g");
	playline("s-cg+cegec-gcg+cegec-g");
	playline("s-cg+cegec-gcg+cegec-g");
	playline("s-cg+cegec-gc+c-c+c--b+b-b+b");
	playline("s--a+ea+cec-ae-eb+egbgeg");
	playline("s--f+cfacafacg+cgegc-g");
	playline("s--f+cfacafacg+cgegc-g");
	playline("s--f+f-f+f-g+g-g+g--c+ct00sct99sc6c");

	playline("hx");

	playline("i-cx9xex9xfx9g-ggx0");
	playline("i-cx9xex9xfx9g-sgxigab");
	playline("");

	return 0;
}
#endif
