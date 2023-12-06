/* params.c	-- The evil tile params
 * $Id: params.c,v 1.3 2005/06/29 03:20:34 kvance Exp $
 * Copyright (C) 2001 Kev Vance <kvance@kvance.com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "zzt.h"

/* The all-powerful min/max/swap macros */
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define swap(a, b, type) { type c = (a); (a) = (b); (b) = c; }

/* Param profile look-up table */
/* THIS IS ZZT IN A NUT-SHELL! */
const ZZTprofile _zzt_param_profile_table[] = {
	/* Type                  { properties, default cycle, data uses } */
	/* ZZT_EMPTY          */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_EDGE           */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_MESSAGETIMER   */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_MONITOR        */ { ZZT_PROPERTY_CYCLE, 1, { 0, 0, 0 } },
	/* ZZT_PLAYER         */ { ZZT_PROPERTY_CYCLE, 1, { 0, 0, 0 } },
	/* ZZT_AMMO           */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_TORCH          */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_GEM            */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_KEY            */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_DOOR           */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_SCROLL         */ { ZZT_PROPERTY_CYCLE | ZZT_PROPERTY_PROGRAM, 1, { 0, 0, 0 } },
	/* ZZT_PASSAGE        */ { ZZT_PROPERTY_NONE, 0, { 0, 0, ZZT_DATAUSE_PASSAGEDEST } },
	/* ZZT_DUPLICATOR     */ { ZZT_PROPERTY_STEP, 1, { 0, ZZT_DATAUSE_DUPRATE, 0 } },
	/* ZZT_BOMB           */ { ZZT_PROPERTY_CYCLE, 6, { ZZT_DATAUSE_TIMELEFT, 0, 0 } },
	/* ZZT_ENERGIZER      */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_STAR           */ { ZZT_PROPERTY_STEP | ZZT_PROPERTY_CYCLE, 1, { 0, 0, 0 } },
	/* ZZT_CWCONV         */ { ZZT_PROPERTY_CYCLE, 3, { 0, 0, 0 } },
	/* ZZT_CCWCONV        */ { ZZT_PROPERTY_CYCLE, 2, { 0, 0, 0 } },
	/* ZZT_BULLET         */ { ZZT_PROPERTY_STEP | ZZT_PROPERTY_CYCLE, 1, { ZZT_DATAUSE_OWNER, 0, 0 } },
	/* ZZT_WATER          */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_FOREST         */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_SOLID          */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_NORMAL         */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_BREAKABLE      */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_BOULDER        */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_NSSLIDER       */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_EWSLIDER       */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_FAKE           */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_INVISIBLE      */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_BLINK          */ { ZZT_PROPERTY_CYCLE | ZZT_PROPERTY_STEP, 1, { ZZT_DATAUSE_STARTTIME, ZZT_DATAUSE_PERIOD, 0 } },
	/* ZZT_TRANSPORTER    */ { ZZT_PROPERTY_CYCLE | ZZT_PROPERTY_STEP, 2, { 0, 0, 0 } },
	/* ZZT_LINE           */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_RICOCHET       */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_BLINKHORIZ     */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_BEAR           */ { ZZT_PROPERTY_CYCLE, 3, { ZZT_DATAUSE_SENSITIVITY, 0, 0 } },
	/* ZZT_RUFFIAN        */ { ZZT_PROPERTY_CYCLE, 1, { ZZT_DATAUSE_INTELLIGENCE, ZZT_DATAUSE_RESTTIME, 0 } },
	/* ZZT_OBJECT         */ { ZZT_PROPERTY_CYCLE | ZZT_PROPERTY_PROGRAM, 3, { ZZT_DATAUSE_CHAR, ZZT_DATAUSE_LOCKED, 0 } },
	/* ZZT_SLIME          */ { ZZT_PROPERTY_CYCLE, 3, { 0, ZZT_DATAUSE_SPEED, 0 } },
	/* ZZT_SHARK          */ { ZZT_PROPERTY_CYCLE, 3, { ZZT_DATAUSE_INTELLIGENCE, 0, 0 } },
	/* ZZT_SPINNINGGUN    */ { ZZT_PROPERTY_CYCLE, 3, { ZZT_DATAUSE_INTELLIGENCE, ZZT_DATAUSE_FIRERATEMODE, 0 } },
	/* ZZT_PUSHER         */ { ZZT_PROPERTY_CYCLE | ZZT_PROPERTY_STEP, 4, { 0, 0, 0 } },
	/* ZZT_LION           */ { ZZT_PROPERTY_CYCLE, 2, { ZZT_DATAUSE_INTELLIGENCE, 0, 0 } },
	/* ZZT_TIGER          */ { ZZT_PROPERTY_CYCLE, 2, { ZZT_DATAUSE_INTELLIGENCE, ZZT_DATAUSE_FIRERATEMODE, 0 } },
	/* ZZT_BLINKVERT      */ { 0, 0, { 0, 0, 0 } },
	/* ZZT_CENTHEAD       */ { ZZT_PROPERTY_CYCLE | ZZT_PROPERTY_LEADER, 2, { ZZT_DATAUSE_INTELLIGENCE, ZZT_DATAUSE_DEVIANCE, 0 } },
	/* ZZT_CENTBODY       */ { ZZT_PROPERTY_CYCLE | ZZT_PROPERTY_LEADER, 2, { 0, 0, 0 } }
};

