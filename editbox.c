/* editbox.c  -- text editor/viewer in kevedit
 * $Id: editbox.c,v 1.35 2002/02/17 22:41:51 bitman Exp $
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "editbox.h"

#include "screen.h"

#include "svector.h"
#include "zzm.h"
#include "colours.h"

#include "register.h"
#include "help.h"

#include "scroll.h"
#include "panel_ed.h"

#include "display.h"

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

/* ZZT Object Code colours */

#define ZOC_DEFAULT_COLOUR     WHITE_F
#define ZOC_OPERATOR_COLOUR    YELLOW_F | BRIGHT_F
#define ZOC_HEADING_COLOUR     WHITE_F | BRIGHT_F
#define ZOC_HYPER_COLOUR       WHITE_F | BRIGHT_F
#define ZOC_HYPARROW_COLOUR    RED_F

#define ZOC_OBJNAME_COLOUR     BLUE_F | BRIGHT_F
#define ZOC_COMMENT_COLOUR     CYAN_F | BRIGHT_F
#define ZOC_TEXT_COLOUR        GREEN_F | BRIGHT_F
#define ZOC_HIGHLIGHT_COLOUR   BLACK_F | WHITE_B

#define ZOC_STDCOMMAND_COLOUR  GREEN_F
#define ZOC_STDITEM_COLOUR     WHITE_F | BRIGHT_F
#define ZOC_STDKIND_COLOUR     CYAN_F | BRIGHT_F
#define ZOC_STDDIR_COLOUR      WHITE_F | BRIGHT_F
#define ZOC_STDMESSAGE_COLOUR  MAGENTA_F | BRIGHT_F
#define ZOC_STDLABEL_COLOUR    RED_F | BRIGHT_F
#define ZOC_STDFLAG_COLOUR     YELLOW_F | BRIGHT_F

#define ZOC_MESSAGE_COLOUR     MAGENTA_F
#define ZOC_LABEL_COLOUR       RED_F
#define ZOC_FLAG_COLOUR        YELLOW_F

#define ZOC_MTIME_COLOUR       CYAN_F
#define ZOC_MTIMEMOD_COLOUR    CYAN_F | BRIGHT_F
#define ZOC_MOCTAVE_COLOUR     YELLOW_F | BRIGHT_F
#define ZOC_MPITCH_COLOUR      BROWN_F
#define ZOC_MREST_COLOUR       GREEN_F
#define ZOC_MNOTE_COLOUR       GREEN_F | BRIGHT_F
#define ZOC_MDRUM_COLOUR       MAGENTA_F | BRIGHT_F
#define ZOC_MPLAY_COLOUR       WHITE_F | BRIGHT_F

/* zzt components for special highlighting */

#define ZZTCOMMANDCOUNT 27
const char zztcommands[ZZTCOMMANDCOUNT][12] =
{
	"become",  "bind",    "change", "char",
	"clear",   "cycle",   "die",    "end",
	"endgame", "give",    "go",     "idle",
	"if",      "lock",    "play",   "put",
	"restart", "restore", "send",   "set",
	"shoot",   "take",    "throwstar",
	"try",     "unlock",  "walk",   "zap"
};


/* Command syntax:
 * Each type of argument to a command is given a letter and stored
 * in a string in zztcommandtax[], which corresponds to that same
 * numbered element in zztcommands */

#define CTAX_KIND 'k'
#define CTAX_OBJECTNAME 'o'
#define CTAX_NUMBER 'n'
#define CTAX_FLAG 'f'
#define CTAX_ITEM 'i'
#define CTAX_DIRECTION 'd'
#define CTAX_THENMESSAGE 't'
#define CTAX_SOUND 's'
#define CTAX_MESSAGE 'm'

const char zztcommandtax[ZZTCOMMANDCOUNT][30] =
{
	"k",  "o",   "kk", "n",
	"f",  "n",   "",   "",
	"",   "in",  "d",  "",
	"ft", "",    "s",  "dk",
	"",   "m",   "m",  "f",
	"d",  "int", "d",
	"dt", "",    "d",  "m"
};

