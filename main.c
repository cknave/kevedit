/* main.c       -- The buck starts here
 * $Id: main.c,v 1.69 2002/06/02 03:55:01 bitman Exp $
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

	/* Update everything initially */
	myinfo->updateflags = UD_ALL | UD_BOARDTITLE;

	/* Main loop begins */

	while (quit == 0) {

		/* Update the interface */
		keveditUpdateInterface(mydisplay, myinfo, myworld);

		/* Get the key */
		key = mydisplay->getch();

		/* Undo the cursorspace (draw as a normal tile) */
		drawspot(mydisplay, myworld, myinfo);

		/* Cursor almost always needs updated */
		myinfo->updateflags |= UD_CURSOR;

		/* Check for text entry */
		if (myinfo->textentrymode == 1) {
			if (key == DKEY_ENTER || key == DKEY_ESC) {
				/* Leave text entry mode */
				key = DKEY_F4;
			} else if (key == DKEY_BACKSPACE && myinfo->cursorx > 0) {
				/* Backspace */
				myinfo->cursorx--;
				zztErase(myworld, myinfo->cursorx, myinfo->cursory);
				myinfo->updateflags |= UD_CURSOR;
			} else if ((key < 0x80 && key >= 0x20) || key == DKEY_CTRL_A) {
				/* Insert the current keystroke as text */
				ZZTtile textTile = { ZZT_BLUETEXT, 0x00, NULL };

				if (key == DKEY_CTRL_A) { /* ASCII selection */
					key = charselect(mydisplay, -1);
					myinfo->updateflags |= UD_BOARD;
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

		if (myinfo->vimovement == 1) {
			/* vi movement is on */
			switch (key) {
				case 'h': key = DKEY_LEFT;  break;
				case 'j': key = DKEY_DOWN;  break;
				case 'k': key = DKEY_UP;    break;
				case 'l': key = DKEY_RIGHT; break;
			}
		} else if (myinfo->vimovement == 2) {
			/* dvorak vi movement is on (yes, bitman uses this) */
			switch (key) {
				case 'd': key = DKEY_LEFT;  break;
				case 'h': key = DKEY_DOWN;  break;
				case 't': key = DKEY_UP;    break;
				case 'n': key = DKEY_RIGHT; break;
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
			myinfo->updateflags |= UD_PATTERNS;
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
				myinfo->updateflags |= UD_PANEL;
			}
			break;
		case 'n':
		case 'N':
			if (confirmprompt(mydisplay, "Make new world?") == CONFIRM_YES) {
				myworld = clearworld(myworld);

				myinfo->updateflags |= UD_BOARD;
			}

			myinfo->updateflags |= UD_PANEL;
			break;
		case 'z':
		case 'Z':
			if (confirmprompt(mydisplay, "Clear board?") == CONFIRM_YES) {
				clearboard(myworld);

				myinfo->updateflags |= UD_BOARD;
			}

			myinfo->updateflags |= UD_PANEL;
			break;

		case 'b':
		case 'B':
			switchboard(myworld, mydisplay);

			myinfo->updateflags |= UD_ALL | UD_BOARDTITLE;
			break;
		case DKEY_PAGEDOWN:
			/* Switch to next board (bounds checking is automatic) */
			zztBoardSelect(myworld, zztBoardGetCurrent(myworld) + 1);

			myinfo->updateflags |= UD_BOARD | UD_OBJCOUNT | UD_BOARDTITLE;
			break;
		case DKEY_PAGEUP:
			/* Switch to previous board (bounds checking is automatic) */
			zztBoardSelect(myworld, zztBoardGetCurrent(myworld) - 1);

			myinfo->updateflags |= UD_BOARD | UD_OBJCOUNT | UD_BOARDTITLE;
			break;

		case 'i':
		case 'I':
			/* Board Info */
			editboardinfo(myworld, mydisplay);

			myinfo->updateflags |= UD_ALL;
			break;
		case 'w':
		case 'W':
			/* World Info */
			editworldinfo(myworld, mydisplay);
			myinfo->changed_title = 1;

			myinfo->updateflags |= UD_ALL;
			break;
		case DKEY_CTRL_T:
			/* Tile Info */
			tileinfo(mydisplay, myworld, myinfo->cursorx, myinfo->cursory);

			myinfo->updateflags |= UD_ALL;
			break;

		/********************* File operations ****************/

		case 's':
		case 'S':
			/* Save world */
			saveworld(mydisplay, myworld);
			myinfo->changed_title = 1;

			myinfo->updateflags |= UD_ALL;
			break;
		case 'l':
		case 'L':
			/* Load world */
			myworld = loadworld(mydisplay, myworld);
			myinfo->changed_title = 1;

			myinfo->updateflags |= UD_ALL | UD_BOARDTITLE;
			break;
		case 't':
		case 'T':
			/* Transfer board */
			boardtransfer(mydisplay, myworld);

			myinfo->updateflags |= UD_ALL | UD_BOARDTITLE;
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
			myinfo->updateflags |= UD_COLOR;
			break;
		case 'C':
			/* Change background colour */
			myinfo->backc++;
			if (myinfo->backc == 8)
				myinfo->backc = 0;
			pat_applycolordata(myinfo->standard_patterns, myinfo);
			myinfo->updateflags |= UD_COLOR;
			break;
		case 'k':
		case 'K':
		case DKEY_CTRL_K:
			/* Kolor selector */
			colorselector(mydisplay, &myinfo->forec, &myinfo->backc,
										&myinfo->blinkmode);

			myinfo->updateflags |= UD_COLOR | UD_BOARD;
			break;
		case 'v':
		case 'V':
			/* Toggle blink mode */
			myinfo->blinkmode ^= 1;
			pat_applycolordata(myinfo->standard_patterns, myinfo);

			myinfo->updateflags |= UD_BLINKMODE;
			break;
		case 'd':
		case 'D':
			/* Toggle DefC mode */
			myinfo->defc ^= 1;
			myinfo->updateflags |= UD_COLORMODE;
			break;

		/****************** Misc *********************/

		case 'h':
		case 'H':
			/* Help */
			help(mydisplay);

			myinfo->updateflags |= UD_ALL;
			break;
		case 'r':
		case 'R':
		case DKEY_ALT_T:  /* By popular demand */
			/* run zzt */
			/* Load current world into zzt */
			mydisplay->end();
			runzzt(datapath, zztWorldGetFilename(myworld));
			
			/* restart display from scratch */
			mydisplay->init();

			myinfo->updateflags |= UD_ALL;
			myinfo->changed_title = 1;
			break;
		case '!':
			/* Open text editor */
			texteditor(mydisplay);

			myinfo->updateflags |= UD_ALL;
			break;

		/***************** Drawing *****************/

		case ' ':
			/* Plot */
			plot(myworld, myinfo, mydisplay);
			myinfo->updateflags |= UD_SPOT;
			break;
		case DKEY_TAB:
			/* Toggle draw mode */
			if (toggledrawmode(myinfo) != 0) {
				/* Update changes and start plotting if we entered draw mode */
				plot(myworld, myinfo, mydisplay);
				myinfo->updateflags |= UD_SPOT;
			}
			myinfo->updateflags |= UD_DRAWMODE;
			break;
		case DKEY_SHIFT_TAB:
			/* Shift-tab */
			if (togglegradientmode(myinfo) != 0) {
				/* Plot only when first turning gradmode on */
				plot(myworld, myinfo, mydisplay);
				myinfo->updateflags |= UD_SPOT;
			}

			myinfo->updateflags |= UD_DRAWMODE;
			break;
		case DKEY_BACKSPACE:
		case DKEY_DELETE:
			zztErase(myworld, myinfo->cursorx, myinfo->cursory);
			myinfo->updateflags |= UD_OBJCOUNT;
			break;
		case 'f':
		case 'F':
			dofloodfill(mydisplay, myworld, myinfo, key == 'F');
			myinfo->updateflags |= UD_BOARD | UD_OBJCOUNT;
			break;
		case 'g':
		case 'G':
			dogradient(mydisplay, myworld, myinfo);
			myinfo->updateflags |= UD_ALL;
			break;

		/***************** Backbuffer Actions ****************/

		case 'p':
			/* Select new pattern forwards */
			nextpattern(myinfo);
			myinfo->updateflags |= UD_PATTERNS;
			break;
		case 'P':
			/* Select new pattern backwards */
			previouspattern(myinfo);
			myinfo->updateflags |= UD_PATTERNS;
			break;
		case '<':
			/* Decrease size of backbuffer */
			if (myinfo->backbuffer->size > 1) {
				patbuffer_resize(myinfo->backbuffer, -1);
				myinfo->updateflags |= UD_PATTERNS;
			}
			break;
		case '>':
			/* Increase size of backbuffer */
			if (myinfo->backbuffer->size < MAX_BACKBUF) {
				patbuffer_resize(myinfo->backbuffer, 1);
				myinfo->updateflags |= UD_PATTERNS;
			}
			break;
		case '/':
			/* Toggle backbuffer push locking */
			if (myinfo->backbuffer->lock == PATBUF_UNLOCK)
				myinfo->backbuffer->lock = PATBUF_NOPUSH;
			else
				myinfo->backbuffer->lock = PATBUF_UNLOCK;
			myinfo->updateflags |= UD_PATTERNS;
			break;
		case 'a':
		case 'A':
			/* Toggle aqu mode - cursor movement loads pattern buffer automatically */
			if (myinfo->aqumode) {
				myinfo->aqumode = 0;
			} else {
				if (key == 'a') {
					myinfo->aqumode = AQUMODE_RESIZE;
				} else {
					myinfo->aqumode = AQUMODE_NORESIZE;
				}

				/* drawmode & gradmode can't be on while in aqumode */
				exitgradientmode(myinfo);

				/* Grab now that aqumode is on */
				if (myinfo->aqumode == AQUMODE_RESIZE && myinfo->backbuffer->patterns[myinfo->backbuffer->size - 1].type != ZZT_EMPTY)
					/* Resize the backbuffer if necessary */
					patbuffer_resize(myinfo->backbuffer, 1);
				push(myinfo->backbuffer, zztTileGet(myworld, myinfo->cursorx, myinfo->cursory));
			}

			myinfo->updateflags |= UD_PATTERNS;
			break;

		/***************** Function Keys *******************/

		case DKEY_F1:
			/* F1 panel */
			itemmenu(mydisplay, myworld, myinfo);
			myinfo->updateflags |= UD_ALL;  /* TODO: can we narrow this down? */
			break;
		case DKEY_F2:
			/* F2 panel */
			creaturemenu(mydisplay, myworld, myinfo);
			myinfo->updateflags |= UD_ALL;  /* TODO: can we narrow this down? */
			break;
		case DKEY_F3:
			/* F3 panel */
			terrainmenu(mydisplay, myworld, myinfo);
			myinfo->updateflags |= UD_ALL;  /* TODO: can we narrow this down? */
			break;
		case DKEY_F4:
			/* F4 - Enter Text */
			myinfo->textentrymode ^= 1;
			myinfo->updateflags |= UD_TEXTMODE;
			break;
		case DKEY_ENTER:
			/* Modify / Grab */
			modifyparam(mydisplay, myworld, myinfo->cursorx, myinfo->cursory);

			/* When all is said and done, push the tile */
			push(myinfo->backbuffer, zztTileGet(myworld, myinfo->cursorx, myinfo->cursory));

			myinfo->updateflags |= UD_ALL;
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

			myinfo->updateflags |= UD_PATTERNS;
			break;
		}

		/* Common code for all movement actions */
		if (key == DKEY_LEFT     || key == DKEY_RIGHT     ||
		    key == DKEY_UP       || key == DKEY_DOWN      ||
		    key == DKEY_ALT_LEFT || key == DKEY_ALT_RIGHT ||
		    key == DKEY_ALT_UP   || key == DKEY_ALT_DOWN) {
			int repeat = 0;
			if (key == DKEY_ALT_LEFT || key == DKEY_ALT_RIGHT) {
				repeat = 10;
				if (myinfo->drawmode || myinfo->gradmode)
					myinfo->updateflags |= UD_BOARD;
			}
			else if(key == DKEY_ALT_UP   || key == DKEY_ALT_DOWN) {
				repeat = 5;
				if (myinfo->drawmode || myinfo->gradmode)
					myinfo->updateflags |= UD_BOARD;
			}

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
					if (myinfo->aqumode == AQUMODE_RESIZE && myinfo->backbuffer->patterns[myinfo->backbuffer->size - 1].type != ZZT_EMPTY)
						/* Resize the backbuffer if necessary */
						patbuffer_resize(myinfo->backbuffer, 1);
					push(myinfo->backbuffer, zztTileGet(myworld, myinfo->cursorx, myinfo->cursory));
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

			} while (repeat);

			myinfo->updateflags |= UD_PATTERNS | UD_OBJCOUNT | UD_SPOT;
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
