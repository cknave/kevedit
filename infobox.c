/* infobox.c  -- 
 * $Id: infobox.c,v 1.2 2001/10/22 02:48:22 bitman Exp $
 * Copyright (C) 2000 Ryan Phillips <bitman@scn.org>
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

#include "infobox.h"
#include "zzt.h"
#include "display.h"
#include "screen.h"
#include "help.h"

#include <stdio.h>
#include <stdlib.h>

/* Eventually these should go in DISPLAY_DOS */
#define DDOSKEY_EXT      0x100

#define DKEY_ENTER      13
#define DKEY_ESC        27
#define DKEY_BACKSPACE  '\b'
#define DKEY_TAB        '\t'
#define DKEY_CTRL_A     0x01
#define DKEY_CTRL_Y     25
#define DKEY_UP         0x48 | DDOSKEY_EXT
#define DKEY_DOWN       0x50 | DDOSKEY_EXT
#define DKEY_LEFT       0x4B | DDOSKEY_EXT
#define DKEY_RIGHT      0x4D | DDOSKEY_EXT
#define DKEY_HOME       0x47 | DDOSKEY_EXT
#define DKEY_END        0x4F | DDOSKEY_EXT
#define DKEY_INSERT     0x52 | DDOSKEY_EXT
#define DKEY_DELETE     0x53 | DDOSKEY_EXT
#define DKEY_F1         0x3B | DDOSKEY_EXT

/*************** Board Info *******************/

/* Types of board info */
#define BRDINFO_TITLE    0

#define BRDINFO_DARKNESS 1
#define BRDINFO_REENTER  2
#define BRDINFO_TIMELIM  3
#define BRDINFO_MAXSHOTS 4

#define BRDINFO_BRDNORTH 5
#define BRDINFO_BRDSOUTH 6
#define BRDINFO_BRDEAST  7
#define BRDINFO_BRDWEST  8

/* Functions used by editboardinfo */
void drawstaticboardinfo(displaymethod* d);
void drawboardinfo(world* myworld, int curboard, displaymethod* d);
int boardinfoeditoption(int curoption, world* myworld, int curboard,
												int cursorx, int cursory, displaymethod* d);
int boardinfodirectionoption(int curoption, world* myworld, int curboard,
												int cursorx, int cursory, int dir, displaymethod* d);
void boardinfostaroption(int curoption, world* myworld, int curboard);

/* editboardinfo() - brings up dialog box for editing board info */
void editboardinfo(world* myworld, int curboard, displaymethod* d)
{
	int curoption = BRDINFO_TITLE;
	int cursorx, cursory;
	int done = 0;
	drawstaticboardinfo(d);

	drawboardinfo(myworld, curboard, d);

	do {
		int key;

		/* Position the cursors */
		cursory = curoption + 7;
		if (curoption > BRDINFO_TITLE)
			cursory++;
		if (curoption >= BRDINFO_BRDNORTH)
			cursory += 2;

		if (curoption == BRDINFO_TITLE)
			cursorx = 30 - (strlen(myworld->board[curboard]->title) / 2);
		else if (curoption >= BRDINFO_BRDNORTH)
			cursorx = 14;
		else
			cursorx = 36;

		/* Draw the selector */
		d->putch( 7, cursory, '\xAF', 0x02);
		d->putch(51, cursory, '\xAE', 0x02);
		d->cursorgo(cursorx, cursory);

		/* Get the key */
		key = d->getch();
		if (key == 0)
			key = d->getch() | DDOSKEY_EXT;

		/* Erase the selector */
		d->putch( 7, cursory, ' ', 0x00);
		d->putch(51, cursory, ' ', 0x00);

		switch (key) {
			case DKEY_UP:
				if (curoption > 0) curoption--; else curoption = BRDINFO_BRDWEST;
				break;

			case DKEY_DOWN:
				if (curoption < BRDINFO_BRDWEST) curoption++; else curoption = 0;
				break;

			case DKEY_ESC:
				done = 1;
				break;

			case DKEY_ENTER:
				if (boardinfoeditoption(curoption, myworld, curboard,
																cursorx, cursory, d)) {
					drawstaticboardinfo(d);
					drawboardinfo(myworld, curboard, d);
				}
				break;

			case DKEY_LEFT:
			case '-':
				if (boardinfodirectionoption(curoption, myworld, curboard,
																		 cursorx, cursory, -1, d)) {
					drawstaticboardinfo(d);
					drawboardinfo(myworld, curboard, d);
				}
				break;

			case DKEY_RIGHT:
			case '+':
				if (boardinfodirectionoption(curoption, myworld, curboard,
																		 cursorx, cursory, 1, d)) {
					drawstaticboardinfo(d);
					drawboardinfo(myworld, curboard, d);
				}
				break;

			case '*':
				boardinfostaroption(curoption, myworld, curboard);
				drawstaticboardinfo(d);
				drawboardinfo(myworld, curboard, d);
				break;

			case DKEY_F1:
				helpsectiontopic("kbrdinfo", NULL, d);
				drawstaticboardinfo(d);
				drawboardinfo(myworld, curboard, d);
				break;
		}
	} while (!done);
}

