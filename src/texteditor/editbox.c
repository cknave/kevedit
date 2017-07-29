/* editbox.c  -- text editor/viewer in kevedit
 * $Id: editbox.c,v 1.3 2005/05/28 03:17:46 bitman Exp $
 * Copyright (C) 2000 Ryan Phillips <bitman@users.sf.net>
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


#include "editbox.h"

#include "kevedit/screen.h"

#include "structures/svector.h"
#include "zzm.h"

#include "register.h"
#include "help/help.h"

#include "themes/theme.h"

#include "synth/synth.h"
#include "synth/zzm.h"

#include "libzzt2/zztoop.h"

#include "display/display.h"
#include "display/colours.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


/* What portion of display box needs update? */
#define U_NONE      0x00
#define U_CENTER    0x01
#define U_BOTTOM    0x02
#define U_TOP       0x04
#define U_EDITAREA  0x07
#define U_TITLE     0x08
#define U_PANEL     0x10
#define U_ALL       0xFF

/* string leader: string to appear at beginning and end of edit field
 * (now it looks like in zzt, except centered) */
#define SLEADER  "\x07    \x07    \x07    \x07    \x07    \x07    \x07    \x07    \x07"

#define ZOC_TEXT_COLOUR        GREEN_F | BRIGHT_F
#define ZOC_HIGHLIGHT_COLOUR   BLACK_F | WHITE_B
#define ZOC_MPLAY_COLOUR       WHITE_F | BRIGHT_F

void testMusic(stringvector* sv, int slur, int editwidth, int flags, displaymethod* d);

/* how to display a line of text in updateditbox() */
#define displayline(x, y, s, edit, flags, firstline, d) ((flags & EDITBOX_ZOCMODE) ? displayzoc((x), (y), (s), !(edit), (firstline), (d)) : d->print_discrete((x), (y), ZOC_TEXT_COLOUR, (s)))


/***** draweditpanel() ***********/
void draweditpanel(int insertflag, int wrapwidth, int zocmode, displaymethod * d)
{
	char buf[10] = "";
	drawsidepanel(d, PANEL_EDIT);
	
	d->print(76, 6,  YELLOW_F | BRIGHT_F | BLUE_B, (insertflag ? "on" : "off"));

	sprintf(buf, "%d", wrapwidth);

	if (wrapwidth)
		d->print(76, 8, YELLOW_F | BRIGHT_F | BLUE_B, buf);
	else
		d->print(72, 8, YELLOW_F | BRIGHT_F | BLUE_B, "off");
}

void updateditbox(displaymethod * d, stringvector* sv, int updateflags, int editwidth, int flags, char* title, int doupdate)
{
	/* update title if needed */
	if (updateflags & U_TITLE) {
		drawscrollbox(d, 1, 17, 0);
		d->print_discrete(30 - (strlen(title) / 2), 4, 0x0a, title);
	}

	/* clear the scrollbox */
	if (updateflags & U_TOP)
		drawscrollbox(d, 3, 9, 0);
	if (updateflags & U_CENTER)
		drawscrollbox(d, 10, 8, 0);
	if (updateflags & U_BOTTOM)
		drawscrollbox(d, 11, 1, 0);

	if (updateflags & (U_CENTER)) {
		/* Draw the center */
		displayline(9, 13, sv->cur->s, editwidth, flags, sv->cur->prev == NULL, d);
	}

	if (updateflags & (U_BOTTOM)) {
		/* Draw bottom half */
		int i;
		stringnode* loopstr = sv->cur->next;
		for (i = 1; i < 8 && loopstr != NULL; i++, loopstr = loopstr->next)
			displayline(9, i + 13, loopstr->s, editwidth, flags, 0, d);

		if (i < 8)
			d->print_discrete(9, i + 13, 0x07, SLEADER);
	}
	if (updateflags & U_TOP) {
		/* Draw top half */
		int i;
		stringnode* loopstr = sv->cur->prev;
		for (i = -1; i > -8 && loopstr != NULL; i--, loopstr = loopstr->prev)
			displayline(9, i + 13, loopstr->s, editwidth, flags, loopstr->prev == NULL, d);

		if (!editwidth && loopstr == NULL && sv->first->s[0] == '@')
			i++;

		if (i > -8)
			d->print_discrete(9, i + 13, 0x07, SLEADER);
	}

	/* Update the display */
	if (doupdate)
		d->update(3, 4, 51, 19);
}


/* Space to reserve for the saved file name */
#define SAVEFILENAME_LEN 1024

