/* svector.c   -- string vectors
 * Copyright (C) 2000 Ryan Phillips <bitman@scn.org>
 * $Id: svector.c,v 1.6 2000/10/20 02:17:18 bitman Exp $
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

#include "editbox.h"

#include <stdlib.h>


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

	if (v->cur == NULL)
		return 1;

	if (v->first == NULL)
		return pushstring(v, s);

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

