/* infobox.c - board/world information dialogs
 * $Id: infobox.c,v 1.11 2002/02/18 08:04:40 bitman Exp $
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

#include "screen.h"

#include "libzzt2/zzt.h"
#include "help.h"

#include "panel_bi.h"
#include "panel_wi.h"

#include "display.h"

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
void drawboardinfo(ZZTworld* myworld, displaymethod* d);
int boardinfoeditoption(int curoption, ZZTworld* myworld,
												int cursorx, int cursory, displaymethod* d);
int boardinfodirectionoption(int curoption, ZZTworld* myworld,
												int cursorx, int cursory, int dir, displaymethod* d);
void boardinfostaroption(int curoption, ZZTworld* myworld);

/* editboardinfo() - brings up dialog box for editing board info */
void editboardinfo(ZZTworld* myworld, displaymethod* d)
{
	int curoption = BRDINFO_TITLE;
	int cursorx, cursory;
	int done = 0;
	drawstaticboardinfo(d);

	drawboardinfo(myworld, d);

	do {
		int key;

		/* Position the cursors */
		cursory = curoption + 7;
		if (curoption > BRDINFO_TITLE)
			cursory++;
		if (curoption >= BRDINFO_BRDNORTH)
			cursory += 2;

		if (curoption == BRDINFO_TITLE)
			cursorx = 30 - (strlen(zztBoardGetTitle(myworld)) / 2);
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
				if (boardinfoeditoption(curoption, myworld,
																cursorx, cursory, d)) {
					drawstaticboardinfo(d);
					drawboardinfo(myworld, d);
				}
				break;

			case DKEY_LEFT:
			case '-':
				if (boardinfodirectionoption(curoption, myworld,
																		 cursorx, cursory, -1, d)) {
					drawstaticboardinfo(d);
					drawboardinfo(myworld, d);
				}
				break;

			case DKEY_RIGHT:
			case '+':
				if (boardinfodirectionoption(curoption, myworld,
																		 cursorx, cursory, 1, d)) {
					drawstaticboardinfo(d);
					drawboardinfo(myworld, d);
				}
				break;

			case '*':
				boardinfostaroption(curoption, myworld);
				drawstaticboardinfo(d);
				drawboardinfo(myworld, d);
				break;

			case DKEY_F1:
				helpsectiontopic("kbrdinfo", NULL, d);
				drawstaticboardinfo(d);
				drawboardinfo(myworld, d);
				break;
		}
	} while (!done);
}

void drawstaticboardinfo(displaymethod* d)
{
	/* Draw the side panel */
	drawsidepanel(d, PANEL_BOARDINFO);

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

	d->print(18, 19, 0x0a, "Board Size:");
}

