/* infobox.c  -- 
 * $Id: infobox.c,v 1.5 2001/11/06 07:33:05 bitman Exp $
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
#include <string.h>


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

/* Types of World Info */
#define WLDINFO_NAME       0
#define WLDINFO_KEYS       1
#define WLDINFO_AMMO       2
#define WLDINFO_GEMS       3
#define WLDINFO_HEALTH     4
#define WLDINFO_TORCHES    5
#define WLDINFO_SCORE      6
#define WLDINFO_TCYCLES    7
#define WLDINFO_ECYCLES    8
#define WLDINFO_TIMEPASSED 9
#define WLDINFO_ISSAVED    10
#define WLDINFO_FLAGS      11

/* Order of the keys */
#define WLDKEY_BLUE        0
#define WLDKEY_GREEN       1
#define WLDKEY_CYAN        2
#define WLDKEY_RED         3
#define WLDKEY_PURPLE      4
#define WLDKEY_YELLOW      5
#define WLDKEY_WHITE       6

void drawstaticworldinfo(displaymethod* d);
void drawworldinfo(world* myworld, displaymethod* d);
int worldinfoeditoption(int curoption, world* myworld,
												int cursorx, int cursory, displaymethod* d);
int worldinfodirectionoption(int curoption, world* myworld,
												int cursorx, int cursory, int dir, displaymethod* d);
void worldinfotogglekey(world* myworld, int whichkey);
void editworldflags(world* myworld, displaymethod* d);

/* editworldinfo() - brings up dialog box for editing world info */
void editworldinfo(world* myworld, displaymethod* d)
{
	int curoption = WLDINFO_NAME;
	int cursorx, cursory;
	int done = 0;

	drawstaticworldinfo(d);
	drawworldinfo(myworld, d);

	cursorx = 31;

	do {
		int key;

		/* Position the cursors */
		cursory = curoption + 6;
		if (curoption > WLDINFO_NAME)
			cursory++;
		if (curoption > WLDINFO_SCORE)
			cursory++;
		if (curoption > WLDINFO_ISSAVED)
			cursory++;

		if (curoption == WLDINFO_FLAGS)
			cursorx = 23;
		else if (curoption != WLDINFO_KEYS)
			/* Don't change the cursorx when messing w/ keys */
			cursorx = 31;

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
				if (curoption > 0) curoption--; else curoption = WLDINFO_FLAGS;
				break;

			case DKEY_DOWN:
				if (curoption < WLDINFO_FLAGS) curoption++; else curoption = 0;
				break;

			case DKEY_ESC:
				done = 1;
				break;
		}

		if (curoption != WLDINFO_KEYS) {
			switch (key) {
				case DKEY_ENTER:
					if (worldinfoeditoption(curoption, myworld,
																	cursorx, cursory, d)) {
						drawstaticworldinfo(d);
						drawworldinfo(myworld, d);
					}
					break;

				case DKEY_LEFT:
					if (worldinfodirectionoption(curoption, myworld,
																			 cursorx, cursory, -10, d)) {
						drawstaticworldinfo(d);
						drawworldinfo(myworld, d);
					}
					break;

				case '-':
					if (worldinfodirectionoption(curoption, myworld,
																			 cursorx, cursory, -1, d)) {
						drawstaticworldinfo(d);
						drawworldinfo(myworld, d);
					}
					break;

				case DKEY_RIGHT:
					if (worldinfodirectionoption(curoption, myworld,
																			 cursorx, cursory, 10, d)) {
						drawstaticworldinfo(d);
						drawworldinfo(myworld, d);
					}
					break;

				case '+':
					if (worldinfodirectionoption(curoption, myworld,
																			 cursorx, cursory, 1, d)) {
						drawstaticworldinfo(d);
						drawworldinfo(myworld, d);
					}
					break;

				case DKEY_F1:
					helpsectiontopic("kwldinfo", NULL, d);
					drawstaticworldinfo(d);
					drawworldinfo(myworld, d);
					break;
			}

		} else {
			/* Current option is BRDINFO_KEYS */

			switch (key) {
				case DKEY_LEFT:  if (cursorx > 31) cursorx--;
													 else cursorx = 31 + WLDKEY_WHITE; break;
				case DKEY_RIGHT: if (cursorx < 31 + WLDKEY_WHITE) cursorx++;
													 else cursorx = 31; break;

				case DKEY_ENTER:
				case ' ':
					worldinfotogglekey(myworld, cursorx - 31);
					drawstaticworldinfo(d);
					drawworldinfo(myworld, d);
					break;
			}
		}
	} while (!done);
}