void drawstaticboardinfo(displaymethod* d)
{
	/* Draw the scroll box */
	drawscrollbox(0, 0, d);
	d->putch( 7, 13, ' ', 0x00);
	d->putch(51, 13, ' ', 0x00);

	/* Draw the static contents */
	d->print(25, 4, 0x0a, "Board Info");

	d->print(28, 6, 0x0f, "Title");

	d->print(14,  9, 0x0a, "       Board is dark:");
	d->print(14, 10, 0x0a, "Re-Enter When Zapped:");
	d->print(14, 11, 0x0a, "          Time Limit:");
	d->print(14, 12, 0x0a, "       Maximum Shots:");

	d->print(23, 14, 0x0f, "Adjacent Boards");
	d->print(11, 15, 0x0a, "\x18:");
	d->print(11, 16, 0x0a, "\x19:");
	d->print(11, 17, 0x0a, "\x1A:");
	d->print(11, 18, 0x0a, "\x1B:");
	d->print(9,  19, 0x0f, "* = Links back to this board");
}

void drawboardinfo(world* myworld, int curboard, displaymethod* d)
{
	char buffer[10];    /* Buffer for translating numbers to strings */
	board* brd = myworld->board[curboard];
	boardinfo* info = brd->info;

	/* Title */
	d->print(30 - (strlen(brd->title) / 2), 7, 0x0B, brd->title);

	/* Boolean */
	d->print(36,  9, 0x0B, (info->darkness  ? "Yes" : "No"));
	d->print(36, 10, 0x0B, (info->reenter   ? "Yes" : "No"));

	/* Numerical */
	/* Time Limit */
	if (info->timelimit > 0)
		sprintf(buffer, "%d", info->timelimit);
	else
		strcpy(buffer, "Infinite");
	d->print(36, 11, 0x0B, buffer);

	/* Maximum shots */
	sprintf(buffer, "%d", info->maxshots);
	d->print(36, 12, 0x0B, buffer);

	/* Board links */
	/* North */
	if (info->board_n > 0) {
		d->print(14, 15, 0x0B, myworld->board[info->board_n]->title);
		if (curboard > 0 &&
				myworld->board[info->board_n]->info->board_s == curboard)
			d->putch(9, 15, '*', 0x0B);
	} else {
		d->print(14, 15, 0x03, "(None)");
	}

	/* South */
	if (info->board_s > 0) {
		d->print(14, 16, 0x0B, myworld->board[info->board_s]->title);
		if (curboard > 0 &&
				myworld->board[info->board_s]->info->board_n == curboard)
			d->putch(9, 16, '*', 0x0B);
	} else {
		d->print(14, 16, 0x03, "(None)");
	}

	/* East */
	if (info->board_e > 0) {
		d->print(14, 17, 0x0B, myworld->board[info->board_e]->title);
		if (curboard > 0 &&
				myworld->board[info->board_e]->info->board_w == curboard)
			d->putch(9, 17, '*', 0x0B);
	} else {
		d->print(14, 17, 0x03, "(None)");
	}

	/* West */
	if (info->board_w > 0) {
		d->print(14, 18, 0x0B, myworld->board[info->board_w]->title);
		if (curboard > 0 &&
				myworld->board[info->board_w]->info->board_e == curboard)
			d->putch(9, 18, '*', 0x0B);
	} else {
		d->print(14, 18, 0x03, "(None)");
	}
}


