/* register.c  -- text editor registers
 * $Id: register.c,v 1.1 2001/01/07 23:58:21 bitman Exp $
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

void loadsvector(stringvector * dest, stringnode * startn, stringnode * endn, int startpos, int endpos);
void mergesvector(stringvector * dest, stringvector * src, int inspos, int wrapwidth, int editwidth);

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


void regput(char whichreg, stringvector * dest, int inspos, int wrapwidth, int editwidth)
{
	mergesvector(dest, &reg, inspos, wrapwidth, editwidth);
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

	/* If starting on 0, start with blank line */
	if (startpos == 0)
		pushstring(dest, strcpy((char *) malloc(1), ""));

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


void mergesvector(stringvector * dest, stringvector * src, int inspos, int wrapwidth, int editwidth)
{
	if (src->first == src->last) {
		/* Insert inside current line of dest */
		wordwrap(dest, src->first->s, inspos, 0, wrapwidth, editwidth);
	} else {
		/* Insert src into new line above dest->cur */
		for (src->cur = src->first; src->cur != NULL; src->cur = src->cur->next) {
			preinsertstring(dest, strcpy((char *) malloc(editwidth + 2), src->cur->s));
		}
	}
}

/* testing */
#include <conio.h>
#include <stdlib.h>


void printsv(stringvector sv) {
	printf("Contents of sv:\n");
	for (sv.cur = sv.first; sv.cur != NULL; sv.cur = sv.cur->next)
		printf("%s\n", sv.cur->s);
	printf("\n");
}

void regtest(void) {
	stringvector sv;

	initstringvector(&sv);
	pushstring(&sv, strcpy((char *) malloc(50), "Line number 1"));
	pushstring(&sv, strcpy((char *) malloc(50), "Second line"));
	pushstring(&sv, strcpy((char *) malloc(50), "Line of three"));

	printsv(sv);

	regyank('\"', sv.first, sv.first, 5, 11);

	printsv(reg);
	
	sv.cur = sv.first;
	regput('\"', &sv, 0, 42, 50);

	printsv(sv);

	removestringvector(&sv);
	getch();
	exit(0);
}
/* end testing */

