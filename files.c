/* files.h  -- filesystem routines
 * $Id: files.c,v 1.2 2001/11/10 20:03:43 bitman Exp $
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


#include "files.h"

#include "zlaunch.h"

#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/***************************************************************************/
/**** File I/O *************************************************************/
/***************************************************************************/

#define BUFFERSIZE 1000
/* filetosvector() - loads a textfile into a new stringvector */
stringvector filetosvector(char* filename, int wrapwidth, int editwidth)
{
	stringvector v;
	FILE * fp;
	char buffer[BUFFERSIZE] = "";      /* Be nice and wordwrap long lines */
	char * str = NULL;
	int strpos = 0;
	int c = 0;

	initstringvector(&v);

	/* return on bad data */
	if (wrapwidth > editwidth || editwidth < 1)
		return v;

	fp = fopen(filename, "rb");
	if (fp == NULL)
		return v;

	do {
		strpos = 0;

		while (strpos < BUFFERSIZE && !((c = fgetc(fp)) == EOF || c == 0x0d || c == 0x0a || c == 0)) {
			buffer[strpos++] = c;
		}
		buffer[strpos] = 0;

		/* remove LF after CR (assume not CR format) */
		if (c == 0x0d)
			fgetc(fp);

		str = (char *) malloc(sizeof(char) * (editwidth + 1));
		if (str == NULL) {
			fclose(fp);
			return v;
		}

		if (strpos < wrapwidth) {
			/* simple copy */
			strcpy(str, buffer);
			pushstring(&v, str);
		} else {
			/* Push an empty string and wordwrap the buffer onto it */
			str[0] = 0;
			pushstring(&v, str);
			v.cur = v.last;
			wordwrap(&v, buffer, 0, 0, wrapwidth, editwidth);
		}
	} while (c != EOF);

	fclose(fp);

	/* remove trailing blank line */
	v.cur = v.last;
	if (strlen(v.cur->s) == 0)
		removestring(&v);

	return v;
}


/* svectortofile() - copies a stringvector into a file. sv is not changed */
void svectortofile(stringvector * sv, char *filename)
{
	FILE* fp;
	stringnode * curnode = NULL;
	int i;

	if (sv->first == NULL)
		return;

	fp = fopen(filename, "wb");
	if (fp == NULL)
		return;

	for (curnode = sv->first; curnode != NULL; curnode = curnode->next) {
		for (i = 0; curnode->s[i] != 0; i++)
			fputc(curnode->s[i], fp);
		fputc(0x0d, fp);
		fputc(0x0a, fp);
	}

	/* Done with file; success! */
	fclose(fp);
}


/* filecomp() - compare two file names, considering directories greater */
int filecomp(const char* s1, const char* s2)
{
	if (s1[0] == '!') {
		if (s2[0] == '!')
			return strcmp(s1, s2);
		return 1;         /* s1 is a dir, s2 is not. s1 is greater */
	}
	if (s2[0] == '!')
		return -1;        /* s2 is a dir, s1 is not. s2 is greater */
	return strcmp(s1, s2);
}

/* readdirectorytosvector() - reads a directory listing into an svector */
stringvector readdirectorytosvector(char* dir, char* extension, int filetypes)
{
	stringvector files;
	DIR *dp;

	initstringvector(&files);

	dp = opendir(dir);
	if (dp == NULL)
		return files;

	while (1) {
		char * fulld_name;
		struct dirent *dirent;

		dirent = readdir(dp);

		if (dirent == NULL)
			break;

		fulld_name = fullpath(dir, dirent->d_name, SLASH_DEFAULT);

		if (access(fulld_name, D_OK)) {
			if (filetypes & FTYPE_FILE) {
				/* The current file is not a directory, check the extension */
				if (extension[0] == '*' ||
						(dirent->d_name[strlen(dirent->d_name) - strlen(extension) - 1]
							 == '.' &&
						str_equ(dirent->d_name + strlen(dirent->d_name) - strlen(extension),
										extension, STREQU_UNCASE))) {
					pushstring(&files, str_dup(dirent->d_name));
				}
			}
		} else if (!str_equ(dirent->d_name, ".", 0)) {
			if (filetypes & FTYPE_DIR) {
				/* Current file is a directory */
				char* dirline = (char*) malloc(sizeof(char) *
																			 (strlen(dirent->d_name)*2 + 5));
				strcpy(dirline, "!");
				strcat(dirline, dirent->d_name);
				strcat(dirline, ";[");
				strcat(dirline, dirent->d_name);
				strcat(dirline, "]");
				pushstring(&files, dirline);
			}
		}

		free(fulld_name);
	}
	closedir(dp);

	inssortstringvector(&files, filecomp);

	return files;
}

