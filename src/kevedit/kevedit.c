/* kevedit.c       -- main kevedit environment
 * $Id: kevedit.c,v 1.4 2005/06/29 03:20:34 kvance Exp $
 * Copyright (C) 2000-2001 Kev Vance <kvance@kvance.com>
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
 * Foundation, Inc., 59 Temple Place Suite 330; Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "kevedit.h"
#include "misc.h"
#include "menu.h"
#include "texteditor/editbox.h"
#include "screen.h"

#include "libzzt2/zzt.h"

#include "patbuffer.h"
#include "help/help.h"
#include "dialogs/infobox.h"
#include "dialogs/paramed.h"
#include "texteditor/register.h"

#include "display/display.h"

/* Local functions */

/* Update the interface */
void keveditUpdateInterface(keveditor * myeditor);

/* Handle selection mode */
void keveditHandleSelection(keveditor * myeditor);

/* Update the current selection */
void keveditUpdateSelection(keveditor * myeditor);

/* Handle text entry mode */
void keveditHandleTextEntry(keveditor * myeditor);

/* Handle keybindings (such as vi keys) */
void keveditHandleKeybindings(keveditor * myeditor);

/* Handle number keys (backbuffer selection) */
void keveditHandleNumberKeys(keveditor * myeditor);

/* Handle a distinct keypress */
void keveditHandleKeypress(keveditor * myeditor);

/* Handle movement keys */
void keveditHandleMovement(keveditor * myeditor);

keveditor * createkeveditor(ZZTworld * myworld, displaymethod * mydisplay, char * datapath)
{
	keveditor * myeditor;

	/* Allocate space for the editor data */
	myeditor = (keveditor *) malloc(sizeof(keveditor));

	/* Copy params */
	myeditor->myworld   = myworld;
	myeditor->mydisplay = mydisplay;
	myeditor->datapath  = datapath;

	/* Clear info to default values */
	myeditor->cursorx = 0;
	myeditor->cursory = 0;

	myeditor->width = ZZT_BOARD_X_SIZE;
	myeditor->height = ZZT_BOARD_Y_SIZE;

	myeditor->updateflags = UD_ALL | UD_BOARDTITLE | UD_WORLDTITLE;
	myeditor->quitflag = 0;

	myeditor->key = 0;

	myeditor->drawmode = 0;
	myeditor->gradmode = 0;
	myeditor->aqumode = 0;
	myeditor->textentrymode = 0;
	myeditor->defcmode = 1;

	myeditor->color.blink = 0;
	myeditor->color.fg    = 0x0f;
	myeditor->color.bg    = 0x00;

	myeditor->buffers.standard_patterns = createstandardpatterns();
	myeditor->buffers.backbuffer        = patbuffer_create(10);
	myeditor->buffers.pbuf              = myeditor->buffers.standard_patterns;

	/* Don't color standard patterns by default */
	myeditor->options.colorStandardPatterns = 0;

	myeditor->clearselectflag = 0;
	myeditor->selectmode = SELECT_OFF;
	myeditor->selx = -1; myeditor->sely = -1;
	initselection(&myeditor->selPersistant, ZZT_BOARD_X_SIZE, ZZT_BOARD_Y_SIZE);
	initselection(&myeditor->selCurrent, ZZT_BOARD_X_SIZE, ZZT_BOARD_Y_SIZE);

	myeditor->copyBlock = NULL;
	initselection(&myeditor->copySelection, ZZT_BOARD_X_SIZE, ZZT_BOARD_Y_SIZE);

	/* Use KVI environment variable to decide if vi keys should be used */
	if (getenv("KVI") == NULL) {
		/* No vi movement by default. :-( */
		myeditor->options.vimovement = 0;
	} else {
		if (str_equ(getenv("KVI"), "dvorak", STREQU_UNCASE))
			myeditor->options.vimovement = 2;
		else
			myeditor->options.vimovement = 1;
	}

	pat_applycolordata(myeditor->buffers.standard_patterns, myeditor->color);

        myeditor->char_set = NULL;
        myeditor->palette = NULL;
	return myeditor;
}

