/* infobox.h - board/world information dialogs
 * $Id: infobox.h,v 1.1 2003/11/01 23:45:56 bitman Exp $
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

#ifndef __INFOBOX_H
#define __INFOBOX_H

#include "libzzt2/zzt.h"
#include "display/display.h"
#include "kevedit/kevedit.h"

/* editboardinfo() - brings up dialog box for editing board info */
void editboardinfo(ZZTworld* myworld, displaymethod* d);

/* editworldinfo() - brings up dialog box for editing world info */
void editworldinfo(ZZTworld* myworld, displaymethod* d);

#endif
