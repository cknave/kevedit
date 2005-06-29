/* menu.c       -- Code for using the F1-3 panels
 * $Id: menu.c,v 1.3 2005/06/29 03:20:34 kvance Exp $
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

#include "menu.h"

#include "kevedit.h"
#include "screen.h"
#include "texteditor/editbox.h"

#include "libzzt2/zzt.h"
#include "structures/svector.h"
#include "dialogs/files.h"
#include "texteditor/zzl.h"
#include "help/hypertxt.h"
#include "display/colours.h"

#include "patbuffer.h"

#include "display/display.h"

#include <stdlib.h>

void itemmenu(keveditor * myeditor)
{
	ZZTworld * myworld = myeditor->myworld;
	ZZTtile tile = { ZZT_EMPTY, 0x0, NULL };
	int choice; /* Need signed type */

	tile.color = encodecolor(myeditor->color);

	choice = dothepanel_f1(myeditor);
	tile.type = choice;

	if (tile.type == ZZT_PLAYER) {
		/* The player is a special case */
		zztPlotPlayer(myworld, myeditor->cursorx, myeditor->cursory);
		return;
	}

	/* Create params -- zztParamCreate() returns NULL for types which have no params */
	tile.param = zztParamCreate(tile);


	/* Use default color if in default color mode */
	if (myeditor->defcmode != 0) {
		switch (tile.type) {
			case ZZT_AMMO:      tile.color = 0x03; break;
			case ZZT_TORCH:     tile.color = 0x06; break;
			case ZZT_ENERGIZER: tile.color = 0x05; break;
			case ZZT_DOOR:
				tile.color = myeditor->color.fg > 7 ? ((myeditor->color.fg - 8) << 4) + 0x0f : (myeditor->color.fg << 4) + 0x0f;
				break;
			case ZZT_PASSAGE:
				tile.color = myeditor->color.fg > 7 ? ((myeditor->color.fg - 8) << 4) + 0x0f : (myeditor->color.fg << 4) + 0x0f;
				break;
			case ZZT_DUPLICATOR:
				tile.color = 0x0f;
				break;
		}
	}

	/* Plot and push the created tile */
	if (choice != -1) {
		zztPlot(myworld, myeditor->cursorx, myeditor->cursory, tile);
		push(myeditor->buffers.backbuffer, tile);
	}

	/* Free our copy of the params */
	if (tile.param != NULL)
		zztParamFree(tile.param);
}


void creaturemenu(keveditor * myeditor)
{
	ZZTworld * myworld = myeditor->myworld;
	ZZTtile tile = { ZZT_EMPTY, 0x0, NULL };
	int choice; /* Need signed type */

	tile.color = encodecolor(myeditor->color);

	choice = dothepanel_f2(myeditor);
	tile.type = choice;

	/* Everything has params */
	tile.param = zztParamCreate(tile);

	/* Use default color if in default color mode */
	if (myeditor->defcmode != 0) {
		switch (choice) {
			case ZZT_BEAR:
				tile.color = 0x06;
				break;
			case ZZT_RUFFIAN:
				tile.color = 0x0d;
				break;
			case ZZT_SHARK:
				tile.color = zztTileGet(myworld, myeditor->cursorx, myeditor->cursory).color & 0xf0;
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
		zztPlot(myworld, myeditor->cursorx, myeditor->cursory, tile);
		push(myeditor->buffers.backbuffer, tile);
		
		/* Free our copy of the params */
		if (tile.param != NULL)
			zztParamFree(tile.param);
	}
}


void terrainmenu(keveditor * myeditor)
{
	ZZTworld * myworld = myeditor->myworld;
	ZZTtile tile = { ZZT_EMPTY, 0x0, NULL };
	int choice; /* Need signed type */

	tile.color = encodecolor(myeditor->color);

	choice = dothepanel_f3(myeditor);
	tile.type = choice;

	if (tile.type == ZZT_PLAYER)
		/* The dead player should be paramless */
		tile.param = NULL;
	else
		/* Add params for anything else that needs them */
		tile.param = zztParamCreate(tile);

	/* Use default color if in default color mode */
	if (myeditor->defcmode != 0) {
		switch (choice) {
			case ZZT_WATER:
				tile.color = 0x9f;
				break;
			case ZZT_FOREST:
				tile.color = 0x20;
				break;
			case ZZT_RICOCHET:
				tile.color = 0x0a;
				break;
		}
	}

	/* Plot and push the created tile */
	if (choice != -1) {
		zztPlot(myworld, myeditor->cursorx, myeditor->cursory, tile);
		push(myeditor->buffers.backbuffer, tile);

		/* Free our copy of the params */
		if (tile.param != NULL)
			zztParamFree(tile.param);
	}
}


void loadobjectlibrary(keveditor * myeditor)
{
	displaymethod * mydisplay = myeditor->mydisplay;
	ZZTworld * myworld = myeditor->myworld;
	char* filename;
	stringvector zzlv;
	ZZTtile tile;

	/* No fair drawing over the player */
	if (zztBoardGetCurPtr(myworld)->plx == myeditor->cursorx &&
	    zztBoardGetCurPtr(myworld)->ply == myeditor->cursory)
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
		zztPlot(myworld, myeditor->cursorx, myeditor->cursory, tile);
		push(myeditor->buffers.backbuffer, tile);

		/* Free our copy of the params */
		if (tile.param != NULL)
			zztParamFree(tile.param);
	}

	/* Finish */
	deletestringvector(&zzlv);
}

void saveobjecttolibrary(keveditor * myeditor)
{
	displaymethod * mydisplay = myeditor->mydisplay;
	ZZTworld * myworld = myeditor->myworld;
	char* filename;
	char* title;
	stringvector zzlv;
	ZZTtile obj = zztTileGet(myworld, myeditor->cursorx, myeditor->cursory);

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

void saveobjecttonewlibrary(keveditor * myeditor)
{
	displaymethod * mydisplay = myeditor->mydisplay;
	ZZTworld * myworld = myeditor->myworld;
	char* filename;
	char* title;
	stringvector zzlv;
	ZZTtile obj = zztTileGet(myworld, myeditor->cursorx, myeditor->cursory);

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

void objectlibrarymenu(keveditor * myeditor)
{
	displaymethod * mydisplay = myeditor->mydisplay;
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
		loadobjectlibrary(myeditor);
	else if (str_equ(choice, "save", 0))
		saveobjecttolibrary(myeditor);
	else if (str_equ(choice, "new", 0))
		saveobjecttonewlibrary(myeditor);

	removestringvector(&menu);
}

