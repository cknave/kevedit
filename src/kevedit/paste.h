/* paste.h       -- Paste functionality, including rebinding
 * $Id: paste.h,v 1.6 2023/17/11 13:58:30 kristomu Exp $
 * Copyright (C) 2023 Kristofer Munsterhjelm <kristofer@munsterhjelm.no>
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

#ifndef _PASTE_H
#define _PASTE_H 1

#include <stdbool.h>

#include "kevedit.h"

#include "display/display.h"

#include "structures/selection.h"
#include "structures/gradient.h"
#include "structures/svector.h"

int paste(keveditor * myeditor);

bool pasteblock(ZZTblock *dest, const ZZTblock *src,
	selection destsel, selection srcsel, int x, int y);

#endif