void deletekeveditor(keveditor * myeditor)
{
	/* Free the datapath */
	if (myeditor->datapath != NULL)
		free(myeditor->datapath);

	/* Free pattern buffers */
	deletepatternbuffer(myeditor->buffers.standard_patterns);
	deletepatternbuffer(myeditor->buffers.backbuffer);
	myeditor->buffers.standard_patterns = NULL;
	myeditor->buffers.backbuffer = NULL;

	/* Free the selection */
	deleteselection(&myeditor->selPersistant);
	deleteselection(&myeditor->selCurrent);

	/* Free the copied block */
	if (myeditor->copyBlock != NULL)
		zztBlockFree(myeditor->copyBlock);
	deleteselection(&myeditor->copySelection);

	/* Free everything! Free! Free! Free! Let freedom ring! */
        if(myeditor->char_set != NULL) {
                charset_free(myeditor->char_set);
        }
	free(myeditor);
}

void kevedit(keveditor * myeditor)
{
	if (myeditor == NULL)
		return;

	/* Loop until it is time to quit */
	while (myeditor->quitflag == 0) {

		/* Update the selection */
		keveditUpdateSelection(myeditor);
		/* Update the interface */
		keveditUpdateInterface(myeditor);

		/* Get the key */
                enum displaycontext cx = myeditor->textentrymode ? board_editor_text : board_editor;
		myeditor->key = myeditor->mydisplay->getch_with_context(cx);

		/* Undo the cursorspace (draw as a normal tile) */
		drawspot(myeditor);
		myeditor->updateflags |= UD_CURSOR;

		keveditHandleTextEntry(myeditor);
		keveditHandleSelection(myeditor);
		keveditHandleKeybindings(myeditor);
		keveditHandleNumberKeys(myeditor);

		/* Most action is handled here */
		keveditHandleKeypress(myeditor);

		keveditHandleMovement(myeditor);
	}
}

void keveditUpdateInterface(keveditor * myeditor)
{
	/* Update the kevedit interface */
	int uf = myeditor->updateflags;
	displaymethod * mydisplay = myeditor->mydisplay;

	/* Update panel takes care of panel related update flags */
	updatepanel(myeditor);

	/* Move the display cursor */
	mydisplay->cursorgo(myeditor->cursorx, myeditor->cursory);

	if (uf & UD_BOARD) {
		/* Draw the whole board */
		drawscreen(myeditor);
	} else {
		/* Otherwise, possibly draw parts of the board */

		if (uf & UD_SPOT) {
			/* Draw the spot around the cursor */
			drawspot(myeditor);
		}

		if (uf & UD_CURSOR) {
			/* Draw the tile under the cursor */
			cursorspace(myeditor);
		}
	}

	if (uf & UD_BOARDTITLE) {
		/* Draw the board title this time only */
		char * title = (char *)zztBoardGetTitle(myeditor->myworld);
		mydisplay->print(30 - strlen(title) / 2, 0, 0x70, title);

		/* Neat trick: next time this function is called, update the board
		 * and clear the title at the top of the screen. */
		myeditor->updateflags = UD_BOARD;
	} else {

		myeditor->updateflags = UD_NONE;  /* Everything should be updated now */
	}
}

void keveditHandleSelection(keveditor * myeditor)
{
	/* Block selection is based on whether the shift key is down */
	int selectblockflag = myeditor->mydisplay->shift();

	/* Shift may be used for two purposes that are unrelated to
	   block selection: hotkeys and capitals in text mode.
	   A hotkey is either Shift+literal or Shift+TAB, so we
	   shouldn't enable block selection in that case.
	   Text mode is characterized by the key being DKEY_NONE,
	   so don't enable text mode if the key is DKEY_NONE either.
	   Note that this will disable selection mode even when the
	   Shift+literal combo is not a hotkey. */
	if (myeditor->key == DKEY_NONE || myeditor->key == DKEY_SHIFT_TAB
		|| is_literal_key(myeditor->key))
		selectblockflag = 0;

	if (selectblockflag && myeditor->selectmode != SELECT_BLOCK) {
		/* Begin block selection */
		myeditor->selx = myeditor->cursorx;
		myeditor->sely = myeditor->cursory;
		myeditor->clearselectflag = 0;
		myeditor->selectmode = SELECT_BLOCK;
	}

	if (!selectblockflag && myeditor->selectmode == SELECT_BLOCK) {
		/* Block selection has ended: update selPersistant */
		copyselection(myeditor->selPersistant, myeditor->selCurrent);

		myeditor->selectmode = SELECT_ON;
		myeditor->updateflags |= UD_BOARD;
	}

	/* Flood selection */
	if (myeditor->key == 'x' || myeditor->key == 'X') {
		if (myeditor->key == 'x') {
			/* Persistantly select an area by flooding around the cursor */
			floodselect(zztBoardGetBlock(myeditor->myworld), myeditor->selPersistant,
									myeditor->cursorx, myeditor->cursory);
		} else {
			/* Select all tiles of the type under the cursor */
			tileselect(zztBoardGetBlock(myeditor->myworld), myeditor->selPersistant,
			           zztTileGet(myeditor->myworld, myeditor->cursorx, myeditor->cursory));
		}

		/* Current and persistant selections should be the same now */
		copyselection(myeditor->selCurrent, myeditor->selPersistant);

		myeditor->selx = myeditor->cursorx;
		myeditor->sely = myeditor->cursory;

		myeditor->selectmode = SELECT_ON;
		myeditor->clearselectflag = 0;
		myeditor->updateflags |= UD_BOARD;
	}
}

