/* zlaunch - zzt launching library (everything necessary to run zzt smoothly)
 * $Id: zlaunch.h,v 1.1 2003/11/01 23:45:57 bitman Exp $
 * Copyright (c) 2001 Ryan Phillips <bitman@users.sf.net>
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

#include "structures/svector.h"

#ifndef _ZLAUNCH_H
#define _ZLAUNCH_H 1

/* Default config file to use */
#define ZL_DEFAULTCONFIG "default.zln"
#define ZL_RMFILENAME "rmlist.tmp"

/* Actions to be performed by the launcher */
#define ZL_NONE         0
#define ZL_RUN          1
#define ZL_KEYSTROKES   2
#define ZL_COPY         3
#define ZL_DISPLACE     4
#define ZL_PERMCOPY     5
#define ZL_PERMCOPYOVER 6

/* Action structure */
typedef struct zlauchaction {
	char* name;              /* name of the action */
	int optional;            /* false will force this action to be performed */

	int type;                /* Type of action (one of above defines) */
	char* command;           /* Action command */

	struct zlauchaction* next;    /* Next action in the list */
} zlaunchaction;

/* Configuration settings for the zlauncher */
typedef struct zlaunchinfo {
	stringvector actionstoperform;   /* list of action names to actually use */
	stringvector paramlist;          /* list of parameters */
	char* bindir;                    /* location of binaries */
	char* datadir;                   /* location of datafiles */

	zlaunchaction* actions;    /* Linked list of actions */
} zlaunchinfo;


/* Info operations */
void initzlinfo(zlaunchinfo* zli);
void deletezlinfo(zlaunchinfo* zli);

stringvector defaultzlinfo(void);
zlaunchinfo loadzlinfofromsvector(stringvector info);

/* Load info from a number of sources */
stringvector loadinfo(char* datapath, char* worldfile);

/* zlaunching */
void zlaunchact(zlaunchinfo* zli);
void zlaunchcleanup(zlaunchinfo* zli);

/* Keboard buffer stuffing routines */
void performkeystrokes(char* keystrokes);
void stuffkbdbuffer(unsigned char scancode, unsigned char ch);

#endif