/* Data[] use default value look-up table */
uint16_t _zzt_datause_default_table[] = {
	/* ZZT_DATAUSE_NONE         */ 0,
	/* ZZT_DATAUSE_PASSAGEDEST  */ 0,
	/* ZZT_DATAUSE_DUPRATE      */ 4,
	/* ZZT_DATAUSE_TIMELEFT     */ 0,
	/* ZZT_DATAUSE_SENSITIVITY  */ 8,
	/* ZZT_DATAUSE_INTELLIGENCE */ 4,
	/* ZZT_DATAUSE_RESTTIME     */ 4,
	/* ZZT_DATAUSE_CHAR         */ 1,
	/* ZZT_DATAUSE_LOCKED       */ 0,
	/* ZZT_DATAUSE_SPEED        */ 4,
	/* ZZT_DATAUSE_FIRERATEMODE */ 4,
	/* ZZT_DATAUSE_DEVIANCE     */ 4,
	/* ZZT_DATAUSE_STARTTIME    */ 4,
	/* ZZT_DATAUSE_PERIOD       */ 4,
	/* ZZT_DATAUSE_OWNER        */ 1,
};

/* Names for each data use */
char * _zzt_datause_name_table[] = {
	/* ZZT_DATAUSE_NONE         */ "",
	/* ZZT_DATAUSE_PASSAGEDEST  */ "Passage Destination",
	/* ZZT_DATAUSE_DUPRATE      */ "Duplication Rate",
	/* ZZT_DATAUSE_TIMELEFT     */ "Time Remaining",
	/* ZZT_DATAUSE_SENSITIVITY  */ "Sensitivity",
	/* ZZT_DATAUSE_INTELLIGENCE */ "Intelligence",
	/* ZZT_DATAUSE_RESTTIME     */ "Rest Time",
	/* ZZT_DATAUSE_CHAR         */ "Character",
	/* ZZT_DATAUSE_LOCKED       */ "Locked",
	/* ZZT_DATAUSE_SPEED        */ "Speed",
	/* ZZT_DATAUSE_FIRERATEMODE */ "Fire Rate",
	/* ZZT_DATAUSE_DEVIANCE     */ "Deviance",
	/* ZZT_DATAUSE_STARTTIME    */ "Start Time",
	/* ZZT_DATAUSE_PERIOD       */ "Period",
	/* ZZT_DATAUSE_OWNER        */ "Fired By",
};