void keveditUpdateSelection(keveditor * myeditor)
{
	if (myeditor->selectmode == SELECT_BLOCK) {
		/* Block selection is on: update selCurrent */
		copyselection(myeditor->selCurrent, myeditor->selPersistant);
		selectblock(myeditor->selCurrent,
		            myeditor->selx, myeditor->sely,
		            myeditor->cursorx, myeditor->cursory);

		myeditor->updateflags |= UD_BOARD;
	}

	if (myeditor->selectmode && myeditor->clearselectflag) {
		/* Clear any selection in progress */
		clearselection(myeditor->selPersistant);
		clearselection(myeditor->selCurrent);
		myeditor->selectmode = SELECT_OFF;
		myeditor->clearselectflag = 0;
		myeditor->updateflags |= UD_BOARD;
	}
}


void keveditHandleTextEntry(keveditor * myeditor)
{
	int key = myeditor->key;

	if (myeditor->textentrymode == 0)
		return;

	/* Don't pass on the key if we handle it here */
	myeditor->key = DKEY_NONE;

	if (key == DKEY_ENTER || key == DKEY_ESC) {
		/* Leave text entry mode */
		myeditor->textentrymode ^= 1;
		myeditor->updateflags |= UD_TEXTMODE;
	} else if (key == DKEY_BACKSPACE) {
		/* Backspace */
		if (myeditor->cursorx > 0) {
			/* Move back on this line */
			myeditor->cursorx--;
			zztErase(myeditor->myworld, myeditor->cursorx, myeditor->cursory);
		} else {
			if (myeditor->cursory > 0) {
				/* Move back to previous line */
				myeditor->cursorx = myeditor->width - 1;
				myeditor->cursory--;
				zztErase(myeditor->myworld, myeditor->cursorx, myeditor->cursory);
			}
		}
		myeditor->updateflags |= UD_CURSOR;
	} else if (is_literal_key(key) || key == DKEY_CTRL_A) {
		/* Insert the current keystroke as text */
		ZZTtile textTile = { ZZT_BLUETEXT, 0x00, NULL };

		/* Determine appearance (encoded as color) based on keypress */
		textTile.color = key;

		/* Set color */
		encodetilecolor(&textTile, myeditor->color);

		/* ASCII selection dialog */
		if (key == DKEY_CTRL_A) {
			textTile.color = charselect(myeditor->mydisplay, -1);
			myeditor->updateflags |= UD_BOARD;
		}

		/* Plot the text character */
		zztPlot(myeditor->myworld, myeditor->cursorx, myeditor->cursory, textTile);

		/* Now move right */
		if (myeditor->cursorx < myeditor->width - 1) {
			/* There's room */
			myeditor->cursorx++;
		} else {
			/* Can't move past the edge of the screen, move down to the
			 * next line if possible. */
			if (myeditor->cursory < myeditor->height - 1) {
				myeditor->cursorx = 0;
				myeditor->cursory++;
				myeditor->updateflags |= UD_BOARD;
			}
		}
		myeditor->updateflags |= UD_SPOT;
	} else {
		/* Key could not be handled, pass it on */
		myeditor->key = key;
	}
}

void keveditHandleKeybindings(keveditor * myeditor)
{
	if (myeditor->options.vimovement == 1) {
		/* vi movement is on */
		switch (myeditor->key) {
			case 'h': myeditor->key = DKEY_LEFT;  break;
			case 'j': myeditor->key = DKEY_DOWN;  break;
			case 'k': myeditor->key = DKEY_UP;    break;
			case 'l': myeditor->key = DKEY_RIGHT; break;
		}
	} else if (myeditor->options.vimovement == 2) {
		/* dvorak vi movement is on (yes, bitman uses this) */
		switch (myeditor->key) {
			case 'd': myeditor->key = DKEY_LEFT;  break;
			case 'h': myeditor->key = DKEY_DOWN;  break;
			case 't': myeditor->key = DKEY_UP;    break;
			case 'n': myeditor->key = DKEY_RIGHT; break;
		}
	}
}

