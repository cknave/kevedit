/* help.h  -- hypertext help system
 * $Id: help.h,v 1.1 2003/11/01 23:45:56 bitman Exp $
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

#ifndef __HELP_H
#define __HELP_H 1

#include "display/display.h"
#include "structures/svector.h"

/* inithelpsystem() - init help system and load the given help metafile. true
 *                    on error */
int inithelpsystem(char* datapath);

/* deletehelpsystem() - clear out the help system (use upon exit) */
void deletehelpsystem(void);

/* help() - Displays copyright and default help section */
void help(displaymethod* d);

/* For both helptopic() and helpfiletopic():
 * Return value is true when browsing should continue (i.e. "back") */

/* helptopic() - Displays help topic for topic in given section. For NULL or ""
 *               topic, browsing begins current position in section */
int helptopic(stringvector section, char* topic, displaymethod* d);

/* helptopic() - Displags help for given section title and topic. For NULL or
 *               "" topic, browsing begins at the top of the section */
int helpsectiontopic(char* sectiontitle, char* topic, displaymethod* d);

#endif
