/* menu.c       -- Code for using the F1-3 panels
 * $Id: menu.c,v 1.8 2002/02/16 10:25:22 bitman Exp $
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

#include "menu.h"

#include "kevedit.h"
#include "screen.h"
#include "editbox.h"

#include "libzzt2/zzt.h"
#include "svector.h"
#include "files.h"
#include "zzl.h"
#include "hypertxt.h"

#include "patbuffer.h"

#include "display.h"

#include <stdlib.h>

/* Should be obsolete now */
#if 0
void plotbackbufferfirstslot(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
{
	int oldpos;
	patbuffer* prevbuf = myinfo->pbuf;

	myinfo->pbuf = myinfo->backbuffer;
	oldpos = myinfo->pbuf->pos;
	myinfo->pbuf->pos = 0;

	plot(myworld, myinfo, mydisplay);

	myinfo->pbuf->pos = oldpos;
	myinfo->pbuf = prevbuf;
}
#endif
		
void itemmenu(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
{
#if 0
	int i, x, t;
	param *pm;
#endif
	ZZTtile tile = { ZZT_EMPTY, 0x0, NULL };
	int choice; /* Need signed type */

	tile.color = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);

#if 0
	if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
		return;
#endif

	choice = dothepanel_f1(mydisplay, myinfo);
	tile.type = choice;
	if (tile.type == ZZT_PLAYER) {
		/* The player is a special case */
		zztPlotPlayer(myworld, myinfo->cursorx, myinfo->cursory);
#if 0
		bigboard[(myinfo->playerx + myinfo->playery * 60) * 2] = ZZT_EMPTY;
		bigboard[(myinfo->playerx + myinfo->playery * 60) * 2 + 1] = 0x07;
		if (paramlist[myinfo->cursorx][myinfo->cursory] != 0) {
			/* We're overwriting a parameter */
			param_remove(myworld->board[myinfo->curboard], paramlist, myinfo->cursorx, myinfo->cursory);
		}
		bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] = ZZT_PLAYER;
		bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = 0x1f;
		paramlist[myinfo->cursorx][myinfo->cursory] = 0;
		myworld->board[myinfo->curboard]->params[0]->x = myinfo->playerx = myinfo->cursorx;
		myworld->board[myinfo->curboard]->params[0]->y = myinfo->playery = myinfo->cursory;
		myworld->board[myinfo->curboard]->params[0]->x++;
		myworld->board[myinfo->curboard]->params[0]->y++;
#endif
	} else {
		switch (choice) {
		case -1:
			break;
		case ZZT_GEM:
		case ZZT_KEY:
#if 0
			push(myinfo->backbuffer, i, (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80), NULL);
#endif
			break;
		case ZZT_AMMO:
		case ZZT_TORCH:
		case ZZT_ENERGIZER:
#if 0
			if (myinfo->defc == 0)
				tile.color = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
#endif
			/* TODO: we can do better than if's in a switch */
			if (myinfo->defc != 0) {
				if (tile.type == ZZT_AMMO)
					tile.color = 0x03;
				if (tile.type == ZZT_TORCH)
					tile.color = 0x06;
				if (tile.type == ZZT_ENERGIZER)
					tile.color = 0x05;
			}
#if 0
			push(myinfo->backbuffer, i, x, NULL);
#endif
			break;
		case ZZT_DOOR:
			if (myinfo->defc != 0)
				tile.color = myinfo->forec > 7 ? ((myinfo->forec - 8) << 4) + 0x0f : (myinfo->forec << 4) + 0x0f;
#if 0
			else
				tile.color = (myinfo->backc << 4) + (myinfo->forec) + (myinfo->blinkmode * 0x80);
			push(myinfo->backbuffer, i, x, NULL);
#endif
			break;
		case ZZT_SCROLL:
#if 0
			/* This is all automatic now */
			if (myworld->board[myinfo->curboard]->info->objectcount == 150) {
				i = -1;
				break;
			} else {
				/* Anything important under it? */
				x = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2];
				switch (x) {
				case ZZT_WATER:
				case ZZT_FAKE:
					break;
				default:
					x = ZZT_EMPTY;
					break;
				}
			pm = z_newparam_scroll(myinfo->cursorx + 1, myinfo->cursory + 1, x, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1]);
			t = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
			push(myinfo->backbuffer, i, t, pm);
			break;
			}
#endif
			/* TODO: This is so not right */
			tile.param = NULL;
		case ZZT_PASSAGE:
#if 0
			pm = z_newparam_passage(myinfo->cursorx + 1, myinfo->cursory + 1, boarddialog(myworld, myinfo->curboard, 0, "Passage Destination", mydisplay));
#endif
			if (myinfo->defc != 0)
				tile.color = myinfo->forec > 7 ? ((myinfo->forec - 8) << 4) + 0x0f : (myinfo->forec << 4) + 0x0f;
#if 0
			else
				x = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
			push(myinfo->backbuffer, i, x, pm);
#endif
			break;
		case ZZT_DUPLICATOR:
#if 0
			/* Anything important under it? */
			t = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2];
			switch (t) {
				case ZZT_WATER:
				case ZZT_FAKE:
					break;
				default:
					t = ZZT_EMPTY;
					break;
			}
			pm = z_newparam_duplicator(myinfo->cursorx + 1, myinfo->cursory + 1, -1, 0, 4, t, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2]);
