/* display_vcsa.c       -- Functions for the Linux console display method
 * $Id: display_vcsa.c,v 1.2 2001/10/27 19:30:42 kvance Exp $
 * Copyright (C) 2000 Kev Vance <kev@kvance.com>
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

#include <stdlib.h>

#include "display.h"
#include "display_vcsa.h"

int display_vcsa_init()
{
}

void display_vcsa_end()
{
}

displaymethod display_vcsa =
{
	NULL,
	"Linux Console Display Method",
	"1.0",
	display_vcsa_init,
	display_vcsa_end
};
