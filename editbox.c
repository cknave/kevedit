/* editbox.c  -- text editor/viewer in kevedit
 * $Id: editbox.c,v 1.2 2000/08/18 18:01:30 kvance Exp $
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
int iszztcommand(unsigned char* token);
int iszztmessage(unsigned char* token);
int iszztflag(unsigned char* token);
int iszztitem(unsigned char* token);
int iszztkind(unsigned char* token);
int iszztdir(unsigned char* token);


/* FIXME: any data on a line longer than editwidth will be cut. Should we do
 * something else? Perhaps wordwrap, or long line editing? For now it
 * doesn't crash KevEdit in this special case, but may mess up someone's
 * moredata (very theoretically). */
void
editmoredata(displaymethod* d, param* p)
{
	stringvector sv;           /* list of strings */
	unsigned char* str = NULL; /* temporary string */
	int strpos = 0;            /* position in str */
	int newdatalength = 0;     /* when writing, how big will moredata be? */
  int i = 0, j = 0;          /* general counters */
	const int editwidth = 42;  /* allowable width for editing */

	initstringvector(&sv);

	/* load the vector */
	if (p->moredata == NULL | p->length <= 0) {
		/* We need to create an empty node */
		str = (unsigned char*) malloc(editwidth + 2);
		strcpy(str, "");
		pushstring(&sv, str);
	}
	else {
		/* Let's fill the node from moredata! */
		strpos = 0;
		str = (unsigned char*) malloc(editwidth + 2);
		
		for (i = 0; i < p->length; i++) {
			if (p->moredata[i] == 0x0d) {
				/* end of the line (heh); push the string and start over */
				str[strpos] = 0;
				pushstring(&sv, str);
				strpos = 0;
				str = (unsigned char*) malloc(editwidth + 2);
			}
			else if (strpos >= editwidth) {
				/* hmmm... really long line; must not have been made in ZZT... */
				/* we should probably wordwrap here, just to be nice, but no... */
				str[strpos]   = p->moredata[i];
				str[strpos+1] = 0;
				pushstring(&sv, str);
				strpos = 0;
				str = (unsigned char*) malloc(editwidth + 2);
			}
			else {
				/* just your everyday copying... */
				str[strpos++] = p->moredata[i];
			}
		}

		if (strpos > 0) {
			/* strange... we seem to have an extra line with no CR at the end... */
			str[strpos] = 0;
			pushstring(&sv, str);
		}
		else {
			/* we grabbed all that RAM for nothing. Darn! */
			free(str);
		}
	}

	/* Now that the node is full, we can edit it. */
	sv.cur = sv.first;    /* This is redundant, but hey. */
	editbox(d, "Object Editor", &sv, editwidth, 1);

	/* Okay, lets put the vector back in moredata */

	/* find out how much space we need */
	newdatalength = 0;
	/* and now for a wierdo for loop... */
	for (sv.cur = sv.first; sv.cur != NULL; sv.cur = sv.cur->next)
		newdatalength += strlen(sv.cur->s) + 1;  /* + 1 for CR */

	if (newdatalength == 1) {
		/* sv holds one empty string (it can happen) */
		p->moredata = NULL;
		p->length = 0;
		return;
	}

	/* lets make room for all that moredata */
	strpos = 0;
	str = (unsigned char*) malloc(newdatalength);

	for (sv.cur = sv.first; sv.cur != NULL; sv.cur = sv.cur->next) {
		j = strlen(sv.cur->s);   /* I feel efficient today */
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


void
editbox(displaymethod* d, char* title, stringvector* sv, int editwidth, int zochighlight)
{
	int c = 0, e = 0;       /* Char & ext flag */
	int i, j;               /* general counters */
	int updateflags;        /* flags to determine what needs update */
	stringnode* centerstr;  /* str in center of dialog */

	/* vars only relating to editing */
	int pos = 0;            /* position in sv->cur->s */
	int insertflag = 1;     /* nonzero when in insert mode */
	unsigned char* tmpstr;  /* temporary string for pushing */

	if (sv->cur == NULL) return;
	centerstr = sv->cur;

	drawscrollbox(0, 0, d);
	d->print(23,4, 0x0a, title);

	d->cursorgo(10,13);
	updateflags = U_ALL;

	while (!(c == 27 && e == 0) && !(c == 13 && e == 0 && editwidth == 0)) {

		d->cursorgo(10+pos, 13);

   	/* clear the scrollbox */
		if (updateflags & U_TOP)
			drawscrollbox(3, 9, d);
		if (updateflags & U_CENTER)
     	drawscrollbox(10, 8, d);
		if (updateflags & U_BOTTOM)
     	drawscrollbox(11, 1, d);

		if (updateflags & (U_CENTER | U_TOP)) {
      	/* Draw the center */
			if (zochighlight)
				displayzoc(d, 10, 13, centerstr->s);
			else
				d->print(10, 13, 0x0a, centerstr->s);
		}

		if (updateflags & (U_BOTTOM | U_CENTER | U_TOP)) {
			/* Draw bottom half */
			sv->cur = centerstr->next;
			for (i = 1; i < 8 && sv->cur != NULL ; i++, sv->cur = sv->cur->next)
				if (zochighlight)
					displayzoc(d, 10, i+13, sv->cur->s);
			else
				d->print(10, i+13, 0x0a, sv->cur->s);

			if (i < 8)
				if (zochighlight)
					displayzoc(d, 10, i+13, SLEADER);
			else
				d->print(10, i+13, 0x0a, SLEADER);
		}

		if (updateflags & U_TOP) {
			/* Draw top half */
			sv->cur = centerstr->prev;
			for (i = -1; i > -8 && sv->cur != NULL; i--, sv->cur = sv->cur->prev)
				if (zochighlight)
					displayzoc(d, 10, i+13, sv->cur->s);
			else
				d->print(10, i+13, 0x0a, sv->cur->s);

			if (i > -8)
				if (zochighlight)
					displayzoc(d, 10, i+13, SLEADER);
			else
				d->print(10, i+13, 0x0a, SLEADER);
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
		}
		else if (e == 1 && c == 0x50) {
			/* Down Arrow */
			if (centerstr->next != NULL) {
				centerstr = centerstr->next;
				if (pos > strlen(centerstr->s))
					pos = strlen(centerstr->s);
				updateflags = U_ALL;
			}
		}
		else if (e == 1 && c == 0x49) {
			/* Page Up */
			for (i = 0; i < 7 && centerstr->prev != NULL; i++)
				centerstr = centerstr->prev;
				if (pos > strlen(centerstr->s))
					pos = strlen(centerstr->s);
			updateflags = U_ALL;
		}
		else if (e == 1 && c == 0x51) {
			/* Page Down */
			for (i = 0; i < 7 && centerstr->next != NULL; i++)
				centerstr = centerstr->next;
				if (pos > strlen(centerstr->s))
					pos = strlen(centerstr->s);
				updateflags = U_ALL;
		}
		else if (editwidth) {
			/* We are edititing! Yea! Fun time! */
			if (c == 0x4B && e == 1) {
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
			}
			else if (c == 0x4D && e == 1) {
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
			}
			else if (c == 0x52 && e == 1) {
				/* Insert */
				insertflag = !insertflag;
			}
			else if (c == 0x53 && e == 1) {
				/* Delete */
        if (pos < strlen(centerstr->s)) {
					for (i = pos; i < strlen(centerstr->s); i++)
						centerstr->s[i] = centerstr->s[i+1];
					updateflags = U_CENTER;
				}
				else {
					/* FIXME: join next line (wordwrap) */
				}
			}
			else if (c == 0x47 && e == 1) {
				/* Home */
        pos = 0;
			}
			else if (c == 0x4F && e == 1) {
				/* End */
        pos = strlen(centerstr->s);
			}
			else if (c == 13 && e == 0) {
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
			}
			else if (c == '\b' && e == 0) {
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
					else if (strlen(centerstr->s) + strlen(centerstr->prev->s) <= editwidth) {
						pos = strlen(centerstr->prev->s);
						strcpy(centerstr->prev->s+pos, centerstr->s);
						sv->cur = centerstr;
						centerstr = centerstr->prev;
						deletestring(sv);
						updateflags = U_TOP | U_CENTER;
					}
					else {
          	/* FIXME: wordwrap */
					}
        }
			}
			else if (c == 25 && e == 0) {
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
			}
      else if (c == 27 && e == 0) {
      	/* do nothing for escape (avoids inserting esc char) */
      }
			else {
				/* Normal/weird char for insert/replace */
				if (insertflag) {
					/* insert */
					if (strlen(centerstr->s) < editwidth) {
						/* insert if there is room */
						for (i = strlen(centerstr->s) + 1; i > pos; i--)
							centerstr->s[i] = centerstr->s[i-1];
						centerstr->s[pos++] = c;
						updateflags = U_CENTER;
					}
					else {
						/* FIXME: wordwrap would be great here! */
					}
				}
				else {
					/* easy replace */
					if (centerstr->s[pos] == 0) {
						if (strlen(centerstr->s) < editwidth) {
							centerstr->s[pos+1] = 0;
							centerstr->s[pos++] = c;
							updateflags = U_CENTER;
						}
						else {
							/* FIXME: wordwrap */
						}
					}
					else {
						centerstr->s[pos++] = c;
						updateflags = U_CENTER;
					}
				} /* esle replace */
			} /* esle insert/replace */
		} /* fiesle editwidth */
	} /* elihw */

	sv->cur = centerstr;
}


void
displayzoc(displaymethod* d, int x, int y, unsigned char* s)
{
	int i = 0;                     /* position in s */
	int j = 0;                     /* position in token */
	unsigned char token[80] = "";  /* token buffer */
/* find out what we're dealing with based on the first char */
	switch (s[0]) {
		case '#':
			/* command */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
			for (i = 1; s[i] != ' ' && s[i] != 0; i++)
				token[i-1] = s[i];
			token[i-1] = 0;

			d->print(x+1, y, (iszztcommand(token)?ZOC_STDCOMMAND_COLOUR:ZOC_SENDMESSAGE_COLOUR), token);

			/* TODO: process remainder of s based on value of token */
			d->print(x+i, y, ZOC_DEFAULT_COLOUR, s+i);
			break;

		case ':':
			/* message */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
			for (i = 1; s[i] != '\'' && s[i] != 0; i++)
				token[i-1] = s[i];
			token[i-1] = 0;
			
			d->print(x+1, y, (iszztmessage(token)?ZOC_STDMESSAGE_COLOUR:ZOC_LABEL_COLOUR), token);
			
			if (s[i] == '\'')
				d->print(x+i, y, ZOC_COMMENT_COLOUR, s+i);
			break;

		case '?':
		case '/':
			/* movement */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
			for (i = 1; s[i] != '?'  && s[i] != '/' &&
					        s[i] != '\'' && s[i] != 0; i++) 
				token[i-1] = s[i];
			token[i-1] = 0;

			d->print(x+1, y, (iszztdir(token)?ZOC_STDDIR_COLOUR:ZOC_DEFAULT_COLOUR), token);

			/* Recursively display next movement/comment if there is one */
			if (s[i] != 0)
				displayzoc(d, x+i, y, s+i);
			
			break;

		case '!':
			/* hypermessage */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
			for (i = 1; s[i] != ';' && s[i] != 0; i++)
				d->putch(x+i, y, s[i], ZOC_SENDMESSAGE_COLOUR);

			if (s[i] == 0) break;

			d->putch(x+i, y, s[i], ZOC_OPERATOR_COLOUR);
			for (i++; s[i] != '\'' && s[i] != 0; i++)
				d->putch(x+i, y, s[i], ZOC_HYPER_COLOUR);

			if (s[i] == '\'')
				d->print(x+i, y, ZOC_COMMENT_COLOUR, s+i);
			break;

		case '\'':
			/* comment */
			d->print(x, y, ZOC_COMMENT_COLOUR, s);
			break;

		case '$':
			/* heading */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
			d->print(x+1, y, ZOC_HEADING_COLOUR, s+1);
			break;

		case '@':
			/* objectname */
			d->putch(x, y, s[0], ZOC_OPERATOR_COLOUR);
			d->print(x+1, y, ZOC_OBJNAME_COLOUR, s+1);
			break;
		
		case ' ':
			/* indented comment? */
			for (i = 1; s[i] == ' '; i++)
				;
			d->print(x, y, (s[i]=='\''?ZOC_COMMENT_COLOUR:ZOC_TEXT_COLOUR), s);
		default:
			/* normal text */
			d->print(x, y, ZOC_TEXT_COLOUR, s);
			break;
	}
}


/* zzt components for special highlighting */

#define ZZTCOMMANDCOUNT 27
const char zztcommands[ZZTCOMMANDCOUNT][12] = {
	"become",  "bind",    "change", "char",  "clear", "cycle", "die",  "end",
	"endgame", "give",    "go",     "idle",  "if",    "lock",  "play", "put",
	"restart", "restore", "send",   "set",   "shoot", "take",  "throwstar",
	"try",     "unlock",  "walk",   "zap"
};

#define ZZTMESSAGECOUNT 5
const char zztmessages[ZZTMESSAGECOUNT][10] = {
	"touch", "shot", "bombed", "thud", "energize"
};

#define ZZTFLAGCOUNT 5
const char zztflags[ZZTFLAGCOUNT][12] = {
	"alligned", "contact", "blocked", "energized", "exists"
};

#define ZZTITEMCOUNT 5
const char zztitems[ZZTITEMCOUNT][8] = {
	"ammo", "gems", "torches", "health", "score"
};

#define ZZTKINDCOUNT 1
const char zztkinds[ZZTKINDCOUNT][12] = {
	"key"
};

#define ZZTCOLOURCOUNT 7
const char zztcolours[ZZTCOLOURCOUNT][8] = {
	"blue", "green", "red", "cyan", "purple", "yellow", "white"
};

#define ZZTDIRCOUNT 14
const char zztdirs[ZZTDIRCOUNT][6] = {
	"n", "north", "s", "south", "e", "east", "w", "west", "i", "idle",
	"seek", "flow", "rndns", "rndne"
};

#define ZZTDIRMODCOUNT 4
const char zztdirmods[ZZTDIRMODCOUNT][4] = {
	"cw", "ccw", "rndp", "opp"
};


/* token testers */
int
iszztcommand(unsigned char* token)
{
	return 1;
}

int iszztmessage(unsigned char* token)
{
	return 1;
}

int iszztflag(unsigned char* token)
{
	return 1;
}

int iszztitem(unsigned char* token)
{
	return 1;
}

int iszztkind(unsigned char* token)
{
	return 1;
}

int iszztdir(unsigned char* token)
{
	return 1;
}