#endif
			if(myinfo->defc != 0)
				tile.color = 0x0f;
#if 0
			else
				x = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
			push(myinfo->backbuffer, i, x, pm);
#endif
			/* TODO: actually create params for this thing */
			tile.param = NULL;
			break;
		case ZZT_CWCONV:
		case ZZT_CCWCONV:
#if 0
			pm = z_newparam_conveyer(myinfo->cursorx + 1, myinfo->cursory + 1);
			push(myinfo->backbuffer, i, (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80), pm);
#endif
			/* TODO: param */
			break;
		case ZZT_BOMB:
#if 0
			pm = z_newparam_bomb(myinfo->cursorx + 1, myinfo->cursory + 1);
			push(myinfo->backbuffer, i, (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80), pm);
#endif
			/* TODO: param */
			break;
		}
		if (choice != -1 && tile.type != ZZT_PLAYER) {
			zztPlot(myworld, myinfo->cursorx, myinfo->cursory, tile);
			push(myinfo->backbuffer, tile);
			if (tile.param != NULL)
				zztParamFree(tile.param);
#if 0
			plotbackbufferfirstslot(mydisplay, myworld, myinfo, bigboard, paramlist);
#endif
		}
	}
}


void creaturemenu(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
{
#if 0
	int i, x, t;
	param *pm;
#endif
	ZZTtile tile = { ZZT_EMPTY, 0x0, NULL };
	int choice; /* Need signed type */

	tile.color = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);

#if 0
	if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
		return;

	/* All these need parameter space */
	if (myworld->board[myinfo->curboard]->info->objectcount == 150)
		return;
#endif

	choice = dothepanel_f2(mydisplay, myinfo);
	tile.type = choice;

#if 0
	/* Anything important under it? */
	x = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2];
	switch (x) {
		case ZZT_WATER:
		case ZZT_FAKE:
			break;
		default:
			x = ZZT_EMPTY;
			break;
	}
#endif
	switch (choice) {
	case -1:
		break;
	case ZZT_BEAR:
#if 0
		pm = z_newparam_bear(myinfo->cursorx + 1, myinfo->cursory + 1, x, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], 4);
#endif
		if(myinfo->defc == 1)
			tile.color = 0x06;
#if 0
		else
			t = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
		push(myinfo->backbuffer, i, t, pm);
#endif
		/* TODO: param */
		break;
	case ZZT_RUFFIAN:
#if 0
		pm = z_newparam_ruffian(myinfo->cursorx + 1, myinfo->cursory + 1, 4, 4);
#endif
		if(myinfo->defc == 1)
			tile.color = 0x0d;
#if 0
		else
			t = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
		push(myinfo->backbuffer, i, t, pm);
#endif
		/* TODO: param */
		break;
	case ZZT_SLIME:
#if 0
		pm = z_newparam_slime(myinfo->cursorx + 1, myinfo->cursory + 1, 4);
		t = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
		push(myinfo->backbuffer, i, t, pm);
#endif
		/* TODO: param */
		break;
	case ZZT_SHARK:
