/* main.c       -- The buck starts here
 * $Id: main.c,v 1.66 2002/03/19 19:12:50 kvance Exp $
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

#include "kevedit.h"
#include "misc.h"
#include "menu.h"
#include "editbox.h"
#include "screen.h"

#include "libzzt2/zzt.h"

#include "patbuffer.h"
#include "help.h"
#include "infobox.h"
#include "paramed.h"
#include "register.h"

#include "display.h"

#define MAIN_BUFLEN 255

/* Interrupt signal for CTRL-C (do nothing) */
void sigInt(int i)
{
}

int main(int argc, char **argv)
{
	int key;              /* Keypress */
	int quit = 0;
	displaymethod *mydisplay;
	editorinfo *myinfo;
	char *datapath;       /* Location of help file, zzt.exe, and zzt.dat */
	char buffer[MAIN_BUFLEN];

	ZZTworld *myworld;

	RegisterDisplays();
	mydisplay = pickdisplay(&display);

	printf("Initializing %s version %s...\n", mydisplay->name, mydisplay->version);
	if (!mydisplay->init()) {
		printf("\nDisplay initialization failed.  Exiting.\n");
		return 1;
	}

	/* Allocate space for various data */

	myinfo = (editorinfo *) malloc(sizeof(editorinfo));

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
		myworld = zztWorldLoad(buffer);
		if (myworld == NULL) {
			/* Maybe they left off the .zzt extension? */
			strcat(buffer, ".zzt");
			myworld = zztWorldLoad(buffer);
		}
	}
	/* Create the blank world */
	if (myworld == NULL) {
		myworld = zztWorldCreate(NULL, NULL);
	}

	/* Switch to the start board */
	zztBoardSelect(myworld, zztWorldGetStartboard(myworld));

	/* Trap ctrl+c */
	signal(SIGINT, sigInt);

	/* Draw */
	drawpanel(mydisplay);
	updatepanel(mydisplay, myinfo, myworld);
	drawscreen(mydisplay, myworld);
	mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
	mydisplay->print(30 - strlen(zztBoardGetTitle(myworld)) / 2, 0, 0x70, zztBoardGetTitle(myworld));

	/* Main loop begins */

	while (quit == 0) {
		/* Draw the tile under the cursor as such */
		cursorspace(mydisplay, myworld, myinfo);
		mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory); /* Why not? */

		/* Get the key */
		key = mydisplay->getch();

		/* Undo the cursorspace (draw as a normal tile) */
		drawspot(mydisplay, myworld, myinfo);

		/* Check for text entry */
		if (myinfo->textentrymode == 1) {
			if (key == DKEY_ENTER || key == DKEY_ESC) {
				/* Leave text entry mode */
				key = DKEY_F4;
			} else if (key == DKEY_BACKSPACE && myinfo->cursorx > 0) {
				/* Backspace */
				myinfo->cursorx--;
				zztErase(myworld, myinfo->cursorx, myinfo->cursory);
				updatepanel(mydisplay, myinfo, myworld);
			} else if ((key < 0x80 && key >= 0x20) || key == DKEY_CTRL_A) {
				/* Insert the current keystroke as text */
				ZZTtile textTile = { ZZT_BLUETEXT, 0x00, NULL };

				if (key == DKEY_CTRL_A) { /* ASCII selection */
					key = charselect(mydisplay, -1);
					mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
					drawscreen(mydisplay, myworld);
				}

				/* Determine the text code based on the FG colour */
				if (myinfo->forec == 0 || myinfo->forec == 8 || myinfo->forec == 15)
					textTile.type += 6;
				else if (myinfo->forec > 8)
					textTile.type += myinfo->forec - 9;
				else
					textTile.type += myinfo->forec - 1;

				/* Determine color based on keypress */
				textTile.color = key;

				/* Plot the text character */
				zztPlot(myworld, myinfo->cursorx, myinfo->cursory, textTile);

				/* Now move right */
				key = DKEY_RIGHT;
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

		/* Check for number key */
		if (key >= '0' && key <= '9') {
			/* Change to appropriate backbuffer position */
			if (key == '0') {
				/* Empty tile slot in standard patterns */
				myinfo->pbuf = myinfo->standard_patterns;
				myinfo->pbuf->pos = 4;
			} else {
				/* Change to backbuffer slot */
				if (key - '1' < myinfo->backbuffer->size) {
					/* Only if backbuffer position exists */
					myinfo->pbuf = myinfo->backbuffer;
					myinfo->pbuf->pos = key - '1';
				}
			}
			updatepanel(mydisplay, myinfo, myworld);
			key = DKEY_NONE;
		}

		/* Act on key pressed */
		switch (key) {
		case DKEY_NONE:
			break;

		/**************** Movement Keys *****************/

		case DKEY_LEFT:
			/* Left arrow */
			if (myinfo->cursorx > 0) {
				myinfo->cursorx--;
			}
			break;
		case DKEY_RIGHT:
			/* Right arrow */
			if (myinfo->cursorx < 59) {
				myinfo->cursorx++;
			}
			break;
		case DKEY_UP:
			/* Up arrow */
			if (myinfo->cursory > 0) {
				myinfo->cursory--;
			}
			break;
		case DKEY_DOWN:
			/* Down arrow or P */
			if (myinfo->cursory < 24) {
				myinfo->cursory++;
			}
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
				myworld = clearworld(myworld);

				drawscreen(mydisplay, myworld);
			}

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'z':
		case 'Z':
			if (confirmprompt(mydisplay, "Clear board?") == CONFIRM_YES) {
				clearboard(myworld);
				/* TODO: clear board */

				drawscreen(mydisplay, myworld);
			}

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			break;

		case 'b':
		case 'B':
			switchboard(myworld, mydisplay);

			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld);
			mydisplay->print(30 - strlen(zztBoardGetTitle(myworld)) / 2, 0, 0x70, zztBoardGetTitle(myworld));
			break;
		case DKEY_PAGEDOWN:
			/* Switch to next board (bounds checking is automatic) */
			zztBoardSelect(myworld, zztBoardGetCurrent(myworld) + 1);

			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld);
			mydisplay->print(30 - strlen(zztBoardGetTitle(myworld)) / 2, 0, 0x70, zztBoardGetTitle(myworld));
			break;
		case DKEY_PAGEUP:
			/* Switch to previous board (bounds checking is automatic) */
			zztBoardSelect(myworld, zztBoardGetCurrent(myworld) - 1);

			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld);
			mydisplay->print(30 - strlen(zztBoardGetTitle(myworld)) / 2, 0, 0x70, zztBoardGetTitle(myworld));
			break;

		case 'i':
		case 'I':
			/* Board Info */
			editboardinfo(myworld, mydisplay);

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case 'w':
		case 'W':
			/* World Info */
			editworldinfo(myworld, mydisplay);
			myinfo->changed_title = 1;

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case DKEY_CTRL_T:
			/* Tile Info */
			tileinfo(mydisplay, myworld, myinfo->cursorx, myinfo->cursory);

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;

		/********************* File operations ****************/

		case 's':
		case 'S':
			/* Save world */
			saveworld(mydisplay, myworld);
			myinfo->changed_title = 1;

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case 'l':
		case 'L':
			/* Load world */
			myworld = loadworld(mydisplay, myworld);
			myinfo->changed_title = 1;

			drawscreen(mydisplay, myworld);
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			mydisplay->print(30 - strlen(zztBoardGetTitle(myworld)) / 2, 0, 0x70, zztBoardGetTitle(myworld));
			break;
		case 't':
		case 'T':
			/* Transfer board */
			boardtransfer(mydisplay, myworld);

			drawscreen(mydisplay, myworld);
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			mydisplay->print(30 - strlen(zztBoardGetTitle(myworld)) / 2, 0, 0x70, zztBoardGetTitle(myworld));
			break;
		case 'o':
		case 'O':
			/* Load object from library */
			objectlibrarymenu(mydisplay, myworld, myinfo);
			
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld);
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
		case DKEY_CTRL_K:
			/* Kolor selector */
			colorselector(mydisplay, &myinfo->backc, &myinfo->forec,
										&myinfo->blinkmode);

			pat_applycolordata(myinfo->standard_patterns, myinfo);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			drawscreen(mydisplay, myworld);
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
			drawscreen(mydisplay, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case 'r':
		case 'R':
			/* run zzt */
			/* Load current world into zzt */
			mydisplay->end();
			runzzt(datapath, zztWorldGetFilename(myworld));
			
			/* restart display from scratch */
			mydisplay->init();
			drawpanel(mydisplay);
			drawscreen(mydisplay, myworld);

			/* Redraw */
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case '!':
			/* Open text editor */
			texteditor(mydisplay);

			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			drawscreen(mydisplay, myworld);
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			break;

		/***************** Drawing *****************/

		case ' ':
			/* Plot */
			plot(myworld, myinfo, mydisplay);
			drawspot(mydisplay, myworld, myinfo);
			break;
		case DKEY_TAB:
			/* Toggle draw mode */
			if (toggledrawmode(myinfo) != 0) {
				/* Update changes and start plotting if we entered draw mode */
				plot(myworld, myinfo, mydisplay);
				drawspot(mydisplay, myworld, myinfo);
			}
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case DKEY_SHIFT_TAB:
			/* Shift-tab */
			if (togglegradientmode(myinfo) != 0) {
				/* Plot only when first turning gradmode on */
				plot(myworld, myinfo, mydisplay);
				drawspot(mydisplay, myworld, myinfo);
			}

			updatepanel(mydisplay, myinfo, myworld);
			break;
		case DKEY_BACKSPACE:
		case DKEY_DELETE:
			zztErase(myworld, myinfo->cursorx, myinfo->cursory);
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'f':
		case 'F':
			dofloodfill(mydisplay, myworld, myinfo, key == 'F');
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld);
			break;
		case 'g':
		case 'G':
			dogradient(mydisplay, myworld, myinfo);
			drawscreen(mydisplay, myworld);
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
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
		case '/':
			/* Toggle backbuffer push locking */
			if (myinfo->backbuffer->lock == PATBUF_UNLOCK)
				myinfo->backbuffer->lock = PATBUF_NOPUSH;
			else
				myinfo->backbuffer->lock = PATBUF_UNLOCK;
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case 'a':
		case 'A':
			/* Toggle aqu mode - cursor movement loads pattern buffer automatically */
			myinfo->aqumode ^= 1;

			/* drawmode & gradmode can't be on while in aqumode */
			exitgradientmode(myinfo);

			if (myinfo->aqumode != 0) {
				/* Grab if aqumode is on */
				push(myinfo->backbuffer, zztTileGet(myworld, myinfo->cursorx, myinfo->cursory));
			}
			updatepanel(mydisplay, myinfo, myworld);
			break;

		/***************** Function Keys *******************/

		case DKEY_F1:
			/* F1 panel */
			itemmenu(mydisplay, myworld, myinfo);
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case DKEY_F2:
			/* F2 panel */
			creaturemenu(mydisplay, myworld, myinfo);

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case DKEY_F3:
			/* F3 panel */
			terrainmenu(mydisplay, myworld, myinfo);

			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			drawscreen(mydisplay, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			break;
		case DKEY_F4:
			/* F4 - Enter Text */
			myinfo->textentrymode ^= 1;
			updatepanel(mydisplay, myinfo, myworld);
			break;
		case DKEY_ENTER:
			/* Modify / Grab */
			modifyparam(mydisplay, myworld, myinfo->cursorx, myinfo->cursory);

			/* When all is said and done, push the tile */
			push(myinfo->backbuffer, zztTileGet(myworld, myinfo->cursorx, myinfo->cursory));

			/* redraw everything */
			drawpanel(mydisplay);
			updatepanel(mydisplay, myinfo, myworld);
			mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
			drawscreen(mydisplay, myworld);
			break;

		case DKEY_INSERT:
			/* Insert */
			/* Grab tile */
			if (myinfo->backbuffer->lock == PATBUF_UNLOCK) {
				/* Push if backbuffer is completely unlocked */
				push(myinfo->backbuffer, zztTileGet(myworld, myinfo->cursorx, myinfo->cursory));
			} else {
				/* Otherwise attempt to replace if backbuffer is selected */
				if (myinfo->pbuf == myinfo->backbuffer)
					patreplace(myinfo->backbuffer, zztTileGet(myworld, myinfo->cursorx, myinfo->cursory));
			}

			updatepanel(mydisplay, myinfo, myworld);
			break;
		}

		/* Common code for all movement actions */
		if (key == DKEY_LEFT     || key == DKEY_RIGHT     ||
		    key == DKEY_UP       || key == DKEY_DOWN      ||
		    key == DKEY_ALT_LEFT || key == DKEY_ALT_RIGHT ||
		    key == DKEY_ALT_UP   || key == DKEY_ALT_DOWN) {
			int repeat = 0;
			if (key == DKEY_ALT_LEFT || key == DKEY_ALT_RIGHT)
				repeat = 10;
			else if(key == DKEY_ALT_UP   || key == DKEY_ALT_DOWN)
				repeat = 5;

			do {
				/* Consider alt-direction actions */
				switch (key) {
					case DKEY_ALT_LEFT:
						/* Alt+Left */
						myinfo->cursorx--; repeat--;
						if (myinfo->cursorx < 0) { myinfo->cursorx = 0; repeat = 0; }
						break;
					case DKEY_ALT_RIGHT:
						/* Alt+Right */
						myinfo->cursorx++; repeat--;
						if (myinfo->cursorx > 59) { myinfo->cursorx = 59; repeat = 0; }
						break;
					case DKEY_ALT_UP:
						/* Alt+Up */
						myinfo->cursory--; repeat--;
						if (myinfo->cursory < 0) { myinfo->cursory = 0; repeat = 0; }
						break;
					case DKEY_ALT_DOWN:
						/* Alt+Down */
						myinfo->cursory++; repeat--;
						if (myinfo->cursory > 24) { myinfo->cursory = 24; repeat = 0; }
						break;
				}

				/* Act on keystrokes */
				if (myinfo->aqumode != 0) {
					/* Get if aquire mode is on */
					push(myinfo->backbuffer, zztTileGet(myworld, myinfo->cursorx, myinfo->cursory));
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
					plot(myworld, myinfo, mydisplay);
				}
				mydisplay->cursorgo(myinfo->cursorx, myinfo->cursory);
				updatepanel(mydisplay, myinfo, myworld);
				drawspot(mydisplay, myworld, myinfo);
			} while (repeat);
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
	deletepatternbuffer(myinfo->standard_patterns);
	deletepatternbuffer(myinfo->backbuffer);
	free(myinfo->standard_patterns);
	free(myinfo->backbuffer);

	/* Free everything! Free! Free! Free! Let freedom ring! */
	free(myinfo);
	zztWorldFree(myworld);

	return 0;
}