void keveditHandleNumberKeys(keveditor * myeditor)
{
	int key = myeditor->key;

	/* TODO: avoid handling the backbuffers directly */

	/* Check for number key */
	if (key >= '0' && key <= '9') {
		/* Change to appropriate backbuffer position */
		if (key == '0') {
			/* Empty tile slot in standard patterns */
			myeditor->buffers.pbuf = myeditor->buffers.standard_patterns;
			myeditor->buffers.pbuf->pos = 4;
		} else {
			/* Change to backbuffer slot */
			if (key - '1' < myeditor->buffers.backbuffer->size) {
				/* Only if backbuffer position exists */
				myeditor->buffers.pbuf = myeditor->buffers.backbuffer;
				myeditor->buffers.pbuf->pos = key - '1';
			}
		}
		myeditor->updateflags |= UD_PATTERNS;
		myeditor->key = DKEY_NONE;
	}
}

void keveditHandleKeypress(keveditor * myeditor)
{
	int next_key = DKEY_NONE;

	/* Act on key pressed */
	switch (myeditor->key) {
		case DKEY_NONE:
			break;

		case DKEY_ESC:
			myeditor->clearselectflag = 1;
			myeditor->drawmode = 0;
			myeditor->gradmode = 0;
			myeditor->aqumode  = 0;
			myeditor->textentrymode = 0;
			myeditor->updateflags |= UD_DRAWMODE | UD_TEXTMODE | UD_PATTERNS;
			break;

		/****************** Major actions ****************/

		case 'q':
		case 'Q':
		case DKEY_QUIT: {
			/* Quit */
			int result = confirmprompt(myeditor->mydisplay, "Quit?");
			if(result == CONFIRM_YES || result == CONFIRM_QUIT) {
				myeditor->quitflag = 1;
			} else {
				myeditor->updateflags |= UD_PANEL;
			}
			break;
		}
		case 'n':
		case 'N': {
			int result = confirmprompt(myeditor->mydisplay, "Make new world?");
			if (result == CONFIRM_YES) {
				myeditor->myworld = clearworld(myeditor->myworld);
				myeditor->updateflags |= UD_BOARD;
			} else if (result == CONFIRM_QUIT) {
				next_key = DKEY_QUIT;
			}

			myeditor->updateflags |= UD_PANEL;
			break;
		}
		case 'z':
		case 'Z': {
			int result = confirmprompt(myeditor->mydisplay, "Clear board?");
			if (result == CONFIRM_YES) {
				clearboard(myeditor->myworld);
				myeditor->updateflags |= UD_BOARD;
			} else if (result == CONFIRM_QUIT) {
				next_key = DKEY_QUIT;
			}

			myeditor->updateflags |= UD_PANEL;
			break;
		}
		case 'b':
		case 'B':
			if(switchboard(myeditor->myworld, myeditor->mydisplay) == DKEY_QUIT) {
				next_key = DKEY_QUIT;
			}
			myeditor->updateflags |= UD_ALL | UD_BOARDTITLE;
			break;
		case DKEY_PAGEDOWN:
			/* Switch to next board (bounds checking is automatic) */
			zztBoardSelect(myeditor->myworld, zztBoardGetCurrent(myeditor->myworld) + 1);

			myeditor->updateflags |= UD_BOARD | UD_OBJCOUNT | UD_BOARDTITLE;
			break;
		case DKEY_PAGEUP:
			/* Switch to previous board (bounds checking is automatic) */
			zztBoardSelect(myeditor->myworld, zztBoardGetCurrent(myeditor->myworld) - 1);

			myeditor->updateflags |= UD_BOARD | UD_OBJCOUNT | UD_BOARDTITLE;
			break;
		case DKEY_CTRL_PAGEDOWN:
			/* Switch to next page of boards (bounds checking is automatic) */
			zztBoardSelect(myeditor->myworld, zztBoardGetCurrent(myeditor->myworld) + 7);

			myeditor->updateflags |= UD_BOARD | UD_OBJCOUNT | UD_BOARDTITLE;
			break;
		case DKEY_CTRL_PAGEUP:
			/* Switch to previous page of boards (bounds checking is automatic) */
			zztBoardSelect(myeditor->myworld, zztBoardGetCurrent(myeditor->myworld) - 7);

			myeditor->updateflags |= UD_BOARD | UD_OBJCOUNT | UD_BOARDTITLE;
			break;
		case DKEY_CTRL_UP:
			if (zztBoardGetBoard_n(myeditor->myworld) > 0) {
				zztBoardSelect(myeditor->myworld, zztBoardGetBoard_n(myeditor->myworld));
	                        myeditor->updateflags |= UD_BOARD | UD_OBJCOUNT | UD_BOARDTITLE;
			}
			break;
		case DKEY_CTRL_DOWN:
			if (zztBoardGetBoard_s(myeditor->myworld) > 0) {
				zztBoardSelect(myeditor->myworld, zztBoardGetBoard_s(myeditor->myworld));
				myeditor->updateflags |= UD_BOARD | UD_OBJCOUNT | UD_BOARDTITLE;
			}
			break;
		case DKEY_CTRL_LEFT:
			if (zztBoardGetBoard_w(myeditor->myworld) > 0) {
				zztBoardSelect(myeditor->myworld, zztBoardGetBoard_w(myeditor->myworld));
				myeditor->updateflags |= UD_BOARD | UD_OBJCOUNT | UD_BOARDTITLE;
			}
			break;
		case DKEY_CTRL_RIGHT:
			if (zztBoardGetBoard_e(myeditor->myworld) > 0) {
				zztBoardSelect(myeditor->myworld, zztBoardGetBoard_e(myeditor->myworld));
				myeditor->updateflags |= UD_BOARD | UD_OBJCOUNT | UD_BOARDTITLE;
			}
			break;
		case 'i':
		case 'I':
			/* Board Info */
			next_key = editboardinfo(myeditor->myworld, myeditor->mydisplay);
			myeditor->updateflags |= UD_ALL;
			break;
		case 'w':
		case 'W':
			/* World Info */
			next_key = editworldinfo(myeditor);

			myeditor->updateflags |= UD_ALL | UD_WORLDTITLE;
			break;
		case DKEY_CTRL_T:
			/* Tile Info */
			next_key = tileinfo(myeditor->mydisplay, myeditor->myworld, myeditor->cursorx, myeditor->cursory);

			myeditor->updateflags |= UD_ALL;
			break;

		case DKEY_CTRL_S:
			/* Stats Info */
			if (statsinfo(myeditor->mydisplay, myeditor->myworld) == EDITBOX_QUIT)
				next_key = DKEY_QUIT;

			myeditor->updateflags |= UD_ALL;
			break;

		/********************* File operations ****************/

		case 's':
		case 'S':
			/* Save world */
			next_key = saveworld(myeditor->mydisplay, myeditor->myworld);
			myeditor->updateflags |= UD_ALL | UD_WORLDTITLE;
			break;
		case 'l':
		case 'L': {
			/* Load world */
			bool quit = false;
			myeditor->myworld = loadworld(myeditor->mydisplay, myeditor->myworld, NULL, &quit);
			if(quit) {
				next_key = DKEY_QUIT;
			}
			myeditor->updateflags |= UD_ALL | UD_BOARDTITLE | UD_WORLDTITLE;
			break;
		}
		case 't':
		case 'T':
			/* Transfer board */
			next_key = boardtransfer(myeditor->mydisplay, myeditor->myworld);
			myeditor->updateflags |= UD_ALL | UD_BOARDTITLE;
			break;
		case 'o':
		case 'O':
			/* Load object from library */
			next_key = objectlibrarymenu(myeditor);
			myeditor->updateflags |= UD_ALL;
			break;

		/****************** Colour ******************/

		case 'c':
			/* Change foregeound colour */
			myeditor->color.fg++;
			if (myeditor->color.fg == 16)
				myeditor->color.fg = 0;
			pat_applycolordata(myeditor->buffers.standard_patterns, myeditor->color);
			myeditor->updateflags |= UD_COLOR;
			break;
		case 'C':
			/* Change background colour */
			myeditor->color.bg++;
			if (myeditor->color.bg == 8)
				myeditor->color.bg = 0;
			pat_applycolordata(myeditor->buffers.standard_patterns, myeditor->color);
			myeditor->updateflags |= UD_COLOR;
			break;
                case 'r':
                case 'R':
                        /* Reverse Colors */
                        myeditor->color.fg = myeditor->color.fg + myeditor->color.bg;
                        myeditor->color.bg = myeditor->color.fg - myeditor->color.bg;
                        myeditor->color.fg = myeditor->color.fg - myeditor->color.bg;
                        if (myeditor->color.blink == 1) {
                                myeditor->color.fg = myeditor->color.fg + 8;
                        }
                        myeditor->color.blink = 0;
                        if (myeditor->color.bg > 7) {
                                myeditor->color.blink = 1;
                                myeditor->color.bg = myeditor->color.bg - 8;
                        }
                        pat_applycolordata(myeditor->buffers.standard_patterns, myeditor->color);
                        myeditor->updateflags |= UD_COLOR;
                        break;
		case 'k':
		case 'K':
		case DKEY_CTRL_K:
			/* Kolor selector */
			next_key = colorselector(myeditor->mydisplay, &(myeditor->color));
			pat_applycolordata(myeditor->buffers.standard_patterns, myeditor->color);

			myeditor->updateflags |= UD_COLOR | UD_BOARD;
			break;
		case 'v':
		case 'V':
			/* Toggle blink mode */
			myeditor->color.blink ^= 1;
			pat_applycolordata(myeditor->buffers.standard_patterns, myeditor->color);

			myeditor->updateflags |= UD_BLINKMODE;
			break;
		case 'd':
		case 'D':
			/* Toggle DefC mode */
			myeditor->defcmode ^= 1;
			myeditor->updateflags |= UD_COLORMODE;
			break;

		/****************** Misc *********************/

		case 'h':
		case 'H':
			/* Help */
			next_key = help(myeditor->mydisplay);

			myeditor->updateflags |= UD_ALL;
			break;
		case DKEY_ALT_T:  /* Alt-t (by popular demand) */
			/* run zzt */
			/* Load current world into zzt */
			myeditor->mydisplay->end();
			runzzt(myeditor->datapath, zztWorldGetFilename(myeditor->myworld));
			
			/* restart display from scratch */
			myeditor->mydisplay->init();

			myeditor->updateflags |= UD_ALL | UD_BOARDTITLE | UD_WORLDTITLE;
			break;
		case '!':
			/* Open text editor */
			next_key = texteditordialog(myeditor->mydisplay);

			myeditor->updateflags |= UD_ALL;
			break;
		case DKEY_CTRL_O:
			/* Show all objects */
			next_key = showObjects(myeditor);

			myeditor->updateflags |= UD_BOARD;
			break;

		/***************** Drawing *****************/

		case ' ':
			/* Plot */
			plot(myeditor);
			myeditor->updateflags |= UD_SPOT;
			break;
		case DKEY_TAB:
			/* Toggle draw mode */
			if (toggledrawmode(myeditor) != 0) {
				/* Update changes and start plotting if we entered draw mode */
				plot(myeditor);
				myeditor->updateflags |= UD_SPOT;
			}
			myeditor->updateflags |= UD_DRAWMODE;
			break;
		case DKEY_SHIFT_TAB:
			/* Shift-tab */
			if (togglegradientmode(myeditor) != 0) {
				/* Plot only when first turning gradmode on */
				plot(myeditor);
				myeditor->updateflags |= UD_SPOT;
			}

			myeditor->updateflags |= UD_DRAWMODE;
			break;
		case DKEY_BACKSPACE:
		case DKEY_DELETE:
			zztErase(myeditor->myworld, myeditor->cursorx, myeditor->cursory);
			myeditor->updateflags |= UD_OBJCOUNT | UD_SPOT;
			break;
		case 'f':
		case 'F':
			dofloodfill(myeditor, myeditor->key == 'F');
			myeditor->updateflags |= UD_BOARD | UD_OBJCOUNT;
			break;
		case 'g':
		case 'G':
			next_key = dogradient(myeditor);
			myeditor->updateflags |= UD_ALL;
			break;

		case DKEY_CTRL_C:
			copy(myeditor);
			break;

		case DKEY_CTRL_V:
			next_key = paste(myeditor);
			break;

		case DKEY_CTRL_X:
			copy(myeditor);
			dofloodfill(myeditor, 0);
			myeditor->updateflags |= UD_BOARD | UD_OBJCOUNT;
			break;

		/***************** Backbuffer Actions ****************/

		case 'p':
			/* Select new pattern forwards */
			nextpattern(myeditor);
			myeditor->updateflags |= UD_PATTERNS;
			break;
		case 'P':
			/* Select new pattern backwards */
			previouspattern(myeditor);
			myeditor->updateflags |= UD_PATTERNS;
			break;
		case '<':
			/* Decrease size of backbuffer */
			if (myeditor->buffers.backbuffer->size > 1) {
				patbuffer_resize(myeditor->buffers.backbuffer, -1);
				myeditor->updateflags |= UD_PATTERNS;
			}
			break;
		case '>':
			/* Increase size of backbuffer */
			if (myeditor->buffers.backbuffer->size < MAX_BACKBUF) {
				patbuffer_resize(myeditor->buffers.backbuffer, 1);
				myeditor->updateflags |= UD_PATTERNS;
			}
			break;
		case '/':
			/* Toggle backbuffer push locking */
			if (myeditor->buffers.backbuffer->lock == PATBUF_UNLOCK)
				myeditor->buffers.backbuffer->lock = PATBUF_NOPUSH;
			else
				myeditor->buffers.backbuffer->lock = PATBUF_UNLOCK;
			myeditor->updateflags |= UD_PATTERNS;
			break;
		case 'a':
		case 'A':
			/* TODO: this entry is far too long: move it somewhere else */
			/* Toggle aqu mode - cursor movement loads pattern buffer automatically */
			if (myeditor->aqumode) {
				myeditor->aqumode = 0;
			} else {
				if (myeditor->key == 'a') {
					myeditor->aqumode = AQUMODE_RESIZE;
				} else {
					myeditor->aqumode = AQUMODE_NORESIZE;
				}

				/* drawmode & gradmode can't be on while in aqumode */
				exitgradientmode(myeditor);
				myeditor->drawmode = 0;

				/* Grab now that aqumode is on */
				if (myeditor->aqumode == AQUMODE_RESIZE &&
				    myeditor->buffers.backbuffer->patterns[myeditor->buffers.backbuffer->size - 1].type != ZZT_EMPTY)
					/* Resize the backbuffer if necessary */
					patbuffer_resize(myeditor->buffers.backbuffer, 1);
				push(myeditor->buffers.backbuffer, zztTileGet(myeditor->myworld, myeditor->cursorx, myeditor->cursory));
			}

			myeditor->updateflags |= UD_PATTERNS | UD_DRAWMODE;
			break;

		/***************** Function Keys *******************/

		case DKEY_F1:
			/* F1 panel */
			next_key = itemmenu(myeditor);
			myeditor->updateflags |= UD_ALL;  /* TODO: can we narrow this down? */
			break;
		case DKEY_F2:
			/* F2 panel */
			next_key = creaturemenu(myeditor);
			myeditor->updateflags |= UD_ALL;  /* TODO: can we narrow this down? */
			break;
		case DKEY_F3:
			/* F3 panel */
			next_key = terrainmenu(myeditor);
			myeditor->updateflags |= UD_ALL;  /* TODO: can we narrow this down? */
			break;
		case DKEY_F4:
			/* F4 - Enter Text */
			myeditor->textentrymode ^= 1;
			myeditor->updateflags |= UD_TEXTMODE;
			break;

		case DKEY_ENTER: {
			/* Modify / Grab */
			next_key = modifyparam(myeditor->mydisplay, myeditor->myworld, myeditor->cursorx, myeditor->cursory);

			/* If the current tile is text, edit the char */
			ZZTtile tile = zztTileGet(myeditor->myworld, myeditor->cursorx, myeditor->cursory);
			if (zztTileIsText(tile)) {
				/* It's a text tile, open character selector to
				 * change the character. */
                                int result = charselect(myeditor->mydisplay, tile.color);
                                if(result == -1) {
                                        myeditor->updateflags |= UD_ALL;
                                        break;
                                }
                                tile.color = result;
                                /* And plot. */
                                zztPlot(myeditor->myworld, myeditor->cursorx, myeditor->cursory, tile);
			}

			/* When all is said and done, push the tile */
			push(myeditor->buffers.backbuffer, tile);

			myeditor->updateflags |= UD_ALL;
			break;
		}

		case DKEY_INSERT:
			/* Insert */
			/* Grab tile */
			if (myeditor->buffers.backbuffer->lock == PATBUF_UNLOCK) {
				/* Push if backbuffer is completely unlocked */
				push(myeditor->buffers.backbuffer, zztTileGet(myeditor->myworld, myeditor->cursorx, myeditor->cursory));
			} else {
				/* Otherwise attempt to replace if backbuffer is selected */
				if (myeditor->buffers.pbuf == myeditor->buffers.backbuffer)
					patreplace(myeditor->buffers.backbuffer, zztTileGet(myeditor->myworld, myeditor->cursorx, myeditor->cursory));
			}

			myeditor->updateflags |= UD_PATTERNS;
			break;

		case DKEY_DROPFILE:
			// TODO(kvance): only allow this on startup
			// or make dropping push a quit event and check drop on quit
			myeditor->myworld = loadworld(myeditor->mydisplay, myeditor->myworld, display.dropped_file, NULL);
			myeditor->updateflags |= UD_ALL | UD_BOARDTITLE | UD_WORLDTITLE;
			break;
	}

	/* Pass through quit immediately. */
	if(next_key == DKEY_QUIT) {
		myeditor->key = DKEY_QUIT;
		keveditHandleKeypress(myeditor);
	}
}

