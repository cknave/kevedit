/* hypertxt.h  -- hypertext link system
 * $Id: hypertxt.h,v 1.1 2003/11/01 23:45:56 bitman Exp $
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

#ifndef __HYPERTXT_H
#define __HYPERTXT_H 1

#include "structures/svector.h"

/* Test for !message;text on current line of sv. !;text returns true */
int   ishypermessage(stringvector sv);

/* Retrieve message from sv->cur if hyperline into malloc()ed string */
char* gethypermessage(stringvector sv);

/* Move sv->cur to first instance of message msg in sv */
int   findhypermessage(char* msg, stringvector* sv);

/* Determines whether a message refers to a section as well */
int   ishypersection(char* msg);

/* Retrieve the section component of msg into malloc()ed string */
char* gethypersection(char* msg);

/* Retrieve the msg component of msg into malloc()ed string */
char* gethypersectionmessage(char* msg);

#endif
