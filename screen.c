/* screen.c    -- Functions for drawing
 * $Id: screen.c,v 1.1 2000/06/15 03:58:10 kvance Exp $
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

#include <string.h>
#include <dirent.h>

#include "panel_f1.h"
#include "panel_f2.h"
#include "panel_f3.h"
#include "display.h"
#include "kevedit.h"
#include "panel.h"
#include "tbox.h"
#include "cbox.h"
#include "zzt.h"

extern char filelist[500][13];
extern patdef patdefs[16];
extern param *patparams[10];

void drawpanel(displaymethod * d)
{
	int x, y, i = 0;

	for (y = 0; y < 25; y++) {
		for (x = 0; x < 20; x++) {
			d->putch(x + 60, y, PANEL_MAIN[i], PANEL_MAIN[i + 1]);
			i += 2;
		}
	}
}

void updatepanel(displaymethod * d, editorinfo * e, world * w)
{
	int i, x;
	char s[255];

	/* (x, y) position */
	d->putch(62, 0, ' ', 0x1f);
	d->putch(63, 0, ' ', 0x1f);
	d->putch(76, 0, ' ', 0x1f);
	d->putch(77, 0, ' ', 0x1f);
	sprintf(s, "(%d, %d) %d/150", e->cursorx + 1, e->cursory + 1, w->board[e->curboard]->info->objectcount);
	i = 70 - strlen(s) / 2;
	for (x = 0; x < strlen(s); x++) {
		d->putch(i + x, 0, s[x], 0x1c);
	}

	/* Draw Mode? */
	if (e->drawmode == 0)
		i = 0x1f;
	else
		i = 0x1e;
	d->putch(69, 15, 'D', i);
	d->putch(70, 15, 'r', i);
	d->putch(71, 15, 'a', i);
	d->putch(72, 15, 'w', i);

	/* Blink Mode? */
	if (e->blinkmode == 0)
		i = 0x1f;
	else
		i = 0x1e;
	d->putch(65, 19, 'B', i);
	d->putch(66, 19, 'l', i);
	d->putch(67, 19, 'i', i);
	d->putch(68, 19, 'n', i);
	d->putch(69, 19, 'k', i);

	/* Arrows to point at current colour/pattern */
	for (i = 61; i < 77; i++) {
		d->putch(i, 22, ' ', 0x1f);
	}
	for (i = 61; i < 77; i++) {
		d->putch(i, 24, ' ', 0x1f);
	}
	for (i = 61; i < 78; i++) {
		d->putch(i, 20, ' ', 0x1f);
	}
	d->putch(61 + e->forec, 22, 31, 0x17);
	d->putch(69 + e->backc, 24, 30, 0x17);
	if (e->pattern < 6)
		d->putch(61 + e->pattern, 20, 31, 0x17);
	else
		d->putch(62 + e->pattern, 20, 31, 0x17);

	/* Default colour mode? */
	if (e->defc == 1)
		d->putch(78, 23, 'D', 0x1e);
	else
		d->putch(78, 23, 'd', 0x18);

	/* Too long title */
	if (strlen(e->currenttitle) > 8) {
		for (x = 0; x < 5; x++) {
			d->putch(71 + x, 1, e->currenttitle[x], 0x17);
		}
		for (x = 0; x < 3; x++) {
			d->putch(76 + x, 1, '.', 0x1f);
		}
	} else {
		/* Regular title */
		d->print(71, 1, 0x17, e->currenttitle);
	}
/* NOTE: For DOS/Windows95, we should add a bit to change the titlebar:
   INT 2F - Windows95 - TITLE - SET APPLICATION TITLE
   AX = 168Eh
   DX = 0000h
   ES:DI -> ASCIZ application title (max 79 chars+NUL)
   Return: AX = status
   0000h failed
   0001h successful
   Note:   if ES:DI is 0000h:0000h or points at an empty string, the current
   title is removed
   BUG:    this function can return a successful status even though the title was
   not changed; reportedly, waiting for two clock ticks after program
   startup solves this problem
 */

	/* Draw pattern stack */
	for (i = 0; i < 10; i++) {
		d->putch(68 + i, 21,
			 z_getchar(patdefs[6 + i].type, patdefs[6 + i].color, patparams[i],
				   NULL, 0, 0),
			 z_getcolour(patdefs[6 + i].type, patdefs[6 + i].color, patparams[i]));
	}
}

void drawscreen(displaymethod * d, world * w, editorinfo * e, char *bigboard, unsigned char paramlist[60][25])
{
	int x, y, z, i = 0;

	for (y = 0; y < 25; y++) {
		for (x = 0; x < 60; x++) {
			d->putch(x, y,
				 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y]], bigboard, x, y),
				 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y]])
			    );
			i += 2;
		}
	}
}

