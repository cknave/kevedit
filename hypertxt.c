/* hypertxt.c  -- hypertext link system
 * $Id: hypertxt.c,v 1.4 2001/12/12 22:08:02 bitman Exp $
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "hypertxt.h"
#include "svector.h"

#include <stdlib.h>
#include <string.h>

/* Returns true if sv.cur contains a line in the form of "!message;text"
 * In the case of message == "", returns false.
 * Pre-condition: sv, sv.cur, and sv.cur->s != NULL */
int ishypermessage(stringvector sv)
{
	char* sptr = sv.cur->s;   // Only for looking, not for touching
	if (sptr[0] != '!')        // Must start with '!'
		return 0;

	// Advance token to ';' or end of string
	do {
		sptr++;
	} while (*sptr != '\0' && *sptr != ';');
	if (*sptr == '\0')        // Reach the end w/o finding a ';'
		return 0;

	// When all other possibilities are ruled out...
	return 1;
}

/* Retrieve message from sv->cur if hyperline into malloc()ed string */
char* gethypermessage(stringvector sv)
{
	if (ishypermessage(sv)) {
		int end = 1;   /* Start after the '!' */

		while (sv.cur->s[end] != 0 && sv.cur->s[end] != ';')
			end++;

		/* Copy end chars, starting at position 1 */
		return str_dupmax(sv.cur->s + 1, end - 1);
	} else {
		return str_dup("");
	}
}

/* Locates :msg in sv and points sv->cur there. Returns true on success.
 * Pre-condition: no NULLs */
int findhypermessage(char* msg, stringvector* sv)
{
	stringnode* curNode = sv->first;

	/* Scour sv for any sign of :msg */
	while (curNode != NULL) {
		if (curNode->s[0] == ':') {
			/* Could this be it? */
			if (str_equ(curNode->s + 1, msg, STREQU_UNCASE | STREQU_RFRONT)) {
				if (curNode->s[strlen(msg) + 1] == '\0' ||
						curNode->s[strlen(msg) + 1] == ';') {
					/* This is it! */
					sv->cur = curNode;
					return 1;
				}
			}
		}

		curNode = curNode->next;
	}
	
	/* Found nothing, report falure */
	return 0;
}


/* Determines whether a message refers to a section as well */
int ishypersection(char* msg)
{
	if (msg[0] == '-')
		return 1;
	else
		return 0;
}

/* Retrieve the section component of msg into malloc()ed string */
char* gethypersection(char* msg)
{
	if (ishypersection(msg)) {
		int end = 1;    /* Start after the '-' */

		while (msg[end] != 0 && msg[end] != ':')
			end++;

		/* Copy end chars, starting at position 1 */
		return str_dupmax(msg + 1, end - 1);
	} else {
		return str_dup("");
	}
}

/* Retrieve the msg component of msg into malloc()ed string */
char* gethypersectionmessage(char* msg)
{
	int start = 0;

	while (msg[start] != '\0' && msg[start - 1] != ':')
		start++;

	return str_dup(msg + start);
}
