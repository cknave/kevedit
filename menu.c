/* menu.c       -- Code for using the F1-3 panels
 * $Id: menu.c,v 1.11 2002/03/17 09:35:58 bitman Exp $
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

void itemmenu(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
{
	ZZTtile tile = { ZZT_EMPTY, 0x0, NULL };
	int choice; /* Need signed type */

	tile.color = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);

	choice = dothepanel_f1(mydisplay, myinfo);
	tile.type = choice;

	/* Create params -- zztParamCreate() returns NULL for types which have no params */
	tile.param = zztParamCreate(tile);

	if (tile.type == ZZT_PLAYER) {
		/* The player is a special case */
		zztPlotPlayer(myworld, myinfo->cursorx, myinfo->cursory);
		return;
	}

	/* Use default color if in default color mode */
	if (myinfo->defc != 0) {
		switch (tile.type) {
			case ZZT_AMMO:      tile.color = 0x03; break;
			case ZZT_TORCH:     tile.color = 0x06; break;
			case ZZT_ENERGIZER: tile.color = 0x05; break;
			case ZZT_DOOR:
				tile.color = myinfo->forec > 7 ? ((myinfo->forec - 8) << 4) + 0x0f : (myinfo->forec << 4) + 0x0f;
				break;
			case ZZT_PASSAGE:
				tile.color = myinfo->forec > 7 ? ((myinfo->forec - 8) << 4) + 0x0f : (myinfo->forec << 4) + 0x0f;
				break;
			case ZZT_DUPLICATOR:
				tile.color = 0x0f;
				break;
		}
	}

	/* Plot and push the created tile */
	if (choice != -1) {
		zztPlot(myworld, myinfo->cursorx, myinfo->cursory, tile);
		push(myinfo->backbuffer, tile);

		/* Free our copy of the params */
		if (tile.param != NULL)
			zztParamFree(tile.param);
	}
}


void creaturemenu(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
{
	ZZTtile tile = { ZZT_EMPTY, 0x0, NULL };
	int choice; /* Need signed type */

	tile.color = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);

	choice = dothepanel_f2(mydisplay, myinfo);
	tile.type = choice;

	/* Everything has params */
	tile.param = zztParamCreate(tile);

	/* Use default color if in default color mode */
	if (myinfo->defc != 0) {
		switch (choice) {
			case ZZT_BEAR:
				tile.color = 0x06;
				break;
			case ZZT_RUFFIAN:
				tile.color = 0x0d;
				break;
			case ZZT_SHARK:
				tile.color = zztTileGet(myworld, myinfo->cursorx, myinfo->cursory).color & 0xf0;
				if (tile.color > 0x70)
					tile.color -= 0x80;
				tile.color += 0x07;
				break;
			case ZZT_LION:
				tile.color = 0x0C;
				break;
			case ZZT_TIGER:
				tile.color = 0x0B;
				break;
			case ZZT_BULLET:
				tile.color = 0x0F;
				break;
			case ZZT_STAR:
				tile.color = 0x0F;
				break;
		}
	}

	/* Plot and push the created tile */
	if (choice != -1) {
		zztPlot(myworld, myinfo->cursorx, myinfo->cursory, tile);
		push(myinfo->backbuffer, tile);
		
		/* Free our copy of the params */
		if (tile.param != NULL)
			zztParamFree(tile.param);
	}
}


void terrainmenu(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
{
	ZZTtile tile = { ZZT_EMPTY, 0x0, NULL };
	int choice; /* Need signed type */

	tile.color = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);

	choice = dothepanel_f3(mydisplay, myinfo);
	tile.type = choice;

	/* Nothing has params, but why not? */
	tile.param = zztParamCreate(tile);

	/* Plot and push the created tile */
	if (choice != -1) {
		zztPlot(myworld, myinfo->cursorx, myinfo->cursory, tile);
		push(myinfo->backbuffer, tile);

		/* Free our copy of the params */
		if (tile.param != NULL)
			zztParamFree(tile.param);
	}
}


void loadobjectlibrary(displaymethod * mydisplay, ZZTworld * myworld, editorinfo * myinfo)
{
	char* filename;
	stringvector zzlv;
	ZZTtile tile;

	/* No fair drawing over the player */
	if (zztBoardGetCurPtr(myworld)->plx == myinfo->cursorx &&
	    zztBoardGetCurPtr(myworld)->ply == myinfo->cursory)
		return;

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

		/* Free our copy of the params */
		if (tile.param != NULL)
			zztParamFree(tile.param);
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
