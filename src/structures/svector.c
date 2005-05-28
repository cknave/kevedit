/* svector.c   -- string vectors
 * Copyright (C) 2000 Ryan Phillips <bitman@scn.org>
 * $Id: svector.c,v 1.3 2005/05/28 03:17:45 bitman Exp $
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

#include "svector.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>


void initstringvector(stringvector * v)
{
	v->first = v->last = v->cur = NULL;
}


/* pushstring - pushes string s to end of vector v */
int pushstring(stringvector * v, char *s)
{
	stringnode *newnode = NULL;

	if (v == NULL || s == NULL)
		return 1;

	if (v->first == NULL) {
		newnode = (stringnode *) malloc(sizeof(stringnode));
		newnode->next = NULL;
		newnode->prev = NULL;
		newnode->s = s;

		v->cur = v->last = v->first = newnode;
	} else if (v->last != NULL) {
		newnode = (stringnode *) malloc(sizeof(stringnode));
		newnode->s = s;
		newnode->prev = v->last;
		newnode->next = NULL;
		v->last->next = newnode;
		v->last = newnode;
	} else
		return 1;

	return 0;
}

/* insertstring - inserts string s after cur in vector v */
int insertstring(stringvector * v, char *s)
{
	stringnode *newnode = NULL;

	if (v == NULL || s == NULL)
		return 1;

	if (v->first == NULL || v->last == v->cur)
		return pushstring(v, s);

	if (v->cur == NULL || v->cur->next == NULL)
		return 1;

	newnode = (stringnode *) malloc(sizeof(stringnode));
	newnode->s = s;
	newnode->prev = v->cur;
	newnode->next = v->cur->next;
	newnode->next->prev = newnode;
	newnode->prev->next = newnode;

	return 0;
}

/* preinsertstring - inserts string s before cur in vector v */
int preinsertstring(stringvector * v, char *s)
{
	stringnode *newnode = NULL;

	if (v == NULL || s == NULL)
		return 1;

	if (v->first == NULL)
		return pushstring(v, s);

	if (v->cur == NULL)
		return 1;

	newnode = (stringnode *) malloc(sizeof(stringnode));
	newnode->s = s;
	newnode->prev = v->cur->prev;
	newnode->next = v->cur;
	newnode->next->prev = newnode;
	/* if we are at the start of the list, newnode->prev may be NULL */
	if (newnode->prev != NULL)
		newnode->prev->next = newnode;
	else if (v->first == v->cur)
		v->first = newnode;
	else
		return 1;

	return 0;
}


/* removestring - removes cur node and returns pointer to s */
char *
 removestring(stringvector * v)
{
	char *s;
	stringnode *cur;

	if (v == NULL || v->cur == NULL)
		return NULL;

	s = v->cur->s;
	cur = v->cur;

	/* if there is a previous node link it to the next node. If cur
	 * is at the end of the vector, then the previous will point to
	 * NULL, which is okay. */
	if (cur->prev != NULL)
		cur->prev->next = cur->next;

	/* else if the current node is not first, list is messed up */
	else if (cur != v->first)
		return NULL;

	/* now that the cur node is also first, move first to value of next node,
	 * which may be NULL if cur is the only item in the list. */
	else
		v->first = cur->next;

	/* v->cur will either be set to the next node or the last node,
	 * depending on whether cur is at the end of the list */

	/* if there is a next node, link it backward to the previous node */
	if (cur->next != NULL) {
		cur->next->prev = cur->prev;
		v->cur = cur->next;
	}
	/* else if the current node is not last, list is messed up */
	else if (cur != v->last)
		return NULL;

	/* now that the cur node is also last, move last to value of prev node,
	 * which may be NULL if cur is the only item in the vector. */
	else
		v->last = v->cur = cur->prev;

	/* now we can get rid of the current node */
	free(cur);

	return s;
}

/* deletestring - free()s cur node & s */
int deletestring(stringvector * v)
{
	char *s = NULL;

	s = removestring(v);
	if (s == NULL)
		return 1;

	free(s);

	return 0;
}

/* deletestringvector - deletes entire vector and every s */
int deletestringvector(stringvector * v)
{
	if (v == NULL)
		return 1;

	v->cur = v->last;
	while (deletestring(v) != 1);

	return 0;
}

