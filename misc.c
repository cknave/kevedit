/* misc.c       -- General routines for everyday KevEditing
 * $Id: misc.c,v 1.2 2001/05/12 21:15:28 bitman Exp $
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
#include "display.h"
#include "svector.h"
#include "editbox.h"
#include "screen.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


void runzzt(char *args)
{
	char runcommand[256];	/* [12] should be enough, but... */

	strcpy(runcommand, "zzt ");

	/* Now now, no naughty overflowing the buffer */
	strncpy(runcommand+4, args, 251);
	runcommand[255] = '\0';

	system(runcommand);
}


void help(displaymethod* d)
{
	stringvector helpdialog, readmefile;

	initstringvector(&helpdialog);
	readmefile = filetosvector("readme", 42, 42);

	pushstringcopy(&helpdialog, "KevEdit R5, Version " VERSION);
	pushstringcopy(&helpdialog, "Copyright (C) 2000 Kev Vance, et al.");
	pushstringcopy(&helpdialog, "Distribute under the terms of the GNU GPL");

	if (readmefile.first != NULL) {
		pushstringcopy(&helpdialog, "");
		pushstringcopy(&helpdialog, "$=-=-=-=-=-=-=-=- README =-=-=-=-=-=-=-=-=");
		pushstringcopy(&helpdialog, "");

		helpdialog.last->next = readmefile.first;
		helpdialog.last->next->prev = helpdialog.last;
	}

	editbox("About KevEdit", &helpdialog, 0, 1, d);

	deletestringvector(&helpdialog);
}


void showParamData(param * p, int paramNumber, displaymethod * d)
{
	char buffer[50];
	stringvector data;
	initstringvector(&data);

	pushstringcopy(&data, "$Param Data");
	pushstringcopy(&data, "");

	sprintf(buffer, "param#:      %d / 150", paramNumber);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "x:           %d", p->x);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "y:           %d", p->y);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "xstep:       %d", p->xstep);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "ystep:       %d", p->ystep);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "cycle:       %d", p->cycle);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "data1:       %d", p->data1);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "data2:       %d", p->data2);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "data3:       %d", p->data3);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "magic:       %d", p->magic);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "undert:      0x%X", p->undert);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "underc:      0x%X", p->underc);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "instruction: %d", p->instruction);
	pushstringcopy(&data, buffer);
	sprintf(buffer, "length:      %d", p->length);
	pushstringcopy(&data, buffer);

	editbox("Param Data", &data, 0, 1, d);

	deletestringvector(&data);
}


void texteditor(displaymethod * mydisplay)
{
	/* open text file for edit */
	char buffer[14];
	stringvector editvector;

	strcpy(buffer, "");
	initstringvector(&editvector);
	editbox("Text Editor", &editvector, 42, 0, mydisplay);
	if (filenamedialog(buffer, "Save As", "", 1, mydisplay) != NULL)
		svectortofile(&editvector, buffer);
	deletestringvector(&editvector);
}


void clearboard(world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	int i, x;
	char * buffer;
	board * curboard;

	curboard = myworld->board[myinfo->curboard];
	for (i = 0; i < curboard->info->objectcount + 1; i++) {
		if (curboard->params[i]->moredata != NULL)
			free(curboard->params[i]->moredata);
		free(curboard->params[i]);
	}
	free(curboard->data);
	free(curboard->info);
	buffer = (char *) malloc(sizeof(char) * (strlen(curboard->title) + 2));
	strcpy(buffer, curboard->title);
	free(curboard->title);
	free(curboard);

	curboard = z_newboard(buffer);
	myworld->board[myinfo->curboard] = curboard;

	free(buffer);
	rle_decode(curboard->data, bigboard);
	for (i = 0; i < 25; i++)
		for (x = 0; x < 60; x++)
			paramlist[x][i] = 0;

	myinfo->playerx = curboard->params[0]->x - 1;
	myinfo->playery = curboard->params[0]->y - 1;
}


world * clearworld(world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	int i, x;

	z_delete(myworld);
	myworld = z_newworld();
	myworld->board[0] = z_newboard("KevEdit World");
	rle_decode(myworld->board[0]->data, bigboard);
	for (i = 0; i < 25; i++)
		for (x = 0; x < 60; x++)
			paramlist[x][i] = 0;
	strcpy(myinfo->currenttitle, "UNTITLED");
	strcpy(myinfo->currentfile,  "untitled.zzt");
	myinfo->curboard = 0;

	myinfo->playerx = myworld->board[0]->params[0]->x - 1;
	myinfo->playery = myworld->board[0]->params[0]->y - 1;

	return myworld;
}


