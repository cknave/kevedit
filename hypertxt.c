/* hypertxt.c  -- hypertext link system
 * $Id: hypertxt.c,v 1.1 2001/10/09 01:14:36 bitman Exp $
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

/* Returns true if sv->cur contains a line in the form of "!message;text"
 * In the case of message == "", returns false.
 * Pre-condition: sv, sv->cur, and sv->cur->s != NULL */
int ishypermessage(stringvector* sv)
{
	char* sptr = sv->cur->s;   // Only for looking, not for touching
	if (sptr[0] != '!')        // Must start with '!'
		return 0;
	if (sptr[1] == ';')
		return 0;                // Cannot accept empty message field

	// Advance token to ';' or end of string
	do {
		sptr++;
	} while (*sptr != '\0' && *sptr != ';');
	if (*sptr == '\0')        // Reach the end w/o finding a ';'
		return 0;

	// When all other possibilities are ruled out...
	return 1;
}

/* Returns buffer, filled with value of message on current line of sv, up
 * to buflen. If no message can be found, returns buffer truncated. */
char* gethypermessage(char* buffer, stringvector* sv, int buflen)
{
	int i = 0;
	if (ishypermessage(sv)) {
		char* s = sv->cur->s + 1;   // Go right to the head of the message

		for (i = 0; (i < buflen - 1) && (s[i] != ';') ; i++) {
			buffer[i] = s[i];
		}
	}
	buffer[i] = '\0';            // Cap the buffer, whether or not we had a msg

	return buffer;
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
			if (str_equ(curNode->s + 1, msg, STREQU_UNCASE | STREQU_FRONT)) {
				/* This is it! */
				sv->cur = curNode;
				return 1;
			}
		}

		curNode = curNode->next;
	}
	
	/* Found nothing, report falure */
	return 0;
}
