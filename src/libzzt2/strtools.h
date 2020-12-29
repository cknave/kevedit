/* strtools  -- portable string-handling functions */
/* $Id: strtools.h,v 1.1 2003/11/01 23:45:57 bitman Exp $ */
/* Copyright (C) 2002 Ryan Phillips <bitman@users.sourceforge.net>
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

#ifndef LIBZZT2_STRTOOLS_H
#define LIBZZT2_STRTOOLS_H 1

#ifdef __cplusplus
extern "C" {
#endif

#define STREQU_UNCASE  0x01     /* Capitalization is ignored */
#define STREQU_FRONT   0x02     /* Equal if the fronts of the strings
																	 are the same */
#define STREQU_RFRONT  (0x04 | STREQU_FRONT)
                                /* Equal if the right string is the front
																 * of the left string */

/* String duplication using malloc
 * str_dup()    - reserves just enough space for the string
 * str_dupmin() - reserves at least min+1 space
 * str_dupmax() - reserves at  most max+1 space
 * str_duplen() - reserves  exactly len+1 space
 * str_dupadd() - reserves just enough space for the string + add chars
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

/* lookupString - Table lookup for a string using str_equ()
 * Returns index of string if found, otherwise -1 */
int lookupString(const char * table[], int size, char * value, int equflags);

#ifdef __cplusplus
}
#endif

#endif /* LIBZZT2_STRTOOLS_H */
