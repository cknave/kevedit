/* svector.h  -- string vectors
 * $Id: svector.h,v 1.4 2000/09/09 02:33:51 bitman Exp $
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

#ifndef __SVECTOR_H
#define __SVECTOR_H 1


/* basic structures for a string vector */

typedef struct stringnode {
	unsigned char *s;
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
 * In each function, the pointer s is copied, not the string. */

/* pushstring - pushes string s to end of vector v */
int pushstring(stringvector * v, unsigned char *s);

/* insertstring - inserts string s after cur in vector v */
int insertstring(stringvector * v, unsigned char *s);

/* preinsertstring - inserts string s before cur in vector v */
int preinsertstring(stringvector * v, unsigned char *s);

/* removestring - removes cur node and returns pointer to s */
unsigned char *removestring(stringvector * v);

/* deletestring - free()s cur node & s */
int deletestring(stringvector * v);

/* deletestringvector - deletes entire vector and every s */
int deletestringvector(stringvector * v);

/* removestringvector - empties a stringvector without free()ing any s */
void removestringvector(stringvector * v);

#endif
