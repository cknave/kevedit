/* register.h  -- text editor memory registers
 * $Id: register.h,v 1.4 2002/12/04 23:53:06 kvance Exp $
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
 * Foundation, Inc., 59 Temple Place Suite 330; Boston, MA 02111-1307, USA.
 */

#ifndef __REGISTER_H
#define __REGISTER_H 1


#include "svector.h"

/* yanktext() stores data into a register.
 * puttext() pulls data from a register, storing it in a stringvector
 * clearregister() empties a register
 * deleteregister() frees all memory used by registers
 */

void regyank(char whichreg, stringnode * startn, stringnode * endn, int startpos, int endpos);
void regstore(char whichreg, stringvector src);
int regput(char whichreg, stringvector * dest, int inspos, int wrapwidth, int editwidth);
void clearregister(char whichreg);
void deleteregisters(void);

#endif
