/* files.h  -- filesystem routines
 * $Id: files.h,v 1.1 2001/11/10 07:42:39 bitman Exp $
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

#ifndef __FILES_H
#define __FILES_H

#include "svector.h"

/* General file types */
#define FTYPE_FILE 1
#define FTYPE_DIR  2
#define FTYPE_ALL  3

/* filetosvector() - loads a textfile into a new stringvector */
stringvector filetosvector(char* filename, int wrapwidth, int editwidth);

/* svectortofile() - copies a stringvector into a file. sv is not changed */
void svectortofile(stringvector * sv, char *filename);

/* readdirectorytosvector() - reads a directory listing into an svector */
stringvector readdirectorytosvector(char * dir, char * extension,
																		int filetypes);

#endif
