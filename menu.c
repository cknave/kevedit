/* menu.c       -- Code for using the F1-3 panels
 * $Id: menu.c,v 1.4 2001/10/26 23:36:05 bitman Exp $
 * Copyright (C) 2000 Kev Vance <kvance@tekktonik.net>
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

#include "kevedit.h"
#include "zzt.h"
#include "display.h"
#include "screen.h"
#include "patbuffer.h"
#include "svector.h"
#include "zzl.h"
#include "editbox.h"
#include "hypertxt.h"

#include <stdlib.h>

void plotbackbufferfirstslot(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	int oldpos;
	patbuffer* prevbuf = myinfo->pbuf;

	myinfo->pbuf = myinfo->backbuffer;
	oldpos = myinfo->pbuf->pos;
	myinfo->pbuf->pos = 0;

	plot(myworld, myinfo, mydisplay, bigboard, paramlist);

	myinfo->pbuf->pos = oldpos;
	myinfo->pbuf = prevbuf;
}
		
void itemmenu(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	int i, x, t;
	param *pm;

	if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
		return;

	i = dothepanel_f1(mydisplay, myinfo);
	if (i == Z_PLAYER) {
		/* The player is a special case */
		bigboard[(myinfo->playerx + myinfo->playery * 60) * 2] = Z_EMPTY;
		bigboard[(myinfo->playerx + myinfo->playery * 60) * 2 + 1] = 0x07;
		if (paramlist[myinfo->cursorx][myinfo->cursory] != 0) {
			/* We're overwriting a parameter */
			param_remove(myworld->board[myinfo->curboard], paramlist, myinfo->cursorx, myinfo->cursory);
		}
		bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] = Z_PLAYER;
		bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = 0x1f;
		paramlist[myinfo->cursorx][myinfo->cursory] = 0;
		myworld->board[myinfo->curboard]->params[0]->x = myinfo->playerx = myinfo->cursorx;
		myworld->board[myinfo->curboard]->params[0]->y = myinfo->playery = myinfo->cursory;
		myworld->board[myinfo->curboard]->params[0]->x++;
		myworld->board[myinfo->curboard]->params[0]->y++;
	} else {
		switch (i) {
		case -1:
			break;
		case Z_GEM:
		case Z_KEY:
			push(myinfo->backbuffer, i, (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80), NULL);
			break;
		case Z_AMMO:
		case Z_TORCH:
		case Z_ENERGIZER:
			if (myinfo->defc == 0)
				x = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
			else {
				if (i == Z_AMMO)
					x = 0x03;
				if (i == Z_TORCH)
					x = 0x06;
				if (i == Z_ENERGIZER)
					x = 0x05;
			}
			push(myinfo->backbuffer, i, x, NULL);
			break;
		case Z_DOOR:
			if (myinfo->defc == 1)
				x = myinfo->forec > 7 ? ((myinfo->forec - 8) << 4) + 0x0f : (myinfo->forec << 4) + 0x0f;
			else
				x = (myinfo->backc << 4) + (myinfo->forec) + (myinfo->blinkmode * 0x80);
			push(myinfo->backbuffer, i, x, NULL);
			break;
		case Z_SCROLL:
			if (myworld->board[myinfo->curboard]->info->objectcount == 150) {
				i = -1;
				break;
			} else {
				/* Anything important under it? */
				x = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2];
				switch (x) {
				case Z_WATER:
				case Z_FAKE:
					break;
				default:
					x = Z_EMPTY;
					break;
				}
			pm = z_newparam_scroll(myinfo->cursorx + 1, myinfo->cursory + 1, x, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1]);
			t = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
			push(myinfo->backbuffer, i, t, pm);
			break;
			}
		case Z_PASSAGE:
			pm = z_newparam_passage(myinfo->cursorx + 1, myinfo->cursory + 1, boarddialog(myworld, myinfo->curboard, 0, "Passage Destination", mydisplay));
			if (myinfo->defc == 1)
				x = myinfo->forec > 7 ? ((myinfo->forec - 8) << 4) + 0x0f : (myinfo->forec << 4) + 0x0f;
			else
				x = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
			push(myinfo->backbuffer, i, x, pm);
			break;
		case Z_DUPLICATOR:
			/* Anything important under it? */
			t = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2];
			switch (t) {
				case Z_WATER:
				case Z_FAKE:
					break;
				default:
					t = Z_EMPTY;
					break;
			}
			pm = z_newparam_duplicator(myinfo->cursorx + 1, myinfo->cursory + 1, -1, 0, 4, t, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2]);
			if(myinfo->defc == 1)
				x = 0x0f;
			else
				x = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
			push(myinfo->backbuffer, i, x, pm);
			break;
		case Z_CWCONV:
		case Z_CCWCONV:
			pm = z_newparam_conveyer(myinfo->cursorx + 1, myinfo->cursory + 1);
			push(myinfo->backbuffer, i, (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80), pm);
			break;
		case Z_BOMB:
			pm = z_newparam_bomb(myinfo->cursorx + 1, myinfo->cursory + 1);
			push(myinfo->backbuffer, i, (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80), pm);
			break;
		}
		if (i != -1 && i != Z_PLAYER) {
			plotbackbufferfirstslot(mydisplay, myworld, myinfo, bigboard, paramlist);
		}
	}
}


