/* editbox.c  -- text editor/viewer in kevedit
 * $Id: editbox.c,v 1.11 2000/10/20 02:17:18 bitman Exp $
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
#include <stdio.h>
#include <string.h>

#include "scroll.h"
#include "colours.h"
#include "panel_ed.h"
#include "zzm.h"


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

#define ZOC_STDCOMMAND_COLOUR  GREEN_F
#define ZOC_STDITEM_COLOUR     WHITE_F | BRIGHT_F
#define ZOC_STDKIND_COLOUR     WHITE_F | BRIGHT_F
#define ZOC_STDDIR_COLOUR      WHITE_F | BRIGHT_F
#define ZOC_STDMESSAGE_COLOUR  MAGENTA_F | BRIGHT_F
#define ZOC_STDLABEL_COLOUR    RED_F | BRIGHT_F
#define ZOC_STDFLAG_COLOUR     YELLOW_F | BRIGHT_F

#define ZOC_MESSAGE_COLOUR     MAGENTA_F
#define ZOC_LABEL_COLOUR       RED_F
#define ZOC_FLAG_COLOUR        YELLOW_F

/* token testing functions */
int iszztcommand(char *token);
int iszztmessage(char *token);
int iszztflag(char *token);
int iszztitem(char *token);
int iszztkind(char *token);
int iszztdir(char *token);
int iszztcolour(char *token);

/* needed to use filedialog */
extern char filelist[500][13];


/***** strequ() *****************/
/* TODO: Move this function to a more appropriate file & add prototype */

#include <malloc.h>

#define STREQU_UNCASE  0x01
#define STREQU_FRONT   0x02

int strequ(const char *str1, const char *str2, int flags)
{
	char *lwr1, *lwr2;
	int i;
	int isequ = 1;		/* Strings are equal until proven otherwise */

	if (str1[0] == 0 && str2[0] == 0)
		return 1;
   else if (str1[0] == 0 || str2[0] == 0)
   	return 0;

	lwr1 = (char *) malloc(strlen(str1) * sizeof(char) + 1);
	lwr2 = (char *) malloc(strlen(str2) * sizeof(char) + 1);
	if (lwr1 == NULL || lwr2 == NULL)
		return -1;

	strcpy(lwr1, str1);
	strcpy(lwr2, str2);

	if (flags & STREQU_UNCASE) {
		strlwr(lwr1);
		strlwr(lwr2);
	}

	for (i = 0; lwr1[i] != 0 && lwr2[i] != 0; i++)
		if (lwr1[i] != lwr2[i]) {
			isequ = 0;
			break;
		}

	/* Strings must be equal */
	if (lwr1[i] != lwr2[i] && !(flags & STREQU_FRONT))
		isequ = 0;

	free(lwr1);
	free(lwr2);

	return isequ;
}


/***** draweditpanel() ***********/

void draweditpanel(int insertflag, int wrapwidth, int zocformatting, displaymethod * d)
{
	int x, y, i = 0;
	char buf[10] = "";

	for (y = 3; y < PANEL_EDIT_DEPTH + 3; y++) {
		for (x = 0; x < PANEL_EDIT_WIDTH; x++) {
			d->putch(x + 60, y, PANEL_EDIT[i], PANEL_EDIT[i + 1]);
			i += 2;
		}
	}
	
	d->print(76, 6,  YELLOW_F | BRIGHT_F | BLUE_B, (insertflag ? "on" : "off"));
	d->print(76, 10, YELLOW_F | BRIGHT_F | BLUE_B, (zocformatting ? "on" : "off"));

	sprintf(buf, "%d", wrapwidth);

	if (wrapwidth)
		d->print(76, 8, YELLOW_F | BRIGHT_F | BLUE_B, buf);
	else
		d->print(72, 8, YELLOW_F | BRIGHT_F | BLUE_B, "off");
}


/***** editmoredata() *********/

