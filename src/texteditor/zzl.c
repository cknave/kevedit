/* zzl.c  -- ZZT Object Library file routines
 * $Id: zzl.c,v 1.2 2005/05/28 03:17:46 bitman Exp $
 * Copyright (C) 2000 Ryan Phillips <bitman@users.sf.net>
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


#include "zzl.h"
#include "editbox.h"
#include "structures/svector.h"
#include "kevedit/kevedit.h"
#include "libzzt2/zzt.h"
#include "kevedit/misc.h"
#include "dialogs/paramed.h"

#include <string.h>
#include <stdio.h>

/* zzlpickobject() - presents a list of objects in the zzl file. zzlv->cur is
 *                   set to the title line of the selected object */
int zzlpickobject(stringvector * zzlv, displaymethod * d)
{
	stringvector namelist;
	stringnode* finder;
	int result;

	initstringvector(&namelist);

	if (zzlv->first == NULL || zzlv->first->next == NULL)
		return -1;

	finder = zzlv->first->next;

	while (finder != NULL && finder->s[0] == '*')
		finder = finder->next;

	if (finder == NULL)
		return -1;

	/* Gather a listing of object names */
	do {
		pushstring(&namelist, finder->s);

		finder = finder->next;
		if (finder != NULL) {
			int i, advanceby;
			sscanf(finder->s, "%d", &advanceby);

			finder = finder->next;
			for (i = 0; i < advanceby && finder != NULL; i++)
				finder = finder->next;
		}
	} while (finder != NULL);

	result = scrolldialog("Select An Object", &namelist, d);
	
	if (result == EDITBOX_OK) {
		finder = zzlv->first->next;
		while (finder != NULL && finder->s != namelist.cur->s)
			finder = finder->next;

		if (finder == NULL) {
			fprintf(stderr, "KevEdit bug: could not find zzl selection in list\n");
			removestringvector(&namelist);
			return -1;
		}

		/* Found it! */
		zzlv->cur = finder;
	}

	/* For the love of stability, don't free the strings! */
	removestringvector(&namelist);

	return result;
}

/* zzlpullobject() - pulls the object who's zzl definition starts on the
 *                   currentt line. */
ZZTtile zzlpullobject(stringvector zzlv, int x, int y, int undert, int underc)
{
	int lines, ch, fgcolour, bgcolour, xstep, ystep, cycle;
	ZZTtile result;
	ZZTparam objcodeparam;
	stringvector objectcode;
	stringnode* curline;
	int i;

	initstringvector(&objectcode);

	curline = zzlv.cur->next;

	/* Retrieve the specifications for this object */
	if (curline != NULL && curline->s != NULL) {
		if (sscanf(curline->s, "%d,%d,%d,%d,%d,%d,%d",
               &lines, &ch, &fgcolour, &bgcolour, &xstep, &ystep, &cycle) < 7) {
			result.type = -1;
			return result;
		}
	}

	/* Retrieve the object code */
	for (i = 0,       curline = curline->next;
			 i < lines && curline != NULL;
			 i++,         curline = curline->next) {
		pushstring(&objectcode, str_dup(curline->s));
	}

	objcodeparam = svectortoprogram(objectcode);

	/* Create the new pattern definition */
	result.type = ZZT_OBJECT;
	result.color = fgcolour | (bgcolour << 4);
	result.param = zztParamCreate(result);

	result.param->length  = objcodeparam.length;
	result.param->program = objcodeparam.program;

	result.param->data[0] = ch;
	result.param->xstep = xstep;
	result.param->ystep = ystep;
	result.param->cycle = cycle;
	/* TODO: param (use most of below): */
#if 0
	result.param = z_newparam_object(x, y, ch, undert, underc);
	result.param->length   = objcodeparam.length;
	result.param->moredata = objcodeparam.moredata;

	result.param->xstep = xstep;
	result.param->ystep = ystep;
	result.param->cycle = cycle;
#endif

	return result;
}

int zzlappendobject(stringvector * zzlv, ZZTtile obj, char* title, int editwidth)
{
	int lines, ch, fgcolour, bgcolour, xstep, ystep, cycle;
	char buffer[256];
	stringvector objcode;

	/* Convert the moredata to a string vector */
	objcode = programtosvector(obj.param, editwidth);

	/* Count the lines */
	lines = 0;
	for (objcode.cur = objcode.first;
			 objcode.cur != NULL;
			 objcode.cur = objcode.cur->next)
		lines++;

	/* Determine the colour */
	fgcolour =  obj.color & 0x0F;
	bgcolour = (obj.color & 0xF0) >> 4;

	/* Gather information to describe object */
	ch = obj.param->data[0];
	xstep = obj.param->xstep;
	ystep = obj.param->ystep;
	cycle = obj.param->cycle;

	sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d",
				  lines, ch, fgcolour, bgcolour, xstep, ystep, cycle);

	pushstring(zzlv, str_dupmin(title, editwidth));
	pushstring(zzlv, str_dupmin(buffer, editwidth));
	
	stringvectorcat(zzlv, &objcode);

	return 0;
}