/* Which data[] element does each datause occur in? */
uint16_t _zzt_datause_location_table[] = {
	/* ZZT_DATAUSE_NONE         */ 0,
	/* ZZT_DATAUSE_PASSAGEDEST  */ 2,
	/* ZZT_DATAUSE_DUPRATE      */ 1,
	/* ZZT_DATAUSE_TIMELEFT     */ 0,
	/* ZZT_DATAUSE_SENSITIVITY  */ 0,
	/* ZZT_DATAUSE_INTELLIGENCE */ 0,
	/* ZZT_DATAUSE_RESTTIME     */ 1,
	/* ZZT_DATAUSE_CHAR         */ 0,
	/* ZZT_DATAUSE_LOCKED       */ 1,
	/* ZZT_DATAUSE_SPEED        */ 1,
	/* ZZT_DATAUSE_FIRERATEMODE */ 1,
	/* ZZT_DATAUSE_DEVIANCE     */ 1,
	/* ZZT_DATAUSE_STARTTIME    */ 0,
	/* ZZT_DATAUSE_PERIOD       */ 1,
	/* ZZT_DATAUSE_OWNER        */ 0,
};

/* Set the program hash of a ZZT param using the FNV-1a hash. */
void zztParamRehash(ZZTparam * param)
{
	const uint32_t FNV_32_PRIME = 0x01000193; /* 16777619 */

	uint32_t h = 0x811c9dc5; /* 2166136261 */
	size_t i;

	for (i = 0; i < param->length; ++i) {
		/* xor the bottom with the current octet */
		h ^= param->program[i];
		/* multiply by the 32 bit FNV magic prime mod 2^32 */
		h *= FNV_32_PRIME;
	}

	param->program_hash = h;
}

int zztParamEqualProgram(const ZZTparam * p1, const ZZTparam * p2)
{
	if (p1->length != p2->length) {
		return 0;
	}

	if (p1->program_hash != p2->program_hash) {
		return 0;
	}

	if (memcmp(p1->program, p2->program, p1->length) == 0) {
		return 1;
	}
	return 0;
}

ZZTparam *zztParamCreateBlank(void)
{
	ZZTparam *param;

	/* Allocate the param structure */
	param = (ZZTparam *) malloc(sizeof(ZZTparam));

	/* Zero-ify */
	param->index = -1;
	param->x = 0;
	param->y = 0;
	param->xstep = 0;
	param->ystep = 0;
	param->cycle = 0;
	memset(param->data, 0, sizeof(param->data));
	param->leaderindex = -1;
	param->followerindex = -1;
	param->utype = ZZT_EMPTY;
	param->ucolor = 0x0F;
	memset(param->magic, 0, sizeof(param->magic));
	param->instruction = 0;
	param->length = 0;
	param->program = NULL;
	param->bindindex = 0;

	zztParamRehash(param);

	return param;
}

ZZTparam *zztParamCreate(ZZTtile tile)
{
	ZZTparam *param;
	ZZTprofile profile = _zzt_param_profile_table[tile.type];
	int i;

	/* No params for tiles outside the profile table */
	if (tile.type > ZZT_CENTBODY)
		return NULL;

	/* No params for param-less types */
	if (profile.properties == ZZT_PROPERTY_NOPARAM)
		return NULL;

	param = zztParamCreateBlank();

	/* Consider profile for given tile type */

	if (profile.properties & ZZT_PROPERTY_STEP)
		param->ystep = -1;  /* North by default */

	param->cycle = profile.cycledefault;

	/* Consider each data[] element */
	for (i = 0; i < 3; i++) {
		int datause = profile.datause[i];
		param->data[i] = _zzt_datause_default_table[datause];
	}

	return param;
}

int zztParamFree(ZZTparam *param)
{
	if (param->program)
		free(param->program);
	free(param);
	return 1;
}

int zztParamCopyPtr(ZZTparam *dest, ZZTparam *src)
{
	if (src == NULL || dest == NULL)
		return 0;

	memcpy(dest, src, sizeof(ZZTparam));
	if (src->program != NULL) {
		/* dup. the data, too */
		dest->program = (uint8_t *) malloc(src->length);
		if (dest->program == NULL)
			return 0;
		memcpy(dest->program, src->program, src->length);
		dest->program_hash = src->program_hash;
	}
	return 1;
}