#define ZZTMESSAGECOUNT 5
const char zztmessages[ZZTMESSAGECOUNT][10] =
{
	"touch", "shot", "bombed", "thud", "energize"
};

#define ZZTFLAGCOUNT 6
const char zztflags[ZZTFLAGCOUNT][12] =
{
	"alligned", "contact", "blocked", "energized", "exists", "any"
};

#define ZZTITEMCOUNT 5
const char zztitems[ZZTITEMCOUNT][8] =
{
	"ammo", "gems", "torches", "health", "score"
};

#define ZZTKINDCOUNT 41
const char zztkinds[ZZTKINDCOUNT][12] =
{
	"empty", "player", "ammo", "torch",
	"gem", "key", "door", "scroll",
	"passage", "duplicator", "bomb", "energizer",
	"star", "clockwise", "counter", "bullet",
	"water", "forest", "solid", "normal",
	"breakable", "boulder", "sliderns", "sliderew",
	"fake", "invisible", "blinkwall", "transporter",
	"line", "ricochet", "bear", "ruffian",
	"object", "slime", "shark", "spinninggun",
	"pusher", "lion", "tiger", "head",
	"segment"
};

#define ZZTCOLOURCOUNT 7
const char zztcolours[ZZTCOLOURCOUNT][8] =
{
	"blue", "green", "red", "cyan", "purple", "yellow", "white"
};

#define ZZTDIRCOUNT 15
const char zztdirs[ZZTDIRCOUNT][6] =
{
	"north", "south", "east", "west", "idle",
	"seek", "flow", "rnd", "rndns", "rndne",
	"n", "s", "e", "w", "i"
};

#define ZZTDIRMODCOUNT 4
const char zztdirmods[ZZTDIRMODCOUNT][5] =
{
	"cw", "ccw", "rndp", "opp"
};


/* token testing functions */
int iszztcommand(char *token);
int iszztmessage(char *token);
int iszztflag(char *token);
int iszztitem(char *token);
int iszztkind(char *token);
int iszztdir(char *token);
int iszztcolour(char *token);

void testMusic(stringvector* sv, int slur, int editwidth, int flags, displaymethod* d);

/* how to display a line of text in updateditbox() */
#define displayline(x, y, s, edit, flags, firstline, d) ((flags & EDITBOX_ZOCMODE) ? displayzoc((x), (y), (s), !(edit), (firstline), (d)) : d->print((x), (y), ZOC_TEXT_COLOUR, (s)))


/***** draweditpanel() ***********/
void draweditpanel(int insertflag, int wrapwidth, int zocmode, displaymethod * d)
{
	char buf[10] = "";
	drawsidepanel(d, PANEL_EDIT);
	
	d->print(76, 4,  YELLOW_F | BRIGHT_F | BLUE_B, (insertflag ? "on" : "off"));
	d->print(76, 8, YELLOW_F | BRIGHT_F | BLUE_B, (zocmode ? "on" : "off"));

	sprintf(buf, "%d", wrapwidth);

	if (wrapwidth)
		d->print(76, 6, YELLOW_F | BRIGHT_F | BLUE_B, buf);
	else
		d->print(72, 6, YELLOW_F | BRIGHT_F | BLUE_B, "off");
}

void updateditbox(stringvector* sv, int updateflags, int editwidth, int flags,
									char* title, displaymethod* d)
{
	/* update title if needed */
	if (updateflags & U_TITLE) {
		drawscrollbox(1, 17, d);
		d->print(30 - (strlen(title) / 2), 4, 0x0a, title);
	}

	/* clear the scrollbox */
	if (updateflags & U_TOP)
		drawscrollbox(3, 9, d);
	if (updateflags & U_CENTER)
		drawscrollbox(10, 8, d);
	if (updateflags & U_BOTTOM)
		drawscrollbox(11, 1, d);

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
			d->print(9, i + 13, 0x07, SLEADER);
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
			d->print(9, i + 13, 0x07, SLEADER);
	}
}