void drawboardinfo(ZZTworld* myworld, displaymethod* d)
{
	char buffer[10];    /* Buffer for translating numbers to strings */
	int curboard = zztBoardGetCurrent(myworld);
	int boardsize = zztBoardGetSize(zztBoardGetCurPtr(myworld));

	/* Title */
	d->print(30 - (strlen(zztBoardGetTitle(myworld)) / 2), 7, 0x0B, zztBoardGetTitle(myworld));

	/* Boolean */
	d->print(36,  9, 0x0B, (zztBoardGetDarkness(myworld)  ? "Yes" : "No"));
	d->print(36, 10, 0x0B, (zztBoardGetReenter(myworld)   ? "Yes" : "No"));

	/* Numerical */
	/* Time Limit */
	if (zztBoardGetTimelimit(myworld) > 0)
		sprintf(buffer, "%d", zztBoardGetTimelimit(myworld));
	else
		strcpy(buffer, "Infinite");
	d->print(36, 11, 0x0B, buffer);

	/* Maximum shots */
	sprintf(buffer, "%d", zztBoardGetMaxshots(myworld));
	d->print(36, 12, 0x0B, buffer);

	/* Board links */
	/* North */
	if (zztBoardGetBoard_n (myworld)> 0) {
		d->print(14, 15, 0x0B, myworld->boards[zztBoardGetBoard_n(myworld)].title);
		if (curboard > 0 &&
				myworld->boards[zztBoardGetBoard_n(myworld)].info.board_s == curboard)
			d->putch(9, 15, '*', 0x0B);
	} else {
		d->print(14, 15, 0x03, "(None)");
	}

	/* South */
	if (zztBoardGetBoard_s (myworld)> 0) {
		d->print(14, 16, 0x0B, myworld->boards[zztBoardGetBoard_s(myworld)].title);
		if (curboard > 0 &&
				myworld->boards[zztBoardGetBoard_s(myworld)].info.board_n == curboard)
			d->putch(9, 16, '*', 0x0B);
	} else {
		d->print(14, 16, 0x03, "(None)");
	}

	/* East */
	if (zztBoardGetBoard_e (myworld)> 0) {
		d->print(14, 17, 0x0B, myworld->boards[zztBoardGetBoard_e(myworld)].title);
		if (curboard > 0 &&
				myworld->boards[zztBoardGetBoard_e(myworld)].info.board_w == curboard)
			d->putch(9, 17, '*', 0x0B);
	} else {
		d->print(14, 17, 0x03, "(None)");
	}

	/* West */
	if (zztBoardGetBoard_w (myworld)> 0) {
		d->print(14, 18, 0x0B, myworld->boards[zztBoardGetBoard_w(myworld)].title);
		if (curboard > 0 &&
				myworld->boards[zztBoardGetBoard_w(myworld)].info.board_e == curboard)
			d->putch(9, 18, '*', 0x0B);
	} else {
		d->print(14, 18, 0x03, "(None)");
	}

	sprintf(buffer, "%d", boardsize);
	d->print(30, 19, (boardsize < 20000 ? 0x0B : 0x0C), buffer);
	d->print(31 + strlen(buffer), 19, 0x03, "bytes");

	sprintf(buffer, "%.3f", (float)boardsize / 1024);
	d->print(30, 20, (boardsize < 20000 ? 0x0B : 0x0C), buffer);
	d->print(31 + strlen(buffer), 20, 0x03, "KB");
}


int boardinfoeditoption(int curoption, ZZTworld* myworld,
												int cursorx, int cursory, displaymethod* d)
{
	int i;
	char buffer[35];

	switch (curoption) {
		case BRDINFO_TITLE:
			/* Change the title of the board */
			strcpy(buffer, zztBoardGetTitle(myworld));
			if (line_editor(13, cursory, 0x0f, buffer, 34, LINED_NORMAL, d)
					== LINED_OK) {
				zztBoardSetTitle(myworld, buffer);
			}
			/* Update */
			return 1;

		case BRDINFO_BRDNORTH:
			zztBoardSetBoard_n(myworld, boarddialog(myworld, zztBoardGetBoard_n(myworld), "Board to the North", 1, d));
			return 2;

		case BRDINFO_BRDSOUTH:
			zztBoardSetBoard_s(myworld, boarddialog(myworld, zztBoardGetBoard_s(myworld), "Board to the South", 1, d));
			return 2;

		case BRDINFO_BRDEAST:
			zztBoardSetBoard_e(myworld, boarddialog(myworld, zztBoardGetBoard_e(myworld), "Board to the East", 1, d));
			return 2;

		case BRDINFO_BRDWEST:
			zztBoardSetBoard_w(myworld, boarddialog(myworld, zztBoardGetBoard_w(myworld), "Board to the West", 1, d));
			return 2;

		case BRDINFO_DARKNESS:
			zztBoardSetDarkness(myworld, !zztBoardGetDarkness(myworld));
			return 1;

		case BRDINFO_REENTER:
			zztBoardSetReenter(myworld, !zztBoardGetReenter(myworld));
			return 1;

		case BRDINFO_MAXSHOTS:
			sprintf(buffer, "%d", zztBoardGetMaxshots(myworld));
			if (line_editor(cursorx, cursory, 0x0f, buffer, 3,
											LINED_NOALPHA | LINED_NOPUNCT | LINED_NOSPACES, d)
					== LINED_OK) {
				int maxshots;
				sscanf(buffer, "%d", &maxshots);
				if (maxshots > 255)
					zztBoardSetMaxshots(myworld, 255);
				else
					zztBoardSetMaxshots(myworld, (u_int8_t) maxshots);
			}
			return 1;

		case BRDINFO_TIMELIM:
			/* Clear the word "Infinite" if it's there */
			for (i = 0; i < 10; i++)
				d->putch(cursorx + i, cursory, ' ', 0x00);

			/* Load the timelimit into the buffer */
			if (zztBoardGetTimelimit(myworld) != 0)
				sprintf(buffer, "%d", zztBoardGetTimelimit(myworld));
			else
				strcpy(buffer, "");

			if (line_editor(cursorx, cursory, 0x0f, buffer, 5,
											LINED_NOALPHA | LINED_NOPUNCT | LINED_NOSPACES, d)
					== LINED_OK) {
				long int timelimit;
				sscanf(buffer, "%ld", &timelimit);
				if (strlen(buffer) == 0)
					zztBoardSetTimelimit(myworld, 0);
				else if (timelimit > 32767)
					zztBoardSetTimelimit(myworld, 32767);
				else
					zztBoardSetTimelimit(myworld, timelimit);
			}
			return 1;
	}
	return 0;
}