ZZTparam *zztParamDuplicate(ZZTparam *param)
{
	ZZTparam* dup = NULL;

	/* don't duplicate null params */
	if (param == NULL)
		return NULL;

	dup = (ZZTparam *) malloc(sizeof(ZZTparam));
	zztParamCopyPtr(dup, param);
	return dup;
}

uint8_t zztParamGetProperties(ZZTtile tile)
{
	if (tile.type > ZZT_CENTBODY)
		return ZZT_PROPERTY_NOPARAM;

	return _zzt_param_profile_table[tile.type].properties;
}

int zztParamDatauseGet(ZZTtile tile, int which)
{
	if (which > 2 || tile.type > ZZT_CENTBODY)
		return ZZT_DATAUSE_NONE;

	return _zzt_param_profile_table[tile.type].datause[which];
}

const char *zztParamDatauseGetName(ZZTtile tile, int which)
{
	return _zzt_datause_name_table[zztParamDatauseGet(tile, which)];
}

int zztParamDatauseLocate(int datause)
{
	if (datause > ZZT_DATAUSE_MAX) return 0;
	return _zzt_datause_location_table[datause];
}

int zztParamGetProperty(ZZTparam * param, int property)
{
	return param->data[zztParamDatauseLocate(property)];
}

/* Fix potentially corrupted #BIND object references. KevEdit
 * itself can produce corrupted references due to other bugs;
 * this will work as a last measure while fixing those bugs, and
 * keep any unfixed bugs from causing weird behavior inside ZZT.
 *
 * An object being bound to another bound object is not allowed.
 * We'll try to resolve such so that all these objects are bound
 * to the object at the end of the chain. Cyclical references get
 * set to zero (unbound). */

/* We use a depth first search to resolve cycles and chains. */

/* BIND_UNVISITED: We haven't investigated this stat yet.
   BIND_VISITED: This stat has been visited in the current
	recursion; if we see it again, we have a cycle.
   BIND_PROCESSED_UNALTERED: This stat has been visited in a prior
	loop and need not be descended into again.
   BIND_PROCESSED_ALTERED: Same, but its bind index has been changed
	by the function.
*/

/* It would be nice to somehow report any such fixups when
 * loading/saving. But that would probably require a more thorough
 * modification with the read and write functions returning either
 * OK, warning, or error. */

const int BIND_UNVISITED = 0, BIND_VISITED = 1,
	BIND_PROCESSED_UNALTERED = 2,
	BIND_PROCESSED_ALTERED = 3;

/* This function is called with a stat's bind index and returns
 * its proper bind index once loops and chains have been dealt
 * with. */

int normalizeBindChainsDFS(ZZTparam ** params,
	int stat_num, int num_stats, int * stat_status) {

	/* If the stat num is out of bounds, return 0. */
	if (stat_num >= num_stats) {
		return 0;
	}

	ZZTparam * param = params[stat_num];

	/* If we're not bound, then just return our value and
	 * set PROCESSED. */
	if (param->bindindex == 0) {
		stat_status[stat_num] = BIND_PROCESSED_UNALTERED;
		return stat_num;
	}

	/* If we get a VISITED, we've been here before, i.e.
	 * we're part of a cycle. Cancel out this part of the
	 * cycle and return 0. */
	if (stat_status[stat_num] == BIND_VISITED) {
		param->bindindex = 0;
		stat_status[stat_num] == BIND_PROCESSED_ALTERED;
		return 0;
	}

	/* If we get a PROCESSED, it's bound with one step
	 * (otherwise the first if check would have fired.)
	 * Return what it's bound to. */

	if (stat_status[stat_num] == BIND_PROCESSED_UNALTERED ||
	    stat_status[stat_num] == BIND_PROCESSED_ALTERED) {
		return param->bindindex;
	}

	/* Set the current node as visited and recurse down
	 * what we bind to. */
	stat_status[stat_num] = BIND_VISITED;
	int end_of_bind = normalizeBindChainsDFS(params,
		param->bindindex, num_stats, stat_status);
	stat_status[stat_num] = BIND_PROCESSED_UNALTERED;
	if (param->bindindex != end_of_bind) {
		param->bindindex = end_of_bind;
		stat_status[stat_num] = BIND_PROCESSED_ALTERED;
	}

	/* If this stat has code, it's most likely the source
	 * for some upstream bound objects. So if its current
	 * bind index is zero but it has code, then return its
	 * ID instead. This is a compromise way to resolve cycles
	 * that should work without having to drag in heavy hitters
	 * like Kosaraju. */
	if (param->bindindex == 0 && param->length > 0) {
		return stat_num;
	}

	return end_of_bind;
}