int editbox(char *title, stringvector * sv, int editwidth, int flags, displaymethod * d)
{
	int key;                /* the key */
	int i, j;               /* general counters */
	int done = 0;           /* true when editing/viewing is done */
	int updateflags;        /* flags to determine what needs update */
	stringnode *centerstr;  /* str in center of dialog */
	stringnode *loopstr;    /* node pointer for use in looping */
	int selChar = 0;

	/* vars only relating to editing */
	int pos = 0;            /* position in sv->cur->s */
	char *tmpstr;           /* temporary string for pushing */
	char strbuf[80] = "";   /* general buffer */
	static char savefilename[SAVEFILENAME_LEN] = "temp.zoc";

	/* selection variables */
	int selectFlag = 0;     /* Status of the shift key */
	int selPos = -1;        /* Position of cursor when selection started; -1 when no selection */
	int selLineOffset = 0;  /* Offset of line where selection started;
	                           positive when below centerstr, negative when above */

	/* statics */
	static int insertflag = 1;   /* nonzero when in insert mode */
	static int wrapwidth = 42;   /* where to wrap */

	/* if there is no string, add one */
	if (sv->cur == NULL || sv->first == NULL || sv->last == NULL)
		pushstring(sv, strcpy((char *) malloc(editwidth + 2), ""));

	if (sv->cur == NULL)
		return 0;

	centerstr = sv->cur;

	if (editwidth == EDITBOX_NOEDIT) {
		d->cursorgo(9, 13);
		/* Look for @title on first line */
		if ((flags & EDITBOX_ZOCMODE) && sv->first != NULL && sv->first->s[0] == '@') {
			/* Display the first line as the title, not in the box itself */
			if (sv->first->s[1] != '\x0')
				title = sv->first->s + 1; /* What's the harm? We're only looking. */
			if (centerstr == sv->first)
				centerstr = centerstr->next;
		}
	}
	
	/* Check for NULL after advancing past @title, if we did so */
	if (centerstr == NULL)
		return 0;

	drawscrollbox(d, 0, 0, 1);
	updateflags = U_ALL;

	while (!done) {
		if (editwidth)
			d->cursorgo(9 + pos, 13);

		/* If in select mode, center line should be updated no matter what */
		if (selPos != -1)
			updateflags |= U_CENTER;

		if (updateflags & U_PANEL && editwidth)
			draweditpanel(insertflag, wrapwidth, flags & EDITBOX_ZOCMODE, d);

		sv->cur = centerstr;
		updateditbox(d, sv, updateflags, editwidth, flags, title, selPos == -1);
		updateflags = U_NONE;

		/* Draw highlighted text if applicable */
		if (selPos != -1) {
			int startPos = 0, endPos = 0;
			if (selLineOffset > 0) {
				startPos = pos;
				endPos = strlen(centerstr->s);
			} else if (selLineOffset < 0) {
				startPos = 0;
				endPos = pos;
			} else {
				if (selPos > pos) {
					startPos = pos;
					endPos = selPos;
				} else {
					startPos = selPos;
					endPos = pos;
				}
			}
			for (j = startPos; j < endPos; j++)
				d->putch(9 + j, 13, centerstr->s[j], ZOC_HIGHLIGHT_COLOUR);

			if (selLineOffset != 0) {
				/* Draw meat lines */
				if (selLineOffset > 0) {
					for (i = 1, loopstr = centerstr->next; i < selLineOffset && i < 8 && loopstr != NULL; i++, loopstr = loopstr->next)
						d->print_discrete(9, 13 + i, ZOC_HIGHLIGHT_COLOUR, loopstr->s);
				} else {
					for (i = -1, loopstr = centerstr->prev; i > selLineOffset && i > -8 && loopstr != NULL; i--, loopstr = loopstr->prev)
						d->print_discrete(9, 13 + i, ZOC_HIGHLIGHT_COLOUR, loopstr->s);
				}

				/* Draw farthest line from centerstr */
				if (i < 8 && i > -8 && loopstr != NULL) {
					if (selLineOffset < 0) {
						startPos = selPos;
						endPos = strlen(loopstr->s);
					} else if (selLineOffset > 0) {
						startPos = 0;
						endPos = selPos;
					}
					for (j = startPos; j < endPos; j++)
						d->putch_discrete(9 + j, 13 + i, loopstr->s[j], ZOC_HIGHLIGHT_COLOUR);
				}
			}

			/* Update the display */
			d->update(3, 4, 51, 19);
		}

		/* Get the key */
		key = d->getch();

		selectFlag = d->shift();

		/* If we just started selecting, remember where we started */
		if (selectFlag && selPos == -1)
			selPos = pos;

		/* Keys which work when editing and browsing */
		switch (key) {
			case DKEY_UP:  /* Up Arrow */
				if (centerstr->prev != NULL && !(!editwidth && centerstr->prev == sv->first && sv->first->s[0] == '@')) {
					centerstr = centerstr->prev;
					if (pos > strlen(centerstr->s))
						pos = strlen(centerstr->s);
					if (selectFlag)
						selLineOffset++;
					updateflags = U_EDITAREA;
				}
				break;

			case DKEY_DOWN:  /* Down Arrow */
				if (centerstr->next != NULL) {
					centerstr = centerstr->next;
					if (pos > strlen(centerstr->s))
						pos = strlen(centerstr->s);
					if (selectFlag)
						selLineOffset--;
					updateflags = U_EDITAREA;
				}
				break;

			case DKEY_PAGEUP:  /* Page Up */
				for (i = 0; i < 7 && centerstr->prev != NULL && !(!editwidth && centerstr->prev == sv->first && sv->first->s[0] == '@'); i++) {
					centerstr = centerstr->prev;
					if (selectFlag)
						selLineOffset++;
				}
				if (pos > strlen(centerstr->s))
					pos = strlen(centerstr->s);
				updateflags = U_EDITAREA;
				break;

			case DKEY_PAGEDOWN:  /* Page Down */
				for (i = 0; i < 7 && centerstr->next != NULL; i++) {
					centerstr = centerstr->next;
					if (selectFlag)
						selLineOffset--;
				}
				if (pos > strlen(centerstr->s))
					pos = strlen(centerstr->s);
				updateflags = U_EDITAREA;
				break;

			case DKEY_CTRL_C:
			case DKEY_CTRL_X:
				/* Copy to register */
				if (selPos != -1) {
					stringnode *selStart = centerstr, *selEnd = centerstr;
					int selStartPos, selEndPos;
					selectionBounds bounds;

					if (selLineOffset > 0) {
						/* Other end of selection is below current line, move end down to meet it. */
						selStartPos = pos;
						selEndPos = selPos;
						for (i = 0; i < selLineOffset; i++)
							if (selEnd->next != NULL)
								selEnd = selEnd->next;
					} else if (selLineOffset < 0) {
						/* Other end of selection is above current line, move end up to meet it. */
						selStartPos = selPos;
						selEndPos = pos;
						for (i = 0; i > selLineOffset; i--)
							if (selStart->prev != NULL)
								selStart = selStart->prev;
					} else {
						/* Selection is only on current line: selStartPos gets the lesser of selPos & pos */
						if (selPos > pos) {
							selStartPos = pos;
							selEndPos = selPos;
						} else {
							selStartPos = selPos;
							selEndPos = pos;
						}
					}

					bounds.startLine = selStart;
					bounds.endLine = selEnd;
					bounds.startPos = selStartPos;
					bounds.endPos = selEndPos;

					regyank('\"', bounds);
				}
				break;

			case DKEY_ESC:
				if (editwidth > EDITBOX_NOEDIT)
					done = EDITBOX_OK;
				else
					done = EDITBOX_CANCEL;
				break;

			case DKEY_QUIT:
				done = EDITBOX_QUIT;
				break;
		}

		/* Keys pertaining to browsing only */
		if (editwidth == EDITBOX_NOEDIT) {
			switch (key) {
				case DKEY_ENTER:
					done = EDITBOX_OK;
					break;
			}

			/* If movement is enabled... */
			if (flags & EDITBOX_MOVEMENT) {
				switch (key) {
					case DKEY_BACKSPACE:
						done = EDITBOX_BACK;
						break;

					case DKEY_RIGHT:
						done = EDITBOX_FORWARD;
						break;

					case DKEY_LEFT:
						done = EDITBOX_BACKWARD;
						break;

					case DKEY_F1:
						done = EDITBOX_HELP;
						break;
				}
			}
		}

		/* Keys pertaining to editing only */
		if (editwidth > EDITBOX_NOEDIT) {
			/* We are edititing! Yea! Fun time! */

			switch (key) {
				case DKEY_NONE:
					break;

				/********** Movement ***********/

				case DKEY_LEFT:  /* Left Arrow */
					if (pos > 0)
						pos--;
					else {
						/* Move to end of previous line (or current line) */
						if (centerstr->prev != NULL) {
							centerstr = centerstr->prev;
							updateflags = U_EDITAREA;
						}
						pos = strlen(centerstr->s);
						if (selectFlag)
							selLineOffset++;
					}
					break;

				case DKEY_RIGHT:    /* Right Arrow */
					if (pos < strlen(centerstr->s))
						pos++;
					else {
						/* Move to begining of next line (or current line) */
						if (centerstr->next != NULL) {
							centerstr = centerstr->next;
							updateflags = U_EDITAREA;
						}
						pos = 0;
						if (selectFlag)
							selLineOffset--;
					}
					break;

				case DKEY_HOME: /* Home */
					pos = 0;
					break;

				case DKEY_END: /* End */
					pos = strlen(centerstr->s);
					break;

				case DKEY_UP:
				case DKEY_DOWN:
				case DKEY_PAGEUP:
				case DKEY_PAGEDOWN:
				case DKEY_CTRL_C:
					/* Avoid inserting these keys */
					break;

				/********** Insert & Delete ***********/

				case DKEY_INSERT:
					/* Insert */
					insertflag = !insertflag;
					updateflags = U_PANEL;
					break;

				case DKEY_DELETE:
					/* Delete */
					if (pos < strlen(centerstr->s)) {
						for (i = pos; i < strlen(centerstr->s); i++)
							centerstr->s[i] = centerstr->s[i+1];
						updateflags = U_CENTER;
					}
					else if (strlen(centerstr->s) == 0 && !(sv->first == sv->last)) {
						/* This string is empty: destroy */
						sv->cur = centerstr;
						deletestring(sv);
						centerstr = sv->cur;
						pos = strlen(centerstr->s);
						updateflags = U_EDITAREA;
					}
					else if (centerstr->next != NULL) {
						if (strlen(centerstr->next->s) == 0) {
							/* Next string is empty: destroy */
							sv->cur = centerstr->next;
							deletestring(sv);
							updateflags = U_BOTTOM;
						}
						else if (strlen(centerstr->s) + 1 < wrapwidth) {
							/* merge lines; wordwrap */
							i = strlen(centerstr->s);
							if (centerstr->s[i-1] != ' ' && centerstr->next->s[0] != ' ') {
								/* add a space at the end */
								centerstr->s[i]   = ' ';
								centerstr->s[++i] = 0;
							}
							sv->cur = centerstr->next;
							tmpstr = removestring(sv);
							sv->cur = centerstr;
							pos = wordwrap(sv, tmpstr, i, -1, wrapwidth, editwidth);
							centerstr = sv->cur;
							free(tmpstr);
							updateflags = U_CENTER | U_BOTTOM | U_TOP;
						}
					}
					break;

				/****** ZOC Mode & Wordwrap settings **********/

				case DKEY_ALT_Z: /* alt-z - toggle ZOC mode */
					flags ^= EDITBOX_ZOCMODE;
					updateflags = U_PANEL | U_EDITAREA;
					break;

				case DKEY_ALT_MINUS: /* alt - */
					if (wrapwidth > 10)
						wrapwidth--;
					else
						wrapwidth = editwidth;
					updateflags = U_PANEL;
					break;

				case DKEY_ALT_PLUS:    /* alt + */
					if (wrapwidth < editwidth)
						wrapwidth++;
					else
						wrapwidth = 10;
					updateflags = U_PANEL;
					break;

				/****** Help dialog ******/

				case DKEY_F1: /* F1: help dialog */
					/* Look for #command on current line for lookup in help */
					i = pos;
					while (i > 0 && centerstr->s[i] != '#')
						i--;

					if (centerstr->s[i] == '#') {
						/* Copy the command onto tmpstr */
						tmpstr = str_dup(centerstr->s + i + 1);
						for (i = 0; tmpstr[i] != ' ' && tmpstr[i] != '\0'; i++)
							;
						tmpstr[i] = '\0';

						if (zztoopFindCommand(tmpstr) == -1) {
							/* If it's not a valid command, don't bother looking for it */
							tmpstr[0] = '\0';
						}

						/* Display the help file with the command as the topic */
						helpsectiontopic("langref", tmpstr, d);
						
						free(tmpstr);
					} else {
						/* Display the oop help file */
						helpsectiontopic("langref", NULL, d);
					}

					updateflags = U_ALL;
					break;

				/********* ZZM Testing ********************/
				case DKEY_CTRL_T:
				case DKEY_ALT_T:
					sv->cur = centerstr;
					testMusic(sv, key == DKEY_CTRL_T, editwidth, flags, d);
					updateflags = U_EDITAREA;
					break;

				/********* File access operations *********/
				case DKEY_ALT_O: /* alt+o: open file */
				case DKEY_ALT_I:
					/* alt+i: insert file */
					{
						stringvector filetypelist;
						char* filename = NULL;
						
						initstringvector(&filetypelist);

						pushstring(&filetypelist, "*.zoc");
						pushstring(&filetypelist, "*.txt");
						pushstring(&filetypelist, "*.hlp");
						pushstring(&filetypelist, "*.zzm");
						pushstring(&filetypelist, "*.*");
						if (editbox("Select A File Type", &filetypelist, 0, 1, d) == 27) {
							updateflags = U_EDITAREA | U_TITLE;
							break;
						}

						if (filetypelist.cur != NULL) {
                            bool quit = false;
                            filename =
                                    filedialog(".", filetypelist.cur->s + 2,
                                               (key == DKEY_ALT_O ?
                                                "Open ZZT Object Code (ZOC) File" :
                                                "Insert ZZT Object Code (ZOC) File"),
                                               FTYPE_ALL, d, &quit);
                            if(quit) {
                                done = EDITBOX_QUIT;
                                break;
                            }
                        }
						if (filename != NULL && strlen(filename) != 0) {
							stringvector newsvector;
							newsvector = filetosvector(filename, wrapwidth, editwidth);
							if (newsvector.first != NULL) {
								if (key == DKEY_ALT_O) {
									strcpy(savefilename, filename);
									/* erase & replace sv */
									deletestringvector(sv);
									*sv = newsvector;
									centerstr = sv->first;
								} else {
									/* insert newsvector before centerstr */
									sv->cur = centerstr;
									if (sv->cur == sv->first) {
										/* first node */
										sv->first = newsvector.first;
										sv->cur->prev = newsvector.last;
										newsvector.last->next = sv->cur;
										centerstr = newsvector.first;
									} else if (sv->cur->prev != NULL) {
										/* middle/end node */
										newsvector.first->prev = sv->cur->prev;
										sv->cur->prev->next = newsvector.first;
										newsvector.last->next = sv->cur;
										sv->cur->prev = newsvector.last;
										centerstr = newsvector.first;
									} else {
										/* this code should be unreachable */
										deletestringvector(&newsvector);
									}
								} /* esle alt-i */
							}	/* fi file selected */
						}		/* fi not empty */
						free(filename);
						removestringvector(&filetypelist);
					}			/* block */
					updateflags = U_EDITAREA | U_TITLE | U_PANEL;
					break;

				case DKEY_ALT_S: /* alt-s: save to file */
					{
						bool quit = false;
						char* filename;
						filename = filenamedialog(savefilename, "", "Save Object Code As",
																			1, d, &quit);
						if (quit) {
							done = EDITBOX_QUIT;
							break;
						}
						if (filename != NULL) {
							/* Save to the file */
							svectortofile(sv, filename);
							/* Remember the file name */
							strncpy(savefilename, filename, SAVEFILENAME_LEN - 1);
							savefilename[SAVEFILENAME_LEN - 1] = '\x0';

							free(filename);
						}
					}
					updateflags = U_EDITAREA | U_PANEL | U_PANEL;
					break;

				case DKEY_ALT_M: /* alt-m: load .zzm music */
					{
                        bool quit = false;
						char* filename;
						filename = filedialog(".", "zzm", "Choose ZZT Music (ZZM) File",
																	FTYPE_ALL, d, &quit);
                        if(quit) {
                            done = EDITBOX_QUIT;
                            break;
                        }
						if (filename != NULL) {
							stringvector zzmv;
							zzmv = filetosvector(filename, 80, 80);
							if (zzmv.first != NULL) {
								stringvector song;
								song = zzmpullsong(&zzmv, zzmpicksong(&zzmv, d));
								if (song.first != NULL) {
									/* copy song into sv */
									sv->cur = centerstr;
									for (song.cur = song.first; song.cur != NULL; song.cur = song.cur->next) {
										tmpstr = (char*) malloc(editwidth + 2);

										if (flags & EDITBOX_ZOCMODE) {
											strcpy(tmpstr, "#play ");
											strncat(tmpstr, song.cur->s, editwidth - 6);
										} else {
											strncpy(tmpstr, song.cur->s, editwidth);
										}

										preinsertstring(sv, tmpstr);
									}
									deletestringvector(&song);
								}
								deletestringvector(&zzmv);
							}
						}
						free(filename);
					}
					updateflags = U_EDITAREA | U_TITLE | U_PANEL;
					break;

				case DKEY_CTRL_R: /* ctrl-r: rip music */
					{
						/* This is mostly worthless just now */
						stringvector ripped;
						sv->cur = centerstr;
						ripped = zzmripsong(sv, 4);
						scrolldialog("Ripped Music", &ripped, d);
						deletestringvector(&ripped);
						updateflags = U_ALL;
					}
					break;

				/******** Cut operation *********/

				case DKEY_CTRL_DELETE:    /* ctrl-delete: clear selected text */
				case DKEY_CTRL_X:         /* ctrl-x: cut selected text */
					sv->cur = centerstr;
					/* Destroy the meat of the selection */
					if (selPos != -1) {
						int selStartPos, selEndPos, offset = selLineOffset;
						if (offset < 0) {
							/* Other end is above centerstr */
							offset = -offset;
							selStartPos = selPos;
							selEndPos = pos;
							/* Move back to top of selection */
							for (i = 0; i < offset; i++) {
								if (sv->cur->prev != NULL)
									sv->cur = sv->cur->prev;
							}
							/* Change centerstr to reflect the top of the selection */
							centerstr = sv->cur;
						} else {
							selStartPos = pos;
							selEndPos = selPos;
						}
						if (offset == 0) {
							/* Only one line to work with */
							int deltaPos;

							/* Reverse selStartPos and selEndPos if start is bigger */
							if (selStartPos > selEndPos) {
								int swapPos = selStartPos;
								selStartPos = selEndPos;
								selEndPos = swapPos;
							}
							
							/* Remove everything between selStartPos and selEndPos */
							deltaPos = selEndPos - selStartPos;
							for (i = selEndPos; i < strlen(centerstr->s); i++) {
								centerstr->s[i - deltaPos] = centerstr->s[i];
							}
							centerstr->s[i - deltaPos] = '\0';
							
							/* Move the cursor to the starting position of the cut */
							pos = selStartPos;
						} else {
							/* Multiple lines were involved */

							/* Remove lines following the first line of the block */
							sv->cur = centerstr->next;
							for (i = 0; i + 1 < offset; i++) {
								deletestring(sv);
							}

							/* Remove the string at the end of the cut */
							sv->cur = centerstr->next;
							tmpstr = removestring(sv);
							/* Remove first selEndPos chars from end string */
							for (i = 0; i < (strlen(tmpstr) - selEndPos); i++)
								tmpstr[i] = tmpstr[i+selEndPos];
							tmpstr[i] = 0;

							/* Truncate the string at the start of the cut */
							sv->cur = centerstr;
							sv->cur->s[selStartPos] = '\0';
							/* Wordwrap the end string onto this one */
							/* The -1 tells wordwrap to track the cursor position at
							 * the beginning of tmpstr. Negative tracking values should
							 * be used only by wordwrap for internal purposes, but
							 * necessity warrents in this case.     vv    */
							pos = wordwrap(sv, tmpstr, selStartPos, -1, wrapwidth, editwidth);
							centerstr = sv->cur;  /* Follow cursor */
							/* tmpstr is our responsability */
							free(tmpstr);
						}
						updateflags = U_EDITAREA;
					}
					break;

				case DKEY_CTRL_V:     /* ctrl-v: paste register */
					sv->cur = centerstr;
					pos = regput('\"', sv, pos, wrapwidth, editwidth);
					centerstr = sv->cur;
					updateflags = U_EDITAREA;
					break;

				case DKEY_TAB: /* Tab */
					/* determine tab amount */
					j = 4 - (pos % 4);
					if (strlen(centerstr->s) + j < (wrapwidth?wrapwidth:editwidth)) {
						/* insert if there is room */
						for (i = strlen(centerstr->s) + j; i > pos; i--)
							centerstr->s[i] = centerstr->s[i-j];
						for (i = 0; i < j; i++)
							centerstr->s[pos++] = ' ';
						updateflags = U_CENTER;
					}
					else {
						/* no room; wordwrap */
						for (i = 0; i < j; i++)
							strbuf[i] = ' ';
						strbuf[i] = 0;
						sv->cur = centerstr;
						pos = wordwrap(sv, strbuf, pos, pos, wrapwidth, editwidth);
						centerstr = sv->cur;
						updateflags = U_EDITAREA;
					}
					break;

				case DKEY_ENTER:
					/* Enter */
					tmpstr = (char*) malloc(editwidth + 2);
					for (i = pos, j = 0; i < strlen(centerstr->s); i++, j++)
						tmpstr[j] = centerstr->s[i];
					centerstr->s[pos] = 0;

					tmpstr[j] = 0;
					sv->cur = centerstr;
					insertstring(sv, tmpstr);
					centerstr = centerstr->next;
					pos = 0;
					updateflags = U_EDITAREA;
					break;

				case DKEY_BACKSPACE:
					/* Backspace */
					if (pos > 0) {
						for (i = pos - 1; i < strlen(centerstr->s); i++)
							centerstr->s[i] = centerstr->s[i+1];
						pos--;
						updateflags = U_CENTER;
					}
					else if (centerstr->prev != NULL) {
						if (strlen(centerstr->s) == 0) {
							/* remove current line & move up & to eol */
							sv->cur = centerstr;
							centerstr = centerstr->prev;
							pos = strlen(centerstr->s);
							deletestring(sv);
							updateflags = U_TOP | U_CENTER;
						}
						else if (strlen(centerstr->prev->s) == 0) {
							/* remove previous line */
							sv->cur = centerstr->prev;
							deletestring(sv);
							/* update center too, in case @ line has moved to top now */
							updateflags = U_TOP | U_CENTER;
						}
						else if (strlen(centerstr->prev->s) + 1 < wrapwidth) {
							/* merge lines; wordwrap */
							i = strlen(centerstr->prev->s);
							if (centerstr->prev->s[i-1] != ' ' && centerstr->s[0] != ' ') {
								/* add a space at the end */
								centerstr->prev->s[i]     = ' ';
								centerstr->prev->s[i + 1] = 0;
							}
							sv->cur = centerstr->prev;
							tmpstr = removestring(sv);
							sv->cur = centerstr;
							pos = wordwrap(sv, tmpstr, 0, 0, wrapwidth, editwidth);
							centerstr = sv->cur;
							free(tmpstr);
							updateflags = U_EDITAREA;
						}
					}
					break;

				case DKEY_CTRL_Y: /* ctrl-y: delete line */
					pos = 0;
					sv->cur = centerstr;
					if (centerstr->next != NULL) {
						centerstr = centerstr->next;
						deletestring(sv);
						updateflags = U_CENTER | U_BOTTOM;
					}
					else if (centerstr->prev != NULL) {
						centerstr = centerstr->prev;
						deletestring(sv);
						updateflags = U_TOP | U_CENTER;
					}
					else {
						centerstr->s[0] = 0;
						updateflags = U_CENTER;
					}
					break;

				case DKEY_ESC: /* escape when done */
					done = EDITBOX_OK;
					break;

				case DKEY_CTRL_A: /* ctrl-a: insert ascii char/decimal-value */
					strcpy(strbuf, centerstr->s);
					updateflags = U_EDITAREA;

					if (str_equ(strbuf, "#char", STREQU_UNCASE | STREQU_RFRONT)) {
						/* append dec value for ascii char */

						sscanf(strbuf + 5, "%d", &selChar);
						key = charselect(d, selChar);
						if (key == -1)
							break;
						selChar = key;
						centerstr->s[5] = ' ';
						centerstr->s[6] = 0;

						/* change c to a string */
						sprintf(strbuf, "%d", selChar);
						strcat(centerstr->s, strbuf);
						pos = strlen(centerstr->s);
						updateflags = U_EDITAREA;
						break;
					}
					else {
						/* ctrl-a: insert ascii char */
						key = charselect(d, selChar);
						if (key == -1)
							break;
						else
							selChar = key;
					}
					/* no break; we just changed the key & want to insert it */

				default:
					key = key & 0xFF;  /* Clear all but the first 8 bits */
					/* Normal/weird char for insert/replace */
					if (insertflag) {
						/* insert */
						if (strlen(centerstr->s) < (wrapwidth?wrapwidth:editwidth)) {
							/* insert if there is room */
							for (i = strlen(centerstr->s) + 1; i > pos; i--)
								centerstr->s[i] = centerstr->s[i-1];
							centerstr->s[pos++] = key;
							updateflags |= U_CENTER;
						}
						else if (wrapwidth) {
							/* no room; wordwrap */
							strbuf[0] = key;
							strbuf[1] = 0;
							sv->cur = centerstr;
							pos = wordwrap(sv, strbuf, pos, pos, wrapwidth, editwidth);
							centerstr = sv->cur;
							updateflags = U_EDITAREA;
						}
					}
					else {
						/* easy replace */
						if (centerstr->s[pos] == 0) {
							if (strlen(centerstr->s) < (wrapwidth?wrapwidth:editwidth)) {
								centerstr->s[pos+1] = 0;
								centerstr->s[pos++] = key;
								updateflags |= U_CENTER;
							}
							else if (wrapwidth) {
								/* no room; wordwrap */
								strbuf[0] = key;
								strbuf[1] = 0;
								sv->cur = centerstr;
								pos = wordwrap(sv, strbuf, pos, pos, wrapwidth, editwidth);
								centerstr = sv->cur;
								updateflags = U_EDITAREA;
							}
						}
						else {
							centerstr->s[pos++] = key;
							updateflags |= U_CENTER;
						}
					} /* esle replace */
					break;
			}
		}		/* esle in editmode */

		/* if the shift key is not still held down and we are selecting,
		 * then stop select mode */
		/* also stop if true ASCII key was pressed and selection is active */
		if ((!selectFlag && selPos != -1) || (key < 0x7F && selPos != -1)) {
			selPos = -1;
			selLineOffset = 0;
			updateflags |= U_EDITAREA;
		}
	}			/* elihw */

	sv->cur = centerstr;

	return done;
}