void drawstaticworldinfo(displaymethod* d)
{
	/* Draw the scroll box */
	drawscrollbox(0, 0, d);
	d->putch( 7, 13, ' ', 0x00);
	d->putch(51, 13, ' ', 0x00);

	/* Draw the static contents */
	d->print(25, 4, 0x0A, "World Info");

	d->print(13,  6, 0x0F, "      World Name:");

	d->print(13,  8, 0x0A, "            Keys:");
	d->print(13,  9, 0x0A, "            Ammo:");
	d->print(13, 10, 0x0A, "            Gems:");
	d->print(13, 11, 0x0A, "          Health:");
	d->print(13, 12, 0x0A, "         Torches:");
	d->print(13, 13, 0x0A, "           Score:");

	d->print(13, 15, 0x0A, "    Torch Cycles:");
	d->print(13, 16, 0x0A, "Energizer Cycles:");
	d->print(13, 17, 0x0A, "    Time Elapsed:");
	d->print(13, 18, 0x0A, "   Is Saved Game:");
	d->print(23, 20, 0x0F, "Set/Clear Flags");
}

#define drawkey(which)  d->putch(31 + (which), 8, '\x0C', 0x08 +  (which) + 1)
#define drawdoor(which) d->putch(31 + (which), 8, '\x0A', 0x0F + (((which) + 1) << 4))

void drawworldinfo(world* myworld, displaymethod* d)
{
	char buffer[10];    /* Buffer for translating numbers to strings */
	zztheader* zhead = myworld->zhead;

	/* Start at the top */
	d->print(31,  6, 0x0B, zhead->title);

	/* List the keys */
	if (zhead->bluekey) drawkey(WLDKEY_BLUE); else drawdoor(WLDKEY_BLUE);
	if (zhead->greenkey) drawkey(WLDKEY_GREEN); else drawdoor(WLDKEY_GREEN);
	if (zhead->cyankey) drawkey(WLDKEY_CYAN); else drawdoor(WLDKEY_CYAN);
	if (zhead->redkey) drawkey(WLDKEY_RED); else drawdoor(WLDKEY_RED);
	if (zhead->purplekey) drawkey(WLDKEY_PURPLE); else drawdoor(WLDKEY_PURPLE);
	if (zhead->yellowkey) drawkey(WLDKEY_YELLOW); else drawdoor(WLDKEY_YELLOW);
	if (zhead->whitekey) drawkey(WLDKEY_WHITE); else drawdoor(WLDKEY_WHITE);

	/* Inventory */
	sprintf(buffer, "%d", zhead->ammo);     d->print(31,  9, 0x0B, buffer);
	sprintf(buffer, "%d", zhead->gems);     d->print(31, 10, 0x0B, buffer);
	sprintf(buffer, "%d", zhead->health);   d->print(31, 11, 0x0B, buffer);
	sprintf(buffer, "%d", zhead->torches);  d->print(31, 12, 0x0B, buffer);
	sprintf(buffer, "%d", zhead->score);    d->print(31, 13, 0x0B, buffer);

	/* Misc */
	sprintf(buffer, "%d", zhead->torchcycles);    d->print(31, 15, 0x0B, buffer);
	sprintf(buffer, "%d", zhead->energizercycles);d->print(31, 16, 0x0B, buffer);
	sprintf(buffer, "%d", zhead->timepassed);     d->print(31, 17, 0x0B, buffer);

	/* Saved Game boolean */
	d->print(31, 18, 0x0B, zhead->savebyte ? "Yes" : "No");
}

