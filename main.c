/* main.c       -- The buck starts here
 * $Id: main.c,v 1.45 2001/11/10 04:48:12 bitman Exp $
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
	int i, x;             /* General counters */
#if 0
	int c, e;
#endif
	int key;              /* Keypress */
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
		/* Draw the tile under the cursor as such */
		cursorspace(mydisplay, myworld, myinfo, bigboard, paramlist);

		/* Get the key */
		key = mydisplay->getch();
		if (key == 0)
			key = mydisplay->getch() | DDOSKEY_EXT;

		/* Undo the cursorspace (draw as a normal tile) */
		i = (myinfo->cursorx + myinfo->cursory * 60) * 2;
		mydisplay->putch(myinfo->cursorx, myinfo->cursory,
				 z_getchar(bigboard[i], bigboard[i + 1], myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]], bigboard, myinfo->cursorx, myinfo->cursory),
				 z_getcolour(bigboard[i], bigboard[i + 1], myworld->board[myinfo->curboard]->params[paramlist[myinfo->cursorx][myinfo->cursory]]));

		/* Check for text entry */
		if (myinfo->textentrymode == 1) {
			if (key == DKEY_ENTER || key == DKEY_ESC) {
				/* Leave text entry mode */
				key = DKEY_F4;
			} else if (key == DKEY_BACKSPACE && myinfo->cursorx > 0) {
				/* Backspace */
				myinfo->cursorx--;
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				if (paramlist[myinfo->cursorx][myinfo->cursory] != 0) {
					/* We're overwriting a parameter */
					param_remove(myworld->board[myinfo->curboard], paramlist, myinfo->cursorx, myinfo->cursory);
				}
				bigboard[(myinfo->cursorx+myinfo->cursory*60)*2] = Z_EMPTY;
				bigboard[(myinfo->cursorx+myinfo->cursory*60)*2+1] = 0x07;
				updatepanel(mydisplay, myinfo, myworld);
			} else if ((key < 0x80 && key >= 0x20) || key == DKEY_CTRL_A) {
				/* Insert the current keystroke as text */

				if (key == DKEY_CTRL_A) { /* ASCII selection */
					key = charselect(mydisplay, -1);
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
					tilecolor(bigboard, myinfo->cursorx, myinfo->cursory) = key;
					/* Move right now */
					key = DKEY_RIGHT;
				} else
					continue;
			}
		}

		/* bitman's addition: vi keys, only when envar "KVI" is set will
		 * these apply. */
		if (getenv("KVI") != NULL) {
			switch (key) {
			/* Look for vi keys */
			case 'h': key = DKEY_LEFT;  break;
			case 'j': key = DKEY_DOWN;  break;
			case 'k': key = DKEY_UP;    break;
			case 'l': key = DKEY_RIGHT; break;
			}
		}

		/* Act on key pressed */
		switch (key) {

			/**************** Movement Keys *****************/

		case DKEY_LEFT:
			/* Left arrow */
			if (myinfo->cursorx > 0) {
				myinfo->cursorx--;
			}
			break;
		case DKEY_ALT_LEFT:
			/* Alt+Left */
			myinfo->cursorx -= 10;
			if (myinfo->cursorx < 0)
				myinfo->cursorx = 0;
			break;
		case DKEY_RIGHT:
			/* Right arrow */
			if (myinfo->cursorx < 59) {
				myinfo->cursorx++;
			}
			break;
		case DKEY_ALT_RIGHT:
			/* Alt+Right */
			myinfo->cursorx += 10;
			if (myinfo->cursorx > 59)
				myinfo->cursorx = 59;
			break;
		case DKEY_UP:
			/* Up arrow */
			if (myinfo->cursory > 0) {
				myinfo->cursory--;
			}
			break;
		case DKEY_ALT_UP:
			/* Alt+Up */
			myinfo->cursory -= 5;
			if (myinfo->cursory < 0)
				myinfo->cursory = 0;
			break;
		case DKEY_DOWN:
			/* Down arrow or P */
			if (myinfo->cursory < 24) {
				myinfo->cursory++;
			}
			break;
		case DKEY_ALT_DOWN:
			/* Alt+Down */
			myinfo->cursory += 5;
			if (myinfo->cursory > 24)
				myinfo->cursory = 24;
			break;

			/****************** Major actions ****************/
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
		case 'n':
		case 'N':
			if (confirmprompt(mydisplay, "Make new world?") == CONFIRM_YES) {
				myworld = clearworld(myworld, myinfo, bigboard, paramlist);

				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			}

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'z':
		case 'Z':
			if (confirmprompt(mydisplay, "Clear board?") == CONFIRM_YES) {
				clearboard(myworld, myinfo, bigboard, paramlist);

				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			}

			drawpanel(mydisplay);
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

		/********************* File operations ****************/

		case 's':
		case 'S':
				saveworldprompt(mydisplay, myworld, myinfo, bigboard);
				drawpanel(mydisplay);
				updatepanel(mydisplay, myinfo, myworld);
				drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				break;
		case 'L':
		case 'l':
			/* Load world */
			{
				char* filename =
					filedialog(".", "zzt", "Load World", FTYPE_ALL, mydisplay);
				if (filename) {
					world* newworld = loadworld(filename);
					if (newworld != NULL) {
						char* newpath = (char*) malloc(sizeof(char)*(strlen(filename)+1));

						/* Out with the old and in with the new */
						z_delete(myworld);
						myworld = newworld;

						/* Change directory */
						pathof(newpath, filename, strlen(filename) + 1);
						chdir(newpath);
						free(newpath);

						/* Copy the file portion of the filename */
						fileof(myinfo->currentfile, filename, 14);
						updateinfo(myworld, myinfo, bigboard);
						updateparamlist(myworld, myinfo, paramlist);
					}
					free(filename);
				}
			}
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			mydisplay->print(30 - strlen(myworld->board[myinfo->curboard]->title) / 2, 0, 0x70, myworld->board[myinfo->curboard]->title);
			break;
		case 'o':
		case 'O':
			/* Load object from library */
			objectlibrarymenu(mydisplay, myworld, myinfo, bigboard, paramlist);
			
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;

		/****************** Colour ******************/

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
		case 'k':
		case 'K':
			/* Kolor selector */
			colorselector(mydisplay, &myinfo->backc, &myinfo->forec,
										&myinfo->blinkmode);

			pat_applycolordata(myinfo->standard_patterns, myinfo);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'v':
		case 'V':
			/* Toggle blink mode */
			myinfo->blinkmode ^= 1;
			pat_applycolordata(myinfo->standard_patterns, myinfo);
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'd':
		case 'D':
			/* Toggle DefC mode */
			myinfo->defc ^= 1;
			updatepanel(mydisplay, myinfo, myworld);
			break;

		/****************** Misc *********************/

		case 'h':
		case 'H':
			/* Help */
			help(mydisplay);

			/* Update everything */
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case 'r':
		case 'R':
			/* run zzt */
			/* Load current world into zzt */
			mydisplay->end();
			runzzt(datapath, myinfo->currentfile);
			
			/* restart display from scratch */
			mydisplay->init();
			drawpanel(mydisplay);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);

			/* Redraw */
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case '!':
			/* Open text editor */
			texteditor(mydisplay);

			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			break;

		/***************** Drawing *****************/

		case ' ':
			/* Plot */
			plot(myworld, myinfo, mydisplay, bigboard, paramlist);
			drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			break;
		case DKEY_TAB:
			/* Toggle draw mode */
			if (toggledrawmode(myinfo) != 0) {
				/* Update changes and start plotting if we entered draw mode */
				plot(myworld, myinfo, mydisplay, bigboard, paramlist);
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			}
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case DKEY_SHIFT_TAB:
			/* Shift-tab */
			if (togglegradientmode(myinfo) != 0) {
				/* Plot only when first turning gradmode on */
				plot(myworld, myinfo, mydisplay, bigboard, paramlist);
				drawspot(mydisplay, myworld, myinfo, bigboard, paramlist);
			}

			updatepanel(mydisplay, myinfo, myworld);
			break;
		case DKEY_BACKSPACE:
		case DKEY_DELETE:
			/* Plot an empty */
			{
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
			dofloodfill(mydisplay, myworld, myinfo, bigboard, paramlist, key == 'F');
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			break;

		/***************** Backbuffer Actions ****************/

		case 'p':
			/* Select new pattern forwards */
			nextpattern(myinfo);
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'P':
			/* Select new pattern backwards */
			previouspattern(myinfo);
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case '<':
			/* Decrease size of backbuffer */
			if (myinfo->backbuffer->size > 1) {
				patbuffer_resize(myinfo->backbuffer, -1);
				updatepanel(mydisplay, myinfo, myworld);
			}
			break;
		case '>':
			/* Increase size of backbuffer */
			if (myinfo->backbuffer->size < MAX_BACKBUF) {
				patbuffer_resize(myinfo->backbuffer, 1);
				updatepanel(mydisplay, myinfo, myworld);
			}
			break;
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

		/***************** Function Keys *******************/

		case DKEY_F1:
			/* F1 panel */
			itemmenu(mydisplay, myworld, myinfo, bigboard, paramlist);
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case DKEY_F2:
			/* F2 panel */
			creaturemenu(mydisplay, myworld, myinfo, bigboard, paramlist);

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case DKEY_F3:
			/* F3 panel */
			terrainmenu(mydisplay, myworld, myinfo, bigboard, paramlist);

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld, myinfo, bigboard, paramlist);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case DKEY_F4:
			/* F4 - Enter Text */
			myinfo->textentrymode ^= 1;
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case DKEY_ENTER:
			/* Modify / Grab */
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
			/* Don't break here! When we modify, we grab too! */
		case DKEY_INSERT:
			/* Insert */
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

		/* Common code for all movement actions */
		if (key == DKEY_LEFT     || key == DKEY_RIGHT     ||
		    key == DKEY_UP       || key == DKEY_DOWN      ||
		    key == DKEY_ALT_LEFT || key == DKEY_ALT_RIGHT ||
		    key == DKEY_ALT_UP   || key == DKEY_ALT_DOWN) {
			/* TODO: consider ALT-movents seperately */
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