void creaturemenu(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	int i, x, t;
	param *pm;

	if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
		return;

	/* All these need parameter space */
	if (myworld->board[myinfo->curboard]->info->objectcount == 150)
		return;

	i = dothepanel_f2(mydisplay, myinfo);
	/* Anything important under it? */
	x = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2];
	switch (x) {
		case Z_WATER:
		case Z_FAKE:
			break;
		default:
			x = Z_EMPTY;
			break;
	}
	switch (i) {
	case -1:
		break;
	case Z_BEAR:
		pm = z_newparam_bear(myinfo->cursorx + 1, myinfo->cursory + 1, x, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], 4);
		if(myinfo->defc == 1)
			t = 0x06;
		else
			t = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
		push(myinfo->backbuffer, i, t, pm);
		break;
	case Z_RUFFIAN:
		pm = z_newparam_ruffian(myinfo->cursorx + 1, myinfo->cursory + 1, 4, 4);
		if(myinfo->defc == 1)
			t = 0x0d;
		else
			t = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
		push(myinfo->backbuffer, i, t, pm);
		break;
	case Z_SLIME:
		pm = z_newparam_slime(myinfo->cursorx + 1, myinfo->cursory + 1, 4);
		t = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
		push(myinfo->backbuffer, i, t, pm);
		break;
	case Z_SHARK:
		pm = z_newparam_shark(myinfo->cursorx + 1, myinfo->cursory + 1, x, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], 4);
		if(myinfo->defc == 1) {
			t = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] & 0xf0;
			if(t > 0x70)
				t -= 0x80;
			t += 0x07;
		} else
			t = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
		push(myinfo->backbuffer, i, t, pm);
		break;
	case Z_OBJECT:
		pm = z_newparam_object(myinfo->cursorx + 1, myinfo->cursory + 1, charselect(mydisplay, -1), x, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1]);
		t = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
		push(myinfo->backbuffer, i, t, pm);
		break;
	}
	if (i != -1) {
		plotbackbufferfirstslot(mydisplay, myworld, myinfo, bigboard, paramlist);
	}
}


void terrainmenu(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	int i, x;

	if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
		return;

	i = dothepanel_f3(mydisplay, myinfo);
	switch (i) {
		case -1:
			break;
		case Z_FAKE:
		case Z_SOLID:
		case Z_NORMAL:
		case Z_BREAKABLE:
		case Z_BOULDER:
		case Z_NSSLIDER:
		case Z_EWSLIDER:
		case Z_INVISIBLE:
			push(myinfo->backbuffer, i, (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80), NULL);
			break;
		case Z_WATER:
		case Z_FOREST:
		case Z_RICOCHET:
			if (myinfo->defc == 0)
				x = (myinfo->backc << 4) + myinfo->forec + (myinfo->blinkmode * 0x80);
			else {
				if (i == Z_WATER)
					x = 0x9f;
				if (i == Z_FOREST)
					x = 0x20;
				if (i == Z_RICOCHET)
					x = 0x0a;
			}
			push(myinfo->backbuffer, i, x, NULL);
			break;
		case Z_EDGE:
			push(myinfo->backbuffer, i, 0x07, NULL);
			break;
	}

	if (i != -1) {
		plotbackbufferfirstslot(mydisplay, myworld, myinfo, bigboard, paramlist);
	}
}