#if 0
/***** editmoredata() *********/

void editmoredata(param * p, displaymethod * d)
{
	stringvector sv;
	param newparam;

	sv = moredatatosvector(p, EDITBOX_ZZTWIDTH);

	/* Now that the node is full, we can edit it. */
	sv.cur = sv.first;	/* This is redundant, but hey. */
	editbox("Object Editor", &sv, EDITBOX_ZZTWIDTH, 1, d);

	/* Okay, let's put the vector back in moredata */
	newparam = svectortomoredata(sv);

	deletestringvector(&sv);
	if (p->moredata != NULL)
		free(p->moredata);

	p->length = newparam.length;
	p->moredata = newparam.moredata;
}
#endif


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

	drawscrollbox(0, 0, d);
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
		updateditbox(sv, updateflags, editwidth, flags, title, d);
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
						d->print(9, 13 + i, ZOC_HIGHLIGHT_COLOUR, loopstr->s);
				} else {
					for (i = -1, loopstr = centerstr->prev; i > selLineOffset && i > -8 && loopstr != NULL; i--, loopstr = loopstr->prev)
						d->print(9, 13 + i, ZOC_HIGHLIGHT_COLOUR, loopstr->s);
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
						d->putch(9 + j, 13 + i, loopstr->s[j], ZOC_HIGHLIGHT_COLOUR);
				}
			}
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

					regyank('\"', selStart, selEnd, selStartPos, selEndPos);
				}
				break;

			case DKEY_ESC:
				if (editwidth > EDITBOX_NOEDIT)
					done = EDITBOX_OK;
				else
					done = EDITBOX_CANCEL;
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
					else if (strlen(centerstr->s) == 0) {
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

						if (!iszztcommand(tmpstr)) {
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

						if (filetypelist.cur != NULL)
							filename =
								filedialog(".", filetypelist.cur->s + 2,
													 (key == DKEY_ALT_O ?
													   "Open ZZT Object Code (ZOC) File" :
													   "Insert ZZT Object Code (ZOC) File"),
													 FTYPE_ALL, d);

						if (filename != NULL && strlen(filename) != 0) {
							stringvector newsvector;
							newsvector = filetosvector(filename, wrapwidth, editwidth);
							if (newsvector.first != NULL) {
								if (key == DKEY_ALT_O) {
									strcpy(savefilename, filename);
									if (str_equ(filetypelist.cur->s, "*.zoc", 0))
										flags &= EDITBOX_ZOCMODE;  /* Set ZOCMODE */
									else
										flags |= ~EDITBOX_ZOCMODE; /* Clear ZOCMODE */
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
					updateflags = U_EDITAREA | U_TITLE;
					break;

				case DKEY_ALT_S: /* alt-s: save to file */
					{
						char* filename;
						filename = filenamedialog(savefilename, "", "Save Object Code As",
																			1, d);
						if (filename != NULL) {
							/* Save to the file */
							svectortofile(sv, filename);
							/* Remember the file name */
							strncpy(savefilename, filename, SAVEFILENAME_LEN - 1);
							savefilename[SAVEFILENAME_LEN] = '\x0';

							free(filename);
						}
					}
					updateflags = U_EDITAREA | U_PANEL;
					break;

				case DKEY_ALT_M: /* alt-m: load .zzm music */
					{
						char* filename;
						filename = filedialog(".", "zzm", "Choose ZZT Music (ZZM) File",
																	FTYPE_ALL, d);
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
					updateflags = U_EDITAREA | U_TITLE;
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
	/* Loop through the stringvector looking for #play statements */
	while (sv->cur != NULL && !d->kbhit()) {
		if (str_equ(sv->cur->s, "#play ", STREQU_UNCASE | STREQU_RFRONT)) {
			char* tune = sv->cur->s + 6;

			/* Get ready to parse that play line */
			zzmplaystate s;
			resetzzmplaystate(&s);
			s.slur = slur;

			/* Update everything because we have likely shifted to a new line. */
			updateditbox(sv, U_EDITAREA, editwidth, flags, "", d);

			while (s.pos < strlen(tune) && !d->kbhit()) {
				int oldpos = s.pos;
				char* strpart;

				zzmnote note = zzmgetnote(tune, &s);

				/* Display the whole line */
				updateditbox(sv, U_CENTER, editwidth, flags, "", d);

				/* Display the part of the string which will be played now */
				strpart = str_duplen(tune + oldpos, s.pos - oldpos);
				d->print(oldpos + 15, 13, ZOC_MPLAY_COLOUR, strpart);
				free(strpart);
				d->cursorgo(oldpos + 15, 13);

				zzmPCspeakerPlaynote(note);
			}

			zzmPCspeakerFinish();
		}
		sv->cur = sv->cur->next;
	}

	if (d->kbhit())
		d->getch();
}

/***************************************************************************/
/**** Syntax highlighting functions ****************************************/
/***************************************************************************/


void displayzoc(int x, int y, char *s, int format, int firstline, displaymethod * d)
{
	int i = 0;			/* position in s */
	int j = 0;			/* position in token */
	char token[80] = "";	/* token buffer */

	/* find out what we're dealing with based on the first char */
	switch (s[0]) {
		case '#':
			/* command */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
			for (i = 1; s[i] != ' ' && s[i] != 0; i++)
				token[i - 1] = s[i];
			token[i - 1] = 0;

			if (iszztcommand(token)) {
				d->print(x + 1, y, ZOC_STDCOMMAND_COLOUR, token);

				displaycommand(x + i, y, token, s + i, d);
			} else {
				/* Display as #send call */
				if (strchr(token, ':') != NULL) {
					for (j = 1; s[j] != ':' && s[j] != 0; j++)
						d->putch(x + j, y, s[j], ZOC_OBJNAME_COLOUR);
					d->putch(x + j, y, ':', ZOC_OPERATOR_COLOUR);
					if (j < strlen(s)) j++;
					d->print(x + j, y, (iszztmessage(s + j) ? ZOC_STDMESSAGE_COLOUR : ZOC_MESSAGE_COLOUR), s + j);
				} else
					d->print(x + 1, y, (iszztmessage(token) ? ZOC_STDMESSAGE_COLOUR : ZOC_MESSAGE_COLOUR), token);
				d->print(x + i, y, ZOC_DEFAULT_COLOUR, s + i);
			}

			break;

		case ':':
			/* message */
			if (firstline) {
				/* This requires some explaination: When a message label occurs at the
				 * beginning of an object's code, it cannot be sent to. So, we shall
				 * display it in the default colour to make this apparent. */
				d->print(x, y, ZOC_DEFAULT_COLOUR, s);
				break;
			}
			if (format) {
				s = strstr(s, ";");
				if (s != NULL)
					d->print(x, y, ZOC_HEADING_COLOUR, s + 1);
			} else {
				d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
				for (i = 1; s[i] != '\'' && s[i] != 0; i++)
					token[i - 1] = s[i];
				token[i - 1] = 0;
				
				d->print(x + 1, y, (iszztmessage(token) ? ZOC_STDLABEL_COLOUR : ZOC_LABEL_COLOUR), token);
				
				if (s[i] == '\'')
					d->print(x + i, y, ZOC_COMMENT_COLOUR, s + i);
			}
			break;

		case '?':
		case '/':
			/* movement */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);

			for (i = 1; s[i] != 0 && s[i] != '/' && s[i] != '?' && s[i] != '\'' && s[i] != ' ' && s[i] != '#'; i++)
				token[i-1] = s[i];
			token[i-1] = 0;

			while (!iszztdir(token) && s[i] != 0 && s[i] != '/' && s[i] != '?' && s[i] != '\'' && s[i] != '#') {
				while (s[i] == ' ') { token[i - 1] = ' '; i++; }
				for (; s[i] != 0 && s[i] != '/' && s[i] != '?' && s[i] != '\'' && s[i] != ' ' && s[i] != '#'; i++)
					token[i-1] = s[i];
				token[i-1] = 0;
			}

			if (iszztdir(token)) {
				/* token is a proper direction */
				d->print(x + 1, y, ZOC_STDDIR_COLOUR, token);
			} else
				d->print(x + 1, y, ZOC_DEFAULT_COLOUR, token);

			if (s[i] == '/' || s[i] == '?' || s[i] == '\'' || s[i] == ' ' || s[i] == '#')
				displayzoc(x + i, y, s + i, format, 0, d);

			break;

		case '!':
			/* hypermessage */
			if (format) {
				/* white and -> indented */
				d->putch(x + 2, y, 0x10, ZOC_HYPARROW_COLOUR);
				s = strstr(s, ";");
				if (s != NULL)
					d->print(x + 5, y, ZOC_HYPER_COLOUR, s + 1);
			} else {
				d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
				for (i = 1; s[i] != ';' && s[i] != 0; i++)
					d->putch(x + i, y, s[i], ZOC_MESSAGE_COLOUR);

				if (s[i] == 0) break;

				d->putch(x + i, y, s[i], ZOC_OPERATOR_COLOUR);
				for (i++; s[i] != 0; i++)
					d->putch(x + i, y, s[i], ZOC_HYPER_COLOUR);
			}

			break;

		case '\'':
			/* comment */
			d->print(x, y, ZOC_COMMENT_COLOUR, s);
			break;

		case '$':
			/* heading */
			if (format) {
				/* white and centered */
				s++;
				d->print(x + 20 - (strlen(s)/2), y, ZOC_HEADING_COLOUR, s);
			} else {
				d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
				d->print(x + 1, y, ZOC_HEADING_COLOUR, s + 1);
			}
			break;

		case '@':
			/* objectname */
			if (firstline) {
				/* it's only an objectname on the first line */
				d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
				d->print(x + 1, y, ZOC_OBJNAME_COLOUR, s + 1);
			}
			else
				d->print(x, y, ZOC_TEXT_COLOUR, s);
			break;
		
		case ' ':
			/* indented comment? */
			for (i = 1; s[i] == ' '; i++)
				;
			d->print(x, y, (s[i]=='\'' ? ZOC_COMMENT_COLOUR : ZOC_TEXT_COLOUR), s);
		default:
			/* normal text */
			d->print(x, y, ZOC_TEXT_COLOUR, s);
			break;
	}
}



/* display a zzt #command */
void displaycommand(int x, int y, char *command, char *args, displaymethod * d)
{
	int t;			/* Index in zztcommands syntax list */
	int i;			/* Index in zztcommandtax[t] */
	int j;			/* Index in args */
	int k;			/* Length of buffer */
	int l;			/* General counter */
	char ctax;	/* Current command arg syntax */
	char token[41] = "";

	for (t = 0; t < ZZTCOMMANDCOUNT; t++)
		if (str_equ(zztcommands[t], command, STREQU_UNCASE))
			break;

	if (t == ZZTCOMMANDCOUNT) {
		d->print(x, y, ZOC_DEFAULT_COLOUR, args);
		return;
	}

	j = 0;
	for (i = 0; i < strlen(zztcommandtax[t]); i++) {
		/* Advance to next token */
		k = tokenadvance(token, args, &j);

		ctax = zztcommandtax[t][i];

		/* Determine current token type (stage 1) */
		switch (ctax) {
			case CTAX_OBJECTNAME:
				d->print(x + j - k, y, ZOC_OBJNAME_COLOUR, token);
				break;

			case CTAX_NUMBER:
				d->print(x + j - k, y, ZOC_TEXT_COLOUR, token);
				break;

			case CTAX_FLAG:
				if (str_equ(token, "not", STREQU_UNCASE)) {
					d->print(x + j - k, y, ZOC_STDFLAG_COLOUR, token);
					/* Advance to next token */
					k = tokenadvance(token, args, &j);
				}
				/* If it's a built-in, display it as such */
				if (iszztflag(token))
					d->print(x + j - k, y, ZOC_STDFLAG_COLOUR, token);
				else
					d->print(x + j - k, y, ZOC_FLAG_COLOUR, token);
				/* Check the blocked flag for directions */
				if (str_equ(token, "blocked", STREQU_UNCASE)) {
					k = tokenadvance(token, args, &j);
					ctax = CTAX_DIRECTION;
				}
				if (str_equ(token, "any", STREQU_UNCASE)) {
					k = tokenadvance(token, args, &j);
					ctax = CTAX_KIND;
				}
				break;

			case CTAX_ITEM:
				if (iszztitem(token)) {
					d->print(x + j - k, y, ZOC_STDITEM_COLOUR, token);
				} else
					d->print(x + j - k, y, ZOC_DEFAULT_COLOUR, token);
				break;

			case CTAX_THENMESSAGE:
				if (str_equ(token, "then", STREQU_UNCASE)) {
					d->print(x + j - k, y, ZOC_STDCOMMAND_COLOUR, token);
					k = tokenadvance(token, args, &j);
				}
				if (token[0] == '#' || token[0] == '/' || token[0] == '?') {
					/* remainder of args is a #command or movement */
					displayzoc(x + j - k, y, args + j - k, 1, 0, d);
					j = strlen(args);  /* Avoid overwriting */
				} else {
					if (iszztcommand(token)) {
						d->print(x + j - k, y, ZOC_STDCOMMAND_COLOUR, token);
						displaycommand(x + j, y, token, args + j, d);
						j = strlen(args);  /* Avoid overwriting */
					} else {
						/* Thenmessage was not a command, so display it as a normal
						 * message */
						ctax = CTAX_MESSAGE;
					}
				}
				break;

			case CTAX_SOUND:
				displayzzm(x + j - k, y, token, d);
				break;
		}

		/* Determine current token type (stage 2) */
		switch (ctax) {
			case CTAX_KIND:
				if (iszztkind(token)) {
					d->print(x + j - k, y, ZOC_STDKIND_COLOUR, token);
				} else {
					k = tokengrow(token, args, &j);
					if (iszztkind(token)) {
						d->print(x + j - k, y, ZOC_STDKIND_COLOUR, token);
					} else
						d->print(x + j - k, y, ZOC_DEFAULT_COLOUR, token);
				}
				break;

			case CTAX_DIRECTION:
				while (!iszztdir(token) && j < strlen(args))
					k = tokengrow(token, args, &j);
				if (iszztdir(token))
					d->print(x + j - k, y, ZOC_STDDIR_COLOUR, token);
				else
					d->print(x + j - k, y, ZOC_DEFAULT_COLOUR, token);
				break;

			case CTAX_MESSAGE:
				if (strchr(token, ':') != NULL) {
					/* We have an objectname included */
					for (l = 0; token[l] != ':'; l++)
						d->putch(x + j - k + l, y, token[l], ZOC_OBJNAME_COLOUR);
					d->putch(x + j - k + l, y, ':', ZOC_OPERATOR_COLOUR);
					l++;
					d->print(x + j - k + l, y, (iszztmessage(token + l) ? ZOC_STDMESSAGE_COLOUR : ZOC_MESSAGE_COLOUR), token + l);
				} else
					d->print(x + j - k, y, (iszztmessage(token) ? ZOC_STDMESSAGE_COLOUR : ZOC_MESSAGE_COLOUR), token);
				break;
		}
	}

	/* Finish anything we haven't dealt with */
	for (; j < strlen(args); j++)
		d->putch(x + j, y, args[j], ZOC_DEFAULT_COLOUR);
}

/* displayzzm() - displays zzm music highlighted */
void displayzzm(int x, int y, char *music, displaymethod * d)
{
	int i;

	for (i = 0; i < strlen(music); i++) {
		int colour = ZOC_DEFAULT_COLOUR;

		switch (toupper(music[i])) {
			/* Time determiners */
			case 'T':
			case 'S':
			case 'I':
			case 'Q':
			case 'H':
			case 'W': colour = ZOC_MTIME_COLOUR; break;
			/* Time modifiers */
			case '3':
			case '.': colour = ZOC_MTIMEMOD_COLOUR; break;
			/* Octave switch */
			case '+':
			case '-': colour = ZOC_MOCTAVE_COLOUR; break;
			/* Sharp/flat: pitch modifiers */
			case '!':
			case '#': colour = ZOC_MPITCH_COLOUR; break;
			/* Rest */
			case 'X': colour = ZOC_MREST_COLOUR; break;
			default:
				if (toupper(music[i]) >= 'A' && toupper(music[i]) <= 'G')
					colour = ZOC_MNOTE_COLOUR;
				if (toupper(music[i]) >= '0' && toupper(music[i]) <= '9')
					colour = ZOC_MDRUM_COLOUR;
		}
		d->putch(x + i, y, music[i], colour);
	}
}


/******** token testers **************/
int iszztcommand(char *token)
{
	int i = 0;
	char buffer[41] = "";

	memcpy(buffer, token, 40);
	buffer[40] = 0;

	for (i = 0; i < ZZTCOMMANDCOUNT; i++)
		if (str_equ(buffer, zztcommands[i], STREQU_UNCASE))
			return 1;

	return 0;
}

int iszztmessage(char *token)
{
	int i = 0;
	char buffer[41] = "";

	memcpy(buffer, token, 40);
	buffer[40] = 0;

	for (i = 0; i < ZZTMESSAGECOUNT; i++)
		if (str_equ(buffer, zztmessages[i], STREQU_UNCASE))
			return 1;

	return 0;
}

int iszztflag(char *token)
{
	int i = 0;
	char buffer[41] = "";

	memcpy(buffer, token, 40);
	buffer[40] = 0;

	for (i = 0; i < ZZTFLAGCOUNT; i++)
		if (str_equ(buffer, zztflags[i], STREQU_UNCASE))
			return 1;

	return 0;
}

int iszztitem(char *token)
{
	int i = 0;
	char buffer[41] = "";

	memcpy(buffer, token, 40);
	buffer[40] = 0;

	for (i = 0; i < ZZTITEMCOUNT; i++)
		if (str_equ(buffer, zztitems[i], STREQU_UNCASE))
			return 1;

	return 0;
}

int iszztkind(char *token)
{
	int i = 0;
	char buffer[41] = "";

	memcpy(buffer, token, 40);
	buffer[40] = 0;

	for (i = 0; i < ZZTCOLOURCOUNT; i++)
		if (str_equ(buffer, zztcolours[i], STREQU_UNCASE | STREQU_FRONT)) {
			/* Advance token to nearest space */
			while (token[0] != ' ' && token[0] != 0) token++;
			/* Advance token to nearest nonspace */
			while (token[0] == ' ') token++; 

			/* now see if next word is a valid token */
			return iszztkind(token);
		}

	for (i = 0; i < ZZTKINDCOUNT; i++)
		if (str_equ(buffer, zztkinds[i], STREQU_UNCASE))
			return 1;

	return 0;
}

int iszztdir(char *token)
{
	int i = 0;
	char buffer[41] = "";

	memcpy(buffer, token, 40);
	buffer[40] = 0;

	for (i = 0; i < ZZTDIRMODCOUNT; i++)
		if (str_equ(buffer, zztdirmods[i], STREQU_UNCASE | STREQU_RFRONT)) {
			/* Advance token to nearest space */
			while (token[0] != ' ' && token[0] != 0) token++; 
			/* Advance token to nearest nonspace */
			while (token[0] == ' ') token++; 

			/* now see if next word is a valid token */
			return iszztdir(token);
		}

	for (i = 0; i < ZZTDIRCOUNT; i++)
		if (str_equ(buffer, zztdirs[i], STREQU_UNCASE))
			return 1;

	return 0;
}