int boardinfoeditoption(int curoption, world* myworld, int curboard,
												int cursorx, int cursory, displaymethod* d)
{
	int i;
	char buffer[35];
	board* brd = myworld->board[curboard];

	switch (curoption) {
		case BRDINFO_TITLE:
			/* Change the title of the board */
			strcpy(buffer, brd->title);
			if (line_editor(13, cursory, 0x0f, buffer, 34, LINED_NORMAL, d)) {
				/* Success! Reserve enough space and copy the buffer over */
				free(brd->title);
				brd->title = (u_int8_t*) malloc(sizeof(u_int8_t) * (strlen(buffer)+1));
				strcpy(brd->title, buffer);
			}
			/* Update */
			return 1;

		case BRDINFO_BRDNORTH:
			brd->info->board_n = boarddialog(myworld, brd->info->board_n, 1, "Board to the North", d);
			return 2;

		case BRDINFO_BRDSOUTH:
			brd->info->board_s = boarddialog(myworld, brd->info->board_s, 1, "Board to the South", d);
			return 2;

		case BRDINFO_BRDEAST:
			brd->info->board_e = boarddialog(myworld, brd->info->board_e, 1, "Board to the East", d);
			return 2;

		case BRDINFO_BRDWEST:
			brd->info->board_w = boarddialog(myworld, brd->info->board_w, 1, "Board to the West", d);
			return 2;

		case BRDINFO_DARKNESS:
			brd->info->darkness = !brd->info->darkness;
			return 1;

		case BRDINFO_REENTER:
			brd->info->reenter = !brd->info->reenter;
			return 1;

		case BRDINFO_MAXSHOTS:
			sprintf(buffer, "%d", brd->info->maxshots);
			if (line_editor(cursorx, cursory, 0x0f, buffer, 3,
											LINED_NOALPHA | LINED_NOPUNCT | LINED_NOSPACES, d)) {
				int maxshots;
				sscanf(buffer, "%d", &maxshots);
				if (maxshots > 255)
					brd->info->maxshots = 255;
				else
					brd->info->maxshots = (u_int8_t) maxshots;
			}
			return 1;

		case BRDINFO_TIMELIM:
			/* Clear the word "Infinite" if it's there */
			for (i = 0; i < 10; i++)
				d->putch(cursorx + i, cursory, ' ', 0x00);

			/* Load the timelimit into the buffer */
			if (brd->info->timelimit != 0)
				sprintf(buffer, "%d", brd->info->timelimit);
			else
				strcpy(buffer, "");

			if (line_editor(cursorx, cursory, 0x0f, buffer, 5,
											LINED_NOALPHA | LINED_NOPUNCT | LINED_NOSPACES, d)) {
				long int timelimit;
				sscanf(buffer, "%ld", &timelimit);
				if (strlen(buffer) == 0)
					brd->info->timelimit = 0;
				else if (timelimit > 32767)
					brd->info->timelimit = 32767;
				else
					brd->info->timelimit = timelimit;
			}
			return 1;
	}
	return 0;
}

int boardinfodirectionoption(int curoption, world* myworld, int curboard,
												int cursorx, int cursory, int dir, displaymethod* d)
{
	board* brd = myworld->board[curboard];
	boardinfo* info = brd->info;

	switch (curoption) {
		case BRDINFO_TIMELIM:
			if (info->timelimit + dir <= 32767 &&
					info->timelimit + dir >= 0)
				info->timelimit += dir;
			return 1;

		case BRDINFO_MAXSHOTS:
			if ((int)info->maxshots + dir <= 255 && (int)info->maxshots + dir >= 0)
				info->maxshots += dir;
			return 1;

		case BRDINFO_BRDNORTH:
			if (info->board_n + dir <= myworld->zhead->boardcount &&
					info->board_n + dir >= 0)
				info->board_n += dir;
			return 1;

		case BRDINFO_BRDSOUTH:
			if (info->board_s + dir <= myworld->zhead->boardcount &&
					info->board_s + dir >= 0)
				info->board_s += dir;
			return 1;

		case BRDINFO_BRDEAST:
			if (info->board_e + dir <= myworld->zhead->boardcount &&
					info->board_e + dir >= 0)
				info->board_e += dir;
			return 1;

		case BRDINFO_BRDWEST:
			if (info->board_w + dir <= myworld->zhead->boardcount &&
					info->board_w + dir >= 0)
				info->board_w += dir;
			return 1;

		default:
			return boardinfoeditoption(curoption, myworld, curboard,
																 cursorx, cursory, d);
	}
}

