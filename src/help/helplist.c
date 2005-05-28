/* helplist.c  -- very simple linked list of help sections
 * $Id: helplist.c,v 1.2 2005/05/28 03:17:45 bitman Exp $
 * Copyright (C) 2001 Ryan Phillips <bitman@users.sourceforge.net>
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

#include "helplist.h"

#include "structures/svector.h"

#include <stdlib.h>
#include <string.h>

/* inithelpsection() - returns an empty help section */
void inithelpsection(helpsection* section)
{
	section->title = NULL;
	initstringvector(&section->sv);
	section->next = NULL;
}

/* deletesectionlist() - free()s everything but the section itself */
void deletesectionlist(helpsection* section)
{
	if (section == NULL)
		return;

	/* Destroy next and soforth */
	if (section->next != NULL) {
		deletesectionlist(section->next);
		free(section->next);
		section->next = NULL;
	}

	/* Destroy title */
	if (section->title != NULL) {
		free(section->title);
		section->title = NULL;
	}

	/* Destroy sv */
	deletestringvector(&section->sv);
}

/* appendsection() - appends src to the end of dest */
void appendsection(helpsection* dest, helpsection* src)
{
	if (dest == NULL || src == NULL)
		return;

	/* Advance to last node */
	while (dest->next != NULL)
		dest = dest->next;

	dest->next = src;
}

/* findsection() - locates a section with given title in a section list */
helpsection* findsection(helpsection* sectionlist, char* title)
{
	/* Keep looping until we hit either the end of the list or the title */
	while (sectionlist != NULL &&
				 (sectionlist->title == NULL ||
				  !str_equ(sectionlist->title, title, STREQU_UNCASE)))
		sectionlist = sectionlist->next;

	/* Return whatever we ended up with */
	return sectionlist;
}

/* loadhelpmetafile() - loads a help meta into the given help section list.
 *                      the original meta is destroyed in the process. true on
 *                      error */
int loadhelpmeta(helpsection* section, stringvector* meta)
{
	helpsection* newsection;

	meta->cur = meta->first;

	newsection = (helpsection*) malloc(sizeof(helpsection));
	inithelpsection(newsection);
	newsection->title = str_dup("index");

	do {
		/* Steal strings from meta until we reach an '@@' or the end of meta */
		while (!(meta->first == NULL ||
					  (meta->first->s[0] == '@' && meta->first->s[1] == '@'))) {
			char* transfer = removestring(meta);
			pushstring(&newsection->sv, transfer);
		}

		if (newsection->sv.first != NULL) {
			/* If newsection actually recieved some content, add it to the list */
			appendsection(section, newsection);
			/* Advance section to newsection to speed up future appends */
			section = newsection;
		} else {
			/* No content, erase newsection */
			deletesectionlist(newsection);
			free(newsection);
			newsection = NULL;
		}

		/* Prepare another newsection if we've not yet reached the end */
		if (meta->first != NULL) {
			int i;
			newsection = (helpsection*) malloc(sizeof(helpsection));
			inithelpsection(newsection);
			/* meta->first->s starts with @@, so pull out the title */
			newsection->title = str_dup(meta->first->s + 2);
			deletestring(meta);

			/* Remove any extension from title */
			i = strlen(newsection->title) - 1;
			while (i > 0 && newsection->title[i] != '.')
				i--;
			if (newsection->title[i] == '.')
				newsection->title[i] = '\0';
		}
	} while (meta->first != NULL);

	return 0;
}

