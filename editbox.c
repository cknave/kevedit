/* editbox.c  -- text editor/viewer in kevedit
 * $Id: editbox.c,v 1.6 2000/08/22 00:44:48 bitman Exp $
 * Copyright (C) 2000 Ryan Phillips <bitman@scn.org>
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

#include <stdlib.h>

#include "scroll.h"
#include "colours.h"
#include "panel_ed.h"


/* What portion of display box needs update? */
#define U_NONE   0x00
#define U_CENTER 0x01
#define U_BOTTOM 0x02
#define U_TOP    0x04
#define U_ALL    0x07

/* FIXME: use more descriptive leader than ~ */
#define SLEADER  "~"

/* ZZT Object Code colours */
#define ZOC_STDCOMMAND_COLOUR  GREEN_F
#define ZOC_STDMESSAGE_COLOUR  RED_F | BRIGHT_F
#define ZOC_STDFLAG_COLOUR     WHITE_F | BRIGHT_F
#define ZOC_STDITEM_COLOUR     WHITE_F | BRIGHT_F
#define ZOC_STDKIND_COLOUR     WHITE_F | BRIGHT_F
#define ZOC_STDDIR_COLOUR      WHITE_F | BRIGHT_F

#define ZOC_DEFAULT_COLOUR     WHITE_F
#define ZOC_OPERATOR_COLOUR    YELLOW_F | BRIGHT_F
#define ZOC_HEADING_COLOUR     WHITE_F | BRIGHT_F
#define ZOC_TEXT_COLOUR        GREEN_F | BRIGHT_F

#define ZOC_LABEL_COLOUR       RED_F
#define ZOC_HYPER_COLOUR       MAGENTA_F
#define ZOC_SENDMESSAGE_COLOUR MAGENTA_F | BRIGHT_F
#define ZOC_OBJNAME_COLOUR     BLUE_F | BRIGHT_F
#define ZOC_COMMENT_COLOUR     CYAN_F | BRIGHT_F

/* token testing functions */
int iszztcommand(unsigned char *token);
int iszztmessage(unsigned char *token);
int iszztflag(unsigned char *token);
int iszztitem(unsigned char *token);
int iszztkind(unsigned char *token);
int iszztdir(unsigned char *token);


void draweditpanel(displaymethod * d, int insertflag, int wrapwidth)
{
	int x, y, i = 0;
	char buf[10] = "";

	for (y = 3; y < PANEL_EDIT_DEPTH + 3; y++) {
		for (x = 0; x < PANEL_EDIT_WIDTH; x++) {
			d->putch(x + 60, y, PANEL_EDIT[i], PANEL_EDIT[i + 1]);
			i += 2;
		}
	}
	
	d->print(76, 8, YELLOW_F | BRIGHT_F | BLUE_B, (insertflag ? "on" : "off"));

	if (wrapwidth)
		d->print(76, 11, YELLOW_F | BRIGHT_F | BLUE_B, itoa(wrapwidth, buf, 10));
	else
		d->print(72, 11, YELLOW_F | BRIGHT_F | BLUE_B, "off");
}



/* FIXME: any data on a line longer than editwidth will be cut. Should we do
 * something else? Perhaps wordwrap, or long line editing? Currently, it
 * doesn't crash KevEdit in this special case, but may mess up someone's
 * moredata (very theoretically). */
