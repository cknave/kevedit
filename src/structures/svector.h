/* svector.h  -- string vectors
 * $Id: svector.h,v 1.1 2003/11/01 23:45:57 bitman Exp $
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

#ifndef __SVECTOR_H
#define __SVECTOR_H 1

#include "libzzt2/strtools.h"

/* basic structures for a string vector */

typedef struct stringnode {
	char *s;
	struct stringnode *next;
	struct stringnode *prev;

} stringnode;

typedef struct stringvector {
	stringnode *first;
	stringnode *last;
	stringnode *cur;

} stringvector;

/* Allways initialize a string vector to all NULLs before use */
void initstringvector(stringvector * v);

/* The string() functions return 1 on error, 0 on success.
 * In each function, the pointer s is copied, not the string.
 * Insert functions do not change cur.
 * Delete functions move to the next node, unless there isn't one.
 */

/* pushstring - pushes string s to end of vector v */
int pushstring(stringvector * v, char *s);

/* insertstring - inserts string s after cur in vector v */
int insertstring(stringvector * v, char *s);

/* preinsertstring - inserts string s before cur in vector v */
int preinsertstring(stringvector * v, char *s);

/* removestring - removes cur node and returns pointer to s */
char *removestring(stringvector * v);

/* deletestring - free()s cur node & s */
int deletestring(stringvector * v);

/* deletestringvector - deletes entire vector and every s */
int deletestringvector(stringvector * v);

/* removestringvector - empties a stringvector without free()ing any s */
void removestringvector(stringvector * v);

/* duplcatestringvector - allocate a copy of an svector */
stringvector duplicatestringvector(stringvector v, int minlen);

/* stringvectorcat - concatinates two string vectors. BOTH vectors
 * thereafter share the same memory, only cur differs! */
stringvector * stringvectorcat(stringvector * v1, stringvector * v2);

/* svmovetofirst() - move to first element */
void svmovetofirst(stringvector* v);

/* svmoveby() - move by a number of steps, negative for backward */
int svmoveby(stringvector* v, int delta);

/* svgetposoiton() - count how far cur is from first */
int svgetposition(stringvector* v);

/* inssortstringvector() - insertion sort vector using strcmp()-like funct */
void inssortstringvector(stringvector* v, int (*compare)(const char* s1, const char* s2));

/***** Wordwrap **********************************/

/* wordwrap() - wrap text in sv */
int wordwrap(stringvector * sv, char *str, int inspos, int pos, int wrapwidth, int editwidth);

/* Token manipulation (TODO: may be obsolete) */

/* advance token in source from pos, returning token length */
int tokenadvance(char *token, char *source, int *pos);

/* grow token in source from pos, returning token length */
int tokengrow(char *token, char *source, int *pos);

#endif