#if 0
		pm = z_newparam_shark(myinfo->cursorx + 1, myinfo->cursory + 1, x, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], 4);
		if(myinfo->defc == 1) {
			t = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] & 0xf0;
			if(t > 0x70)
				t -= 0x80;
			t += 0x07;
		} else
			t = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
		push(myinfo->backbuffer, i, t, pm);
#endif
		if (myinfo->defc == 1) {
			tile.color = zztTileGet(myworld, myinfo->cursorx, myinfo->cursory).color & 0xf0;
			if (tile.color > 0x70)
				tile.color -= 0x80;
			tile.color += 0x07;
		}
		/* TODO: param */
		break;
	case ZZT_OBJECT:
#if 0
		pm = z_newparam_object(myinfo->cursorx + 1, myinfo->cursory + 1, charselect(mydisplay, -1), x, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1]);
		t = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
		push(myinfo->backbuffer, i, t, pm);
#endif
		/* TODO: param */
		break;
	}
	if (choice != -1) {
		zztPlot(myworld, myinfo->cursorx, myinfo->cursory, tile);
		push(myinfo->backbuffer, tile);
		if (tile.param != NULL)
			zztParamFree(tile.param);
#if 0
		plotbackbufferfirstslot(mydisplay, myworld, myinfo, bigboard, paramlist);
#endif
	}
}


void terrainmenu(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
{
#if 0
	int i, x;
#endif
	ZZTtile tile = { ZZT_EMPTY, 0x0, NULL };
	int choice; /* Need signed type */

	tile.color = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);

#if 0
	if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
		return;
#endif

	choice = dothepanel_f3(mydisplay, myinfo);
	tile.type = choice;

	switch (choice) {
		case -1:
			break;
		case ZZT_FAKE:
		case ZZT_SOLID:
		case ZZT_NORMAL:
		case ZZT_BREAKABLE:
		case ZZT_BOULDER:
		case ZZT_NSSLIDER:
		case ZZT_EWSLIDER:
		case ZZT_INVISIBLE:
#if 0
			push(myinfo->backbuffer, i, (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80), NULL);
#endif
			break;
		case ZZT_WATER:
		case ZZT_FOREST:
		case ZZT_RICOCHET:
#if 0
			if (myinfo->defc == 0)
				x = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
			else {
				if (i == ZZT_WATER)
					x = 0x9f;
				if (i == ZZT_FOREST)
					x = 0x20;
				if (i == ZZT_RICOCHET)
					x = 0x0a;
			}
			push(myinfo->backbuffer, i, x, NULL);
#endif
			break;
		case ZZT_EDGE:
#if 0
			push(myinfo->backbuffer, i, 0x07, NULL);
#endif
			tile.color = 0x07;
			break;
	}

	if (choice != -1) {
		zztPlot(myworld, myinfo->cursorx, myinfo->cursory, tile);
		push(myinfo->backbuffer, tile);
		if (tile.param != NULL)
			zztParamFree(tile.param);
#if 0
		plotbackbufferfirstslot(mydisplay, myworld, myinfo, bigboard, paramlist);
#endif
	}
}


void loadobjectlibrary(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
{
	char* filename;
	stringvector zzlv;
	ZZTtile tile;

#if 0
	patdef pd;

	/* No fair drawing over the player */
	if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
		return;
#endif
	/* TODO: check for player under cursor */

	/* Prompt for a file and load it */
	filename =
		filedialog(".", "zzl", "Select an Object Library", FTYPE_ALL, mydisplay);
	if (filename == NULL)
		return;
	zzlv = filetosvector(filename, EDITBOX_ZZTWIDTH*2, EDITBOX_ZZTWIDTH*2);
	free(filename);

	/* Have the user select an object */
	if (zzlpickobject(&zzlv, mydisplay) != EDITBOX_OK) {
		deletestringvector(&zzlv);
		return;
	}

	/* Put the object into the backbuffer and plot it to the screen */
	tile = zzlpullobject(zzlv, 0, 0, 0, 0);
	if (tile.type == ZZT_OBJECT) {
		zztPlot(myworld, myinfo->cursorx, myinfo->cursory, tile);
		push(myinfo->backbuffer, tile);
		if (tile.param != NULL)
			zztParamFree(tile.param);
#if 0
		push(myinfo->backbuffer, pd.type, pd.color, pd.patparam);
		plotbackbufferfirstslot(mydisplay, myworld, myinfo, bigboard, paramlist);
#endif
	}

	/* Finish */
	deletestringvector(&zzlv);
}

void saveobjecttolibrary(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
{
	char* filename;
	char* title;
	stringvector zzlv;
	ZZTtile obj = zztTileGet(myworld, myinfo->cursorx, myinfo->cursory);

	/* Can't save what's under the cursor unless it's an object */
	if (obj.type != ZZT_OBJECT)
		return;

	/* Prompt for and load a file */
	filename =
		filedialog(".", "zzl", "Save to Which Object Library?", FTYPE_ALL, mydisplay);
	if (filename == NULL)
		return;

	zzlv = filetosvector(filename, EDITBOX_ZZTWIDTH*2, EDITBOX_ZZTWIDTH*2);

	/* Prompt for a title */
	title = titledialog("Name Your Object", mydisplay);

#if 0
	/* Copy the object under the cursor onto obj */
	obj.type  = ZZT_OBJECT;
	obj.color = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1];
	obj.patparam = myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]];
