/* files.h  -- filesystem routines
 * $Id: files.c,v 1.2 2005/05/28 03:17:45 bitman Exp $
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#if HAVE_SYS_TYPES_H
#	include <sys/types.h>
#endif
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>   /* For getcwd() */
#include <sys/stat.h>

#include <glob.h>

#include "files.h"

#include "zlaunch/zlaunch.h"


#define BUFFERSIZE 42     /* Expanding buffer for text files */
#define CWDMAXLEN 4096    /* Max to allow getcwd() to reserve */
#define CPBUFFERSIZE 1024 /* Binary file transfer buffer */ 

/***************************************************************************/
/**** File I/O *************************************************************/
/***************************************************************************/

/* filetosvector() - loads a textfile into a new stringvector */
stringvector filetosvector(char* filename, int wrapwidth, int editwidth)
{
	stringvector v;
	FILE * fp;
	char * str = NULL;
	int strpos = 0;
	int c = 0;

	initstringvector(&v);

	/* Wordwarp requires a greater or equal editwidth */
	if (wrapwidth > editwidth)
		wrapwidth = editwidth;

	/* Open the file */
	fp = fopen(filename, "rb");
	if (fp == NULL)
		return v;

	do {
		int done = 0;
		int bufsize = BUFFERSIZE;
		char *buffer = str_duplen("", bufsize);

		strpos = 0;
		do {
			/* Copy characters until bufsize is reached */
			while (strpos < bufsize &&
						 !((c = fgetc(fp)) == EOF || c == 0x0d || c == 0x0a || c == '\x0'))
				buffer[strpos++] = c;

			if (strpos == bufsize) {
				/* Reached the end of the buffer -- make it bigger and keep going */
				char* newbuf;
				bufsize *= 2;
				newbuf = str_duplen(buffer, bufsize);
				free(buffer);
				buffer = newbuf;
			} else {
				done = 1;
			}
		} while (!done);

		buffer[strpos] = 0;

		/* remove LF after CR (assume not CR format) */
		if (c == 0x0d)
			fgetc(fp);

		if (editwidth == 0) {
			/* No editwidth, reserve just enough space to hold the string */
			/* Assume no wordwrap (checked above), may wish to change this some day */
			str = str_dup(buffer);
			if (str == NULL) {
				fclose(fp);
				return v;
			}

			pushstring(&v, str);
		} else {
			/* Positive editwidth, reserve enough space for that many chars */
			str = str_duplen("", editwidth);
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
				pushstring(&v, str);
				v.cur = v.last;
				wordwrap(&v, buffer, 0, 0, wrapwidth, editwidth);
			}
		}

		free(buffer);
	} while (c != EOF);

	fclose(fp);

	/* remove trailing blank line, if present */
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

		if (!fileisdir(fulld_name)) {
			if (filetypes & FTYPE_FILE) {
				/* The current file is not a directory, check the extension */
				if (extension[0] == '*' ||
						(dirent->d_name[strlen(dirent->d_name) - strlen(extension) - 1]
							 == '.' &&
						str_equ(dirent->d_name + strlen(dirent->d_name) - strlen(extension),
										extension, STREQU_UNCASE))) {
					/* Advance past special character '!' if necessary */
					/* TODO: less quick & dirty fix? */
					if (dirent->d_name[0] == '!') {
						char* dest = str_duplen(" ", strlen(dirent->d_name) + 1);
						strcat(dest, dirent->d_name);
						pushstring(&files, dest);
					} else
						pushstring(&files, str_dup(dirent->d_name));
				}
			}
		} else if (!str_equ(dirent->d_name, ".", 0) && fileisdir(fulld_name)) {
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

/* globtosvector() - put raw glob information in an svector */
stringvector globtosvector(char * pattern, int filetypes)
{
	int i;

	stringvector files;
	glob_t listing;

	initstringvector(&files);

	/* Get the listing */
	glob(pattern, GLOB_MARK, NULL, &listing);

	/* Cycle through the listing */
	for (i = 0; i < listing.gl_pathc; i++) {
		char* filename = str_dup(listing.gl_pathv[i]);
		/* Determine whether it is a directory or file */
		if (filename[strlen(filename) - 1] == '/') {
			if (filetypes & FTYPE_DIR)
				pushstring(&files, filename);
		} else if (filetypes & FTYPE_FILE) {
			pushstring(&files, filename);
		}
	}

	globfree(&listing);
	return files;
}

/* globdirectorytosvector() - globs a directory listing into an svector */
stringvector globdirectorytosvector(char * dir, char * pattern, int filetypes)
{
	char* globpattern = fullpath(dir, pattern, SLASH_DEFAULT);

	stringvector fullfiles = globtosvector(globpattern, filetypes);
	stringvector files;

	free(globpattern);
	initstringvector(&files);

	for (svmovetofirst(&fullfiles);
			 fullfiles.cur != NULL;
			 fullfiles.cur = fullfiles.cur->next) {
		char* smallname = (char*) malloc(sizeof(char) * strlen(fullfiles.cur->s));

		if (fullfiles.cur->s[strlen(fullfiles.cur->s) - 1] == '/') {
			char* zocname;
			fullfiles.cur->s[strlen(fullfiles.cur->s) - 1] = '\x0';

			fileof(smallname, fullfiles.cur->s, strlen(fullfiles.cur->s));
			zocname = str_duplen("!", strlen(smallname) * 2 + 4);
			strcat(zocname, smallname);
			strcat(zocname, ";[");
			strcat(zocname, smallname);
			strcat(zocname, "]");
			pushstring(&files, zocname);

			free(smallname);
		}
		
		fileof(smallname, fullfiles.cur->s, strlen(fullfiles.cur->s));
		pushstring(&files, smallname);
	}

	deletestringvector(&fullfiles);

	return files;
}

int fileexists(char* filename)
{
	return !access(filename, F_OK);
}

int fileisdir(char* filename)
{
	struct stat statent;
	stat(filename, &statent);
	return S_ISDIR(statent.st_mode);
}

int
copyfile(char* srcname, char* destname, int flags)
{
	FILE* src, * dest;
	size_t readsize;
	int* copybuffer;

	if (!(flags & COPY_OVERWRITE)) {
		/* Check for existence of destination file */
		dest = fopen(destname, "rb");
		if (dest != NULL) {
			/* The destination file already exists */
			fclose(dest);
			return COPY_EXISTS;
		}
	}

	/* Open the source file for reading */
	src = fopen(srcname, "rb");
	if (src == NULL) {
		/* The source file cannot be opened! Error! */
		fprintf(stderr, "Cannot open %s for reading.", srcname);
		return COPY_BADSOURCE;
	}
	
	/* Open the destination file for writing */
	dest = fopen(destname, "wb");
	if (dest == NULL) {
		/* Can't write to this file */
		fprintf(stderr, "Cannot open %s for writing.", destname);

		fclose(src);
		return COPY_BADDEST;
	}

	/* Copy src to dest */
	copybuffer = (int*) malloc(sizeof(int) * CPBUFFERSIZE);
	do {
		readsize = fread(copybuffer, sizeof(int), CPBUFFERSIZE, src);
		fwrite(copybuffer, sizeof(int), readsize, dest);
	} while (readsize == CPBUFFERSIZE);
	free(copybuffer);

	/* Close everything down */
	fclose(src);
	fclose(dest);

	return COPY_SUCCESS;
}


int
copyfilebydir(char* srcdir, char* destdir, char* filename, int flags)
{
	int result;

	char* srcname = fullpath(srcdir, filename, SLASH_DEFAULT);
	char* destname = fullpath(destdir, filename, SLASH_DEFAULT);

	result = copyfile(srcname, destname, flags);

	free(destname);
	free(srcname);

	return result;
}


int
copyfilepatternbydir(char* srcdir, char* destdir, char* pattern,
										 int flags, stringvector * successlist)
{
	/* glob the pattern in the source directory */
	char* globpattern = fullpath(srcdir, pattern, SLASH_DEFAULT);
	stringvector files = globtosvector(globpattern, FTYPE_FILE);

	/* Copy each file */
	for (svmovetofirst(&files); files.cur != NULL; files.cur = files.cur->next) {
		char* destfile = str_duplen("", strlen(files.cur->s));
		char* destfullname;

		/* Determine the full name of the destination file */
		fileof(destfile, files.cur->s, strlen(files.cur->s));
		destfullname = fullpath(destdir, destfile, SLASH_DEFAULT);

		/* Copy */
		if (copyfile(files.cur->s, destfullname, flags) == COPY_SUCCESS) {
			if (successlist != NULL)
				pushstring(successlist, str_dup(destfullname));
		}
		else if ((flags & COPY_DISPLACE)) {
			/* If copy failed and the displacement flag is set and
			 * the destination file can be over-written, then displace it. */
			char* displacefile = str_duplen(DISPLACE_LEADER, strlen(destfile) +
																			strlen(DISPLACE_LEADER));
			char* displacefullname;

			/* Determine full pathname */
			strcat(displacefile, destfile);
			displacefullname = fullpath(destdir, displacefile, SLASH_DEFAULT);

			/* Rename w/o over-writing */
			if (access(displacefullname, F_OK) &&
					!rename(destfullname, displacefullname)) {
				/* If the displace filename was available and copy completed,
				 * force the original copy and record to successlist */
				if (copyfile(files.cur->s, destfullname, COPY_OVERWRITE) ==
						COPY_SUCCESS) {
					if (successlist != NULL) {
						char* record = str_duplen(destfullname, strlen(destfullname) +
																			strlen(DISPLACE_SEPERATOR) +
																			strlen(displacefullname));
						strcat(record, DISPLACE_SEPERATOR);
						strcat(record, displacefullname);
						pushstring(successlist, record);
					}
				}
			}

			/* Cleanup */
			free(displacefile);
			free(displacefullname);
		}

		/* Cleanup */
		free(destfile);
		free(destfullname);
	}

	free(globpattern);
	deletestringvector(&files);
	return 0;
}


/* Returns a malloc()ed string bearing the location of this program based
 * on main's argv[0]. Not useful for much else. */
char*
locateself(char* argv0)
{
	char* path = (char*) malloc(sizeof(char) * (strlen(argv0) + 1));
	char* cwd = NULL;
	char* fullpathname = NULL;

	pathof(path, argv0, strlen(argv0));

	/* If we find a ':' in the path of this program, it's probably a
	 * DOS full path (anyone know of an exception to this???) */
	if (strchr(path, ':') != NULL)
		return path;

	/* path is a relative path, so we need to prepend the current dir */

	cwd = getcwd(NULL, CWDMAXLEN + 1);
	if (cwd == NULL)
		return path;

	fullpathname = (char*) malloc(sizeof(char) * (strlen(path)+strlen(cwd)+2));
	strcpy(fullpathname, cwd);
	strcat(fullpathname, "/");
	strcat(fullpathname, path);

	free(path);
	free(cwd);
	return fullpathname;
}

/* Extracts the local filename from filename given by fullpath */
char*
fileof(char* buffer, char* fullpath, int buflen)
{
	char* lastslash, * lastbackslash, * start;
	int i;

	lastslash     = strrchr(fullpath, '/');
	lastbackslash = strrchr(fullpath, '\\');

	if (lastslash == NULL && lastbackslash == NULL)
		start = fullpath;
	else if (lastslash > lastbackslash)
		start = lastslash + 1;
	else
		start = lastbackslash + 1;

	for (i = 0; start[i] != '\0' && i < buflen; i++)
		buffer[i] = start[i];
	buffer[i] = '\0';

	return buffer;
}

/* Extracts the path from filename given by fullpath */
char*
pathof(char* buffer, char* fullpath, int buflen)
{
	char* lastslash, * lastbackslash;
	int end;
	int i;

	lastslash     = strrchr(fullpath, '/');
	lastbackslash = strrchr(fullpath, '\\');

	if (lastslash == NULL && lastbackslash == NULL) {
		strcpy(buffer, ".");
		return buffer;
	} else if (lastslash > lastbackslash)
		end = lastslash - fullpath;
	else
		end = lastbackslash - fullpath;

	for (i = 0; i < end && i < buflen; i++)
		buffer[i] = fullpath[i];
	buffer[i] = '\0';

	return buffer;
}

char*
fullpath(char* path, char* file, int slashtype)
{
	char* fullpath = (char*) malloc(sizeof(char)*(strlen(path)+strlen(file)+2));
	strcpy(fullpath, path);
	strcat(fullpath, (slashtype == SLASH_FORWARD ? "/" : "\\"));
	strcat(fullpath, file);

	return fullpath;
}

/* Changes all slashes in a pathname to the given slashtype
 * pathname is modified!
 * pathname is returned
 */
char*
reslash(char* pathname, int slashtype)
{
	int i;
	if (slashtype == SLASH_FORWARD) {
		/* Change backslashes to forward slashes */
		for (i = 0; i < strlen(pathname); i++)
			if (pathname[i] == '\\')
				pathname[i] = '/';
	} else {
		/* Change forward slashes to backslashes */
		for (i = 0; i < strlen(pathname); i++)
			if (pathname[i] == '/')
				pathname[i] = '\\';
	}

	return pathname;
}

int
run(char* path, char* program, char* args)
{
	int result;
	char* command = malloc(sizeof(char) *
												 (strlen(path)+strlen(program)+strlen(args)+3));

	/* Surround the path with quotes in case of spaces */
	strcpy(command, "\"");
	strcat(command, path);
	strcat(command, "/");
	reslash(command, SLASH_DEFAULT);
	strcat(command, "\"");

	strcat(command, program);
	strcat(command, " ");
	strcat(command, args);

	result = system(command);

	free(command);

	return result;
}


