/* tiles.c	-- All those ZZT tiles
 * $Id: tiles.c,v 1.1 2002/01/30 07:20:57 kvance Exp $
 * Copyright (C) 2001 Kev Vance <kev@kvance.com>
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

#include "zzt.h"

int zztParamInsert(ZZTworld *world, ZZTparam *param)
{
	ZZTparam *backup, *new;
	int pcount = zztBoardGetParamcount(world);
	int curboard = zztBoardGetCurrent(world);

	/* Check for max */
	if(pcount == 151)
		return 0;

	if(pcount != 0) {
		/* Make backup of existing param list */
		backup = malloc(sizeof(ZZTparam)*pcount);
		memcpy(backup, world->boards[curboard].params, sizeof(ZZTparam)*pcount);
		free(world->boards[curboard].params);
	}
	/* Allocate bigger param list */
	zztBoardSetParamcount(world, ++pcount);
	world->boards[curboard].params = malloc(sizeof(ZZTparam)*pcount);
	/* Restore existing param list */
	if(pcount != 1) {
		memcpy(world->boards[curboard].params, backup, sizeof(ZZTparam)*(pcount-1));
		free(backup);
	}

	/* Create new param */
	new = &world->boards[curboard].params[pcount-1];
	/* Copy data */
	memcpy(new, param, sizeof(ZZTparam));
	/* copy program */
	if(param->length != 0) {
		new->program = malloc(param->length);
		memcpy(new->program, param->program, param->length);
	}
	return 1;
}

int zztParamDelete(ZZTworld *world, int number)
{
	ZZTparam *backup, *p;
	int pcount = zztBoardGetParamcount(world);
	int curboard = zztBoardGetCurrent(world);
	int i;

	/* Check that it's in range */
	if(number < 0 || number >= pcount)
		return 0;

	/* Delete the param */
	p = &world->boards[curboard].params[number];
	if(p->length != 0)
		free(p->program);
	if(pcount != 1) {
		/* Make backup list */
		backup = malloc(sizeof(ZZTparam)*pcount);
		memcpy(backup, world->boards[curboard].params, sizeof(ZZTparam)*pcount);
		free(world->boards[curboard].params);
		/* Make new list */
		zztBoardSetParamcount(world, --pcount);
		world->boards[curboard].params = malloc(sizeof(ZZTboard)*pcount);
		/* Copy params before deleted one */
		for(i = 0; i < number; i++)
			memcpy(&world->boards[curboard].params[i], &backup[i], sizeof(ZZTparam));
		/* Copy params after deleted one */
		for(i = number+1; i < pcount+1; i++)
			memcpy(&world->boards[curboard].params[i-1], &backup[i], sizeof(ZZTparam));
		free(backup);
	} else {
		/* Delete the last param */
		free(world->boards[curboard].params);
		world->boards[curboard].params = NULL;
		zztBoardSetParamcount(world, 0);
	}
	return 1;
}

int zztParamDeleteAt(ZZTworld *world, int x, int y)
{
	ZZTparam *p = world->boards[zztBoardGetCurrent(world)].params;
	int pcount = zztBoardGetParamcount(world);
	int i;

	/* Cycle through, check for matching x+y */
	for(i = 0; i < pcount; i++) {
		if(p[i].x == x && p[i].y == y)
			return zztParamDelete(world, i);
	}
	/* Not found */
	return 0;
}

int zztPlot(ZZTworld *world, int x, int y, u_int8_t type, u_int8_t color)
{
	/* Check bounds */
	if(x < 0 || x >= ZZT_BOARD_X_SIZE || y < 0 || y >= ZZT_BOARD_Y_SIZE)
		return 0;

	world->bigboard[(x+y*ZZT_BOARD_X_SIZE)*2] = type;
	world->bigboard[(x+y*ZZT_BOARD_X_SIZE)*2+1] = color;
	return 1;
}

int zztPlotPlayer(ZZTworld *world, int x, int y)
{
	ZZTparam p;

	/* Check bounds */
	if(x < 0 || x >= ZZT_BOARD_X_SIZE || y < 0 || y >= ZZT_BOARD_Y_SIZE)
		return 0;
	
	/* Set up parameter */
	memset(&p, 0, sizeof(ZZTparam));
	p.x = x;
	p.y = y;
	p.cycle = 1;
	if(!zztParamInsert(world, &p))
		return 0;
	if(!zztPlot(world, x, y, ZZT_PLAYER, 0x1F))
		return 0;
	return 1;
}
