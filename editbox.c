/* editbox.c  -- text editor/viewer in kevedit
 * $Id: editbox.c,v 1.23 2001/10/22 02:48:22 bitman Exp $
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
#include "scroll.h"
#include "colours.h"
#include "svector.h"
#include "panel_ed.h"
#include "zzm.h"
#include "register.h"
#include "screen.h"
#include "help.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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
	"d",  "inm", "d",
	"dm", "",    "d",  "m"
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

#define ZZTDIRCOUNT 14
const char zztdirs[ZZTDIRCOUNT][6] =
{
	"north", "south", "east", "west", "idle",
	"seek", "flow", "rndns", "rndne",
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


/***** draweditpanel() ***********/
void draweditpanel(int insertflag, int wrapwidth, int zocmode, displaymethod * d)
{
	int x, y, i = 0;
	char buf[10] = "";

	for (y = 0; y < PANEL_EDIT_DEPTH; y++) {
		for (x = 0; x < PANEL_EDIT_WIDTH; x++) {
			d->putch(x + 60, y + 3, PANEL_EDIT[i], PANEL_EDIT[i + 1]);
			i += 2;
		}
	}
	
	d->print(76, 4,  YELLOW_F | BRIGHT_F | BLUE_B, (insertflag ? "on" : "off"));
	d->print(76, 8, YELLOW_F | BRIGHT_F | BLUE_B, (zocmode ? "on" : "off"));

	sprintf(buf, "%d", wrapwidth);

	if (wrapwidth)
		d->print(76, 6, YELLOW_F | BRIGHT_F | BLUE_B, buf);
	else
		d->print(72, 6, YELLOW_F | BRIGHT_F | BLUE_B, "off");
}

stringvector moredatatosvector(param * p, int editwidth)
{
	stringvector sv;    /* list of strings */
	char *str = NULL;   /* temporary string */
	int strpos = 0;     /* position in str */
	int i;

	initstringvector(&sv);

	/* load the vector */
	if ((p->moredata == NULL) | (p->length <= 0)) {
		/* No data! We need to create an empty node */
#if 0
		str = (char *) malloc(editwidth + 1);
		strcpy(str, "");
		pushstring(&sv, str);
#endif
		pushstring(&sv, str_dupmin("", editwidth + 1));
		return sv;
	}

	/* Let's fill the node from moredata! */
	strpos = 0;
	str = (char *) malloc(sizeof(char) * (editwidth + 1));

	for (i = 0; i < p->length; i++) {
		if (p->moredata[i] == 0x0d) {
			/* end of the line (heh); push the string and start over */
			str[strpos] = 0;
			pushstring(&sv, str);
			strpos = 0;
			str = (char *) malloc(sizeof(char) * (editwidth + 1));
		} else if (strpos > editwidth) {
			/* hmmm... really long line; must not have been made in ZZT... */
			/* let's truncate! */
			str[strpos] = 0;
			pushstring(&sv, str);
			strpos = 0;
			str = (char *) malloc(sizeof(char) * (editwidth + 1));
			/* move to next 0x0d */
			do i++; while (i < p->length && p->moredata[i] != 0x0d);
		} else {
			/* just your everyday copying... */
			str[strpos++] = p->moredata[i];
		}
	}

	if (strpos > 0) {
		/* strange... we seem to have an extra line with no CR at the end... */
		str[strpos] = 0;
		pushstring(&sv, str);
	} else {
		/* we grabbed all that RAM for nothing. Darn! */
		free(str);
	}

	return sv;
}

param svectortomoredata(stringvector sv)
{
	param p;
	int pos;

	/* find out how much space we need */
	p.length = 0;
	/* and now for a wierdo for loop... */
	for (sv.cur = sv.first; sv.cur != NULL; sv.cur = sv.cur->next)
		p.length += strlen(sv.cur->s) + 1;		/* + 1 for CR */

	if (p.length <= 1) {
		/* sv holds one empty string (it can happen) */
		p.moredata = NULL;
		p.length = 0;
		return p;
	}

	/* lets make room for all that moredata */
	pos = 0;
	p.moredata = (char *) malloc(sizeof(char) * p.length);

	for (sv.cur = sv.first; sv.cur != NULL; sv.cur = sv.cur->next) {
		int i;
		int linelen = strlen(sv.cur->s);	/* I feel efficient today */
		for (i = 0; i < linelen; i++) {
			p.moredata[pos++] = sv.cur->s[i];
		}
		p.moredata[pos++] = 0x0d;
	}

	return p;
}

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