/* removestringvector - empties a stringvector without free()ing any s */
void removestringvector(stringvector * v)
{
	if (v == NULL)
		return;

	v->cur = v->last;
	while (removestring(v) != NULL);

	return;
}

/* duplcatestringvector - allocate a copy of an svector */
stringvector duplicatestringvector(stringvector v, int minlen)
{
	stringvector dup;
	initstringvector(&dup);

	for (svmovetofirst(&v); v.cur != NULL; v.cur = v.cur->next) {
		pushstring(&dup, str_dupmin(v.cur->s, minlen));
	}

	return dup;
}

stringvector * stringvectorcat(stringvector * v1, stringvector * v2)
{
	if (v1->first == NULL) {
		*v1 = *v2;
		return v1;
	}
	v1->last->next       = v2->first;
	v2->first->prev      = v1->last;
	v1->last             = v2->last;
	v2->first            = v1->first;
	return v1;
}

/* svmovetofirst() - move to first element */
void svmovetofirst(stringvector* v)
{
	v->cur = v->first;
}

/* svmoveby() - move by a number of steps, negative for backward */
int svmoveby(stringvector* v, int delta)
{
	int i;
	
	if (delta > 0) {
		for (i = 0; i < delta && v->cur->next != NULL; i++)
			v->cur = v->cur->next;
		return i;
	} else {
		for (i = 0; i > delta && v->cur->prev != NULL; i--)
			v->cur = v->cur->prev;
		return -i;
	}
}

/* svgetposoiton() - count how far cur is from first */
int svgetposition(stringvector* v)
{
	int pos = 0;
	stringnode *finder = v->first;

	while (finder != v->cur && finder != NULL) {
		finder = finder->next;
		pos++;
	}

	return pos;
}

/* inssortstringvector() - insertion sort vector using strcmp()-like funct */
void inssortstringvector(stringvector* v, int (*compare)(const char* s1, const char* s2))
{
	char *curstr = NULL;
	stringnode *nodepos = NULL, * sortnode = NULL;

	if (v->first == NULL || v->first->next == NULL)
		return;  /* No point in sorting one or none nodes. */

	for (sortnode = v->first->next; sortnode != NULL; sortnode = sortnode->next) {
		curstr = sortnode->s;
		
		/* Find place to insert curstr */
		nodepos = sortnode;
		while (nodepos != v->first && compare(nodepos->prev->s, curstr) > 0) {
			/* So long as the previous node is bigger, move it forward and step back. */
			nodepos->s = nodepos->prev->s;
			nodepos = nodepos->prev;
		}
		nodepos->s = curstr;
	}
	/* TODO: don't use v->cur */
	v->cur = v->first;
}


/***************************************************************************/
/**** Wordwrap *************************************************************/
/***************************************************************************/

/* wordwrap
 * purpose: Inserts string into current string of svector, wordwrapping if
 *          necessary.
 * args:    sv:        stringvector to manipulate
 *          str:       string to insert in sv->cur->s
 *          inspos:    where in sv to insert str
 *          pos:       cursor position in sv->cur->s to track; a negative value
 *                     indicates that -pos - 1 is position in str
 *          wrapwidth: at what cursor position to wrap to next line
 *          editwidth: maximum width of a line in sv
 * return:  new location of pos. sv->cur is changed to reflect line on which
 *          pos now resides.
 *
 * NOTE: str will not be modified nor free()d in any way.
 */
