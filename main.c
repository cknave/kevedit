/* main.c       -- The buck starts here
 * $Id: main.c,v 1.42 2001/11/06 05:44:58 bitman Exp $
 * Copyright (C) 2000-2001 Kev Vance <kev@kvance.com>
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
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "display.h"
#include "kevedit.h"
#include "screen.h"
#include "scroll.h"
#include "zzt.h"
#include "editbox.h"
#include "register.h"
#include "patbuffer.h"
#include "misc.h"
#include "menu.h"
#include "help.h"
#include "infobox.h"

#define MAIN_BUFLEN 255

/* Interrupt signal for CTRL-C (do nothing) */
void sigInt(int i)
{
}

int main(int argc, char **argv)
{
	int i, x;
	int c, e;
	int quit = 0;
	displaymethod *mydisplay;
	editorinfo *myinfo;
	char *bigboard;
	char *datapath;       /* Location of help file, zzt.exe, and zzt.dat */
	char buffer[MAIN_BUFLEN];
	unsigned char paramlist[60][25];

	world *myworld;

	RegisterDisplays();
	mydisplay = pickdisplay(&display);

	printf("Initializing %s version %s...\n", mydisplay->name, mydisplay->version);
	if (!mydisplay->init()) {
		printf("\nDisplay initialization failed.  Exiting.\n");
		return 1;
	}

	/* Allocate space for various data */

	myinfo = (editorinfo *) malloc(sizeof(editorinfo));
	bigboard = (char *) malloc(BOARD_MAX * 2);

	/* Set up initial info */
	initeditorinfo(myinfo);

	/* Assume DOS model of keeping program data the same dir as kevedit.exe */
	datapath = locateself(argv[0]);  
	inithelpsystem(datapath);

	/* Did we get a world on the command line? */
	myworld = NULL;
	if (argc > 1) {
		/* Switch to the directory given within the filename */
		pathof(buffer, argv[1], MAIN_BUFLEN);
		chdir(buffer);

		/* Open the file */
		fileof(buffer, argv[1], MAIN_BUFLEN - 5);
		myworld = loadworld(buffer);
		if (myworld == NULL) {
			/* Maybe they left off the .zzt extension? */
			strcat(buffer, ".zzt");
			myworld = loadworld(buffer);
		}
		if (myworld != NULL) {
			strncpy(myinfo->currentfile, buffer, 13);

			updateinfo(myworld, myinfo, bigboard);
			updateparamlist(myworld, myinfo, paramlist);
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

	/* Trap ctrl+c */
	signal(SIGINT, sigInt);

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

					tiletype (bigboard, myinfo->cursorx, myinfo->cursory) = Z_BLUETEXT + i;
					tilecolor(bigboard, myinfo->cursorx, myinfo->cursory) = c;
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
				if (confirmprompt(mydisplay, "Clear board?") == CONFIRM_YES) {
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
				if (confirmprompt(mydisplay, "Make new world?") == CONFIRM_YES) {
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
			if (confirmprompt(mydisplay, "Quit?") == CONFIRM_YES) {
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
		case 'a':
		case 'A':
			/* Toggle aqu mode - cursor movement loads pattern buffer automatically */
			myinfo->aqumode ^= 1;

			/* drawmode & gradmode can't be on while in aqumode */
			myinfo->drawmode = 0;
			myinfo->gradmode = 0;

			if (myinfo->aqumode != 0) {
				/* Grab if aqumode is on */
				if (paramlist[myinfo->cursorx][myinfo->cursory] != 0)
					push(myinfo->backbuffer,
							 tiletype (bigboard, myinfo->cursorx, myinfo->cursory),
							 tilecolor(bigboard, myinfo->cursorx, myinfo->cursory),
							 myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]);
				else
					push(myinfo->backbuffer,
							 tiletype (bigboard, myinfo->cursorx, myinfo->cursory),
							 tilecolor(bigboard, myinfo->cursorx, myinfo->cursory),
							 NULL);
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
		case 'F':
		case 'f':
			dofloodfill(mydisplay, myworld, myinfo, bigboard, paramlist, c == 'F');
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			break;
		case 'L':
		case 'l':
			/* Load world */
			if (filedialog(buffer, "zzt", "Load World", mydisplay)) {
				if (strlen(buffer) != 0) {
					world* newworld = loadworld(buffer);
					if (newworld != NULL) {
						char newpath[MAIN_BUFLEN];

						/* Out with the old and in with the new */
						z_delete(myworld);
						myworld = newworld;

						/* Change directory */
						pathof(newpath, buffer, MAIN_BUFLEN);
						chdir(newpath);

						/* Copy the file portion of the filename */
						fileof(myinfo->currentfile, buffer, 14);
						updateinfo(myworld, myinfo, bigboard);
						updateparamlist(myworld, myinfo, paramlist);
					}
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
		case 'k':
			/* Left arrow */
			if (e == 1) {
				if (myinfo->cursorx > 0) {
					myinfo->cursorx--;
				}
			} else {
				/* Kolor selector */
				colorselector(mydisplay, &myinfo->backc, &myinfo->forec,
											&myinfo->blinkmode);

				pat_applycolordata(myinfo->standard_patterns, myinfo);
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				updatepanel(mydisplay, myinfo, myworld);
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

				/* Update everything */
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
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
				previouspattern(myinfo);
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
			nextpattern(myinfo);
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 59:
			/* F1 panel */
			if (e == 1) {
				itemmenu(mydisplay, myworld, myinfo, bigboard, paramlist);
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			}
			break;
		case 60: /* '<' */
			/* F2 panel */
			if (e == 1) {
				creaturemenu(mydisplay, myworld, myinfo, bigboard, paramlist);

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
			if (e == 1) {
				terrainmenu(mydisplay, myworld, myinfo, bigboard, paramlist);

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
				if (myinfo->backbuffer->size < MAX_BACKBUF) {
					patbuffer_resize(myinfo->backbuffer, 1);
					updatepanel(mydisplay, myinfo, myworld);
				}
			}
			break;
		case 'o':
		case 'O':
			/* Load object from library */
			if (e == 0) {
				objectlibrarymenu(mydisplay, myworld, myinfo, bigboard, paramlist);
				
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			}
			break;
		case 'i':
		case 'I':
			/* Board Info */
			editboardinfo(myworld, myinfo->curboard, mydisplay);

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case 'w':
		case 'W':
			/* World Info */
			editworldinfo(myworld, mydisplay);

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case 13:
			/* Modify / Grab */
			if (e == 0) {
				if (paramlist[myinfo->cursorx][myinfo->cursory] != 0) {
					if (myworld->board[myinfo->curboard]->
							  params[paramlist[myinfo->cursorx][myinfo->cursory]] != NULL) {
						/* we have params; lets edit them! */
						if(tiletype(bigboard, myinfo->cursorx, myinfo->cursory) == Z_OBJECT) {
							myworld->board[myinfo->curboard]->
								params[paramlist[myinfo->cursorx][myinfo->cursory]]->data1
								= charselect(mydisplay, myworld->board[myinfo->curboard]->
														 params[paramlist[myinfo->cursorx][myinfo->cursory]]->data1);
						}
						if (tiletype(bigboard, myinfo->cursorx, myinfo->cursory) == Z_SCROLL ||
								tiletype(bigboard, myinfo->cursorx, myinfo->cursory) == Z_OBJECT) {
							/* Load editor on current moredata */
							editmoredata(myworld->board[myinfo->curboard]->
													 params[paramlist[myinfo->cursorx][myinfo->cursory]], mydisplay);
						}
						if(tiletype(bigboard, myinfo->cursorx, myinfo->cursory) == Z_PASSAGE) {
							param* p = myworld->board[myinfo->curboard]->
								params[paramlist[myinfo->cursorx][myinfo->cursory]];
							/* Choose passage destination */
							p->data3 = boarddialog(myworld, p->data3, 0, "Passage Destination", mydisplay);
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
					push(myinfo->backbuffer,
							 tiletype (bigboard, myinfo->cursorx, myinfo->cursory),
							 tilecolor(bigboard, myinfo->cursorx, myinfo->cursory),
							 myworld->board[myinfo->curboard]->
							   params[paramlist[myinfo->cursorx][myinfo->cursory]]);
				else
					push(myinfo->backbuffer,
							 tiletype (bigboard, myinfo->cursorx, myinfo->cursory),
							 tilecolor(bigboard, myinfo->cursorx, myinfo->cursory),
							 NULL);

				updatepanel(mydisplay, myinfo, myworld);
				break;
			} else {
				/* 'r'un zzt */
				/* Load current world into zzt */
				if (e == 0) {
					mydisplay->end();
					runzzt(datapath, myinfo->currentfile);
					
					/* restart display from scratch */
					mydisplay->init();
					drawpanel(mydisplay);
					drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);

					/* Ask to reload world (I don't like it any more) */
#if 0
					if (confirmprompt(mydisplay, "Reload World?") == CONFIRM_YES) {
						/* Load the world from file, in case ZZT modified it */
						if (fileexists(myinfo->currentfile)) {
							z_delete(myworld);
							myworld = loadworld(myinfo->currentfile);

							updateinfo(myworld, myinfo, bigboard);
							updateparamlist(myworld, myinfo, paramlist);

							drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
						}
					}
#endif

					/* Redraw */
					drawpanel(mydisplay);
					updatepanel(mydisplay, myinfo, myworld);
					mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				}
			}
			break;

		case '?':
			/* display param data (for developement purposes and debugging) */
			if (paramlist[myinfo->cursorx][myinfo->cursory] != 0) {
				if (myworld->board[myinfo->curboard]->
						  params[paramlist[myinfo->cursorx][myinfo->cursory]] != NULL) {
					/* we have params; lets show them! */
					showParamData(myworld->board[myinfo->curboard]->
												  params[paramlist[myinfo->cursorx][myinfo->cursory]],
												paramlist[myinfo->cursorx][myinfo->cursory], mydisplay);

					drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
					mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				}
			}
			break;
		}

		if ((e == 1) && (c == 75  || c == 77 || c == 72 || c == 80 ||
										 c == 155 || c == 157 || c == 152 || c == 160)) {
			/* Common code for all movement actions */
			if (myinfo->aqumode != 0) {
				/* Get if aquire mode is on */
				if (paramlist[myinfo->cursorx][myinfo->cursory] != 0)
					push(myinfo->backbuffer,
							 tiletype (bigboard, myinfo->cursorx, myinfo->cursory),
							 tilecolor(bigboard, myinfo->cursorx, myinfo->cursory),
							 myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]);
				else
					push(myinfo->backbuffer,
							 tiletype (bigboard, myinfo->cursorx, myinfo->cursory),
							 tilecolor(bigboard, myinfo->cursorx, myinfo->cursory),
							 NULL);
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

	/* Free the loaded help system */
	deletehelpsystem();
	if (datapath != NULL)
		free(datapath);

	/* Free pattern buffers */
	free(myinfo->standard_patterns);
	free(myinfo->backbuffer);

	/* Free myinfo stuff */
	free(myinfo->currentfile);
	free(myinfo->currenttitle);

	/* Free everything! Free! Free! Free! Let freedom ring! */
	free(myinfo);
	free(bigboard);
	z_delete(myworld);

	return 0;
}
