/* zzm.c  -- zzm file routines
 * $Id: zzm.c,v 1.2 2005/05/28 03:17:46 bitman Exp $
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
#include "kevedit/kevedit.h"
#include "editbox.h"
#include "structures/svector.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>


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

stringvector zzmripsong(stringvector * zoc, int maxseparation)
{
	stringvector songlines;
	stringnode * curline = zoc->cur;
	int separation = 0;

	initstringvector(&songlines);

	while (curline != NULL && separation <= maxseparation) {
		char* tune = strstr(curline->s, "#");
		if (tune != NULL && str_equ(tune, "#play ", STREQU_UNCASE | STREQU_RFRONT)) {
			separation = 0;  /* Reset separation counter */
			tune += 6;       /* Advance to notes! */

			/* Push the remainder of the line into songlines */
			pushstring(&songlines, str_dup(tune));
		} else {
			separation++;
		}

		curline = curline->next;
	}

	return songlines;
}