int worldinfoeditoption(int curoption, world* myworld,
												int cursorx, int cursory, displaymethod* d)
{
	int tempnum;
	char buffer[35];

	switch (curoption) {
		case WLDINFO_NAME:
			/* Change the title of the board */
			strcpy(buffer, myworld->zhead->title);
			if (line_editor(cursorx, cursory, 0x0f, buffer, 19, LINED_NORMAL, d)) {
				/* Success! Copy the thing! */
				strcpy(myworld->zhead->title, buffer);
				myworld->zhead->titlelength = strlen(myworld->zhead->title);
			}
			/* Update */
			return 1;

		case WLDINFO_ISSAVED:
			myworld->zhead->savebyte = !myworld->zhead->savebyte;
			return 1;

		case WLDINFO_FLAGS:
			editworldflags(myworld, d);
			return 1;

		case WLDINFO_AMMO:
			tempnum = myworld->zhead->ammo;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			myworld->zhead->ammo = tempnum;
			return 1;

		case WLDINFO_GEMS:
			tempnum = myworld->zhead->gems;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			myworld->zhead->gems = tempnum;
			return 1;

		case WLDINFO_HEALTH:
			tempnum = myworld->zhead->health;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			myworld->zhead->health = tempnum;
			return 1;

		case WLDINFO_TORCHES:
			tempnum = myworld->zhead->torches;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			myworld->zhead->torches = tempnum;
			return 1;

		case WLDINFO_SCORE:
			tempnum = myworld->zhead->score;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			myworld->zhead->score = tempnum;
			return 1;

		case WLDINFO_TCYCLES:
			tempnum = myworld->zhead->torchcycles;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			myworld->zhead->torchcycles = tempnum;
			return 1;

		case WLDINFO_ECYCLES:
			tempnum = myworld->zhead->energizercycles;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			myworld->zhead->energizercycles = tempnum;
			return 1;

		case WLDINFO_TIMEPASSED:
			tempnum = myworld->zhead->timepassed;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			myworld->zhead->timepassed = tempnum;
			return 1;
	}

	return 0;
}

int worldinfodirectionoption(int curoption, world* myworld, int cursorx,
														 int cursory, int dir, displaymethod* d)
{
	switch (curoption) {
		case WLDINFO_AMMO:
			if (myworld->zhead->ammo + dir <= 32767 &&
					myworld->zhead->ammo + dir >= 0)
				myworld->zhead->ammo += dir;
			return 1;

		case WLDINFO_GEMS:
			if (myworld->zhead->gems + dir <= 32767 &&
					myworld->zhead->gems + dir >= 0)
				myworld->zhead->gems += dir;
			return 1;

		case WLDINFO_HEALTH:
			if (myworld->zhead->health + dir <= 32767 &&
					myworld->zhead->health + dir >= 0)
				myworld->zhead->health += dir;
			return 1;

		case WLDINFO_TORCHES:
			if (myworld->zhead->torches + dir <= 32767 &&
					myworld->zhead->torches + dir >= 0)
				myworld->zhead->torches += dir;
			return 1;

		case WLDINFO_SCORE:
			if (myworld->zhead->score + dir <= 32767 &&
					myworld->zhead->score + dir >= 0)
				myworld->zhead->score += dir;
			return 1;

		case WLDINFO_TCYCLES:
			if (myworld->zhead->torchcycles + dir <= 32767 &&
					myworld->zhead->torchcycles + dir >= 0)
				myworld->zhead->torchcycles += dir;
			return 1;

		case WLDINFO_ECYCLES:
			if (myworld->zhead->energizercycles + dir <= 32767 &&
					myworld->zhead->energizercycles + dir >= 0)
				myworld->zhead->energizercycles += dir;
			return 1;

		case WLDINFO_TIMEPASSED:
			if (myworld->zhead->timepassed + dir <= 32767 &&
					myworld->zhead->timepassed + dir >= 0)
				myworld->zhead->timepassed += dir;
			return 1;

		default:
			return worldinfoeditoption(curoption, myworld, cursorx, cursory, d);
	}
}

