/* main.c       -- The buck starts here
 * $Id: main.c,v 1.32 2001/05/05 21:34:17 bitman Exp $
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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "display.h"
#include "kevedit.h"
#include "screen.h"
#include "scroll.h"
#include "zzt.h"
#include "editbox.h"
#include "register.h"
#include "patbuffer.h"
#include "misc.h"


int main(int argc, char **argv)
{
	int i, x, t;
	int c, e;
	int subc, sube;
	int listpos;
	int quit = 0;
	displaymethod *mydisplay = (displaymethod *) malloc(sizeof(displaymethod));
	editorinfo *myinfo = (editorinfo *) malloc(sizeof(editorinfo));
	char *string = (char *) malloc(sizeof(char) * 256);
	char *bigboard = (char *) malloc(BOARD_MAX * 2);
	char buffer[255];
	unsigned char paramlist[60][25];

	world *myworld;
	FILE *fp;
	param *pm;

	RegisterDisplays();
	mydisplay = &display;

	/* How many display methods? */
	for (x = 1; x < 9; x++) {
		if (mydisplay->next == NULL) {
			break;
		}
		mydisplay = mydisplay->next;
	}

	if (x > 1) {
		/* More than 1 display method available, user must choose */
		printf("Hi.  This seems to be your first time running KevEdit.  What display method\n"
		       "works best on your platform?\n\n");

		mydisplay = &display;
		for (i = 0; i < x; i++) {
			printf("[%d] %s\n", i + 1, mydisplay->name);
			if (mydisplay->next != NULL)
				mydisplay = mydisplay->next;
		}
		do {
			printf("\nSelect [1-%i]: ", x);
			fgets(string, 255, stdin);
			i = string[0];
		}
		while (i > 49 && i < 51);
		i -= '1';
		printf("\n%d SELECT\n", i);
		mydisplay = &display;
		for (x = 0; x < i; x++) {
			mydisplay = mydisplay->next;
		}
	} else {
		mydisplay = &display;
	}

	printf("Initializing %s version %s...\n", mydisplay->name, mydisplay->version);
	if (!mydisplay->init()) {
		printf("\nDisplay initialization failed.  Exiting.\n");
		free(mydisplay);
		free(myinfo);
		free(string);
		free(bigboard);
		return 1;
	}
	/* Set up initial info */

	myinfo->cursorx = myinfo->playerx = 0;
	myinfo->cursory = myinfo->playery = 0;
	myinfo->drawmode = 0;
	myinfo->gradmode = 0;
	myinfo->getmode = 0;
	myinfo->blinkmode = 0;
	myinfo->textentrymode = 0;
	myinfo->defc = 1;
	myinfo->forec = 0x0f;
	myinfo->backc = 0x00;
	myinfo->standard_patterns = patbuffer_create(6);
	myinfo->backbuffer = patbuffer_create(10);
	myinfo->pbuf = myinfo->standard_patterns;

	myinfo->currenttitle = (char *) malloc(21);
	strcpy(myinfo->currenttitle, "UNTITLED");
	myinfo->currentfile = (char *) malloc(14);
	strcpy(myinfo->currentfile, "untitled.zzt");

	myinfo->curboard = 0;

	/* Did we get a world on the command line? */
	/* TODO: strip full path if given (as Windows would give it to us)
	 * and change to that directory, putting only the name of the file itself
	 * in myinfo->currentfile. */
	myworld = NULL;
	if (argc > 1) {
		myworld = loadworld(argv[1]);
		if (myworld == NULL) {
			/* Maybe it's a .zzt */
			strcpy(buffer, argv[1]);
			strcat(buffer, ".zzt");
			myworld = loadworld(buffer);
		}
		if (myworld != NULL) {
			strncpy(myinfo->currentfile, argv[1], 13);
			strncpy(myinfo->currenttitle, myworld->zhead->title, 20);
			myinfo->currenttitle[myworld->zhead->titlelength] = '\0';
			myinfo->curboard = myworld->zhead->startboard;
			rle_decode(myworld->board[myworld->zhead->startboard]->data, bigboard);
			for (i = 0; i < 25; i++)
				for (x = 0; x < 60; x++)
					paramlist[x][i] = 0;
			for (i = 0; i < myworld->board[myinfo->curboard]->info->objectcount + 1; i++) {
				if (myworld->board[myinfo->curboard]->params[i]->x > 0 && myworld->board[myinfo->curboard]->params[i]->x < 61 && myworld->board[myinfo->curboard]->params[i]->y > 0 && myworld->board[myinfo->curboard]->params[i]->y < 26)
					paramlist[myworld->board[myinfo->curboard]->params[i]->x - 1][myworld->board[myinfo->curboard]->params[i]->y - 1] = i;
				myinfo->playerx = myworld->board[myinfo->curboard]->params[0]->x - 1;
				myinfo->playery = myworld->board[myinfo->curboard]->params[0]->y - 1;
			}
		}
	}
	/* Create the blank world */
	if (myworld == NULL) {
		myworld = z_newworld();
		myworld->board[0] = z_newboard("KevEdit World");
		rle_decode(myworld->board[0]->data, bigboard);
		for (i = 0; i < 25; i++)
			for (x = 0; x < 60; x++)
				paramlist[x][i] = 0;
	}
	/* Initialize pattern definitions */
	myinfo->standard_patterns->patterns[0].type = Z_SOLID;
	myinfo->standard_patterns->patterns[1].type = Z_NORMAL;
	myinfo->standard_patterns->patterns[2].type = Z_BREAKABLE;
	myinfo->standard_patterns->patterns[3].type = Z_WATER;
	myinfo->standard_patterns->patterns[4].type = Z_EMPTY;
	myinfo->standard_patterns->patterns[5].type = Z_LINE;
	pat_applycolordata(myinfo->standard_patterns, myinfo);

	/* Draw */

	drawpanel(mydisplay);
	updatepanel(mydisplay, myinfo, myworld);
	drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
	mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
	mydisplay->print(30 - strlen(myworld->board[myinfo->curboard]->title) / 2, 0, 0x70, myworld->board[myinfo->curboard]->title);

	/* Main loop begins */

	while (quit == 0) {
		cursorspace(mydisplay, myworld, myinfo, bigboard, paramlist);
		e = 0;
		c = mydisplay->getch();
		if (!c) {
			e = 1;
			c = mydisplay->getch();
		}
		/* Undo the cursorspace */
		i = (myinfo->cursorx + myinfo->cursory * 60) * 2;
		mydisplay->putch(myinfo->cursorx, myinfo->cursory,
				 z_getchar(bigboard[i], bigboard[i + 1], myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]], bigboard, myinfo->cursorx, myinfo->cursory),
				 z_getcolour(bigboard[i], bigboard[i + 1], myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]));

		/* Check for text entry */
		if (myinfo->textentrymode == 1 && e == 0) {
			if (c == 13 || c == 27) {
				/* Leave text entry mode */
				c = 62;
				e = 1;
			} else if(c == 8 && myinfo->cursorx > 0) {
				myinfo->cursorx--;
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				if (paramlist[myinfo->cursorx][myinfo->cursory] != 0) {
					/* We're overwriting a parameter */
					param_remove(myworld->board[myinfo->curboard], paramlist, myinfo->cursorx, myinfo->cursory);
				}
				bigboard[(myinfo->cursorx+myinfo->cursory*60)*2] = Z_EMPTY;
				bigboard[(myinfo->cursorx+myinfo->cursory*60)*2+1] = 0x07;
				updatepanel(mydisplay, myinfo, myworld);
			} else {
				if (c == 1) { /* ASCII selection */
					c = charselect(mydisplay, -1);
					drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				}
				/* Plot the text character */
				if (myinfo->cursorx != myinfo->playerx || myinfo->cursory != myinfo->playery) {
					if (paramlist[myinfo->cursorx][myinfo->cursory] != 0) {
						/* We're overwriting a parameter */
						param_remove(myworld->board[myinfo->curboard], paramlist, myinfo->cursorx, myinfo->cursory);
					}
					/* Determine the text code based on the FG colour */
					if (myinfo->forec == 0 || myinfo->forec == 8 || myinfo->forec == 15)
						i = 6;
					else if (myinfo->forec > 8)
						i = myinfo->forec - 9;
					else
						i = myinfo->forec - 1;
					/* XXX FIXME Was I mistaken about blinking text?
					   if(myinfo->blinkmode)
					   i += 8; */
					bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] = Z_BLUETEXT + i;
					bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] = c;
					c = 77;
					e = 1;
				} else
					continue;
			}
		}

		/* bitman's addition: vi keys, only when envar "KVI" is set will
		 * these apply. */
		if ((e == 0) && (getenv("KVI") != NULL)) {
			switch (c) {
			/* Look for vi keys */
			case 'h':
					e = 1;
					c = 75;
				break;
			case 'j':
					e = 1;
					c = 80;
				break;
			case 'k':
					e = 1;
					c = 72;
				break;
			case 'l':
					e = 1;
					c = 77;
				break;
			/* perhaps bitman will someday add jump movement for these keys as well? */
			}
		}

		/* Act on key pressed */
		switch (c) {
		case '!':
			if (e == 0) {
				/* Open text editor */
				texteditor(mydisplay);

				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
			}
			break;
		case 'z':
		case 'Z':
			if (e == 0) {
				if (confirmprompt(mydisplay, "Clear board?") != 0) {
					clearboard(myworld, myinfo, bigboard, paramlist);

					drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				}

				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
			}
			break;
		case 'n':
		case 'N':
			if (e == 0) {
				if (confirmprompt(mydisplay, "Make new world?") != 0) {
					myworld = clearworld(myworld, myinfo, bigboard, paramlist);

					drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				}

				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
			}
			break;
		case 'q':
		case 'Q':
			/* Quit */
			if (confirmprompt(mydisplay, "Quit?") != 0) {
				quit = 1;
			} else {
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
			}
			break;
		case 'd':
		case 'D':
			/* Toggle DefC mode */
			myinfo->defc ^= 1;
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 9:
			/* Toggle draw mode */
			if (toggledrawmode(myinfo) != 0) {
				/* Update changes and start plotting if we entered draw mode */
				plot(myworld, myinfo, mydisplay, bigboard, paramlist);
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			}
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'g':
		case 'G':
			/* Toggle get mode - cursor movement loads pattern buffer automatically */
			myinfo->getmode ^= 1;

			/* drawmode & gradmode can't be on while in getmode */
			myinfo->drawmode = 0;
			myinfo->gradmode = 0;

			if (myinfo->getmode != 0) {
				/* Grab if getmode is on */
				if (paramlist[myinfo->cursorx][myinfo->cursory] != 0)
					push(myinfo->backbuffer, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]);
				else
					push(myinfo->backbuffer, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], NULL);
			}
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 15:
			/* Shift-tab */
			if (e == 0)
				break;

			if (togglegradientmode(myinfo) != 0) {
				/* Plot only when first turning gradmode on */
				plot(myworld, myinfo, mydisplay, bigboard, paramlist);
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			}

			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'v':
		case 'V':
			/* Toggle blink mode */
			myinfo->blinkmode ^= 1;
			pat_applycolordata(myinfo->standard_patterns, myinfo);
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case ' ':
			/* Plot */
			plot(myworld, myinfo, mydisplay, bigboard, paramlist);
			drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			break;
		case 'c':
			/* Change foregeound colour */
			myinfo->forec++;
			if (myinfo->forec == 16)
				myinfo->forec = 0;
			pat_applycolordata(myinfo->standard_patterns, myinfo);
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'C':
			/* Change background colour */
			myinfo->backc++;
			if (myinfo->backc == 8)
				myinfo->backc = 0;
			pat_applycolordata(myinfo->standard_patterns, myinfo);
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'b':
		case 'B':
			changeboard(mydisplay, myworld, myinfo, bigboard, paramlist);

			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			mydisplay->print(30 - strlen(myworld->board[myinfo->curboard]->title) / 2, 0, 0x70, myworld->board[myinfo->curboard]->title);
			break;
		case '\b':
			/* backspace: same action as delete */
			c = 's';
			e = 1;
			/* NO BREAK here; continue into following code */
		case 's':
		case 'S':
			if (e != 1) {
				saveworldprompt(mydisplay, myworld, myinfo, bigboard);
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			} else {
				/* Plot an empty */
				patbuffer* prevbuf = myinfo->pbuf;
				myinfo->pbuf = myinfo->standard_patterns;
				x = myinfo->pbuf->pos;
				myinfo->pbuf->pos = 4;	/* That's an empty */
				plot(myworld, myinfo, mydisplay, bigboard, paramlist);
				myinfo->pbuf->pos = x;
				myinfo->pbuf = prevbuf;
				updatepanel(mydisplay, myinfo, myworld);
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			}
			break;
		case 'f':
		case 'F':
			/* Flood fill */
			{
				patdef curpattern = myinfo->pbuf->patterns[myinfo->pbuf->pos];
				if (myinfo->defc == 0) {
					i = (myinfo->backc << 4) + myinfo->forec;
					if (myinfo->blinkmode == 1)
						i += 0x80;
					x = i;
				} else
					x = curpattern.color;

				if (bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] == curpattern.type && (curpattern.type == Z_EMPTY || bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] == x))
					break;
				{
					int randflag = 0;
					if (c == 'F') {
						/* Set randflag to a negative number for bitman's patented
						 * cycle fill (it almost generates even patterns). For the
						 * moment, no keys are mapped to do this. */
						randflag = 1;
						for (i = 0; i < myinfo->pbuf->size; i++)
							/* Break randomization if target type is in pattern buffer */
							if (bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] == myinfo->pbuf->patterns[i].type && (myinfo->pbuf->patterns[i].type == Z_EMPTY || bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1] == myinfo->pbuf->patterns[i].color)) {
								randflag = 0;
								break;
							}
						srand(time(0));
					}
					floodfill(myworld, myinfo, mydisplay, bigboard, paramlist, myinfo->cursorx, myinfo->cursory, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], randflag);
				}
				updatepanel(mydisplay, myinfo, myworld);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			}
			break;
		case 'L':
		case 'l':
			/* Load world */
			filedialog(buffer, "zzt", "Load World", mydisplay);
			if (strlen(buffer) != 0) {
				z_delete(myworld);
				myworld = loadworld(buffer);
				strncpy(myinfo->currentfile, buffer, 13);
				strncpy(myinfo->currenttitle, myworld->zhead->title, 20);
				myinfo->currenttitle[myworld->zhead->titlelength] = '\0';
				myinfo->curboard = myworld->zhead->startboard;
				rle_decode(myworld->board[myworld->zhead->startboard]->data, bigboard);
				for (i = 0; i < 25; i++)
					for (x = 0; x < 60; x++)
						paramlist[x][i] = 0;
				for (i = 0; i < myworld->board[myinfo->curboard]->info->objectcount + 1; i++) {
					if (myworld->board[myinfo->curboard]->params[i]->x > 0 && myworld->board[myinfo->curboard]->params[i]->x < 61 && myworld->board[myinfo->curboard]->params[i]->y > 0 && myworld->board[myinfo->curboard]->params[i]->y < 26)
						paramlist[myworld->board[myinfo->curboard]->params[i]->x - 1][myworld->board[myinfo->curboard]->params[i]->y - 1] = i;
					myinfo->playerx = myworld->board[myinfo->curboard]->params[0]->x - 1;
					myinfo->playery = myworld->board[myinfo->curboard]->params[0]->y - 1;
				}
			}
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			mydisplay->print(30 - strlen(myworld->board[myinfo->curboard]->title) / 2, 0, 0x70, myworld->board[myinfo->curboard]->title);
			break;
		/* Code common to all movement keys can be found after this switch statement */
		case 75:
			/* Left arrow */
			if (e == 1) {
				if (myinfo->cursorx > 0) {
					myinfo->cursorx--;
				}
			}
			break;
		case 155:
			/* Alt+Left */
			if (e == 1) {
				myinfo->cursorx -= 10;
				if (myinfo->cursorx < 0)
					myinfo->cursorx = 0;
			}
			break;
		case 77:
			/* Right arrow */
			if (e == 1) {
				if (myinfo->cursorx < 59) {
					myinfo->cursorx++;
				}
			}
			break;
		case 157:
			/* Alt+Right */
			if (e == 1) {
				myinfo->cursorx += 10;
				if (myinfo->cursorx > 59)
					myinfo->cursorx = 59;
			}
			break;
		case 72:
		case 'h':
			/* Up arrow or H */
			if (e == 1) {
				/* Move cursor up */
				if (myinfo->cursory > 0) {
					myinfo->cursory--;
				}
			} else {
				/* Help */
				help(mydisplay);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			}
			break;
		case 152:
			/* Alt+Up */
			if (e == 1) {
				myinfo->cursory -= 5;
				if (myinfo->cursory < 0)
					myinfo->cursory = 0;
			}
			break;
		case 80:
			/* Down arrow or P */
			if (e == 1) {
				if (myinfo->cursory < 24) {
					myinfo->cursory++;
				}
			} else {
				/* Select new pattern backwards */
				myinfo->pbuf->pos--;
				if (myinfo->pbuf->pos == -1) {
					if (myinfo->pbuf == myinfo->standard_patterns)
						myinfo->pbuf = myinfo->backbuffer;
					else
						myinfo->pbuf = myinfo->standard_patterns;
					myinfo->pbuf->pos = myinfo->pbuf->size - 1;
				}
				updatepanel(mydisplay, myinfo, myworld);
			}
			break;
		case 160:
			/* Alt+Down */
			if (e == 1) {
				myinfo->cursory += 5;
				if (myinfo->cursory > 24)
					myinfo->cursory = 24;
			}
			break;
		case 'p':
			/* Select new pattern forwards */
			myinfo->pbuf->pos++;
			if (myinfo->pbuf->pos == myinfo->pbuf->size) {
				if (myinfo->pbuf == myinfo->standard_patterns)
					myinfo->pbuf = myinfo->backbuffer;
				else
					myinfo->pbuf = myinfo->standard_patterns;
				myinfo->pbuf->pos = 0;
			}
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 59:
			/* F1 panel */
			if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
				break;
			if (e == 1) {
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
						pm = z_newparam_passage(myinfo->cursorx + 1, myinfo->cursory + 1, boarddialog(myworld, myinfo, mydisplay));
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
						patbuffer* prevbuf = myinfo->pbuf;
						myinfo->pbuf = myinfo->backbuffer;
						x = myinfo->pbuf->pos;
						myinfo->pbuf->pos = 0;
						plot(myworld, myinfo, mydisplay, bigboard, paramlist);
						myinfo->pbuf->pos = x;
						myinfo->pbuf = prevbuf;
					}
				}
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			}
			break;
		case 60: /* '<' */
			/* F2 panel */
			if (e == 1) {
				if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
					break;

				i = dothepanel_f2(mydisplay, myinfo);
				/* All these need parameter space */
				if (myworld->board[myinfo->curboard]->info->objectcount == 150) {
					i = -1;
					break;
				}
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
					patbuffer* prevbuf = myinfo->pbuf;
					myinfo->pbuf = myinfo->backbuffer;
					x = myinfo->pbuf->pos;
					myinfo->pbuf->pos = 0;
					plot(myworld, myinfo, mydisplay, bigboard, paramlist);
					myinfo->pbuf->pos = x;
					myinfo->pbuf = prevbuf;
				}
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			} else {
				/* Decrease size of backbuffer */
				if (myinfo->backbuffer->size > 1) {
					patbuffer_resize(myinfo->backbuffer, -1);
					updatepanel(mydisplay, myinfo, myworld);
				}
			}
			break;
		case 61:
			/* F3 panel */
			if (myinfo->cursorx == myinfo->playerx && myinfo->cursory == myinfo->playery)
				break;
			if (e == 1) {
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
					patbuffer* prevbuf = myinfo->pbuf;
					myinfo->pbuf = myinfo->backbuffer;
					x = myinfo->pbuf->pos;
					myinfo->pbuf->pos = 0;
					plot(myworld, myinfo, mydisplay, bigboard, paramlist);
					myinfo->pbuf->pos = x;
					myinfo->pbuf = prevbuf;
				}
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			}
			break;
		case 62: /* '>' */
			if (e == 1) {
				/* F4 - Enter Text */
				myinfo->textentrymode ^= 1;
				updatepanel(mydisplay, myinfo, myworld);
			} else {
				/* Increase size of backbuffer */
				if (myinfo->backbuffer->size < 128) {
					patbuffer_resize(myinfo->backbuffer, 1);
					updatepanel(mydisplay, myinfo, myworld);
				}
			}
			break;
		case 'i':
		case 'I':
			/* Board Info */
			break;
		case 13:
			/* Modify / Grab */
			if (e == 0) {
				if (paramlist[myinfo->cursorx][myinfo->cursory] != 0) {
					if (myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]] != NULL) {
						/* we have params; lets edit them! */
						if(bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] == Z_OBJECT) {
							myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]->data1 = charselect(mydisplay, myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]->data1);
						}
						if (bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] == Z_SCROLL || bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] == Z_OBJECT) {
							/* Load editor on current moredata */
							editmoredata(myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]], mydisplay);
						}
						if(bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2] == Z_PASSAGE) {
							/* Choose passage destination */
							myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]->data3 = boarddialog(myworld, myinfo, mydisplay);
						}
						/* TODO: modify other params */
						/* redraw everything */
						drawpanel(mydisplay);
						updatepanel(mydisplay, myinfo, myworld);
						mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
						drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
					}
				}
				e = 1;	/* set ext so that <insert> code below is used */
			}
			/* Don't break here! When we modify, we grab too! */
		case 0x52:
		case 'r':
			/* Insert or 'r' */
			if (e == 1) {
				/* Grab */
				if (paramlist[myinfo->cursorx][myinfo->cursory] != 0)
					push(myinfo->backbuffer, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]);
				else
					push(myinfo->backbuffer, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], NULL);
				updatepanel(mydisplay, myinfo, myworld);
				break;
			} else {
				/* 'r'un zzt */
				/* Load current world into zzt */
				if (e == 0) {
					mydisplay->end();
					runzzt(myinfo->currentfile);

					/* restart display from scratch */
					mydisplay->init();
					drawpanel(mydisplay);
					updatepanel(mydisplay, myinfo, myworld);
					drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
					mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				}
			}
			break;

		case '?':
			/* display param data (for developement purposes and debugging) */
			if (paramlist[myinfo->cursorx][myinfo->cursory] != 0) {
				if (myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]] != NULL) {
					/* we have params; lets show them! */
					showParamData(myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]], paramlist[myinfo->cursorx][myinfo->cursory], mydisplay);
					drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
					mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				}
			}
			break;
		}

		if ((e == 1) && (c == 75  || c == 77 || c == 72 || c == 80 ||
										 c == 155 || c == 157 || c == 152 || c == 160)) {
			/* Common code for all movement actions */
			if (myinfo->getmode != 0) {
				/* Get if getmode is on */
				if (paramlist[myinfo->cursorx][myinfo->cursory] != 0)
					push(myinfo->backbuffer, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]);
				else
					push(myinfo->backbuffer, bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2], bigboard[(myinfo->cursorx + myinfo->cursory * 60) * 2 + 1], NULL);
				updatepanel(mydisplay, myinfo, myworld);
			}
			/* If gradmode is on, cycle through the pattern buffer.
			 * Negative values move backward, positive values forward. */
			if (myinfo->gradmode < 0) {
				if (--myinfo->pbuf->pos < 0)
					myinfo->pbuf->pos = myinfo->pbuf->size - 1;
			} else if (myinfo->gradmode > 0) {
				if (++myinfo->pbuf->pos >= myinfo->pbuf->size)
					myinfo->pbuf->pos = 0;
			}
			/* If drawmode is on, plot */
			if (myinfo->drawmode == 1) {
				plot(myworld, myinfo, mydisplay, bigboard, paramlist);
			}
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			updatepanel(mydisplay, myinfo, myworld);
			drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
		}
	}

	mydisplay->end();

	/* Free the registers used by copy & paste in the ZOC editor */
	deleteregisters();

	/* Free pattern buffers */
	free(myinfo->standard_patterns);
	free(myinfo->backbuffer);

	/* Free myinfo stuff */
	free(myinfo->currentfile);
	free(myinfo->currenttitle);

	/* Free everything! Free! Free! Free! Let freedom ring! */
	free(myinfo);
	free(string);
	free(bigboard);
	z_delete(myworld);

	return 0;
}