/* how to display a line of text in editbox */
#define displayline(x, y, s, edit, flags, firstline, d) ((flags & EDITBOX_ZOCMODE) ? displayzoc((x), (y), (s), !(edit), (firstline), (d)) : d->print((x), (y), ZOC_TEXT_COLOUR, (s)))


int editbox(char *title, stringvector * sv, int editwidth, int flags, displaymethod * d)
{
	int c = 0, e = 0;       /* Char & ext flag */
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
	static char savefilename[15] = "temp.zoc";

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

		/* update title & panel if needed */
		if (updateflags & U_TITLE) {
			drawscrollbox(1, 17, d);
			d->print(30 - (strlen(title) / 2), 4, 0x0a, title);
		}
		if (updateflags & U_PANEL && editwidth)
			draweditpanel(insertflag, wrapwidth, flags & EDITBOX_ZOCMODE, d);

		/* clear the scrollbox */
		if (updateflags & U_TOP)
			drawscrollbox(3, 9, d);
		if (updateflags & U_CENTER)
			drawscrollbox(10, 8, d);
		if (updateflags & U_BOTTOM)
			drawscrollbox(11, 1, d);

		if (updateflags & (U_CENTER)) {
			/* Draw the center */
			displayline(9, 13, centerstr->s, editwidth, flags, centerstr->prev == NULL, d);
		}

		if (updateflags & (U_BOTTOM)) {
			/* Draw bottom half */
			loopstr = centerstr->next;
			for (i = 1; i < 8 && loopstr != NULL; i++, loopstr = loopstr->next)
				displayline(9, i + 13, loopstr->s, editwidth, flags, 0, d);

			if (i < 8)
				d->print(9, i + 13, 0x07, SLEADER);
		}
		if (updateflags & U_TOP) {
			/* Draw top half */
			loopstr = centerstr->prev;
			for (i = -1; i > -8 && loopstr != NULL; i--, loopstr = loopstr->prev)
				displayline(9, i + 13, loopstr->s, editwidth, flags, loopstr->prev == NULL, d);

			if (!editwidth && loopstr == NULL && sv->first->s[0] == '@')
				i++;

			if (i > -8)
				d->print(9, i + 13, 0x07, SLEADER);
		}

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

		/* Input */
		e = 0;
		c = d->getch();
		if (!c) {
			e = 1;
			c = d->getch();
		}
		selectFlag = d->shift();

		/* If we just started selecting, remember where we started */
		if (selectFlag && selPos == -1)
			selPos = pos;

		if (e == 1) {
			/* Keep keypress from being used after this, unless switch defaults. */
			e = -1;
			switch (c) {
				case 0x48:  /* Up Arrow */
					if (centerstr->prev != NULL && !(!editwidth && centerstr->prev == sv->first && sv->first->s[0] == '@')) {
						centerstr = centerstr->prev;
						if (pos > strlen(centerstr->s))
							pos = strlen(centerstr->s);
						if (selectFlag)
							selLineOffset++;
						updateflags = U_EDITAREA;
					}
					break;

				case 0x50:  /* Down Arrow */
					if (centerstr->next != NULL) {
						centerstr = centerstr->next;
						if (pos > strlen(centerstr->s))
							pos = strlen(centerstr->s);
						if (selectFlag)
							selLineOffset--;
						updateflags = U_EDITAREA;
					}
					break;

				case 0x49:  /* Page Up */
					for (i = 0; i < 7 && centerstr->prev != NULL && !(!editwidth && centerstr->prev == sv->first && sv->first->s[0] == '@'); i++) {
						centerstr = centerstr->prev;
						if (selectFlag)
							selLineOffset++;
					}
					if (pos > strlen(centerstr->s))
						pos = strlen(centerstr->s);
					updateflags = U_EDITAREA;
					break;

				case 0x51:  /* Page Down */
					for (i = 0; i < 7 && centerstr->next != NULL; i++) {
						centerstr = centerstr->next;
						if (selectFlag)
							selLineOffset--;
					}
					if (pos > strlen(centerstr->s))
						pos = strlen(centerstr->s);
					updateflags = U_EDITAREA;
					break;

				/******* Copy & Cut **********/

				/* Shift-delete appears the same as plain delete.
				 * For now, we won't consider shift-delete. */
				case 45:     /* alt-x: cut selected text */
				case 46:     /* alt-c: copy selected text */
				case 146:    /* ctrl-insert: copy selected text */
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

						/* Consider cut operation now */
						if (editwidth && (c == 45 || c == 147)) {
							/* Pass key on to editwidth only operations */
							e = 1;
						}
					}
					break;

				default:
					e = 1;
			}
		/* e is 0 */
		} else if (c == 27) {
			e = -1;
			if (editwidth > EDITBOX_NOEDIT)
				done = EDITBOX_OK;
			else
				done = EDITBOX_CANCEL;
		} else if (editwidth == EDITBOX_NOEDIT) {
			if (c == 13) {
				e = -1;
				done = EDITBOX_OK;
			} else if (c == '\b') {
				e = -1;
				done = EDITBOX_BACK;
			}
		}

		if (editwidth > EDITBOX_NOEDIT) {
			/* We are edititing! Yea! Fun time! */
			
			if (e == 1) {
				/* ext keys */
				switch (c) {

					/****** Movement ***********/

					case 0x4B:
						/* Left Arrow */
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

					case 0x4D:
						/* Right Arrow */
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

					case 0x47:
						/* Home */
						pos = 0;
						break;

					case 0x4F:
						/* End */
						pos = strlen(centerstr->s);
						break;

					/****** Insert & Delete ***********/

					case 0x52:
						/* Insert */
						insertflag = !insertflag;
						updateflags = U_PANEL;
						break;

					case 0x53:
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
					
					case 44:
						/* alt-z - toggle ZOC mode */
						flags ^= EDITBOX_ZOCMODE;
						updateflags = U_PANEL | U_EDITAREA;
						break;

					case 130:
						/* alt - */
						if (wrapwidth > 10)
							wrapwidth--;
						else
							wrapwidth = editwidth;
						updateflags = U_PANEL;
						break;

					case 131:
						/* alt + */
						if (wrapwidth < editwidth)
							wrapwidth++;
						else
							wrapwidth = 10;
						updateflags = U_PANEL;
						break;

				/****** Help dialog ******/
				case 0x3B:    /* F1: help dialog */
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

					/********* File access operations *********/

					case 24:
						/* alt+o: open file */
					case 23:
						/* alt+i: insert file */
						{
							stringvector filetypelist;
							
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
								filedialog(strbuf, filetypelist.cur->s + 2, (c == 24 ? "Open ZZT Object Code (ZOC) File" : "Insert ZZT Object Code (ZOC) File"), d);

							if (strlen(strbuf) != 0) {
								stringvector newsvector;
								newsvector = filetosvector(strbuf, wrapwidth, editwidth);
								if (newsvector.first != NULL) {
									if (c == 24) {
										strcpy(savefilename, strbuf);
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
							removestringvector(&filetypelist);
						}			/* block */
						updateflags = U_EDITAREA | U_TITLE;
						break;

					case 31:
						/* alt-s: save to file */
						{
							if (filenamedialog(savefilename, "Save Object Code As", "", 1, d) != NULL)
								svectortofile(sv, savefilename);
							updateflags = U_EDITAREA | U_PANEL;
						}
						break;

					case 50:
						/* alt-m: load .zzm music */
						{
							filedialog(strbuf, "zzm", "Choose ZZT Music (ZZM) File", d);
							if (strlen(strbuf) != 0) {
								stringvector zzmv;
								zzmv = filetosvector(strbuf, 80, 80);
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
						}
						updateflags = U_EDITAREA | U_TITLE;
						break;

					/******** Cut operation *********/

					case 45:     /* alt-x: cut selected text */
					case 147:    /* ctrl-delete: clear selected text */
						/* Clear selected area */
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

					case 47:
						/* alt-v: paste register */
						sv->cur = centerstr;
						pos = regput('\"', sv, pos, wrapwidth, editwidth);
						centerstr = sv->cur;
						/* This next line should not be needed */
						if (pos > strlen(sv->cur->s)) {
							insertstring(sv, strcpy((char *) malloc(100),
							                        "Bug: regput() distorted pos. "
							                        "Report to <bitman@scn.org>"));
							pos = strlen(sv->cur->s);
						}
						updateflags = U_EDITAREA;
						break;

					default:
						/* act as if ext key is really not ext. This way, people used to
						 * using alt key combos to plot special chars will not be
						 * disappointed. */
						e = 0;
						break;
				}
			}
			if (e == 0) {
				/* normal key (or unknown ext key impersonating one) */
				switch (c) {
					case 9:
						/* Tab */
						/* determin tab amount */
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

					case 13:
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

					case '\b':
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

					case 25:
						/* ctrl-y: delete line */
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

					case 27: /* escape when done */
						done = EDITBOX_OK;
						break;

					case 1: 
						/* ctrl-a: insert ascii char/decimal-value */
						strcpy(strbuf, centerstr->s);
						str_lowercase(strbuf);
						updateflags = U_EDITAREA;

						if (str_equ(strbuf, "#char", STREQU_UNCASE | STREQU_FRONT)) {
							/* append dec value for ascii char */

							sscanf(strbuf + 5, "%d", &selChar);
							selChar = charselect(d, selChar);
							centerstr->s[5] = ' ';
							centerstr->s[6] = 0;

							/* change c to a string */
							sprintf(strbuf, "%d", selChar);
							strcat(centerstr->s, strbuf);
							pos = strlen(centerstr->s);
							updateflags = U_EDITAREA;
							break;
						}
						else
							/* ctrl-a: insert ascii char */
							c = selChar = charselect(d, selChar);
						/* no break; we just changed c & want to insert it */

					default:
						/* Normal/weird char for insert/replace */
						if (insertflag) {
							/* insert */
							if (strlen(centerstr->s) < (wrapwidth?wrapwidth:editwidth)) {
								/* insert if there is room */
								for (i = strlen(centerstr->s) + 1; i > pos; i--)
									centerstr->s[i] = centerstr->s[i-1];
								centerstr->s[pos++] = c;
								updateflags |= U_CENTER;
							}
							else if (wrapwidth) {
								/* no room; wordwrap */
								strbuf[0] = c;
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
									centerstr->s[pos++] = c;
									updateflags |= U_CENTER;
								}
								else if (wrapwidth) {
									/* no room; wordwrap */
									strbuf[0] = c;
									strbuf[1] = 0;
									sv->cur = centerstr;
									pos = wordwrap(sv, strbuf, pos, pos, wrapwidth, editwidth);
									centerstr = sv->cur;
									updateflags = U_EDITAREA;
								}
							}
							else {
								centerstr->s[pos++] = c;
								updateflags |= U_CENTER;
							}
						} /* esle replace */
						break;
				}	/* esac */
			}	/* fi extended keys */
		}		/* esle in editmode */
		/* if the shift key is not still held down and we are selecting, then stop select mode */
		/* also stop if non-extended key was pressed and selection is active */
		if ((!selectFlag && selPos != -1) || (e == 0 && selPos != -1)) {
			selPos = -1;
			selLineOffset = 0;
			updateflags |= U_EDITAREA;
		}
	}			/* elihw */

	sv->cur = centerstr;

	return done;
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

			for (i = 1; s[i] != 0 && s[i] != '/' && s[i] != '?' && s[i] != '\'' && s[i] != ' '; i++)
				token[i-1] = s[i];
			token[i-1] = 0;

			while (!iszztdir(token) && s[i] != 0 && s[i] != '/' && s[i] != '?' && s[i] != '\'') {
				while (s[i] == ' ') { token[i - 1] = ' '; i++; }
				for (; s[i] != 0 && s[i] != '/' && s[i] != '?' && s[i] != '\'' && s[i] != ' '; i++)
					token[i-1] = s[i];
				token[i-1] = 0;
			}

			if (iszztdir(token)) {
				/* token is a proper direction */
				d->print(x + 1, y, ZOC_STDDIR_COLOUR, token);
			} else
				d->print(x + 1, y, ZOC_DEFAULT_COLOUR, token);

			if (s[i] == '/' || s[i] == '?' || s[i] == '\'' || s[i] == ' ')
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



/* advance token in source from pos, returning token length */
int tokenadvance(char *token, char *source, int *pos)
{
	int i = 0;

	/* Move forward past any spaces */
	while (*pos < strlen(source) && source[*pos] == ' ') (*pos)++;

	/* Grab next token */
	for (; *pos < strlen(source) != 0 && source[*pos] != ' '; i++, (*pos)++)
		token[i] = source[*pos];
	token[i] = 0;

	return i;
}


/* grow token in source from pos, returning token length */
int tokengrow(char *token, char *source, int *pos)
{
	int i = strlen(token);

	/* Grab any spaces */
	for (; *pos < strlen(source) != 0 && source[*pos] == ' '; i++, (*pos)++)
		token[i] = source[*pos];

	/* Grab next token */
	for (; *pos < strlen(source) != 0 && source[*pos] != ' '; i++, (*pos)++)
		token[i] = source[*pos];
	token[i] = 0;

	return i;
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
				if (token[0] == '#') {
					/* remainder of args is a #command */
					displayzoc(x + j - k, y, args + j - k, 1, 0, d);
					j = strlen(args);
				} else {
					/* Thenmessage was not a command, so display it as a normal message */
					ctax = CTAX_MESSAGE;
				}
				break;

			case CTAX_SOUND:
				d->print(x + j - k, y, ZOC_TEXT_COLOUR, token);
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
		if (str_equ(buffer, zztdirmods[i], STREQU_UNCASE | STREQU_FRONT)) {
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


/***************************************************************************/
/**** Wordwrap *************************************************************/
/***************************************************************************/


/* wordwrap
 * purpose: Inserts string into current string of svector, wordwrapping if
 *          necessary.
 * args:    sv:        stringvector to manipulate
 *          str:       string to insert in sv->cur->s
 *          inspos:    where in sv to insert str
 *          pos:       cursor position in sv->cur->s to track; a negative value
 *                     indicates that -pos - 1 is position in str
 *          wrapwidth: at what cursor position to wrap to next line
 *          editwidth: maximum width of a line in sv
 * return:  new location of pos. sv->cur is changed to reflect line on which
 *          pos now resides.
 *
 * NOTE: str will not be modified nor free()d in any way.
 */
int wordwrap(stringvector * sv, char *str, int inspos, int pos, int wrapwidth, int editwidth)
{
	int i, j, k;		/* general counters */
	char *longstr;	/* Combination of sv->cur->s & str */
	int longlen;		/* Length of longstr */
	int newpos;		/* new position after insert */

	char *newstr;     /* new string for next line */

	/* check for bad data */
	if (sv->cur == NULL || sv->cur->s == NULL || wrapwidth > editwidth || editwidth < 2 || inspos > strlen(sv->cur->s))
		return -1;

	/* first determine longlen and allocate longstr */
	longlen = strlen(sv->cur->s) + strlen(str);
	longstr = (char *) malloc(longlen + 2);
	memset(longstr, 0, longlen + 2);
	
	/* fill longstr
	 * 
	 * i: position in longstr
	 * j: position in sv->cur->s
	 * k: position in str 
	 */

	/* fill from sv until inspos */
	for (i = 0; i < inspos; i++)
		longstr[i] = sv->cur->s[i];
	j = i;

	/* fill from str until end of str */
	for (k = 0; str[k] != 0; k++, i++)
		longstr[i] = str[k];

	/* fill from sv until end */
	for (; i < longlen; i++, j++)
		longstr[i] = sv->cur->s[j];
	
	/* cap longstr */
	longstr[i]   = 0;

	/* determine location of newpos */
	if (pos >= inspos)
		newpos = pos + strlen(str);
	else if (pos < 0)
		newpos = inspos - pos - 1;

	if (longlen <= wrapwidth) {
		/* no need to wordwrap; we can just copy longstr over sv->cur->s */
		strcpy(sv->cur->s, longstr);
		return newpos;
	}

	/* we need to find the first space before wrapwidth 
	 *
	 * i: position in longstr
	 * j: position of last identified space
	 */

	j = -1;
	for (i = 0; i < wrapwidth; i++)
		if (longstr[i] == ' ')
			j = i;

	if (j == -1) {
		/* no space was found before wrap; reject insert */
		return pos;
	}

	/* make newpos the negative differance of location of space and newpos,
	 * if it belongs on next line */
	if (newpos > j)
		newpos = j - newpos;

	/* set newstr to location of string after the space & cap longstr at space */
	newstr = longstr + j + 1;
	longstr[j] = 0;

	/* replace sv->cur->s with shortened longstr */
	strcpy(sv->cur->s, longstr);

	/* finally: wrap onto next line or new line */
	if (sv->cur->next == NULL       || strlen(sv->cur->next->s) == 0 ||
			sv->cur->next->s[0] == '#'  || sv->cur->next->s[0] == '/' ||
			sv->cur->next->s[0] == '?'  || sv->cur->next->s[0] == ':' ||
			sv->cur->next->s[0] == '!'  || sv->cur->next->s[0] == '$' ||
			sv->cur->next->s[0] == '\'' || sv->cur->next->s[0] == '@' ||
			sv->cur->next->s[0] == ' ') {
		/* next line either does not exist, is blank, is a zoc command,
		 * or is indented; so, we create a new, blank line to wordwrap onto */

		char *newnode;
		newnode = (char *) malloc(editwidth + 2);
		newnode[0] = 0;
		insertstring(sv, newnode);
	} else {
		/* we can put text at the beginning of the next line; append a space
		 * to end of newstr in preparation. */
		i = strlen(newstr);
		newstr[i++] = ' ';
		newstr[i] = 0;
	}
	/* it is now okay to put text at the beginning of the next line */


	/* recursively insert newstr at beginning of next line */
	if (newpos < 0) {
		/* cursor should be tracked on next line */
		sv->cur = sv->cur->next;
		newpos = wordwrap(sv, newstr, 0, newpos, wrapwidth, editwidth);
	} else {
		stringnode * nodeptr = sv->cur;
		sv->cur = sv->cur->next;
		wordwrap(sv, newstr, 0, 0, wrapwidth, editwidth);
		sv->cur = nodeptr;
	}

	free(longstr);

	return newpos;
}


/***************************************************************************/
/**** File I/O for editbox editing *****************************************/
/***************************************************************************/


#define BUFFERSIZE 1000
/* filetosvector - loads a textfile into a new stringvector */
stringvector filetosvector(char* filename, int wrapwidth, int editwidth)
{
	stringvector v;
	FILE * fp;
	char buffer[BUFFERSIZE] = "";      /* Be nice and wordwrap long lines */
	char * str = NULL;
	int strpos = 0;
	int c = 0;

	initstringvector(&v);

	/* return on bad data */
	if (wrapwidth > editwidth || editwidth < 1)
		return v;

	fp = fopen(filename, "rb");
	if (fp == NULL)
		return v;

	do {
		strpos = 0;

		while (strpos < BUFFERSIZE && !((c = fgetc(fp)) == EOF || c == 0x0d || c == 0x0a || c == 0)) {
			buffer[strpos++] = c;
		}
		buffer[strpos] = 0;

		/* remove LF after CR (assume not CR format) */
		if (c == 0x0d)
			fgetc(fp);

		str = (char *) malloc(sizeof(char) * (editwidth + 1));
		if (str == NULL) {
			fclose(fp);
			return v;
		}

		if (strpos < wrapwidth) {
			/* simple copy */
			strcpy(str, buffer);
			pushstring(&v, str);
		} else {
			/* Push an empty string and wordwrap the buffer onto it */
			str[0] = 0;
			pushstring(&v, str);
			v.cur = v.last;
			wordwrap(&v, buffer, 0, 0, wrapwidth, editwidth);
		}
	} while (c != EOF);

	fclose(fp);

	/* remove trailing blank line */
	v.cur = v.last;
	if (strlen(v.cur->s) == 0)
		removestring(&v);

	return v;
}


/* Copies a stringvector into a file. sv is not changed */
void svectortofile(stringvector * sv, char *filename)
{
	FILE* fp;
	stringnode * curnode = NULL;
	int i;

	if (sv->first == NULL)
		return;

	fp = fopen(filename, "wb");
	if (fp == NULL)
		return;

	for (curnode = sv->first; curnode != NULL; curnode = curnode->next) {
		for (i = 0; curnode->s[i] != 0; i++)
			fputc(curnode->s[i], fp);
		fputc(0x0d, fp);
		fputc(0x0a, fp);
	}

	/* Done with file; success! */
	fclose(fp);
}