void worldinfotogglekey(world* myworld, int whichkey)
{
	switch (whichkey) {
		case WLDKEY_BLUE:
			myworld->zhead->bluekey = !myworld->zhead->bluekey;
			break;

		case WLDKEY_GREEN:
			myworld->zhead->greenkey = !myworld->zhead->greenkey;
			break;

		case WLDKEY_CYAN:
			myworld->zhead->cyankey = !myworld->zhead->cyankey;
			break;

		case WLDKEY_RED:
			myworld->zhead->redkey = !myworld->zhead->redkey;
			break;

		case WLDKEY_PURPLE:
			myworld->zhead->purplekey = !myworld->zhead->purplekey;
			break;

		case WLDKEY_YELLOW:
			myworld->zhead->yellowkey = !myworld->zhead->yellowkey;
			break;

		case WLDKEY_WHITE:
			myworld->zhead->whitekey = !myworld->zhead->whitekey;
			break;
	}
}

/* World Info Flags */

void drawstaticflags(displaymethod * d);
void drawflags(world * myworld, displaymethod * d);
void worldflagedit(int curoption, world* myworld,
									 int cursorx, int cursory, displaymethod* d);
void worldflagclear(int curoption, world* myworld);

void editworldflags(world* myworld, displaymethod* d)
{
	int curoption = 0;
	int cursorx, cursory;
	int done = 0;

	drawstaticflags(d);
	drawflags(myworld, d);

	cursorx = 31;

	do {
		int key;

		/* Position the cursor */
		cursory = curoption + 9;

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
				if (curoption > 0) curoption--; else curoption = 9;
				break;

			case DKEY_DOWN:
				if (curoption < 9) curoption++; else curoption = 0;
				break;

			case DKEY_ESC:
				done = 1;
				break;

			case DKEY_ENTER:
			case DKEY_LEFT:
			case DKEY_RIGHT:
				worldflagedit(curoption, myworld, cursorx, cursory, d);
				break;

			case DKEY_DELETE:
			case DKEY_BACKSPACE:
				worldflagclear(curoption, myworld);
				drawstaticflags(d);
				drawflags(myworld, d);
				break;
		}
	} while (!done);
}

void drawstaticflags(displaymethod * d)
{
	/* Draw the scroll box */
	drawscrollbox(0, 0, d);
	d->putch( 7, 13, ' ', 0x00);
	d->putch(51, 13, ' ', 0x00);

	/* Draw the static contents */
	d->print(25, 4, 0x0A, "World Info");

	d->print(23,  7, 0x0F, "Set/Clear Flags");
	d->print(22, 9+0, 0x0A, "Flag  1:");
	d->print(22, 9+1, 0x0A, "Flag  2:");
	d->print(22, 9+2, 0x0A, "Flag  3:");
	d->print(22, 9+3, 0x0A, "Flag  4:");
	d->print(22, 9+4, 0x0A, "Flag  5:");
	d->print(22, 9+5, 0x0A, "Flag  6:");
	d->print(22, 9+6, 0x0A, "Flag  7:");
	d->print(22, 9+7, 0x0A, "Flag  8:");
	d->print(22, 9+8, 0x0A, "Flag  9:");
	d->print(22, 9+9, 0x0A, "Flag 10:");
}

#define copyflag(buffer, flag, len) strncpy((buffer), (flag), (len)); buffer[(len)] = 0
#define printflag(buffer, num, flag, len) copyflag((buffer), (flag), (len)); d->print(31, 9+(num), 0x0B, (buffer))

void drawflags(world * myworld, displaymethod * d)
{
	char buffer[21];
	zztheader * zhead = myworld->zhead;

	printflag(buffer, 0, zhead->flag1, zhead->flag1len);
	printflag(buffer, 1, zhead->flag2, zhead->flag2len);
	printflag(buffer, 2, zhead->flag3, zhead->flag3len);
	printflag(buffer, 3, zhead->flag4, zhead->flag4len);
	printflag(buffer, 4, zhead->flag5, zhead->flag5len);
	printflag(buffer, 5, zhead->flag6, zhead->flag6len);
	printflag(buffer, 6, zhead->flag7, zhead->flag7len);
	printflag(buffer, 7, zhead->flag8, zhead->flag8len);
	printflag(buffer, 8, zhead->flag9, zhead->flag9len);
	printflag(buffer, 9, zhead->flag10, zhead->flag10len);
}