/* Make the cursor more visible */
void cursorspace(displaymethod *d, world *w, editorinfo *e, char *bigboard, unsigned char paramlist[60][25])
{
	char c, b, f;
	int i = (e->cursorx + e->cursory * 60) * 2;
	c = z_getcolour(bigboard[i], bigboard[i+1], w->board[e->curboard]->params[paramlist[e->cursorx][e->cursory]]);
	f = c & 0x0f;
	b = (c & 0xf0) >> 4;
	if(f < 8)
		f += 8;
	else
		f -= 8;
	if(f == b)
		c = 7;
	else
		c = (b << 4) + f;
	d->putch(e->cursorx, e->cursory, z_getchar(bigboard[i], bigboard[i+1], w->board[e->curboard]->params[paramlist[e->cursorx][e->cursory]], bigboard, e->cursorx, e->cursory), c);
}

/* Update a spot around the cursor */
void drawspot(displaymethod * d, world * w, editorinfo * e, char *bigboard, unsigned char paramlist[60][25])
{
	int x, y, i;
	x = e->cursorx;
	y = e->cursory;

	if (y - 1 > 0) {
		if (x - 1 > 0) {
			i = ((x - 1) + (y - 1) * 60) * 2;
			d->putch(x - 1, y - 1,
				 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x - 1][y - 1]], bigboard, x - 1, y - 1),
				 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x - 1][y - 1]])
			    );
		}
		i = (x + (y - 1) * 60) * 2;
		d->putch(x, y - 1,
			 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y - 1]], bigboard, x, y - 1),
			 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y - 1]])
		    );
		if (x + 1 < 60) {
			i = ((x + 1) + (y - 1) * 60) * 2;
			d->putch(x + 1, y - 1,
				 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x + 1][y - 1]], bigboard, x + 1, y - 1),
				 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x + 1][y - 1]])
			    );
		}
	}
	if (x - 1 > 0) {
		i = ((x - 1) + y * 60) * 2;
		d->putch(x - 1, y,
			 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x - 1][y]], bigboard, x - 1, y),
			 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x - 1][y]])
		    );
	}
	i = (x + y * 60) * 2;
	d->putch(x, y,
		 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y]], bigboard, x, y),
		 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y]])
	    );
	if (x + 1 < 60) {
		i = ((x + 1) + y * 60) * 2;
		d->putch(x + 1, y,
			 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x + 1][y]], bigboard, x + 1, y),
			 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x + 1][y]])
		    );
	}
	if (y + 1 < 25) {
		if (x - 1 > 0) {
			i = ((x - 1) + (y + 1) * 60) * 2;
			d->putch(x - 1, y + 1,
				 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x - 1][y + 1]], bigboard, x - 1, y + 1),
				 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x - 1][y + 1]])
			    );
		}
		i = (x + (y + 1) * 60) * 2;
		d->putch(x, y + 1,
			 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y + 1]], bigboard, x, y + 1),
			 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y + 1]])
		    );
		if (x + 1 < 60) {
			i = ((x + 1) + (y + 1) * 60) * 2;
			d->putch(x + 1, y + 1,
				 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x + 1][y + 1]], bigboard, x + 1, y + 1),
				 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x + 1][y + 1]])
			    );
		}
	}
}

int sort_function(const void *a, const void *b)
{
	return strcmp((char *) a, (char *) b);
}