void editmoredata(displaymethod * d, param * p)
{
	stringvector sv;		/* list of strings */
	unsigned char *str = NULL;	/* temporary string */
	int strpos = 0;			/* position in str */
	int newdatalength = 0;		/* when writing, size of moredata */
	int i = 0, j = 0;		/* general counters */
	const int editwidth = 42;	/* allowable width for editing */

	initstringvector(&sv);

	/* load the vector */
	if (p->moredata == NULL | p->length <= 0) {
		/* We need to create an empty node */
		str = (unsigned char *) malloc(editwidth + 2);
		strcpy(str, "");
		pushstring(&sv, str);
	} else {
		/* Let's fill the node from moredata! */
		strpos = 0;
		str = (unsigned char *) malloc(editwidth + 2);

		for (i = 0; i < p->length; i++) {
			if (p->moredata[i] == 0x0d) {
				/* end of the line (heh); push the string and start over */
				str[strpos] = 0;
				pushstring(&sv, str);
				strpos = 0;
				str = (unsigned char *) malloc(editwidth + 2);
			} else if (strpos >= editwidth) {
				/* hmmm... really long line; must not have been made in ZZT... */
				/* we should probably wordwrap here, just to be nice, but no... */
				str[strpos] = p->moredata[i];
				str[strpos + 1] = 0;
				pushstring(&sv, str);
				strpos = 0;
				str = (unsigned char *) malloc(editwidth + 2);
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
	}

	/* Now that the node is full, we can edit it. */
	sv.cur = sv.first;	/* This is redundant, but hey. */
	editbox(d, "Object Editor", &sv, editwidth, 1);

	/* Okay, let's put the vector back in moredata */

	/* find out how much space we need */
	newdatalength = 0;
	/* and now for a wierdo for loop... */
	for (sv.cur = sv.first; sv.cur != NULL; sv.cur = sv.cur->next)
		newdatalength += strlen(sv.cur->s) + 1;		/* + 1 for CR */

	if (newdatalength == 1) {
		/* sv holds one empty string (it can happen) */
		p->moredata = NULL;
		p->length = 0;
		return;
	}
	/* lets make room for all that moredata */
	strpos = 0;
	str = (unsigned char *) malloc(newdatalength);

	for (sv.cur = sv.first; sv.cur != NULL; sv.cur = sv.cur->next) {
		j = strlen(sv.cur->s);	/* I feel efficient today */
		for (i = 0; i < j; i++) {
			str[strpos++] = sv.cur->s[i];
		}
		str[strpos++] = 0x0d;
	}

	/* Okay, now lets get rid of sv */
	deletestringvector(&sv);

	/* Yea! Data translated. Now we can put it in moredata */
	free(p->moredata);
	p->length = newdatalength;
	p->moredata = str;
	/* and the crowd goes wild! */
}


void editbox(displaymethod * d, char *title, stringvector * sv, int editwidth, int zocformatting)
{
	int c = 0, e = 0;	/* Char & ext flag */
	int i, j;		/* general counters */
	int done = 0;		/* true when editing/viewing is done */
	int updateflags;	/* flags to determine what needs update */
	stringnode *centerstr;	/* str in center of dialog */

	/* vars only relating to editing */
	int pos = 0;		/* position in sv->cur->s */
	int insertflag = 1;	/* nonzero when in insert mode */
	unsigned char *tmpstr;	/* temporary string for pushing */
	unsigned char strbuf[80] = "";  /* general buffer */

	int wrapwidth = editwidth;

	if (sv->cur == NULL)
		return;
	centerstr = sv->cur;

	drawscrollbox(0, 0, d);
	d->print(23, 4, 0x0a, title);
	draweditpanel(d, insertflag, wrapwidth);

	updateflags = U_ALL;

	while (!done && !(c == 13 && e == 0 && editwidth == 0)) {

		d->cursorgo(9 + pos, 13);

		/* clear the scrollbox */
		if (updateflags & U_TOP)
			drawscrollbox(3, 9, d);
		if (updateflags & U_CENTER)
			drawscrollbox(10, 8, d);
		if (updateflags & U_BOTTOM)
			drawscrollbox(11, 1, d);

		if (updateflags & (U_CENTER | U_TOP)) {
			/* Draw the center */
			if (zocformatting)
				displayzoc(d, 9, 13, centerstr->s, centerstr->prev == NULL);
			else
				d->print(9, 13, 0x0a, centerstr->s);
		}
		if (updateflags & (U_BOTTOM | U_CENTER | U_TOP)) {
			/* Draw bottom half */
			sv->cur = centerstr->next;
			for (i = 1; i < 8 && sv->cur != NULL; i++, sv->cur = sv->cur->next)
				if (zocformatting)
					displayzoc(d, 9, i + 13, sv->cur->s, 0);
				else
					d->print(9, i + 13, 0x0a, sv->cur->s);

			if (i < 8)
				d->print(9, i+13, 0x0a, SLEADER);
				if (zocformatting)
					displayzoc(d, 9, i + 13, SLEADER, 0);
				else
					d->print(9, i + 13, 0x0a, SLEADER);
		}
		if (updateflags & U_TOP) {
			/* Draw top half */
			sv->cur = centerstr->prev;
			for (i = -1; i > -8 && sv->cur != NULL; i--, sv->cur = sv->cur->prev)
				if (zocformatting)
					displayzoc(d, 9, i + 13, sv->cur->s, sv->cur->prev == NULL);
				else
					d->print(9, i + 13, 0x0a, sv->cur->s);

			if (i > -8)
				d->print(9, i+13, 0x0a, SLEADER);
		}

		updateflags = U_NONE;

		/* Input */
		e = 0;
		c = d->getch();
		if (!c) {
			e = 1;
			c = d->getch();
		}
		if (e == 1 && c == 0x48) {
			/* Up Arrow */
			if (centerstr->prev != NULL) {
				centerstr = centerstr->prev;
				if (pos > strlen(centerstr->s))
					pos = strlen(centerstr->s);
				updateflags = U_ALL;
			}
		} else if (e == 1 && c == 0x50) {
			/* Down Arrow */
			if (centerstr->next != NULL) {
				centerstr = centerstr->next;
				if (pos > strlen(centerstr->s))
					pos = strlen(centerstr->s);
				updateflags = U_ALL;
			}
		} else if (e == 1 && c == 0x49) {
			/* Page Up */
			for (i = 0; i < 7 && centerstr->prev != NULL; i++)
				centerstr = centerstr->prev;
			if (pos > strlen(centerstr->s))
				pos = strlen(centerstr->s);
			updateflags = U_ALL;
		} else if (e == 1 && c == 0x51) {
			/* Page Down */
			for (i = 0; i < 7 && centerstr->next != NULL; i++)
				centerstr = centerstr->next;
			if (pos > strlen(centerstr->s))
				pos = strlen(centerstr->s);
			updateflags = U_ALL;
		} else if (editwidth) {
			/* We are edititing! Yea! Fun time! */
			
			if (e == 1) {
				/* ext keys */
				switch (c) {
					case 0x4B:
						/* Left Arrow */
						if (pos > 0)
							pos--;
						else {
							/* Move to end of previous line (or current line) */
							if (centerstr->prev != NULL) {
								centerstr = centerstr->prev;
								updateflags = U_ALL;
							}
							pos = strlen(centerstr->s);
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
								updateflags = U_ALL;
							}
							pos = 0;
						}
						break;

					case 0x52:
						/* Insert */
						insertflag = !insertflag;
						draweditpanel(d, insertflag, wrapwidth);
						break;

					case 0x53:
						/* Delete */
						if (pos < strlen(centerstr->s)) {
							for (i = pos; i < strlen(centerstr->s); i++)
								centerstr->s[i] = centerstr->s[i+1];
							updateflags = U_CENTER;
						}
						else {
							/* FIXME: join next line (wordwrap) */
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

					case 130:
						/* alt - */
						if (wrapwidth > 0)
							wrapwidth--;
						else
							wrapwidth = editwidth;
						draweditpanel(d, insertflag, wrapwidth);
						break;

					case 131:
						/* alt + */
						if (wrapwidth < editwidth)
							wrapwidth++;
						else
							wrapwidth = 0;
						draweditpanel(d, insertflag, wrapwidth);
						break;

					default:
						/* act as if ext key is really not. This way, those used to
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
						if (strlen(centerstr->s) + 3 < (wrapwidth?wrapwidth:editwidth)) {
							/* insert if there is room */
							for (i = strlen(centerstr->s) + 4; i > pos; i--)
								centerstr->s[i] = centerstr->s[i-4];
							for (i = 0; i < 4; i++)
								centerstr->s[pos++] = ' ';
							updateflags = U_CENTER;
						}
						else {
							/* no room; wordwrap */
							strcpy(strbuf, "    ");
							sv->cur = centerstr;
							pos = wordwrap(sv, strbuf, pos, pos, wrapwidth, editwidth);
							centerstr = sv->cur;
							updateflags = U_ALL;
						}
						break;

					case 13:
						/* Enter */
						tmpstr = (unsigned char*) malloc(editwidth + 2);
						for (i = pos, j = 0; i < strlen(centerstr->s); i++, j++)
							tmpstr[j] = centerstr->s[i];
						centerstr->s[pos] = 0;

						tmpstr[j] = 0;
						sv->cur = centerstr;
						insertstring(sv, tmpstr);
						centerstr = centerstr->next;
						pos = 0;
						updateflags = U_ALL;
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
								updateflags = U_TOP;
							}
							else if (strlen(centerstr->prev->s) + 1 < wrapwidth) {
								/* merge lines; wordwrap */
								i = strlen(centerstr->prev->s);
								if (centerstr->prev->s[i-1] != ' ') {
									/* add a space at the end */
									centerstr->prev->s[i]     = ' ';
									centerstr->prev->s[i + 1] = 0;
								}
								sv->cur = centerstr->prev;
								tmpstr = removestring(sv);
								sv->cur = centerstr;
								pos = wordwrap(sv, tmpstr, pos, pos, wrapwidth, editwidth);
								centerstr = sv->cur;
								free(tmpstr);
								updateflags = U_ALL;
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
						done = 1;
						break;

					case 1: 
						/* ctrl-a: insert ascii char/decimal-value */
						strcpy(strbuf, centerstr->s);
						strlwr(strbuf);
						updateflags = U_ALL;

						if ((unsigned char*)strstr(strbuf, "#char") == strbuf) {
							/* append dec value for ascii char */

							c = charselect(d);
							centerstr->s[5] = ' ';
							centerstr->s[6] = 0;

							/* warning: itoa() is not ANSI nor POSIX; we should probably
							 * replace it. */
							strcat(centerstr->s, itoa(c, strbuf, 10));
							pos = strlen(centerstr->s);
							updateflags = U_ALL;
							break;
						}
						else
							/* ctrl-a: insert ascii char */
							c = charselect(d);
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
								updateflags = U_ALL;
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
									updateflags = U_ALL;
								}
							}
							else {
								centerstr->s[pos++] = c;
								updateflags |= U_CENTER;
							}
						} /* esle replace */
						break;
				}	/* esac */
			}	/* fi e == 0 */
		}		/* fiesle editwidth */
	}			/* elihw */

	sv->cur = centerstr;
}