#define copytoflag(flag, buffer, i) for (i = 0; i < strlen(buffer); i++) (flag)[i] = (buffer)[i]; for (; i < 20; i++) (flag)[i] = 0

void worldflagedit(int curoption, world* myworld,
									 int cursorx, int cursory, displaymethod* d)
{
	zztheader * zhead = myworld->zhead;
	char buffer[21];
	int i;

	switch (curoption) {
		case 0:
			copyflag(buffer, zhead->flag1, zhead->flag1len);
			if (line_editor(cursorx, cursory, 0x0B, buffer, 20, LINED_NOLOWER, d)) {
				copytoflag(zhead->flag1, buffer, i);
				zhead->flag1len = strlen(buffer);
			}
			break;

		case 1:
			copyflag(buffer, zhead->flag2, zhead->flag2len);
			if (line_editor(cursorx, cursory, 0x0B, buffer, 20, LINED_NOLOWER, d)) {
				copytoflag(zhead->flag2, buffer, i);
				zhead->flag2len = strlen(buffer);
			}
			break;

		case 2:
			copyflag(buffer, zhead->flag3, zhead->flag3len);
			if (line_editor(cursorx, cursory, 0x0B, buffer, 20, LINED_NOLOWER, d)) {
				copytoflag(zhead->flag3, buffer, i);
				zhead->flag3len = strlen(buffer);
			}
			break;

		case 3:
			copyflag(buffer, zhead->flag4, zhead->flag4len);
			if (line_editor(cursorx, cursory, 0x0B, buffer, 20, LINED_NOLOWER, d)) {
				copytoflag(zhead->flag4, buffer, i);
				zhead->flag4len = strlen(buffer);
			}
			break;

		case 4:
			copyflag(buffer, zhead->flag5, zhead->flag5len);
			if (line_editor(cursorx, cursory, 0x0B, buffer, 20, LINED_NOLOWER, d)) {
				copytoflag(zhead->flag5, buffer, i);
				zhead->flag5len = strlen(buffer);
			}
			break;

		case 5:
			copyflag(buffer, zhead->flag6, zhead->flag6len);
			if (line_editor(cursorx, cursory, 0x0B, buffer, 20, LINED_NOLOWER, d)) {
				copytoflag(zhead->flag6, buffer, i);
				zhead->flag6len = strlen(buffer);
			}
			break;

		case 6:
			copyflag(buffer, zhead->flag7, zhead->flag7len);
			if (line_editor(cursorx, cursory, 0x0B, buffer, 20, LINED_NOLOWER, d)) {
				copytoflag(zhead->flag7, buffer, i);
				zhead->flag7len = strlen(buffer);
			}
			break;

		case 7:
			copyflag(buffer, zhead->flag8, zhead->flag8len);
			if (line_editor(cursorx, cursory, 0x0B, buffer, 20, LINED_NOLOWER, d)) {
				copytoflag(zhead->flag8, buffer, i);
				zhead->flag8len = strlen(buffer);
			}
			break;

		case 8:
			copyflag(buffer, zhead->flag9, zhead->flag9len);
			if (line_editor(cursorx, cursory, 0x0B, buffer, 20, LINED_NOLOWER, d)) {
				copytoflag(zhead->flag9, buffer, i);
				zhead->flag9len = strlen(buffer);
			}
			break;

		case 9:
			copyflag(buffer, zhead->flag10, zhead->flag10len);
			if (line_editor(cursorx, cursory, 0x0B, buffer, 20, LINED_NOLOWER, d)) {
				copytoflag(zhead->flag10, buffer, i);
				zhead->flag10len = strlen(buffer);
			}
			break;
	}
}

void worldflagclear(int curoption, world* myworld)
{
	zztheader * zhead = myworld->zhead;
	char buffer[1] = "";
	int i;

	switch (curoption) {
		case 0:
			copytoflag(zhead->flag1, buffer, i);
			zhead->flag1len = 0;
			break;

	}
}