void boardinfostaroption(int curoption, world* myworld, int curboard)
{
	board* brd = myworld->board[curboard];
	boardinfo* info = brd->info;

	if (curboard == 0)
		return;

	switch (curoption) {
		case BRDINFO_BRDNORTH:
			if (info->board_n > 0)
				myworld->board[info->board_n]->info->board_s = curboard;
			break;

		case BRDINFO_BRDSOUTH:
			if (info->board_s > 0)
				myworld->board[info->board_s]->info->board_n = curboard;
			break;

		case BRDINFO_BRDEAST:
			if (info->board_e > 0)
				myworld->board[info->board_e]->info->board_w = curboard;
			break;

		case BRDINFO_BRDWEST:
			if (info->board_w > 0)
				myworld->board[info->board_w]->info->board_e = curboard;
			break;
	}
}


/************* World Info ******************/

void drawstaticworldinfo(displaymethod* d);
void drawworldinfo(world* myworld, displaymethod* d);

/* editworldinfo() - brings up dialog box for editing world info */
void editworldinfo(world* myworld, displaymethod* d)
{
	drawstaticworldinfo(d);
	drawworldinfo(myworld, d);
	d->getch();
}

void drawstaticworldinfo(displaymethod* d)
{
	/* Draw the scroll box */
	drawscrollbox(0, 0, d);
	d->putch( 7, 13, ' ', 0x00);
	d->putch(51, 13, ' ', 0x00);

	/* Draw the static contents */
	d->print(25, 4, 0x0A, "World Info");

	d->print(14,  6, 0x0F, "      World Name:");

	d->print(14,  8, 0x0A, "            Keys:");
	d->print(14,  9, 0x0A, "            Ammo:");
	d->print(14, 10, 0x0A, "            Gems:");
	d->print(14, 11, 0x0A, "          Health:");
	d->print(14, 12, 0x0A, "         Torches:");
	d->print(14, 13, 0x0A, "           Score:");

	d->print(14, 15, 0x0A, "    Torch cycles:");
	d->print(14, 16, 0x0A, "Energizer Cycles:");
	d->print(14, 17, 0x0A, "    Time elapsed:");
	d->print(14, 18, 0x0A, "   Is Saved Game:");
	d->print(23, 20, 0x0F, "Set/Clear Flags");
}

void drawworldinfo(world* myworld, displaymethod* d)
{
	char buffer[10];    /* Buffer for translating numbers to strings */
	zztheader* zhead = myworld->zhead;

	/* Start at the top */
	d->print(32,  6, 0x0B, zhead->title);

	/* List the keys */
	/* TODO: later */

	/* Inventory */
	sprintf(buffer, "%d", zhead->ammo);     d->print(32,  9, 0x0B, buffer);
	sprintf(buffer, "%d", zhead->gems);     d->print(32, 10, 0x0B, buffer);
	sprintf(buffer, "%d", zhead->health);   d->print(32, 11, 0x0B, buffer);
	sprintf(buffer, "%d", zhead->torches);  d->print(32, 12, 0x0B, buffer);
	sprintf(buffer, "%d", zhead->score);    d->print(32, 13, 0x0B, buffer);

	/* Misc */
	sprintf(buffer, "%d", zhead->torchcycles);    d->print(32, 15, 0x0B, buffer);
	sprintf(buffer, "%d", zhead->energizercycles);d->print(32, 16, 0x0B, buffer);
	sprintf(buffer, "%d", zhead->timepassed);     d->print(32, 17, 0x0B, buffer);

	/* Saved Game boolean */
	d->print(32, 18, 0x0B, zhead->savebyte ? "Yes" : "No");
}