void loadobjectlibrary(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	char filename[256];
	stringvector zzlv;
	patdef pd;

	/* No fair drawing over the player */
	if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
		return;

	/* Prompt for a file and load it */
	if (filedialog(filename, "zzl", "Select an Object Library", mydisplay)
			== NULL)
		return;
	zzlv = filetosvector(filename, EDITBOX_ZZTWIDTH*2, EDITBOX_ZZTWIDTH*2);

	/* Have the user select an object */
	if (zzlpickobject(&zzlv, mydisplay) != EDITBOX_OK) {
		deletestringvector(&zzlv);
		return;
	}

	/* Put the object into the backbuffer and plot it to the screen */
	pd = zzlpullobject(zzlv, 0, 0, 0, 0);
	if (pd.type != -1) {
		push(myinfo->backbuffer, pd.type, pd.color, pd.patparam);
		plotbackbufferfirstslot(mydisplay, myworld, myinfo, bigboard, paramlist);
	}

	/* Finish */
	deletestringvector(&zzlv);
}

void saveobjecttolibrary(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	char filename[256];
	char* title;
	stringvector zzlv;
	patdef obj;

	/* Can't save what's under the cursor unless it's an object */
	if (bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] != Z_OBJECT)
		return;

	/* Prompt for and load a file */
	filedialog(filename, "zzl", "Save to Which Object Library?", mydisplay);
	zzlv = filetosvector(filename, EDITBOX_ZZTWIDTH*2, EDITBOX_ZZTWIDTH*2);

	/* Prompt for a title */
	title = titledialog(mydisplay);

	/* Copy the object under the cursor onto obj */
	obj.type  = Z_OBJECT;
	obj.color = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1];
	obj.patparam = myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]];

	/* Append the object to the library */
	if (!zzlappendobject(&zzlv, obj, title, EDITBOX_ZZTWIDTH)) {
		/* If append was successful, save the file */
		svectortofile(&zzlv, filename);
	}

	deletestringvector(&zzlv);
	free(title);
}

void saveobjecttonewlibrary(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	char filename[256];
	char* title;
	stringvector zzlv;
	patdef obj;

	initstringvector(&zzlv);

	/* Can't save what's under the cursor unless it's an object */
	if (bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] != Z_OBJECT)
		return;

	/* Prompt for file name */
	strcpy(filename, "");
	if (filenamedialog(filename, "New Object Library", "zzl", 1, mydisplay)
			== NULL)
		return;

	/* Prompt for a title */
	title = titledialog(mydisplay);

	/* Generate the zzl header */
	pushstring(&zzlv, str_dup("KevEdit Generated Library"));
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

	/* Copy the object under the cursor onto obj */
	obj.type  = Z_OBJECT;
	obj.color = bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1];
	obj.patparam = myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]];

	/* Append the object to the library */
	if (!zzlappendobject(&zzlv, obj, title, EDITBOX_ZZTWIDTH)) {
		/* If append was successful, save the file */
		svectortofile(&zzlv, filename);
	}

	deletestringvector(&zzlv);
	free(title);
}

void objectlibrarymenu(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
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
		loadobjectlibrary(mydisplay, myworld, myinfo, bigboard, paramlist);
	else if (str_equ(choice, "save", 0))
		saveobjecttolibrary(mydisplay, myworld, myinfo, bigboard, paramlist);
	else if (str_equ(choice, "new", 0))
		saveobjecttonewlibrary(mydisplay, myworld, myinfo, bigboard, paramlist);

	removestringvector(&menu);
}