void editmoredata(param * p, displaymethod * d)
{
	stringvector sv;		/* list of strings */
	char *str = NULL;	/* temporary string */
	int strpos = 0;			/* position in str */
	int newdatalength = 0;		/* when writing, size of moredata */
	int i = 0, j = 0;		/* general counters */
	const int editwidth = 42;	/* allowable width for editing */

	initstringvector(&sv);

	/* load the vector */
	if (p->moredata == NULL | p->length <= 0) {
		/* We need to create an empty node */
		str = (char *) malloc(editwidth + 2);
		strcpy(str, "");
		pushstring(&sv, str);
	} else {
		/* Let's fill the node from moredata! */
		strpos = 0;
		str = (char *) malloc(editwidth + 2);

		for (i = 0; i < p->length; i++) {
			if (p->moredata[i] == 0x0d) {
				/* end of the line (heh); push the string and start over */
				str[strpos] = 0;
				pushstring(&sv, str);
				strpos = 0;
				str = (char *) malloc(editwidth + 2);
			} else if (strpos > editwidth) {
				/* hmmm... really long line; must not have been made in ZZT... */
				/* let's truncate! */
				str[strpos] = 0;
				pushstring(&sv, str);
				strpos = 0;
				str = (char *) malloc(editwidth + 2);
				/* move to next 0x0d */
				do i++; while (i < p->length && p->moredata[i] != 0x0d);

				/* code for splitting lines (not in use) */
				/*
				str[strpos] = p->moredata[i];
				str[strpos + 1] = 0;
				pushstring(&sv, str);
				strpos = 0;
				str = (char *) malloc(editwidth + 2);
				*/

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
	editbox("Object Editor", &sv, editwidth, 1, d);

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
	str = (char *) malloc(newdatalength);

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


/*
 * if not zocformatting
 *   d->print in green
 * else
 *   if editmode
 *     displayzoc without format
 *   else
 *     displayzoc with format
 */

/* how to display a line of text */
#define displayline(x, y, s, edit, format, firstline, d) ((format) ? displayzoc((x), (y), (s), !(edit), (firstline), (d)) : d->print((x), (y), ZOC_TEXT_COLOUR, (s)))


int editbox(char *title, stringvector * sv, int editwidth, int zocformatting, displaymethod * d)
{
	int c = 0, e = 0;	/* Char & ext flag */
	int i, j;		/* general counters */
	int done = 0;		/* true when editing/viewing is done */
	int updateflags;	/* flags to determine what needs update */
	stringnode *centerstr;	/* str in center of dialog */

	/* vars only relating to editing */
	int pos = 0;			/* position in sv->cur->s */
	char *tmpstr;		/* temporary string for pushing */
	char strbuf[80] = "";  /* general buffer */

	/* statics */
	static int insertflag = 1;	/* nonzero when in insert mode */
	static int wrapwidth = 42;	/* where to wrap */

	/* if there is no string, add one */
	if (sv->cur == NULL || sv->first == NULL || sv->last == NULL)
		pushstring(sv, strcpy((char *) malloc(editwidth + 2), ""));

	if (sv->cur == NULL)
		return 0;

	centerstr = sv->cur;

	if (!editwidth) {
		/* It would be nice if we could hide the cursor here. */
		d->cursorgo(9, 13);
		if (zocformatting && sv->first->s[0] == '@' && centerstr == sv->first)
			centerstr = centerstr->next;
	}
	
	if (centerstr == NULL)
		return 0;

	drawscrollbox(0, 0, d);
	updateflags = U_ALL;

	while (!done) {

		if (editwidth)
			d->cursorgo(9 + pos, 13);

		/* update title & panel if needed */
		if (updateflags & U_TITLE) {
			drawscrollbox(1, 17, d);
			d->print(30 - (strlen(title) / 2), 4, 0x0a, title);
		}
		if (updateflags & U_PANEL && editwidth)
			draweditpanel(insertflag, wrapwidth, zocformatting, d);

		/* clear the scrollbox */
		if (updateflags & U_TOP)
			drawscrollbox(3, 9, d);
		if (updateflags & U_CENTER)
			drawscrollbox(10, 8, d);
		if (updateflags & U_BOTTOM)
			drawscrollbox(11, 1, d);

		if (updateflags & (U_CENTER)) {
			/* Draw the center */
			displayline(9, 13, centerstr->s, editwidth, zocformatting, centerstr->prev == NULL, d);
		}
		if (updateflags & (U_BOTTOM)) {
			/* Draw bottom half */
			sv->cur = centerstr->next;
			for (i = 1; i < 8 && sv->cur != NULL; i++, sv->cur = sv->cur->next)
				displayline(9, i + 13, sv->cur->s, editwidth, zocformatting, 0, d);

			if (i < 8)
				d->print(9, i + 13, 0x07, SLEADER);
		}
		if (updateflags & U_TOP) {
			/* Draw top half */
			sv->cur = centerstr->prev;
			for (i = -1; i > -8 && sv->cur != NULL; i--, sv->cur = sv->cur->prev)
				displayline(9, i + 13, sv->cur->s, editwidth, zocformatting, sv->cur->prev == NULL, d);

			if (!editwidth && sv->cur == NULL && sv->first->s[0] == '@')
				i++;

			if (i > -8)
				d->print(9, i + 13, 0x07, SLEADER);
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
			if (centerstr->prev != NULL && !(!editwidth && centerstr->prev == sv->first && sv->first->s[0] == '@')) {
				centerstr = centerstr->prev;
				if (pos > strlen(centerstr->s))
					pos = strlen(centerstr->s);
				updateflags = U_EDITAREA;
			}
		} else if (e == 1 && c == 0x50) {
			/* Down Arrow */
			if (centerstr->next != NULL) {
				centerstr = centerstr->next;
				if (pos > strlen(centerstr->s))
					pos = strlen(centerstr->s);
				updateflags = U_EDITAREA;
			}
		} else if (e == 1 && c == 0x49) {
			/* Page Up */
			for (i = 0; i < 7 && centerstr->prev != NULL && !(!editwidth && centerstr->prev == sv->first && sv->first->s[0] == '@'); i++)
				centerstr = centerstr->prev;
			if (pos > strlen(centerstr->s))
				pos = strlen(centerstr->s);
			updateflags = U_EDITAREA;
		} else if (e == 1 && c == 0x51) {
			/* Page Down */
			for (i = 0; i < 7 && centerstr->next != NULL; i++)
				centerstr = centerstr->next;
			if (pos > strlen(centerstr->s))
				pos = strlen(centerstr->s);
			updateflags = U_EDITAREA;
		} else if (e == 0 && c == 27) {
			done = c;
		} else if (!editwidth) {
			if (e == 0 && c == 13)
				done = c;
		} else {
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
								updateflags = U_EDITAREA;
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
								updateflags = U_EDITAREA;
							}
							pos = 0;
						}
						break;

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
						else {
							/* TODO: join next line (wordwrap) */
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
					
					case 44:
						/* alt-z - toggle ZOC mode */
						zocformatting = !zocformatting;
						updateflags = U_PANEL | U_EDITAREA;
						break;

					case 130:
						/* alt - */
						if (wrapwidth > 0)
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
							wrapwidth = 0;
						updateflags = U_PANEL;
						break;

					case 24:
						/* alt+o: open file */
					case 23:
						/* alt+i: insert file */
						{
							stringvector filetypelist;
							int listpos = -1;
							
							initstringvector(&filetypelist);

							pushstring(&filetypelist, "*.zoc");
							pushstring(&filetypelist, "*.txt");
							pushstring(&filetypelist, "*.hlp");
							pushstring(&filetypelist, "*.*");
							if (editbox("Select A File Type", &filetypelist, 0, 1, d) == 27) {
								updateflags = U_EDITAREA | U_TITLE;
								break;
							}

							if (filetypelist.cur != NULL)
								listpos = filedialog(filetypelist.cur->s + 2, (c == 24 ? "Open ZZT Object Code (ZOC) File" : "Insert ZZT Object Code (ZOC) File"), d);

							removestringvector(&filetypelist);

							if (listpos != -1) {
								stringvector newsvector;
								newsvector = filetosvector(filelist[listpos], wrapwidth, editwidth);
								if (newsvector.first != NULL) {
									if (c == 24) {
										/* erase & replace sv */
										deletestringvector(sv);
										memcpy(sv, &newsvector, sizeof(stringvector));
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
							}		/* fi listpos */
						}			/* block */
						updateflags = U_EDITAREA | U_TITLE;
						break;

					case 31:
						/* alt-s: save to file */
						{
							static char savefilename[15] = "temp.zoc";
							if (filenamedialog(savefilename, "Save Object Code As", "", 1, d) != NULL)
								svectortofile(sv, savefilename);
							updateflags = U_EDITAREA | U_PANEL;
						}
						break;

					case 50:
						/* alt-m: load .zzm music */
						{
							int listpos = filedialog("zzm", "Choose ZZT Music (ZZM) File", d);
							if (listpos != -1) {
								stringvector zzmv;
								zzmv = filetosvector(filelist[listpos], 80, 80);
								if (zzmv.first != NULL) {
									stringvector song;
									song = zzmpullsong(&zzmv, zzmpicksong(&zzmv, d));
									if (song.first != NULL) {
										/* copy song into sv */
										sv->cur = centerstr;
										for (song.cur = song.first; song.cur != NULL; song.cur = song.cur->next) {
											tmpstr = (char*) malloc(editwidth + 2);

											if (zocformatting) {
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

					default:
						/* act as if ext key is really not. This way, people used to
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
						done = 1;
						break;

					case 1: 
						/* ctrl-a: insert ascii char/decimal-value */
						strcpy(strbuf, centerstr->s);
						strlwr(strbuf);
						updateflags = U_EDITAREA;

//						if ((char*)strstr(strbuf, "#char") == strbuf) {
						if (strequ(strbuf, "#char", STREQU_UNCASE | STREQU_FRONT)) {
							/* append dec value for ascii char */

							c = charselect(d);
							centerstr->s[5] = ' ';
							centerstr->s[6] = 0;

							/* change c to a string */
							sprintf(strbuf, "%d", c);
							strcat(centerstr->s, strbuf);
							pos = strlen(centerstr->s);
							updateflags = U_EDITAREA;
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
	int k = 0;			/* length of subtoken */
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
			k = iszztdir(s+1);

			if (k) {
				for (i = 1; i <= k; i++)
					d->putch(x + i, y, s[i], ZOC_STDDIR_COLOUR);

				/* Recursiveness is an art. */
				if (s[i] == '/' || s[i] == '?' || s[i] == '\'' || s[i] == ' ')
					displayzoc(x + i, y, s + i, format, 0, d);
				else
					d->print(x + i, y, ZOC_DEFAULT_COLOUR, s + i);
			} else {
				d->print(x + 1, y, ZOC_DEFAULT_COLOUR, s + 1);
			}

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
		if (strequ(zztcommands[t], command, STREQU_UNCASE))
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
				if (strequ(token, "not", STREQU_UNCASE)) {
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
				if (strequ(token, "blocked", STREQU_UNCASE)) {
					k = tokenadvance(token, args, &j);
					ctax = CTAX_DIRECTION;
				}
				if (strequ(token, "any", STREQU_UNCASE)) {
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
				if (strequ(token, "then", STREQU_UNCASE)) {
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
				while (!(iszztdir(token) == strlen(token)) && j < strlen(args))
					k = tokengrow(token, args, &j);
				if (iszztdir(token) == strlen(token))
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


/* token testers */
int iszztcommand(char *token)
{
	int i = 0;
	char buffer[41] = "";

	memcpy(buffer, token, 40);
	buffer[40] = 0;
	strlwr(buffer);

	for (i = 0; i < ZZTCOMMANDCOUNT; i++)
		if (strequ(buffer, zztcommands[i], STREQU_UNCASE))
			return 1;

	return 0;
}

int iszztmessage(char *token)
{
	int i = 0;
	char buffer[41] = "";

	memcpy(buffer, token, 40);
	buffer[40] = 0;
	strlwr(buffer);

	for (i = 0; i < ZZTMESSAGECOUNT; i++)
		if (strequ(buffer, zztmessages[i], STREQU_UNCASE))
			return 1;

	return 0;
}

int iszztflag(char *token)
{
	int i = 0;
	char buffer[41] = "";

	memcpy(buffer, token, 40);
	buffer[40] = 0;
	strlwr(buffer);

	for (i = 0; i < ZZTFLAGCOUNT; i++)
		if (strequ(buffer, zztflags[i], STREQU_UNCASE))
			return 1;

	return 0;
}

int iszztitem(char *token)
{
	int i = 0;
	char buffer[41] = "";

	memcpy(buffer, token, 40);
	buffer[40] = 0;
	strlwr(buffer);

	for (i = 0; i < ZZTITEMCOUNT; i++)
		if (strequ(buffer, zztitems[i], STREQU_UNCASE))
			return 1;

	return 0;
}

int iszztkind(char *token)
{
	int i = 0;
	char buffer[41] = "";

	memcpy(buffer, token, 40);
	buffer[40] = 0;
	strlwr(buffer);

	for (i = 0; i < ZZTCOLOURCOUNT; i++)
		if (strequ(buffer, zztcolours[i], STREQU_UNCASE | STREQU_FRONT)) {
			/* Advance token to nearest space */
			while (token[0] != ' ' && token[0] != 0) token++;
			/* Advance token to nearest nonspace */
			while (token[0] == ' ') token++; 

			/* now see if next word is a valid token */
			return iszztkind(token);
		}

	for (i = 0; i < ZZTKINDCOUNT; i++)
		if (strequ(buffer, zztkinds[i], STREQU_UNCASE))
			return 1;

	return 0;
}

int iszztdir(char *token)
{
	int i = 0;
	char buffer[41] = "";

	memcpy(buffer, token, 40);
	buffer[40] = 0;
	strlwr(buffer);

	for (i = 0; i < ZZTDIRMODCOUNT; i++)
		if (strequ(buffer, zztdirmods[i], STREQU_UNCASE | STREQU_FRONT)) {
			int modlen = 0;
			/* Advance token to nearest space */
			while (token[0] != ' ' && token[0] != 0) { token++; modlen++; } 
			/* Advance token to nearest nonspace */
			while (token[0] == ' ') { token++; modlen++; } 

			/* now see if next word is a valid token */
			i = iszztdir(token);
			return (i ? modlen + i : 0);
		}

	for (i = 0; i < ZZTDIRCOUNT; i++)
		if (strequ(buffer, zztdirs[i], STREQU_UNCASE | STREQU_FRONT))
			return strlen(zztdirs[i]);

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
	int i, j;

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

		str = (char *) malloc(editwidth + 2);
		if (str == NULL) {
			fclose(fp);
			return v;
		}

		if (strpos < wrapwidth) {
			/* simple copy */
			strcpy(str, buffer);
			pushstring(&v, str);
		} else {
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