void keveditHandleMovement(keveditor * myeditor)
{
	int key = myeditor->key;

	/* Common code for all movement actions */
	if (key == DKEY_LEFT     || key == DKEY_RIGHT     ||
			key == DKEY_UP       || key == DKEY_DOWN      ||
			key == DKEY_ALT_LEFT || key == DKEY_ALT_RIGHT ||
			key == DKEY_ALT_UP   || key == DKEY_ALT_DOWN) {
		/* Repeat once for normal motion keys */
		int repeat = 1;
		if (key == DKEY_ALT_LEFT || key == DKEY_ALT_RIGHT) {
			/* Move ten at a time horizontally */
			repeat = 10;
			if (myeditor->drawmode || myeditor->gradmode)
				myeditor->updateflags |= UD_BOARD;
		}
		else if(key == DKEY_ALT_UP   || key == DKEY_ALT_DOWN) {
			/* Move five at a time vertically */
			repeat = 5;
			if (myeditor->drawmode || myeditor->gradmode)
				myeditor->updateflags |= UD_BOARD;
		}

		do {
			/* Consider each direction */
			switch (key) {
				case DKEY_LEFT:      /* Left */
				case DKEY_ALT_LEFT:  /* Alt+Left */
					myeditor->cursorx--; repeat--;
					if (myeditor->cursorx < 0) { myeditor->cursorx = 0; repeat = 0; }
					break;
				case DKEY_RIGHT:     /* Right */
				case DKEY_ALT_RIGHT: /* Alt+Right */
					myeditor->cursorx++; repeat--;
					if (myeditor->cursorx >= myeditor->width) { myeditor->cursorx = myeditor->width - 1; repeat = 0; }
					break;
				case DKEY_UP:        /* Up */
				case DKEY_ALT_UP:    /* Alt+Up */
					myeditor->cursory--; repeat--;
					if (myeditor->cursory < 0) { myeditor->cursory = 0; repeat = 0; }
					break;
				case DKEY_DOWN:      /* Down */
				case DKEY_ALT_DOWN:  /* Alt+Down */
					myeditor->cursory++; repeat--;
					if (myeditor->cursory >= myeditor->height) { myeditor->cursory = myeditor->height - 1; repeat = 0; }
					break;
			}

			/* TODO: do not modify buffers directly */

			/* Act on keystrokes */

			if (myeditor->aqumode != 0) {
				/* Get if aquire mode is on */
				if (myeditor->aqumode == AQUMODE_RESIZE &&
						myeditor->buffers.backbuffer->patterns[myeditor->buffers.backbuffer->size - 1].type != ZZT_EMPTY)
					/* Resize the backbuffer if necessary */
					patbuffer_resize(myeditor->buffers.backbuffer, 1);
				push(myeditor->buffers.backbuffer, zztTileGet(myeditor->myworld, myeditor->cursorx, myeditor->cursory));
			}

			/* If gradmode is on, cycle through the pattern buffer.
			 * Negative values move backward, positive values forward. */
			if (myeditor->gradmode < 0) {
				if (--myeditor->buffers.pbuf->pos < 0)
					myeditor->buffers.pbuf->pos = myeditor->buffers.pbuf->size - 1;
			} else if (myeditor->gradmode > 0) {
				if (++myeditor->buffers.pbuf->pos >= myeditor->buffers.pbuf->size)
					myeditor->buffers.pbuf->pos = 0;
			}

			/* If drawmode is on, plot */
			if (myeditor->drawmode == 1) {
				plot(myeditor);
			}

		} while (repeat > 0);

		myeditor->updateflags |= UD_PATTERNS | UD_OBJCOUNT | UD_SPOT;
	}
}