#endif

	/* Append the object to the library */
	if (!zzlappendobject(&zzlv, obj, title, EDITBOX_ZZTWIDTH)) {
		/* If append was successful, save the file */
		svectortofile(&zzlv, filename);
	}

	deletestringvector(&zzlv);
	free(title);
	free(filename);
}

void saveobjecttonewlibrary(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
{
	char* filename;
	char* title;
	stringvector zzlv;
	ZZTtile obj = zztTileGet(myworld, myinfo->cursorx, myinfo->cursory);

	initstringvector(&zzlv);

	/* Can't save what's under the cursor unless it's an object */
	if (obj.type != ZZT_OBJECT)
		return;

	/* Prompt for file name */
	filename =
		filenamedialog("mylib.zzl", "zzl", "New Object Library", 1, mydisplay);
	if (filename == NULL)
		return;

	/* Generate the zzl header */
	pushstring(&zzlv, titledialog("Name Your Library", mydisplay));
	pushstring(&zzlv, str_dup("*"));
	pushstring(&zzlv, str_dup("* Format (by CyQ):"));
	pushstring(&zzlv, str_dup("* "));
	pushstring(&zzlv, str_dup("* library name"));
	pushstring(&zzlv, str_dup("* comments, starting with *"));
	pushstring(&zzlv, str_dup("* per object:"));
	pushstring(&zzlv, str_dup("* object name"));
	pushstring(&zzlv, str_dup("* lines in program, char,forground color,"
														"background color,x step,y step,cycle"));
	pushstring(&zzlv, str_dup("* program"));
	pushstring(&zzlv, str_dup("* "));

	/* Prompt for an object title */
	title = titledialog("Name Your Object", mydisplay);

#if 0
	/* Copy the object under the cursor onto obj */
	obj.type  = ZZT_OBJECT;
	obj.color = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1];
	obj.patparam = myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]];
#endif

	/* Append the object to the library */
	if (!zzlappendobject(&zzlv, obj, title, EDITBOX_ZZTWIDTH)) {
		/* If append was successful, save the file */
		svectortofile(&zzlv, filename);
	}

	deletestringvector(&zzlv);
	free(title);
	free(filename);
}

void objectlibrarymenu(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
{
	stringvector menu;
	char* choice;

	initstringvector(&menu);

	pushstring(&menu, "!load;Load Object from Library");
	pushstring(&menu, "!save;Save Object to Existing Library");
	pushstring(&menu, "!new;Save Object to New Library");

	do {
		if (scrolldialog("Object Library Menu", &menu, mydisplay) != EDITBOX_OK) {
			removestringvector(&menu);
			return;
		}
	} while (!ishypermessage(menu));

	choice = gethypermessage(menu);

	if (str_equ(choice, "load", 0))
		loadobjectlibrary(mydisplay, myworld, myinfo);
	else if (str_equ(choice, "save", 0))
		saveobjecttolibrary(mydisplay, myworld, myinfo);
	else if (str_equ(choice, "new", 0))
		saveobjecttonewlibrary(mydisplay, myworld, myinfo);

	removestringvector(&menu);
}