int filedialog(char *extention, displaymethod * mydisplay)
{
	DIR *dp;
	struct dirent *dirent;
	int i, t, x, listpos, offset;
	int subc, sube;

	dp = opendir(".");
	if (dp == NULL)
		return -1;
	if (strlen(extention) != 3)
		return -1;

	i = 0;
	drawscrollbox(0, mydisplay);
	mydisplay->print(25, 4, 0x0a, "Load World");
	x = 0;
	while (1) {
		dirent = readdir(dp);
		if (dirent == NULL || x == 500)
			break;
		if (tolower(dirent->d_name[strlen(dirent->d_name) - 1]) == extention[2] && tolower(dirent->d_name[strlen(dirent->d_name) - 2]) == extention[1] && tolower(dirent->d_name[strlen(dirent->d_name) - 3]) == extention[0] && dirent->d_name[strlen(dirent->d_name) - 4] == '.') {
			strcpy(filelist[x], dirent->d_name);
			x++;
		}
	}
	closedir(dp);
	if (x != 0) {
		/* This qsort sure was worth all that wasted memory */
		qsort(filelist, x, 13, sort_function);
	}
	listpos = 0;
	offset = 7;
	subc = 0;
	mydisplay->cursorgo(9, 13);
	while (subc != 27) {
		drawscrollbox(3, mydisplay);
		for (t = (offset >= 0) ? offset : 0; t < 15 && (t - offset) < x; t++) {
			mydisplay->print(9, t + 6, 0x0a, filelist[t - offset]);
		}

		sube = 0;
		subc = mydisplay->getch();
		if (!subc) {
			sube = 1;
			subc = mydisplay->getch();
		}
		if (sube == 1 && subc == 72) {
			/* Up Arrow */
			if (listpos > 0) {
				listpos--;
				offset++;
			}
		}
		if (sube == 1 && subc == 80) {
			/* Down Arrow */
			if (listpos < x - 1) {
				listpos++;
				offset--;
			}
		}
		if (sube == 1 && subc == 73) {
			/* Page Up */
			listpos -= 7;
			if (listpos < 0) {
				offset += 7 + listpos;
				listpos = 0;
			} else
				offset += 7;
		}
		if (sube == 1 && subc == 81) {
			/* Page Down */
			listpos += 7;
			if (listpos > x) {
				offset -= 6 - (listpos - x);
				listpos = x - 1;
			} else
				offset -= 7;
		}
		if (subc == 13) {
			/* Enter */
			if (x == 0)
				return -1;
			return listpos;
		}
	}
	return -1;
}

char *titledialog(displaymethod *d)
{
	char *t;
	int x, y, i = 0;

	t = (char *) malloc(35);
	memset(t, '\0', 35);

	for(y = 12; y < 12 + TITLE_BOX_DEPTH; y++) {
		for(x = 10; x < 10 + TITLE_BOX_WIDTH; x++) {
			d->putch(x, y, TITLE_BOX[i], TITLE_BOX[i + 1]);
			i += 2;
		}
	}
	i = 0;
	while(x != 13) {
		d->cursorgo(12+i, 13);
		x = d->getch();
		switch(x) {
			case 8:
				if(i > 0) {
					i--;
					t[i] = '\0';
					d->putch(12+i, 13, ' ', 0x0f);
				}
				break;
			case 13:
				if(i == 0)
					x = 8;
				break;
			default:
				if(i == 34)
					break;
				if(x < 32)
					break;
				t[i] = x;
				d->putch(12+i, 13, x, 0x0f);
				i++;
				d->cursorgo(12+i, 13);
				break;
		}
	}
	return t;
}

int boarddialog(world * w, editorinfo * e, displaymethod * mydisplay)
{
	int i, t, listpos, offset;
	int subc, sube;

	drawscrollbox(0, mydisplay);
	mydisplay->print(23, 4, 0x0a, "Switch Boards");

	listpos = e->curboard;
	offset = 7 - e->curboard;
	subc = 0;
	mydisplay->cursorgo(9, 13);
	while (subc != 27) {
		drawscrollbox(3, mydisplay);
		for (t = (offset >= 0) ? offset : 0; t < 15 && (t - offset) <= w->zhead->boardcount + 1; t++) {
			if ((t - offset) == w->zhead->boardcount + 1)
				mydisplay->print(9, t + 6, 0x0a, "Add new board");
			else
				mydisplay->print(9, t + 6, 0x0a, w->board[t - offset]->title);
		}

		sube = 0;
		subc = mydisplay->getch();
		if (!subc) {
			sube = 1;
			subc = mydisplay->getch();
		}
		if (sube == 1 && subc == 72) {
			/* Up Arrow */
			if (listpos > 0) {
				listpos--;
				offset++;
			}
		}
		if (sube == 1 && subc == 80) {
			/* Down Arrow */
			if (listpos <= w->zhead->boardcount) {
				listpos++;
				offset--;
			}
		}
		if (sube == 1 && subc == 73) {
			/* Page Up */
			listpos -= 7;
			if (listpos < 0) {
				offset += 7 + listpos;
				listpos = 0;
			} else
				offset += 7;
		}
		if (sube == 1 && subc == 81) {
			/* Page Down */
			listpos += 7;
			if (listpos >= w->zhead->boardcount + 1) {
				offset -= 8 - (listpos - w->zhead->boardcount);
				listpos = w->zhead->boardcount + 1;
			} else
				offset -= 7;
		}
		if (subc == 13) {
			/* Enter */
			if (listpos == w->zhead->boardcount + 1) {
				/* We have to create a new one */
				if (w->zhead->boardcount == 255) {
					/* Oops, too many! */
					subc = 255;
				} else {
					w->zhead->boardcount++;
					w->board[w->zhead->boardcount] = z_newboard(titledialog(mydisplay));
				}
			}
			if (subc != 255)
				return listpos;
		}
	}
	return -1;
}


