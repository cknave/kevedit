/* display_ggi.c        -- Functions for the GGI display method
 * $Id: display_ggi.c,v 1.3 2002/12/04 23:53:06 kvance Exp $
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
 * Foundation, Inc., 59 Temple Place Suite 330; Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <ggi/ggi.h>

#include "display.h"
#include "display_ggi.h"

ggi_visual_t vis;
gii_input_t inp;

int display_ggi_init()
{
	ggiInit();
	giiInit();
	vis = ggiOpen(NULL);
	if (vis == NULL) {
		ggiPanic("Couldn't open default visual!\n");
		return 0;
	}
	if (ggiSetTextMode(vis, 80, 25, 80, 25, 8, 14, GT_TEXT) != 0) {
		/* Perhaps a graphics mode? */
		if (ggiSetSimpleMode(vis, 640, 350, 1, GT_AUTO) != 0) {
			return 0;
		}
	}
	if ((inp = giiOpen("input-stdin", NULL)) == NULL) {
		giiExit();
		ggiPanic("Input not available!\n");
		return 0;
	}
	return -1;
}

void display_ggi_end()
{
	ggiClose(vis);
	giiClose(inp);
	ggiExit();
	giiExit();
}

int display_ggi_getch()
{
	int i;
	struct timeval tv;
	gii_event ev;

	while (1) {
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		if (giiEventPoll(inp, emKey, &tv) != 0) {
			giiEventRead(inp, &ev, emKey);
			break;
		}
	}
	i = (int) ev.key.sym;
	return i;
}

displaymethod display_ggi =
{
	NULL,
	"GGI Display Method",
	"1.0",
	display_ggi_init,
	display_ggi_end,
	display_ggi_getch
};
