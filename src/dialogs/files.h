/* files.h  -- filesystem routines
 * $Id: files.h,v 1.1 2003/11/01 23:45:56 bitman Exp $
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

#ifndef __FILES_H
#define __FILES_H

#include "structures/svector.h"

/* Copy flags */
#define COPY_NOOVERWRITE 0
#define COPY_OVERWRITE   1
#define COPY_DISPLACE    2

/* Copy results */
#define COPY_SUCCESS   0  /* Ok */
#define COPY_EXISTS    1  /* Destination file already exists */
#define COPY_BADSOURCE 2  /* Can't open source file */
#define COPY_BADDEST   3  /* Can't open destination file */

/* General file types */
#define FTYPE_FILE 1
#define FTYPE_DIR  2
#define FTYPE_ALL  3

/* Types of slashes */
#define SLASH_FORWARD 0
#define SLASH_BACK    1

#ifdef WIN32
#define SLASH_DEFAULT SLASH_BACK
#else
#define SLASH_DEFAULT SLASH_FORWARD
#endif

/* Displacement constants */
#define DISPLACE_LEADER "~"
#define DISPLACE_SEPERATOR "?"

/* filetosvector() - loads a textfile into a new stringvector */
stringvector filetosvector(char* filename, int wrapwidth, int editwidth);

/* svectortofile() - copies a stringvector into a file. sv is not changed */
void svectortofile(stringvector * sv, char *filename);

/* readdirectorytosvector() - reads a directory listing into an svector */
stringvector readdirectorytosvector(char * dir, char * extension,
																		int filetypes);

/* globtosvector() - put raw glob information in an svector */
stringvector globtosvector(char * pattern, int filetypes);

/* globdirectorytosvector() - globs a directory listing into an svector */
stringvector globdirectorytosvector(char * dir, char * pattern, int filetypes);

/* File access */
int fileexists(char* filename);
int fileisdir(char* filename);

/* File copying */
int copyfile(char* srcname, char* destname, int flags);
int copyfilebydir(char* srcdir, char* destdir, char* filename, int flags);
int copyfilepatternbydir(char* srcdir, char* destdir, char* pattern, int flags, stringvector* successlist);

/* Determine path of self from main's argv[0] */
char* locateself(char* argv0);

/* Deduce filename or path from a file's full path */
char* fileof(char* buffer, char* fullpath, int buflen);
char* pathof(char* buffer, char* fullpath, int buflen);
char* fullpath(char* path, char* file, int slashtype);

/* Change all the slashes in a pathname */
char* reslash(char* pathname, int slashtype);

/* Run a program with given path and arguments in current directory */
int   run(char* path, char* program, char* args);

#endif