void testMusic(stringvector* sv, int slur, int editwidth, int flags, displaymethod* d)
{
	int done;
#ifdef SDL
	SDL_AudioSpec spec;

	/* IF opening the audio device fails, return now before we crash something. */
	if (OpenSynth(&spec))
		return;
#endif

	done = 0;

	/* Loop through the stringvector looking for #play statements */
	while (sv->cur != NULL && !done) {
		char* tune = strstr(sv->cur->s, "#");
		if (tune != NULL && str_equ(tune, "#play ", STREQU_UNCASE | STREQU_RFRONT)) {
			/* Current note and settings */
			musicalNote note = zzmGetDefaultNote();
			musicSettings settings = zzmGetDefaultSettings();

#ifdef DOS
			int xoff = tune - sv->cur->s;
#endif
			tune += 6;  /* Advance to notes! */

			/* Change the slur setting */
			note.slur = slur;

			/* Update everything because we have likely shifted to a new line. */
			updateditbox(d, sv, U_EDITAREA, editwidth, flags, "", 1);

			while (note.src_pos < strlen(tune) && !done) {
#ifdef DOS
				int oldpos = note.src_pos;
				char* strpart;
#endif

				if (d->getkey() != DKEY_NONE)
					done = 1;

				note = zzmGetNote(tune, note);

#ifdef DOS
				/* In DOS, display everything as we go along */
				/* Display the whole line */
				updateditbox(d, sv, U_CENTER, editwidth, flags, "", 1);

				/* Display the part of the string which will be played now */
				strpart = str_duplen(tune + oldpos, note.src_pos - oldpos);
				d->print(oldpos + 15 + xoff, 13, ZOC_MPLAY_COLOUR, strpart);
				free(strpart);
				d->cursorgo(oldpos + 15 + xoff, 13);

				pcSpeakerPlayNote(note, settings);
#elif defined SDL
				SynthPlayNote(spec, note, settings);
#endif
			}
		}
		sv->cur = sv->cur->next;
	}

#ifdef SDL
	/* TODO: instead of just sitting here, display the progress of playback */
	/* Wait until the music is done or the user presses a key */
	while (!IsSynthBufferEmpty() && d->getkey() == DKEY_NONE)
		;

	CloseSynth();
#elif defined DOS
	pcSpeakerFinish();
#endif
}

/***************************************************************************/
/**** Syntax highlighting functions ****************************************/
/***************************************************************************/

/* Draw highlighted syntax using ZOOPDraw */
#include "zoopdraw.h"

void displayzoc(int x, int y, char *s, int format, int firstline, displaymethod * d)
{
	ZZTOOPparser * parser = zztoopCreateParser(s);
	ZZTOOPdrawer drawer;

	zztoopInitDrawer(&drawer);
	drawer.display = d;

	if (format) {
		/* Non-strict help */
		parser->flags = ZOOPFLAG_HELP;
		drawer.helpformatting = 1;
	}

	if (firstline)
		parser->flags |= ZOOPFLAG_FIRSTLINE;

	drawer.x = x; drawer.y = y;
	drawer.length = 42;  /* TODO: replace 42 w/ constant or something */

	zztoopDraw(drawer, zztoopParseLine(parser));

	zztoopDeleteParser(parser);
}


