/**@file texteditor/select.h  Text editor selection, copy, and paste.
 * $Id: select.h,v 1.1 2003/12/21 03:21:29 bitman Exp $
 * @author Ryan Phillips
 *
 * Copyright (C) 2003 Ryan Phillips <bitman@users.sf.net>
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

#include "texteditor.h"

void texteditHandleCopy(texteditor * editor);
void texteditHandleSelection(texteditor * editor);

void texteditClearSelectedText(texteditor * editor);
void texteditPaste(texteditor * editor);


