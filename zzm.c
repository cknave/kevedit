/* zzm.c  -- zzm file routines
 * $Id: zzm.c,v 1.1 2000/09/09 02:33:51 bitman Exp $
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


#include "kevedit.h"
#include "editbox.h"
#include "zzm.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


stringvector zzmpullsong(stringvector * zzmv, int songnum)
{
	stringvector songlines;		/* List of zzm format music */
	stringnode * curline;		/* Current node */

	initstringvector(&songlines);

	if (songnum < 1)
		return songlines;

	for (curline = zzmv->first; curline != NULL; curline = curline->next) {
		if (curline->s != NULL &&
		    ((unsigned char *) strstr(curline->s, "; $SONG ") == curline->s)) {
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
	stringvector songtitles;		/* List of song names in zzmv */
	stringvector rawtitles;		/* vector sharing strings with zzmv */
	stringnode * curline;		/* Current node */
	unsigned char* newstr;	/* New strings for songtitles */
	int i = 0, j = 0, k = 0;

	int* songnumlist = NULL;
	int songcount = 0;

	if (zzmv == NULL || zzmv->first == NULL)
		return -1;

	initstringvector(&songtitles);
	initstringvector(&rawtitles);


	pushstring(&songtitles, strcpy((char *) malloc(42), "@here"));
	/* put header info at top of songtitles */
	for (curline = zzmv->first; curline != NULL; curline = curline->next) {
		if (curline->s != NULL &&
		    ((unsigned char *) strstr(curline->s, "; $TITLE ") == curline->s)) {
			newstr = (unsigned char*) malloc(43);
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
		    ((unsigned char *) strstr(curline->s, "; $SONG TITLE ") == curline->s)) {

			/* put the song number and title in rawtitles */
			pushstring(&rawtitles, curline->s + 14);
		}
	}

	songcount = 0;
	for (curline = rawtitles.first; curline != NULL; curline = curline->next)
		songcount++;

	songnumlist = (int *) malloc(songcount*sizeof(int));

	i = 0;
	for (curline = rawtitles.first; curline != NULL; curline = curline->next)
		sscanf(curline->s, "%d", &(songnumlist[i++]));

	/* go through each song number, find it, and put it in songtitles */
	for (i = 1; i <= songcount; i++) {
		for (j = 0; j < songcount; j++) {
			if (songnumlist[j] == i) {
				curline = rawtitles.first;
				for (k = 0; k < j; k++)
					curline = curline->next;
				
				/* the title from curline can now be pushed onto songtitles */
				newstr = (unsigned char*) malloc(strlen(curline->s) + 3);
				strcpy(newstr, "!");
				strcat(newstr, curline->s);
				if (strstr(newstr, " ") != NULL)
					*strstr(newstr, " ") = ';';
				for (k = 0; k < strlen(newstr); k++)
					if (newstr[k] == '~') newstr[k] = ' ';

				pushstring(&songtitles, newstr);
			}
		}
	}
	removestringvector(&rawtitles);
	free(songnumlist);

	/* songtitles now contains all the info we need to offer a dialog */
	songtitles.cur = songtitles.first;
	do {
		if (editbox(d, "Pick a ZZM Song", &songtitles, 0, 1) == 27) {
			deletestringvector(&songtitles);
			return -1;
		}
	} while (songtitles.cur->s[0] != '!');

	/* determine the number chosen */
	sscanf(songtitles.cur->s, "%*c%d", &i);
	/*
	i = 1;
	for (curline = songtitles.first; curline != songtitles.cur && curline != NULL; curline = curline->next)
		i++;
		*/

	deletestringvector(&songtitles);

	return i;
}