void displayzoc(displaymethod * d, int x, int y, unsigned char *s, int firstline)
{
	int i = 0;			/* position in s */
	int j = 0;			/* position in token */
	unsigned char token[80] = "";	/* token buffer */

	/* find out what we're dealing with based on the first char */
	switch (s[0]) {
		case '#':
			/* command */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
			for (i = 1; s[i] != ' ' && s[i] != 0; i++)
				token[i - 1] = s[i];
			token[i - 1] = 0;

			d->print(x + 1, y, (iszztcommand(token) ? ZOC_STDCOMMAND_COLOUR : ZOC_SENDMESSAGE_COLOUR), token);

			/* TODO: process remainder of s based on value of token */
			d->print(x + i, y, ZOC_DEFAULT_COLOUR, s + i);
			break;
		case ':':
			/* message */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
			for (i = 1; s[i] != '\'' && s[i] != 0; i++)
				token[i - 1] = s[i];
			token[i - 1] = 0;
			
			d->print(x + 1, y, (iszztmessage(token) ? ZOC_STDMESSAGE_COLOUR : ZOC_LABEL_COLOUR), token);
			
			if (s[i] == '\'')
				d->print(x + i, y, ZOC_COMMENT_COLOUR, s + i);
		if (s[i] == 0)
			break;

		case '?':
		case '/':
			/* movement */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
			for (i = 1; s[i] != '?' && s[i] != '/' && s[i] != '\'' && s[i] != 0; i++) 
				token[i-1] = s[i];
			token[i-1] = 0;

			d->print(x + 1, y, (iszztdir(token) ? ZOC_STDDIR_COLOUR : ZOC_DEFAULT_COLOUR), token);

			/* Recursively display next movement/comment if there is one */
			if (s[i] != 0)
				displayzoc(d, x + i, y, s + i, 0);
			
			break;

		case '!':
			/* hypermessage */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
			for (i = 1; s[i] != ';' && s[i] != 0; i++)
				d->putch(x + i, y, s[i], ZOC_SENDMESSAGE_COLOUR);

			if (s[i] == 0) break;

			d->putch(x + i, y, s[i], ZOC_OPERATOR_COLOUR);
			for (i++; s[i] != '\'' && s[i] != 0; i++)
				d->putch(x + i, y, s[i], ZOC_HYPER_COLOUR);

			if (s[i] == '\'')
				d->print(x + i, y, ZOC_COMMENT_COLOUR, s + i);
			break;

		case '\'':
			/* comment */
			d->print(x, y, ZOC_COMMENT_COLOUR, s);
			break;

		case '$':
			/* heading */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
			d->print(x + 1, y, ZOC_HEADING_COLOUR, s + 1);
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


/* zzt components for special highlighting */

#define ZZTCOMMANDCOUNT 27
const char zztcommands[ZZTCOMMANDCOUNT][12] =
{
      "become", "bind", "change", "char", "clear", "cycle", "die", "end",
	"endgame", "give", "go", "idle", "if", "lock", "play", "put",
	"restart", "restore", "send", "set", "shoot", "take", "throwstar",
	"try", "unlock", "walk", "zap"
};

#define ZZTMESSAGECOUNT 5
const char zztmessages[ZZTMESSAGECOUNT][10] =
{
	"touch", "shot", "bombed", "thud", "energize"
};

#define ZZTFLAGCOUNT 5
const char zztflags[ZZTFLAGCOUNT][12] =
{
	"alligned", "contact", "blocked", "energized", "exists"
};

#define ZZTITEMCOUNT 5
const char zztitems[ZZTITEMCOUNT][8] =
{
	"ammo", "gems", "torches", "health", "score"
};

#define ZZTKINDCOUNT 1
const char zztkinds[ZZTKINDCOUNT][12] =
{
	"key"
};

#define ZZTCOLOURCOUNT 7
const char zztcolours[ZZTCOLOURCOUNT][8] =
{
	"blue", "green", "red", "cyan", "purple", "yellow", "white"
};

#define ZZTDIRCOUNT 14
const char zztdirs[ZZTDIRCOUNT][6] =
{
	"n", "north", "s", "south", "e", "east", "w", "west", "i", "idle",
	"seek", "flow", "rndns", "rndne"
};

#define ZZTDIRMODCOUNT 4
const char zztdirmods[ZZTDIRMODCOUNT][5] =
{
	"cw", "ccw", "rndp", "opp"
};


/* token testers */
int iszztcommand(unsigned char *token)
{
	int i = 0;
	char buffer[12] = "";

	memcpy(buffer, token, 11);
	buffer[11] = 0;
	strlwr(buffer);

	for (i = 0; i < ZZTCOMMANDCOUNT; i++)
		if (!strcmp(buffer, zztcommands[i]))
			return 1;

	return 0;
}

int iszztmessage(unsigned char *token)
{
	int i = 0;
	char buffer[12] = "";

	memcpy(buffer, token, 11);
	buffer[11] = 0;
	strlwr(buffer);

	for (i = 0; i < ZZTMESSAGECOUNT; i++)
		if (!strcmp(buffer, zztmessages[i]))
			return 1;

	return 0;
}

int iszztflag(unsigned char *token)
{
	int i = 0;
	char buffer[12] = "";

	memcpy(buffer, token, 11);
	buffer[11] = 0;
	strlwr(buffer);

	if ((char*)strstr(buffer, "not") == buffer) {
		/* Advance token to nearest space */
		token +=3;
		do token++; while (token[0] != ' ' && token[0] != 0);
		/* Advance token to nearest nonspace */
		do token++; while (token[0] == ' ');

		/* now see if next word is a valid token */
		return iszztflag(token);
	}

	for (i = 0; i < ZZTFLAGCOUNT; i++)
		if (!strcmp(buffer, zztflags[i]))
			return 1;

	return 0;
}

int iszztitem(unsigned char *token)
{
	int i = 0;
	char buffer[12] = "";

	memcpy(buffer, token, 11);
	buffer[11] = 0;
	strlwr(buffer);

	for (i = 0; i < ZZTITEMCOUNT; i++)
		if (!strcmp(buffer, zztitems[i]))
			return 1;

	return 0;
}

int iszztkind(unsigned char *token)
{
	int i = 0;
	char buffer[12] = "";

	memcpy(buffer, token, 11);
	buffer[11] = 0;
	strlwr(buffer);

	for (i = 0; i < ZZTCOLOURCOUNT; i++)
		if ((char*)strstr(buffer, zztcolours[i]) == buffer) {
			/* Advance token to nearest space */
			do token++; while (token[0] != ' ' && token[0] != 0);
			/* Advance token to nearest nonspace */
			do token++; while (token[0] == ' ');

			/* now see if next word is a valid token */
			return iszztkind(token);
		}

	for (i = 0; i < ZZTKINDCOUNT; i++)
		if (!strcmp(buffer, zztkinds[i]))
			return 1;

	return 0;
}

int iszztdir(unsigned char *token)
{
	int i = 0;
	char buffer[12] = "";

	memcpy(buffer, token, 11);
	buffer[11] = 0;
	strlwr(buffer);

	for (i = 0; i < ZZTDIRMODCOUNT; i++)
		if ((char*)strstr(buffer, zztdirmods[i]) == buffer) {
			/* Advance token to nearest space */
			do token++; while (token[0] != ' ' && token[0] != 0);
			/* Advance token to nearest nonspace */
			do token++; while (token[0] == ' ');

			/* now see if next word is a valid token */
			return iszztdir(token);
		}

	for (i = 0; i < ZZTDIRCOUNT; i++)
		if (!strcmp(buffer, zztdirs[i]))
			return 1;

	return 0;
}


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
int wordwrap(stringvector * sv, unsigned char *str, int inspos, int pos, int wrapwidth, int editwidth)
{
	int i, j, k;		/* general counters */
	unsigned char *longstr;	/* Combination of sv->cur->s & str */
	int longlen;		/* Length of longstr */
	int newpos;		/* new position after insert */

	unsigned char *newstr;     /* new string for next line */

	/* check for bad data */
	if (sv->cur == NULL || sv->cur->s == NULL || wrapwidth > editwidth || editwidth < 2 || inspos > strlen(sv->cur->s))
		return -1;

	/* first determine longlen and allocate longstr */
	longlen = strlen(sv->cur->s) + strlen(str);
	longstr = (unsigned char *) malloc(longlen + 2);
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
	if (sv->cur->next != NULL    && strlen(sv->cur->next->s) != 0 &&
			sv->cur->next->s[0] != '#'  && sv->cur->next->s[0] != '/' &&
			sv->cur->next->s[0] != '?'  && sv->cur->next->s[0] != ':' &&
			sv->cur->next->s[0] != '!'  && sv->cur->next->s[0] != '$' &&
			sv->cur->next->s[0] != '\'' && sv->cur->next->s[0] != '@') {
		/* next line exists, is not blank, and isn't a zoc command */

		/* append a space to end of newstr */
		i = strlen(newstr);
		newstr[i++] = ' ';
		newstr[i] = 0;

		/* recursively insert newstr at beginning of next line */
		if (newpos < 0) {
			/* cursor should be tracked on next line */
			sv->cur = sv->cur->next;
			newpos = wordwrap(sv, newstr, 0, newpos, wrapwidth, editwidth);
		} else {
			sv->cur = sv->cur->next;
			wordwrap(sv, newstr, 0, 0, wrapwidth, editwidth);
			sv->cur = sv->cur->prev;
		}
	} else {
		/* cannot wrapped to next line; create a new line and copy newstr into it */
		/* FIXME: what if strlen(newstr) > wrapwidth? */
		unsigned char *newnode;
		newnode = (unsigned char *) malloc(editwidth + 2);

		strcpy(newnode, newstr);
		insertstring(sv, newnode);
		
		if (newpos < 0) {
			/* cursor should be on next line */
			sv->cur = sv->cur->next;
			newpos = -newpos - 1;
		}
	}
	free(longstr);

	return newpos;
}