int boardinfodirectionoption(int curoption, ZZTworld* myworld,
												int cursorx, int cursory, int dir, displaymethod* d)
{
	/* It's easier this way */
	ZZTboardinfo* info = &(myworld->boards[zztBoardGetCurrent(myworld)].info);

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
			if (info->board_n + dir <= zztWorldGetBoardcount(myworld) &&
					info->board_n + dir >= 0)
				info->board_n += dir;
			return 1;

		case BRDINFO_BRDSOUTH:
			if (info->board_s + dir <= zztWorldGetBoardcount(myworld) &&
					info->board_s + dir >= 0)
				info->board_s += dir;
			return 1;

		case BRDINFO_BRDEAST:
			if (info->board_e + dir <= zztWorldGetBoardcount(myworld) &&
					info->board_e + dir >= 0)
				info->board_e += dir;
			return 1;

		case BRDINFO_BRDWEST:
			if (info->board_w + dir <= zztWorldGetBoardcount(myworld) &&
					info->board_w + dir >= 0)
				info->board_w += dir;
			return 1;

		default:
			return boardinfoeditoption(curoption, myworld,
																 cursorx, cursory, d);
	}
}

void boardinfostaroption(int curoption, ZZTworld* myworld)
{
	int curboard = zztBoardGetCurrent(myworld);
	ZZTboardinfo* info = &(myworld->boards[zztBoardGetCurrent(myworld)].info);

	if (curboard == 0)
		return;

	switch (curoption) {
		case BRDINFO_BRDNORTH:
			if (info->board_n > 0)
				myworld->boards[info->board_n].info.board_s = curboard;
			break;

		case BRDINFO_BRDSOUTH:
			if (info->board_s > 0)
				myworld->boards[info->board_s].info.board_n = curboard;
			break;

		case BRDINFO_BRDEAST:
			if (info->board_e > 0)
				myworld->boards[info->board_e].info.board_w = curboard;
			break;

		case BRDINFO_BRDWEST:
			if (info->board_w > 0)
				myworld->boards[info->board_w].info.board_e = curboard;
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

void drawstaticworldinfo(displaymethod* d);
void drawworldinfo(ZZTworld* myworld, displaymethod* d);
int worldinfoeditoption(int curoption, ZZTworld* myworld,
												int cursorx, int cursory, displaymethod* d);
int worldinfodirectionoption(int curoption, ZZTworld* myworld,
												int cursorx, int cursory, int dir, displaymethod* d);
void worldinfotogglekey(ZZTworld* myworld, int whichkey);
void editworldflags(ZZTworld* myworld, displaymethod* d);

/* editworldinfo() - brings up dialog box for editing world info */
void editworldinfo(ZZTworld* myworld, displaymethod* d)
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
													 else cursorx = 31 + ZZT_KEY_WHITE; break;
				case DKEY_RIGHT: if (cursorx < 31 + ZZT_KEY_WHITE) cursorx++;
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
	/* Draw the side panel */
	drawsidepanel(d, PANEL_WORLDINFO);

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

/* TODO: These can probably be unmacro'ed now */
#define drawkey(which)  d->putch(31 + (which), 8, '\x0C', 0x08 +  (which) + 1)
#define drawdoor(which) d->putch(31 + (which), 8, '\x0A', 0x0F + (((which) + 1) << 4))

void drawworldinfo(ZZTworld* myworld, displaymethod* d)
{
	char buffer[10];    /* Buffer for translating numbers to strings */
	int i;

	/* Start at the top */
	d->print(31,  6, 0x0B, zztWorldGetTitle(myworld));

	/* List the keys */
	for (i = ZZT_KEY_BLUE; i <= ZZT_KEY_WHITE; i++) {
		if (zztWorldGetKey(myworld, i) != 0) drawkey(i); else drawdoor(i);
	}

	/* Inventory */
	sprintf(buffer, "%d", zztWorldGetAmmo(myworld));     d->print(31,  9, 0x0B, buffer);
	sprintf(buffer, "%d", zztWorldGetGems(myworld));     d->print(31, 10, 0x0B, buffer);
	sprintf(buffer, "%d", zztWorldGetHealth(myworld));   d->print(31, 11, 0x0B, buffer);
	sprintf(buffer, "%d", zztWorldGetTorches(myworld));  d->print(31, 12, 0x0B, buffer);
	sprintf(buffer, "%d", zztWorldGetScore(myworld));    d->print(31, 13, 0x0B, buffer);

	/* Misc */
	sprintf(buffer, "%d", zztWorldGetTorchcycles(myworld));    d->print(31, 15, 0x0B, buffer);
	sprintf(buffer, "%d", zztWorldGetEnergizercycles(myworld));d->print(31, 16, 0x0B, buffer);
	sprintf(buffer, "%d", zztWorldGetTimepassed(myworld));     d->print(31, 17, 0x0B, buffer);

	/* Saved Game boolean */
	d->print(31, 18, 0x0B, zztWorldGetSavegame(myworld) ? "Yes" : "No");
}

int worldinfoeditoption(int curoption, ZZTworld* myworld,
												int cursorx, int cursory, displaymethod* d)
{
	int tempnum;
	char buffer[35];
	ZZTworldinfo * header = myworld->header;

	switch (curoption) {
		case WLDINFO_NAME:
			/* Change the title of the board */
			strcpy(buffer, header->title);
			if (line_editor(cursorx, cursory, 0x0f, buffer, 19, LINED_NORMAL, d)
					== LINED_OK) {
				zztWorldSetTitle(myworld, buffer);
			}
			/* Update */
			return 1;

		case WLDINFO_ISSAVED:
			header->savegame = !header->savegame;
			return 1;

		case WLDINFO_FLAGS:
			editworldflags(myworld, d);
			return 1;

		case WLDINFO_AMMO:
			tempnum = header->ammo;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			header->ammo = tempnum;
			return 1;

		case WLDINFO_GEMS:
			tempnum = header->gems;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			header->gems = tempnum;
			return 1;

		case WLDINFO_HEALTH:
			tempnum = header->health;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			header->health = tempnum;
			return 1;

		case WLDINFO_TORCHES:
			tempnum = header->torches;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			header->torches = tempnum;
			return 1;

		case WLDINFO_SCORE:
			tempnum = header->score;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			header->score = tempnum;
			return 1;

		case WLDINFO_TCYCLES:
			tempnum = header->torchcycles;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			header->torchcycles = tempnum;
			return 1;

		case WLDINFO_ECYCLES:
			tempnum = header->energizercycles;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			header->energizercycles = tempnum;
			return 1;

		case WLDINFO_TIMEPASSED:
			tempnum = header->timepassed;
			line_editnumber(cursorx, cursory, 0x0f, &tempnum, 32767, d);
			header->timepassed = tempnum;
			return 1;
	}

	return 0;
}

int worldinfodirectionoption(int curoption, ZZTworld* myworld, int cursorx,
														 int cursory, int dir, displaymethod* d)
{
	ZZTworldinfo * header = myworld->header;

	switch (curoption) {
		case WLDINFO_AMMO:
			if (header->ammo + dir <= 32767 &&
					header->ammo + dir >= 0)
				header->ammo += dir;
			return 1;

		case WLDINFO_GEMS:
			if (header->gems + dir <= 32767 &&
					header->gems + dir >= 0)
				header->gems += dir;
			return 1;

		case WLDINFO_HEALTH:
			if (header->health + dir <= 32767 &&
					header->health + dir >= 0)
				header->health += dir;
			return 1;

		case WLDINFO_TORCHES:
			if (header->torches + dir <= 32767 &&
					header->torches + dir >= 0)
				header->torches += dir;
			return 1;

		case WLDINFO_SCORE:
			if (header->score + dir <= 32767 &&
					header->score + dir >= 0)
				header->score += dir;
			return 1;

		case WLDINFO_TCYCLES:
			if (header->torchcycles + dir <= 32767 &&
					header->torchcycles + dir >= 0)
				header->torchcycles += dir;
			return 1;

		case WLDINFO_ECYCLES:
			if (header->energizercycles + dir <= 32767 &&
					header->energizercycles + dir >= 0)
				header->energizercycles += dir;
			return 1;

		case WLDINFO_TIMEPASSED:
			if (header->timepassed + dir <= 32767 &&
					header->timepassed + dir >= 0)
				header->timepassed += dir;
			return 1;

		default:
			return worldinfoeditoption(curoption, myworld, cursorx, cursory, d);
	}
}

void worldinfotogglekey(ZZTworld* myworld, int whichkey)
{
	zztWorldSetKey(myworld, whichkey, !zztWorldGetKey(myworld, whichkey));
}

/* World Info Flags */

void drawstaticflags(displaymethod * d);
void drawflags(ZZTworld * myworld, displaymethod * d);
void worldflagedit(int curoption, ZZTworld* myworld,
									 int cursorx, int cursory, displaymethod* d);
void worldflagclear(int curoption, ZZTworld* myworld);

void editworldflags(ZZTworld* myworld, displaymethod* d)
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

void drawflags(ZZTworld * myworld, displaymethod * d)
{
	int i;

	for (i = 0; i < ZZT_MAX_FLAGS; i++) {
		d->print(31, 9 + i, 0x0B, zztWorldGetFlag(myworld, i));
	}
}

void worldflagedit(int curoption, ZZTworld* myworld,
									 int cursorx, int cursory, displaymethod* d)
{
	char buffer[ZZT_FLAG_SIZE + 1];

	strcpy(buffer, zztWorldGetFlag(myworld, curoption));
	if (line_editor(cursorx, cursory, 0x0B, buffer, ZZT_FLAG_SIZE, LINED_NOLOWER, d) == LINED_OK) {
		zztWorldSetFlag(myworld, curoption, buffer);
	}
}

void worldflagclear(int curoption, ZZTworld* myworld)
{
	/* Very nice */
	zztWorldSetFlag(myworld, curoption, "");
}


