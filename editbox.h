/* editbox.h  -- text editor/viewer in kevedit
 * $Id: editbox.h,v 1.2 2000/08/19 21:41:49 kvance Exp $
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

#ifndef __EDITBOX_H
#define __EDITBOX_H 1

#include "zzt.h"
#include "display.h"
#include "svector.h"

/* editmoredata - edits p->moredata in a scroll box */
void editmoredata(displaymethod * d, param * p);

/* editbox - display/edit a string vector in a scroll box, starting
 * at sv->cur. editwidth tells how long a line can be & how much memory to use
 * on new lines. If editwidth is zero, editsvector acts as a listbox and sets
 * sv->cur to selected node.  If zochighlight is nonzero, it does ZZT Object
 * Code (ZOC) highlighting. */
void editbox(displaymethod * d, char *title, stringvector * sv, int editwidth, int zochighlight);

/* displayzoc - display a string with zoc highlighting */
void displayzoc(displaymethod * d, int x, int y, unsigned char *s);

#endif
