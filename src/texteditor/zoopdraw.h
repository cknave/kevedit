/* zoopdraw - draw zzt oop to a text display */
/* $Id: zoopdraw.h,v 1.1 2003/11/01 23:45:57 bitman Exp $ */
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

#ifndef ZOOPDRAW_H
#define ZOOPDRAW_H 1

#include "libzzt2/zztoop.h"

#include "display/display.h"

/* Music string components */
#define ZOOPMUSIC_NOTE     0
#define ZOOPMUSIC_REST     1
#define ZOOPMUSIC_DRUM     2
#define ZOOPMUSIC_TIME     3
#define ZOOPMUSIC_TIMEMOD  4
#define ZOOPMUSIC_OCTAVE   5
#define ZOOPMUSIC_PITCH    6

#define ZOOPMUSIC_MAX      6

typedef struct ZZTOOPdrawer {
	displaymethod * display;  /* Draw here */

	int x, y;     /* Position to draw at */
	int length;   /* Maximum length of drawn line */

	int helpformatting;  /* True if output should be formatted */

	int *colours;      /* Array of colours indexed by ZZTOOPcomponent.type */
	int *textcolours;  /* Array of colours for displaying text */
	int *musiccolours; /* Array of colours for displaying music */
} ZZTOOPdrawer;

/* Default colour arrays */
extern int zztoopdefaultcolours[ZOOPTYPE_MAX + 1];
extern int zztoopdefaulttextcolours[ZOOPTEXT_MAX + 1];
extern int zztoopdefaultmusiccolours[ZOOPMUSIC_MAX + 1];

/* Initialize a drawer with default colours */
void zztoopInitDrawer(ZZTOOPdrawer * drawer);

/* Draw a component chain */
void zztoopDraw(ZZTOOPdrawer drawer, ZZTOOPcomponent * components);

#endif
