/* svector.h  -- string vectors
 * $Id: svector.h,v 1.16 2002/11/17 05:54:36 bitman Exp $
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

/* General string operations which become useful when paired with svectors */


#define STREQU_UNCASE  0x01     /* Capitalization is ignored */
#define STREQU_FRONT   0x02     /* Equal if the fronts of the strings
																	 are the same */
#define STREQU_RFRONT  (0x04 | STREQU_FRONT)
                                /* Equal if the right string is the front
																 * of the left string */

/* String duplication using malloc
 * str_dup()    - reserves just enough space for the string
 * str_dupmax() - reserves at  most max+1 space
 * str_dupmin() - reserves at least min+1 space
 * str_dupat()  - reserves  exactly len+1 space
 */
char * str_dup   (char * s);
char * str_dupmin(char * s, unsigned int min);
char * str_dupmax(char * s, unsigned int max);
char * str_duplen(char * s, unsigned int len);
char * str_dupadd(char * s, unsigned int add);

/* Create an empty string of given length using malloc */
char * str_create(unsigned int len);

/* str_lowercase - changes string to lowercase and returns it */
char* str_lowercase(char* string);

/* str_equ - compares two strings, ignoring case & length based on flags */
int str_equ(const char *str1, const char *str2, int flags);

/* tokens */
/* advance token in source from pos, returning token length */
int tokenadvance(char *token, char *source, int *pos);

/* grow token in source from pos, returning token length */
int tokengrow(char *token, char *source, int *pos);

#endif
