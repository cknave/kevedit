/* helplist.h  -- very simple linked list of help sections
 * $Id: helplist.h,v 1.1 2003/11/01 23:45:56 bitman Exp $
 * Copyright (C) 2001 Ryan Phillips <bitman@users.sourceforge.net>
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

#ifndef __HELPLIST_H
#define __HELPLIST_H

#include "structures/svector.h"

/* Very simple linked list for compilation of helpsections */
typedef struct helpsection {
	/* Title (file name) and string vector containing the text of the section.
	 * NULL for title indicates a control node, which is safely ignored */
	char* title;
	stringvector sv;

	/* Next node in list (NULL indicates last node) */
	struct helpsection* next;
} helpsection;


/* inithelpsection() - creates an empty help section */
void inithelpsection(helpsection* section);

/* deletesectionlist() - free()s everything but the section itself */
void deletesectionlist(helpsection* section);

/* appendsection() - appends src to the end of dest */
void appendsection(helpsection* dest, helpsection* src);

/* findsection() - locates a section with given title in a section list */
helpsection* findsection(helpsection* sectionlist, char* title);

/* loadhelpmetafile() - loads a help meta into the given help section list.
 *                      the original meta is destroyed in the process. true on
 *                      error */
int loadhelpmeta(helpsection* section, stringvector* meta);


#endif
