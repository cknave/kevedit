/* strtools  -- portable string-handling functions */
/* $Id: strtools.c,v 1.2 2005/05/28 03:17:45 bitman Exp $ */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "strtools.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char * str_dup(char * s)
{
	char* copy = (char *) malloc(sizeof(char) * (strlen(s) + 1));
	if (copy == NULL)
		return NULL;

	strcpy(copy, s);
	return copy;
}

char * str_dupmin(char * s, unsigned int min)
{
	char* copy;
	int duplen = strlen(s);
	if (duplen < min) duplen = min;

	copy = (char *) malloc(sizeof(char) * (duplen + 1));
	if (copy == NULL)
		return NULL;

	strncpy(copy, s, duplen);
	copy[duplen] = '\0';
	return copy;
}

char * str_dupmax(char * s, unsigned int max)
{
	char* copy;
	int duplen = strlen(s);
	if (duplen > max) duplen = max;

	copy = (char *) malloc(sizeof(char) * (duplen + 1));
	if (copy == NULL)
		return NULL;

	strncpy(copy, s, duplen);
	copy[duplen] = '\0';
	return copy;
}

char * str_duplen(char * s, unsigned int len)
{
	char* copy = (char *) malloc(sizeof(char) * (len + 1));
	if (copy == NULL)
		return NULL;

	strncpy(copy, s, len);
	copy[len] = '\0';
	return copy;
}

char * str_dupadd(char * s, unsigned int add)
{
	char* copy;
	int duplen = strlen(s) + add;

	copy = (char *) malloc(sizeof(char) * (duplen + 1));
	if (copy == NULL)
		return NULL;

	strncpy(copy, s, duplen);
	copy[duplen] = '\0';
	return copy;
}

char * str_create(unsigned int len)
{
	char* string = (char *) malloc(sizeof(char) * (len + 1));
	if (string == NULL)
		return NULL;

	string[0] = '\0';
	return string;
}


/* str_lowercase - used by str_equ to convert a string to lowercase,
 *                 same as strlwr() in djgpp, but more compatible.
 *                 introduced by Elchonon Edelson for linux port */
char* str_lowercase(char* string)
{
	char* s = string;
	while (*s) {
		*s = tolower(*s);
		s++;
	}

	return string;
}


/* str_equ - string comparison */
int str_equ(const char *str1, const char *str2, int flags)
{
	char *lwr1, *lwr2;
	int i;
	int isequ = 1;		/* Strings are equal until proven otherwise */

	if (str1[0] == '\x0' && str2[0] == '\x0')
		return 1;
   else if (str1[0] == '\x0' || str2[0] == '\x0')
   	return 0;

	lwr1 = (char *) malloc(strlen(str1) * sizeof(char) + 1);
	if (lwr1 == NULL)
		return -1;
	lwr2 = (char *) malloc(strlen(str2) * sizeof(char) + 1);
	if (lwr2 == NULL) {
		free(lwr1);
		return -1;
	}

	strcpy(lwr1, str1);
	strcpy(lwr2, str2);

	if (flags & STREQU_UNCASE) {
		str_lowercase(lwr1);
		str_lowercase(lwr2);
	}

	for (i = 0; lwr1[i] != '\x0' && lwr2[i] != '\x0'; i++)
		if (lwr1[i] != lwr2[i]) {
			isequ = 0;
			break;
		}

	if (lwr1[i] != lwr2[i]) {        /* If the strings do not end together */
		if (!(flags & STREQU_FRONT))   /* Unless only checking string fronts */
			isequ = 0;
		/* If the left string ends first and the right is expected to be the front
		 * part of the left string, they are not equal */
		if ((flags & STREQU_RFRONT) && lwr1[i] == '\x0')
			isequ = 0;
	}

	free(lwr1);
	free(lwr2);

	return isequ;
}

/* lookupString - Table lookup for a string using str_equ()
 * Returns index of string if found, otherwise -1 */
int lookupString(const char * table[], int size, char * value, int equflags)
{
	int i = 0;

	for (i = 0; i < size; i++)
		if (str_equ(value, table[i], equflags))
			return i;

	return -1;
}

