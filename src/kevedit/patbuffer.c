/* patbuffer.c    -- Pattern buffer (backbuffer) utilities
 * $Id: patbuffer.c,v 1.3 2005/06/29 03:20:34 kvance Exp $
 * Copyright (C) 2000 Kev Vance <kvance@kvance.com>
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

#include "kevedit.h"
#include "libzzt2/zzt.h"
#include "display/display.h"
#include "screen.h"

#include <stdlib.h>
#include <string.h>


/* Create an empty patbuffer of given size */
patbuffer* patbuffer_create(int size)
{
	int i;
	patbuffer* pbuf = NULL;

	if (size <= 0)
		return NULL;

	pbuf = (patbuffer *) malloc(sizeof(patbuffer));

	pbuf->patterns = (ZZTtile *) malloc(sizeof(ZZTtile) * size);
	pbuf->size = size;
	pbuf->pos = 0;

	for (i = 0; i < pbuf->size; i++) {
		pbuf->patterns[i].type = ZZT_EMPTY;
		pbuf->patterns[i].color = 0x07;
		pbuf->patterns[i].param = NULL;
	}

	/* Unlocked by default */
	pbuf->lock = PATBUF_UNLOCK;

	return pbuf;
}

void deletepatternbuffer(patbuffer* pbuf)
{
	int i;

	/* Free all the patterns with param data */
	for (i = 0; i < pbuf->size; i++) {
		if (pbuf->patterns[i].param != NULL) {
			zztParamFree(pbuf->patterns[i].param);
		}
	}

	/* Free the patterns themselves */
	free(pbuf->patterns);
	pbuf->patterns = NULL;

	/* Free the patbuffer */
	free(pbuf);
}

void patbuffer_resize(patbuffer * pbuf, int delta)
{
	int i;
	int resize = pbuf->size + delta;
	ZZTtile * repat = NULL;

	if (resize <= 0)
		return;

	repat = (ZZTtile *) malloc(sizeof(ZZTtile) * resize);

	/* Copy over all the patterns. To save time, we won't bother the param
	 * data for those patterns which remain in the new buffer. */
	for (i = 0; i < resize && i < pbuf->size; i++)
		repat[i] = pbuf->patterns[i];

	/* pbuf shrunk: clear all param data beyond the new size */
	for (; i < pbuf->size; i++) {
		if (pbuf->patterns[i].param != NULL) {
			if (pbuf->patterns[i].param->program != NULL)
				free(pbuf->patterns[i].param->program);
			free(pbuf->patterns[i].param);
		}
	}

	/* pbuf grew: fill the new slots with empties */
	for (; i < resize; i++) {
		repat[i].type = ZZT_EMPTY;
		repat[i].color = 0x07;
		repat[i].param = NULL;
	}

	/* Out with the old, in with the new */
	free(pbuf->patterns);
	pbuf->patterns = repat;
	pbuf->size = resize;

	if (pbuf->pos >= pbuf->size)
		pbuf->pos = pbuf->size - 1;
}


void pat_applycolordata(patbuffer * pbuf, textcolor color)
{
	int i, tilecolor;

	tilecolor = encodecolor(color);

	for (i = 0; i < pbuf->size; i++)
		pbuf->patterns[i].color = tilecolor;
}


void push(patbuffer* pbuf, ZZTtile pattern)
{
	/* Push the given pattern attributes into a pattern buffer */
	int i;
	ZZTtile lastpat = pbuf->patterns[pbuf->size-1];

	/* No fair pushing onto a locked buffer */
	if (pbuf->lock & PATBUF_NOPUSH)
		return;

	if (lastpat.param != NULL) {
		zztParamFree(lastpat.param);
	}
	
	/* Slide everything forward */
	for (i = pbuf->size - 1; i > 0; i--) {
		pbuf->patterns[i] = pbuf->patterns[i - 1];
	}

	pbuf->patterns[0].type = pattern.type;
	pbuf->patterns[0].color = pattern.color;
	pbuf->patterns[0].param = zztParamDuplicate(pattern.param);

	/* Reset utype and ucolor */
	if (pbuf->patterns[0].param != NULL) {
		pbuf->patterns[0].param->utype  = ZZT_EMPTY;
		pbuf->patterns[0].param->ucolor = 0x0F;
	}
}

void patreplace(patbuffer * pbuf, ZZTtile pattern)
{
	/* Replace the current buffer pattern with the given pattern */

	/* No fair replacing in a locked buffer */
	if (pbuf->lock & PATBUF_NOREPLACE)
		return;

	/* Remain within bounds */
	if (pbuf->pos < 0 || pbuf->pos >= pbuf->size)
		return;

	if (pbuf->patterns[pbuf->pos].param != NULL)
		zztParamFree(pbuf->patterns[pbuf->pos].param);

	pbuf->patterns[pbuf->pos].type = pattern.type;
	pbuf->patterns[pbuf->pos].color = pattern.color;
	pbuf->patterns[pbuf->pos].param = zztParamDuplicate(pattern.param);

	/* Reset utype and ucolor */
	if (pbuf->patterns[pbuf->pos].param != NULL) {
		pbuf->patterns[pbuf->pos].param->utype  = ZZT_EMPTY;
		pbuf->patterns[pbuf->pos].param->ucolor = 0x0F;
	}
}