int toggledrawmode(editorinfo * myinfo)
{
	if (myinfo->gradmode != 0) {
		/* If grad mode is on, turn it and drawmode off */
		myinfo->gradmode = myinfo->drawmode = 0;
	} else {
		/* Otherwise toggle draw mode */
		myinfo->drawmode ^= 1;
	}
	/* Get mode should go off either way */
	myinfo->getmode = 0;

	return myinfo->drawmode;
}


int togglegradientmode(editorinfo * myinfo)
{
	/* Toggle gradient mode - pattern changes with each cursor
	 * movement & drawmode is turned on. */
	if (myinfo->pbuf == myinfo->standard_patterns)
		return 0;     /* Cycling through standard_patterns not yet supported */

	myinfo->getmode = 0;
	if (myinfo->gradmode != 0) {
		/* Gradmode is already on, advance once and reverse it */
		if (myinfo->gradmode < 0) {
			if (--myinfo->pbuf->pos < 0)
				myinfo->pbuf->pos = myinfo->pbuf->size - 1;
		} else {
			if (++myinfo->pbuf->pos >= myinfo->pbuf->size)
				myinfo->pbuf->pos = 0;
		}
		myinfo->gradmode = -(myinfo->gradmode);

		return 0;    /* No reason to start plotting yet */
	} else {
		/* Turn gradmode & drawmode on */
		myinfo->drawmode = 1;
		/* Gradmode cycles backward by default */
		myinfo->gradmode = -1;
		return 1;    /* Gradmode went on -- let the plotting begin! */
	}
}


void changeboard(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard, unsigned char paramlist[60][25])
{
	int i, x;

	/* Switch boards */
	i = boarddialog(myworld, myinfo, mydisplay);
	if (i == -1 || i == myinfo->curboard)
		return;

	/* It may become useful to put the following code in another function for
	 * greater flexibility in the future. Presently, such provides no gains. */
	free(myworld->board[myinfo->curboard]->data);
	myworld->board[myinfo->curboard]->data = rle_encode(bigboard);
	myinfo->curboard = i;
	if (myinfo->curboard > myworld->zhead->boardcount)
		myinfo->curboard = 0;
	rle_decode(myworld->board[myinfo->curboard]->data, bigboard);
	for (i = 0; i < 25; i++)
		for (x = 0; x < 60; x++)
			paramlist[x][i] = 0;
	for (i = 0; i < myworld->board[myinfo->curboard]->info->objectcount + 1; i++) {
		if (myworld->board[myinfo->curboard]->params[i]->x > 0 && myworld->board[myinfo->curboard]->params[i]->x < 61 && myworld->board[myinfo->curboard]->params[i]->y > 0 && myworld->board[myinfo->curboard]->params[i]->y < 26)
			paramlist[myworld->board[myinfo->curboard]->params[i]->x - 1][myworld->board[myinfo->curboard]->params[i]->y - 1] = i;
	}

	myinfo->playerx = myworld->board[myinfo->curboard]->params[0]->x - 1;
	myinfo->playery = myworld->board[myinfo->curboard]->params[0]->y - 1;
}


void saveworldprompt(displaymethod * mydisplay, world * myworld, editorinfo * myinfo, char * bigboard)
{
	/* Save World */
	int i;
	char filename[13];

	/* Update boarddata to reflect bigboard */
	free(myworld->board[myinfo->curboard]->data);
	myworld->board[myinfo->curboard]->data = rle_encode(bigboard);

	/* Prompt the user for a filename */
	strcpy(filename, myinfo->currentfile);
	filenamedialog(filename, "Save World As", "zzt", 1, mydisplay);

	if (!strcmp(filename, myinfo->currentfile))
		return;

	/* Copy filename w/o ext onto world's title */
	for (i = 0; i < 9 && filename[i] != '.' && filename[i] != '\0'; i++) {
		myinfo->currenttitle[i] = filename[i];
	}
	myinfo->currenttitle[i] = '\0';

	/* Change internal name of world only on first use of save */
	if (!strcmp(myworld->zhead->title, "UNTITLED")) {
		strcpy(myworld->zhead->title, myinfo->currenttitle);
		myworld->zhead->titlelength = strlen(myworld->zhead->title);
	}

	strcpy(myinfo->currentfile, filename);

	myworld->zhead->startboard = myinfo->curboard;
	saveworld(filename, myworld);

	mydisplay->print(61, 6, 0x1f, "Written.");
	mydisplay->cursorgo(69, 6);
	mydisplay->getch();
}