/* This function fixes corrupted bind indices. It also returns the
 * number of modified bind indices, which should be useful for later
 * debugging and tracking down bugs that invalidate bind indices. */
int zztParamsNormalizeBindChains(ZZTparam ** params, uint16_t paramcount) {
	int * stat_status = malloc(sizeof(int) * paramcount);
	memset(stat_status, BIND_UNVISITED, sizeof(int) * paramcount);

	int stat_num;

	for (stat_num = 0; stat_num < paramcount;
		++stat_num) {
		ZZTparam * param = params[stat_num];

		/* Getting a BIND_VISITED is a bug as we should
		 * always clean up after ourselves. */
		assert(stat_status[stat_num] != BIND_VISITED);

		/* No need to do anything with a stat that isn't bound
		   to anything. */
		if (param->bindindex == 0) {
			continue;
		}

		/* ... or one we've already processed. */
		if (stat_status[stat_num] == BIND_PROCESSED_UNALTERED
			|| stat_status[stat_num] == BIND_PROCESSED_ALTERED) {
			continue;
		}

		int end_of_bind = normalizeBindChainsDFS(
			params, param->bindindex,
			paramcount, stat_status);

		if (end_of_bind == param->bindindex) {
			stat_status[stat_num] = BIND_PROCESSED_UNALTERED;
		} else {
			param->bindindex = end_of_bind;
			stat_status[stat_num] = BIND_PROCESSED_ALTERED;
		}
	}

	/* Count the number of altered bind indices */
	int altered_indices = 0;

	for (stat_num = 0; stat_num < paramcount; ++stat_num) {
		if (stat_status[stat_num] == BIND_PROCESSED_ALTERED) {
			++altered_indices;
		}
	}

	free(stat_status);

	return altered_indices;
}

void zztParamsFixBindOrder(ZZTparam ** params, uint16_t paramcount)
{
	/* Because ZZT automatically copies the data pointer of a source
	 * param to the bound param when it encounters a bound object,
	 * bound params should come after the params they're bound to.
	 * This rearranges the stat order by swapping params that are
	 * out of order, and using a bind map to update the indices. */

	int * bind_map = malloc(paramcount * sizeof(int));
	memset(bind_map, 0, paramcount * sizeof(int));

	int i;

	for (i = 0; i < paramcount; ++i) {
		ZZTparam * param = params[i];

		/* If it's bound to something that has changed place,
		 * update it. */
		if (bind_map[param->bindindex] != 0) {
			param->bindindex = bind_map[param->bindindex];
		}

		/* If the player is bound, just set to zero as it can't
		 * be moved. */
		if (i == 0 && param->bindindex > i) {
			param->bindindex = 0;
			continue;
		}

		/* If it's bound to something that comes later, swap. */
		if (param->bindindex > i) {
			ZZTparam * source = params[param->bindindex];

			/* Update param - the bound param - to refer to the ith
			 * place in the param order. */

			bind_map[param->bindindex] = i;
			source->bindindex = i;
			param->bindindex = 0;

			/* Then swap programs so the source will be in
			 * ith place. */
			swap(param->program, source->program, uint8_t*);
			swap(param->length, source->length, uint16_t);
			swap(param->program_hash,
				source->program_hash, uint32_t);
		}
	}

	free(bind_map); /* cleanup */
}