/* register.c  -- text editor memory registers
 * $Id: register.c,v 1.6 2002/01/12 06:31:58 bitman Exp $
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


#include "register.h"
#include "editbox.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void loadsvector(stringvector * dest, stringnode * startn, stringnode * endn, int startpos, int endpos);
int mergesvector(stringvector * dest, stringvector * src, int inspos, int wrapwidth, int editwidth);

static stringvector reg = { NULL, NULL, NULL };


void regyank(char whichreg, stringnode * startn, stringnode * endn, int startpos, int endpos)
{
	/* NOTE: for the time being, whichreg will be ignored. In the future I hope
	 * to implement a * number of registers, much in the same fashion as vim,
	 * " being the default, * the (X) Windows clipboard, and all other letters
	 * available for general user selection. */

	/* if the register already contains info, erase it. */
	clearregister(whichreg);

	loadsvector(&reg, startn, endn, startpos, endpos);
}

void regstore(char whichreg, stringvector src)
{
	clearregister(whichreg);

	reg = duplicatestringvector(src, 0);
}

int regput(char whichreg, stringvector * dest, int inspos, int wrapwidth, int editwidth)
{
	return mergesvector(dest, &reg, inspos, wrapwidth, editwidth);
}


void clearregister(char whichreg)
{
	deletestringvector(&reg);
	initstringvector(&reg);
}


void deleteregisters(void)
{
	clearregister('\"');
}


void loadsvector(stringvector * dest, stringnode * startn, stringnode * endn, int startpos, int endpos)
{
	stringnode * curnode = NULL;
	char * tempstr = NULL;

	/* We should be able to delete the following commented code */
	/* If starting on 0, start with blank line */

	if (startn == endn) {
		if (startpos < strlen(startn->s) && endpos <= strlen(startn->s)) {
			/* swap startpos & endpos if startpos is greater */
			if (startpos > endpos) { int swapper = startpos; startpos = endpos; endpos = swapper; }

			/* Copy only the section of the node which we need */
			tempstr = (char *) malloc(strlen(startn->s) + 2);
			strcpy(tempstr, endn->s + startpos);
			tempstr[endpos - startpos] = 0;
			pushstring(dest, tempstr);
		}
		return;
	}
	if (startn->next == NULL)
		return;

	/* Copy first line to dest */
	if (startpos < strlen(startn->s))
		pushstring(dest, strcpy((char *) malloc(strlen(startn->s) - startpos + 2), startn->s + startpos));

	/* Copy the meat */
	for (curnode = startn->next; curnode != endn && curnode != NULL; curnode = curnode->next)
		pushstring(dest, strcpy((char *) malloc(strlen(curnode->s) + 2), curnode->s));

	/* See if we failed to reach endn */
	if (curnode == NULL) {
		pushstring(dest, strcpy((char *) malloc(1), ""));
		return;
	}

	/* Copy the last line */
	tempstr = (char *) malloc(strlen(endn->s) + 2);
	strcpy(tempstr, endn->s);
	tempstr[endpos] = 0;
	pushstring(dest, tempstr);
}


/* mergesvector
 *
 * args:    dest:       destination vector
 *          src:        source vector (entirety is dumped into dest)
 *          inspos:     where on dest->cur to insert src
 *          wrapwidth:  position used in wrapping
 *          editwidth:  edit size of new strings
 * return:  new position of inspos. sv->cur is changed to reflect the line on
 *          which pos now resides.
 */
int mergesvector(stringvector * dest, stringvector * src, int inspos, int wrapwidth, int editwidth)
{
	/* The destination must have at least one line! */
	if (dest->first == NULL)
		pushstring(dest, str_dupmin("", editwidth));

	if (src->first == src->last) {
		/* Insert inside current line of dest */
		return wordwrap(dest, src->first->s, inspos, inspos, wrapwidth, editwidth);
	} else {
		/* Chop dest->cur->s in half at inspos. wordwrap the src->first onto
		 * the end of the left half. Insert the remainder of src into dest, then
		 * wordwrap the right half onto the end of that. */
		stringnode* insertionLine = dest->cur;

		/* Make a new line after dest->cur and put the last string in src in */
		insertstring(dest, strcpy((char *) malloc(editwidth + 2), src->last->s));

		/* Wordwrap the last half of the insertion line onto the new line */
		dest->cur = dest->cur->next;
		wordwrap(dest, insertionLine->s + inspos, strlen(dest->cur->s), 0, wrapwidth, editwidth);

		/* Return to insertion line and truncate it */
		dest->cur = insertionLine;
		dest->cur->s[inspos] = 0;

		/* Insert meat of src below the current line in dest, backward to keep
		 * it in order. */
		for (src->cur = src->last->prev; src->cur->prev != NULL; src->cur = src->cur->prev) {
			insertstring(dest, strcpy((char *) malloc(editwidth + 2), src->cur->s));
		}

		/* Finally, wordwrap the first line in src onto the insertion line */
		wordwrap(dest, src->first->s, inspos, 0, wrapwidth, editwidth);

		dest->cur = insertionLine;
		return inspos;
	}
}