int dothepanel_f1(displaymethod * d, editorinfo * e)
{
	int x, y, i = 0;

	for (y = 3; y < 20; y++) {
		for (x = 0; x < 20; x++) {
			d->putch(x + 60, y, PANEL_F1[i], PANEL_F1[i + 1]);
			i += 2;
		}
	}
	d->putch(78, 4, 2, 0x1f);
	d->putch(78, 5, 132, e->defc ? 0x03 : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 6, 157, e->defc ? 0x06 : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 7, 4, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 8, 12, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	if (e->defc == 1)
		d->putch(78, 9, 10, e->forec > 7 ? ((e->forec - 8) << 4) + 0x0f : (e->forec << 4) + 0x0f);
	else
		d->putch(78, 9, 10, (e->backc << 4) + (e->forec));
	d->putch(78, 10, 232, 0x0f);
	if (e->defc == 1)
		d->putch(78, 11, 240, e->forec > 7 ? ((e->forec - 8) << 4) + 0x0f : (e->forec << 4) + 0x0f);
	else
		d->putch(78, 11, 240, (e->backc << 4) + (e->forec));
	d->putch(78, 12, 250, e->defc ? 0x0f : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 13, 11, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 14, 127, e->defc ? 0x05 : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 17, 47, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 18, 92, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));

	while (1) {
		i = d->getch();
		switch (i) {
		case 27:
		case 13:
			return -1;
		case 'Z':
		case 'z':
			return Z_PLAYER;
		case 'A':
		case 'a':
			return Z_AMMO;
		case 'T':
		case 't':
			return Z_TORCH;
		case 'G':
		case 'g':
			return Z_GEM;
		case 'K':
		case 'k':
			return Z_KEY;
		case 'D':
		case 'd':
			return Z_DOOR;
		case 'S':
		case 's':
			return Z_SCROLL;
		case 'P':
		case 'p':
			return Z_PASSAGE;
		case 'U':
		case 'u':
			return Z_DUPLICATOR;
		case 'B':
		case 'b':
			return Z_BOMB;
		case 'E':
		case 'e':
			return Z_ENERGIZER;
		case '1':
			return Z_CWCONV;
		case '2':
			return Z_CCWCONV;
		}
	}
}

int dothepanel_f2(displaymethod * d, editorinfo * e)
{
	int x, y, i = 0;

	for (y = 3; y < 20; y++) {
		for (x = 0; x < 20; x++) {
			d->putch(x + 60, y, PANEL_F2[i], PANEL_F2[i + 1]);
			i += 2;
		}
	}
	d->putch(78, 6, 1, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	while (1) {
		i = d->getch();
		switch (i) {
		case 27:
		case 13:
			return -1;
		case 'O':
		case 'o':
			return Z_OBJECT;
		}
	}
}

int dothepanel_f3(displaymethod * d, editorinfo * e)
{
	int x, y, i = 0;

	for (y = 3; y < 20; y++) {
		for (x = 0; x < 20; x++) {
			d->putch(x + 60, y, PANEL_F3[i], PANEL_F3[i + 1]);
			i += 2;
		}
	}
	d->getch();
}

char charselect(displaymethod *d)
{
	int x, y, e, i = 0;

	for(y = 0; y < CHAR_BOX_DEPTH; y++) {
		for(x = 0; x < CHAR_BOX_WIDTH; x++) {
			d->putch(x+13, y+8, CHAR_BOX[i], CHAR_BOX[i+1]);
			i += 2;
		}
	}
	x = y = i = 0;
	while(1) {
		d->cursorgo(14+x, 9+y);
		d->putch(14+x, 9+y, (x+y*32), 0x0f);
		e = 0;
		i = d->getch();
		d->putch(14+x, 9+y, (x+y*32), 0x0a);
		if(!i) {
			e = 1;
			i = d->getch();
		}
		if(e == 1 && i == 72) {
			/* Up Arrow */
			if(y > 0)
				y--;
		}
		if(e == 1 && i == 80) {
			/* Down Arrow */
			if(y < 7)
				y++;
		}
		if(e == 1 && i == 75) {
			/* Left Arrow */
			if(x > 0)
				x--;
		}
		if(e == 1 && i == 77) {
			/* Left Arrow */
			if(x < 31)
				x++;
		}
		if(e == 0 && i == 13) {
			/* Enter */
			i = (x+y*32);
			break;
		}
	}

	return i;
}