int wordwrap(stringvector * sv, char *str, int inspos, int pos, int wrapwidth, int editwidth)
{
	int i, j, k;		/* general counters */
	char *longstr;	/* Combination of sv->cur->s & str */
	int longlen;		/* Length of longstr */
	int newpos;		/* new position after insert */

	char *newstr;     /* new string for next line */

	/* check for bad data */
	if (sv->cur == NULL || sv->cur->s == NULL || wrapwidth > editwidth || editwidth < 2 || inspos > strlen(sv->cur->s))
		return -1;

	/* first determine longlen and allocate longstr */
	longlen = strlen(sv->cur->s) + strlen(str);
	longstr = (char *) malloc(longlen + 2);
	memset(longstr, 0, longlen + 2);
	
	/* fill longstr
	 * 
	 * i: position in longstr
	 * j: position in sv->cur->s
	 * k: position in str 
	 */

	/* fill from sv until inspos */
	for (i = 0; i < inspos; i++)
		longstr[i] = sv->cur->s[i];
	j = i;

	/* fill from str until end of str */
	for (k = 0; str[k] != 0; k++, i++)
		longstr[i] = str[k];

	/* fill from sv until end */
	for (; i < longlen; i++, j++)
		longstr[i] = sv->cur->s[j];
	
	/* cap longstr */
	longstr[i]   = 0;

	/* determine location of newpos */
	if (pos >= inspos)
		newpos = pos + strlen(str);
	else if (pos < 0)
		newpos = inspos - pos - 1;

	if (longlen <= wrapwidth) {
		/* no need to wordwrap; we can just copy longstr over sv->cur->s */
		strcpy(sv->cur->s, longstr);
		/* don't forget to free longstr before leaving */
		free(longstr);
		return newpos;
	}

	/* we need to find the first space before wrapwidth 
	 *
	 * i: position in longstr
	 * j: position of last identified space
	 */

	j = -1;
	for (i = 0; i < wrapwidth; i++)
		if (longstr[i] == ' ')
			j = i;

	if (j == -1) {
		/* no space was found before wrap; reject insert */
		/* longstr is no longer needed */
		free(longstr);
		return pos;
	}

	/* make newpos the negative differance of location of space and newpos,
	 * if it belongs on next line */
	if (newpos > j)
		newpos = j - newpos;

	/* set newstr to location of string after the space & cap longstr at space */
	newstr = longstr + j + 1;
	longstr[j] = 0;

	/* replace sv->cur->s with shortened longstr */
	strcpy(sv->cur->s, longstr);

	/* finally: wrap onto next line or new line */
	if (sv->cur->next == NULL       || strlen(sv->cur->next->s) == 0 ||
			sv->cur->next->s[0] == '#'  || sv->cur->next->s[0] == '/' ||
			sv->cur->next->s[0] == '?'  || sv->cur->next->s[0] == ':' ||
			sv->cur->next->s[0] == '!'  || sv->cur->next->s[0] == '$' ||
			sv->cur->next->s[0] == '\'' || sv->cur->next->s[0] == '@' ||
			sv->cur->next->s[0] == ' ') {
		/* next line either does not exist, is blank, is a zoc command,
		 * or is indented; so, we create a new, blank line to wordwrap onto */

		char *newnode;
		newnode = (char *) malloc(editwidth + 2);
		newnode[0] = 0;
		insertstring(sv, newnode);
	} else {
		/* we can put text at the beginning of the next line; append a space
		 * to end of newstr in preparation. */
		i = strlen(newstr);
		newstr[i++] = ' ';
		newstr[i] = 0;
	}
	/* it is now okay to put text at the beginning of the next line */


	/* recursively insert newstr at beginning of next line */
	if (newpos < 0) {
		/* cursor should be tracked on next line */
		sv->cur = sv->cur->next;
		newpos = wordwrap(sv, newstr, 0, newpos, wrapwidth, editwidth);
	} else {
		stringnode * nodeptr = sv->cur;
		sv->cur = sv->cur->next;
		wordwrap(sv, newstr, 0, 0, wrapwidth, editwidth);
		sv->cur = nodeptr;
	}

	free(longstr);

	return newpos;
}

/**********************************************************************/
/******* String utility functions *************************************/
/**********************************************************************/

/* advance token in source from pos, returning token length */
int tokenadvance(char *token, char *source, int *pos)
{
	int i = 0;

	/* Move forward past any spaces */
	while (*pos < strlen(source) && source[*pos] == ' ') (*pos)++;

	/* Grab next token */
	for (; *pos < strlen(source) != 0 && source[*pos] != ' '; i++, (*pos)++)
		token[i] = source[*pos];
	token[i] = 0;

	return i;
}


/* grow token in source from pos, returning token length */
int tokengrow(char *token, char *source, int *pos)
{
	int i = strlen(token);

	/* Grab any spaces */
	for (; *pos < strlen(source) != 0 && source[*pos] == ' '; i++, (*pos)++)
		token[i] = source[*pos];

	/* Grab next token */
	for (; *pos < strlen(source) != 0 && source[*pos] != ' '; i++, (*pos)++)
		token[i] = source[*pos];
	token[i] = 0;

	return i;
}


