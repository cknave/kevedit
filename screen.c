/* screen.c    -- Functions for drawing
 * $Id: screen.c,v 1.30 2001/11/09 01:15:09 bitman Exp $
 * Copyright (C) 2000 Kev Vance <kev@kvance.com>
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

#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "screen.h"
#include "display.h"
#include "kevedit.h"
#include "zzt.h"
#include "scroll.h"
#include "editbox.h"
#include "hypertxt.h"
#include "zlaunch.h"

#include "panel.h"
#include "panel_f1.h"
#include "panel_f2.h"
#include "panel_f3.h"
#include "tbox.h"
#include "cbox.h"

/* The following define tells updatepanel to draw the standard patterns
 * in the current colour, rather than plain ol' white */
/* #define STDPATFOLLOWCOLOR */

/* Backbuffer scroll start - where in the backbuffer to start scrolling
 * when bb is larger than visible width */
#define BBSCROLLSTART 7
#define BBVWIDTH      10

int line_editor(int x, int y, int color,
								char* str, int editwidth, int flags, displaymethod* d)
{
	int pos = strlen(str);   /* Position in str */
	int key;                 /* Key being acted on */

	while (1) {
		/* Call the raw line editor */
		key = line_editor_raw(x, y, color, str, editwidth, &pos, flags, d);

		/* Look for return-inducing keys */
		switch (key) {
			case DKEY_ENTER:
				return LINED_OK;

			case DKEY_ESC:
				return LINED_CANCEL;
		}
	}
}

int line_editor_raw(int x, int y, int color, char* str, int editwidth,
										int* position, int flags, displaymethod* d)
{
	int key;                 /* Key being acted on */
	int i;                   /* General counter */
	int pos = *position;     /* Current position */

	while (1) {
		/* Display the line */
		d->print(x, y, color, str);
		for (i = strlen(str); i <= editwidth; i++)
			d->putch(x + i, y, ' ', color);

		/* Move the cursor */
		d->cursorgo(x + pos, y);

		/* Get the key */
		key = d->getch();
		if (key == 0)
			key = d->getch() | DDOSKEY_EXT;

		switch (key) {
			case DKEY_LEFT:  if (pos > 0)           pos--; break;
			case DKEY_RIGHT: if (pos < strlen(str)) pos++; break;
			case DKEY_HOME:  pos = 0;                      break;
			case DKEY_END:   pos = strlen(str);            break;

			case DKEY_BACKSPACE:
				/* Move everything after pos backward */
				if (pos > 0) {
					for (i = pos - 1; i < strlen(str); i++)
						str[i] = str[i + 1];
					pos--;
				}
				break;

			case DKEY_DELETE:
				if (pos < strlen(str)) {
					for (i = pos; i < strlen(str); i++)
						str[i] = str[i+1];
				}
				break;

			case DKEY_CTRL_Y:
				/* Clear line */
				pos = 0;
				str[0] = '\0';
				break;

			case DKEY_TAB:
				/* Insert 4 spaces */
				if (strlen(str) > (editwidth - 4))
					break;

				for (i = strlen(str) + 4; i > pos; i--)
					str[i] = str[i-4];
				for (i = 0; i < 4; i++)
					str[pos++] = ' ';
				break;

			default:
				/* Keys outside the standard ASCII range are returned for
				 * consideration by the calling function */
				if (key < 0x20 || key > 0x7E) {
					*position = pos;
					return key;
				}

				/* Be sure we have room */
				if (strlen(str) >= editwidth)
					break;

				/* Act on flags */
				if ((flags & LINED_NOLOWER) && (flags & LINED_NOUPPER) &&
						((key >= 0x41 && key <= 0x5A) || (key >= 0x61 && key <= 0x7A))) break;
				if ((flags & LINED_NODIGITS) && (key >= 0x30 && key <= 0x39)) break;
				if ((flags & LINED_NOPUNCT) && ((key >= 0x21 && key <= 0x2F) ||
																				(key >= 0x3A && key <= 0x40) ||
																				(key >= 0x5A && key <= 0x60) ||
																				(key >= 0x7B && key <= 0x7E))) break;
				if ((flags & LINED_NOSPACES) && (key == ' ')) break;
				if ((flags & LINED_NOPERIOD) && (key == '.')) break;
				if ((flags & LINED_FILENAME) && (key == '\"' || key == '?' || key == '*' ||
																				 key == '<'  || key == '>' || key == '|')) break;
				if ((flags & LINED_NOPATH)   && (key == '\\' || key == '/' || key == ':')) break;

				if (flags & LINED_NOUPPER) key = tolower(key);
				if (flags & LINED_NOLOWER) key = toupper(key);

				/* Insert character */
				for (i = strlen(str) + 1; i > pos; i--)
					str[i] = str[i-1];
				str[pos++] = key;
				break;
		}
	}
}

int line_editnumber(int x, int y, int color, int * number, int maxval,
                    displaymethod* d)
{
	char* buffer;
	int editwidth = 0;
	int factor;

	for (factor = 1; factor < maxval; factor *= 10)
		editwidth++;

	if (editwidth == 0)
		editwidth = 1;

	buffer = (char *) malloc(sizeof(char) * (editwidth + 1));

	sprintf(buffer, "%d", *number);
	if (line_editor(x, y, color, buffer, editwidth,
									LINED_NOALPHA | LINED_NOPUNCT | LINED_NOSPACES, d)
			== LINED_OK) {
		sscanf(buffer, "%d", number);
		if (*number > maxval)
			*number = maxval;
		free(buffer);
		return LINED_OK;
	}

	free(buffer);
	return LINED_CANCEL;
}

/* filedialog() - Prompts user to enter a file name, returns a malloc()ed
 *                      value representing entered value, NULL on cancel. */
char* filenamedialog(char* initname, char* extension, char* prompt,
													 int askoverwrite, displaymethod * mydisplay)
{
	const int maxlen = 15;   /* 15 chars should be enough for a zzt filename */
	int extlen = strlen(extension);   /* length of given extension */
	int pos;                 /* editing position */
	char* filenamebuffer;
	char* path = NULL;
	char* result = NULL;
	int done = 0;
	int i, x;
	
	if (extlen > 3)
		return NULL;

	/* Reserve some space */
	filenamebuffer = (char *) malloc(sizeof(char) * (maxlen + 1));
	path = (char *) malloc(sizeof(char) * (strlen(initname) + 1));

	/* Parse the initial filename */
	fileof(filenamebuffer, initname, maxlen + 1);
	pathof(path, initname, strlen(initname) + 1);

	/* if extension is given, remove extension from buffer */
	if (extlen > 0)
		for (i = strlen(filenamebuffer) - 1; i >= 0; i--)
			if (filenamebuffer[i] == '.') {
				filenamebuffer[i] = '\x0';
				break;
			}

	/* Clear the panel area */
	for (i = 3; i < 25; i++) {
		for (x = 0; x < 20; x++) {
			mydisplay->putch(x + 60, i, ' ', 0x1f);
		}
	}

	/* Display the prompt */
	if (strlen(prompt) < 20)
		mydisplay->print(61, 3, 0x1f, prompt);

	/* Display the extension if static */
	if (extlen > 0) {
		mydisplay->putch(70, 4, '.', 0x1f);
		mydisplay->print(71, 4, 0x1f, extension);
	}

	/* TODO: allow directory switching */

	pos = strlen(filenamebuffer);

	/* Edit */
	while (!done) {
		int key;  /* Key returned from line editor */

		/* Edit the filename and check return value */
		if (extlen > 0) {
			key = line_editor_raw(61, 4, 0x0f, filenamebuffer, 8, &pos,
													 LINED_FILENAME | LINED_NOPERIOD, mydisplay);
		} else {
			key = line_editor_raw(61, 4, 0x0f, filenamebuffer, 12, &pos,
														LINED_FILENAME, mydisplay);
		}
		switch (key) {
			case DKEY_ENTER:   /* enter: prepare file name and finish */
				/* Don't allow files without names */
				if (strlen(filenamebuffer) == 0)
					break;

				if (extlen > 0) {
					strcat(filenamebuffer, ".");
					strcat(filenamebuffer, extension);
				}

				result = fullpath(path, filenamebuffer, SLASH_DEFAULT);

				if (askoverwrite && !access(result, F_OK)) {
					/* File already exists, confirm overwrite */

					/* TODO: find a better way to present this */
					mydisplay->print(61, 5, 0x1f, "Overwrite?");
					mydisplay->print(72, 5, 0x1e, "y/n");
					do {
						key = mydisplay->getch();
					} while (!(key == 'y' || key == 'Y' || key == 'n' || key == 'N' ||
										 key == DKEY_ESC));
					mydisplay->print(61, 5, 0x1f, "          ");
					mydisplay->print(72, 5, 0x1f, "   ");

					if (key != 'y' && key != 'Y') {
						/* Unless user chose to overwrite, prompt for a different file */
						free(result);
						result = NULL;

						/* Truncate at the period we appended a few moments ago */
						strrchr(filenamebuffer, '.')[0] = '\x0';
						break;
					}
				}
				/* No break */

			case DKEY_ESC:     /* esc: cancel filename dialog */
				done = 1;
				break;

			case DKEY_CTRL_D:  /* ctrl-d: change directory */
			case DKEY_CTRL_F:  /* ctrl-f: change folder */
				{
					char* newpath;

					newpath =
						filedialog(path, "", "Choose a Path (arrows nav, enter selects)",
											 FTYPE_DIR, mydisplay);

					if (newpath != NULL) {
						/* Change the path if the user chose a directory */
						free(path);
						path = newpath;
					}
				}
				break;
		}
	}

	/* Memory cleanup */
	free(filenamebuffer);
	free(path);
	
	return result;
}


void drawscrollbox(int yoffset, int yendoffset, displaymethod * mydisplay)
{
	int t, x, i;
	/* start at yoffset */
	i = yoffset * SCROLL_BOX_WIDTH * 2;

	for (t = 3 + yoffset; t < 3 + (SCROLL_BOX_DEPTH - yendoffset); t++) {
		for (x = 4; x < 4 + SCROLL_BOX_WIDTH; x++) {
			mydisplay->putch(x, t, SCROLL_BOX[i], SCROLL_BOX[i + 1]);
			i += 2;
		}
	}
}

void drawpanel(displaymethod * d)
{
	int x, y, i = 0;

	for (y = 0; y < 25; y++) {
		for (x = 0; x < 20; x++) {
			d->putch(x + 60, y, PANEL_MAIN[i], PANEL_MAIN[i + 1]);
			i += 2;
		}
	}
}

void updatepanel(displaymethod * d, editorinfo * e, world * w)
{
	int i, x;
	char s[255];

	/* (x, y) position */
	d->putch(62, 0, ' ', 0x1f);
	d->putch(63, 0, ' ', 0x1f);
	d->putch(76, 0, ' ', 0x1f);
	d->putch(77, 0, ' ', 0x1f);
	sprintf(s, "(%d, %d) %d/150", e->cursorx + 1, e->cursory + 1, w->board[e->curboard]->info->objectcount);
	i = 70 - strlen(s) / 2;
	for (x = 0; x < strlen(s); x++) {
		d->putch(i + x, 0, s[x], 0x1c);
	}

	/* Draw Mode? */
	if (e->drawmode == 0)
		i = 0x1f;
	else if (e->gradmode == 0)
		i = 0x1e;
	else
		i = 0x1c;
	d->print(69, 15, i, "Draw");

	/* Blink Mode? */
	if (e->blinkmode == 0)
		i = 0x1f;
	else
		i = 0x1e;
	d->print(65, 19, i, "Blink");

	/* Text entry Mode? */
	if (e->textentrymode == 0)
		i = 0x1f;
	else
		i = 0x9e;
	d->print(69, 12, i, "Enter Text");

	/* Arrows to point at current colour */
	for (i = 61; i < 77; i++) {
		d->putch(i, 22, ' ', 0x1f);
	}
	for (i = 61; i < 77; i++) {
		d->putch(i, 24, ' ', 0x1f);
	}
	for (i = 61; i < 78; i++) {
		d->putch(i, 20, ' ', 0x1f);
	}
	d->putch(61 + e->forec, 22, 31, 0x17);
	d->putch(69 + e->backc, 24, 30, 0x17);

	/* Default colour mode? */
	if (e->defc == 1)
		d->putch(78, 23, 'D', 0x1e);
	else
		d->putch(78, 23, 'd', 0x18);

	/* Get mode? */
	if (e->aqumode == 1)
		d->putch(78, 21, 'A', 0x1e);
	else
		d->putch(78, 21, 'a', 0x18);

	/* Too long title */
	if (strlen(e->currenttitle) > 8) {
		for (x = 0; x < 5; x++) {
			d->putch(71 + x, 1, e->currenttitle[x], 0x17);
		}
		for (x = 0; x < 3; x++) {
			d->putch(76 + x, 1, '.', 0x1f);
		}
	} else {
		/* Regular title */
		d->print(71, 1, 0x17, e->currenttitle);
	}

	strcpy(s, "KevEdit - ");
	strncpy(&s[10], e->currenttitle, 244);
	d->titlebar(s);

#ifdef STDPATFOLLOWCOLOR
	/* Draw standard patterns in all their colourful grandure */
	for (i = 0; i < e->standard_patterns->size; i++) {
		patdef pattern = e->standard_patterns->patterns[i];
		d->putch(61 + i, 21,
						 z_getchar(pattern.type, pattern.color, pattern.patparam,
											 NULL, 0, 0),
						 z_getcolour(pattern.type, pattern.color, pattern.patparam));
	}
#endif

	/* Pattern arrow */
	if (e->pbuf == e->standard_patterns) {
		d->putch(61 + e->pbuf->pos, 20, 31, 0x17);
		x = 0;
	} else {
		x = min(e->backbuffer->pos - BBSCROLLSTART, e->backbuffer->size - BBVWIDTH);
		if (x < 0)
			x = 0;
		d->putch(68 + e->pbuf->pos - x, 20, 31, 0x17);
	}

	/* Draw pattern stack */
	for (i = 0; i < BBVWIDTH && i + x < e->backbuffer->size; i++) {
		patdef pattern = e->backbuffer->patterns[i + x];
		d->putch(68 + i, 21,
						 z_getchar(pattern.type, pattern.color, pattern.patparam,
											 NULL, 0, 0),
						 z_getcolour(pattern.type, pattern.color, pattern.patparam));
	}
	/* Start where we left off and fill the rest w/ blue solids */
	for (; i < BBVWIDTH; i++) {
		d->putch(68 + i, 21, ' ', 0x1F);
	}
}

void drawscreen(displaymethod * d, world * w, editorinfo * e, char *bigboard, unsigned char paramlist[60][25])
{
	int x, y, i = 0;

	for (y = 0; y < 25; y++) {
		for (x = 0; x < 60; x++) {
			d->putch(x, y,
							 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y]], bigboard, x, y),
							 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y]])
							);
			i += 2;
		}
	}
}

/* Make the cursor more visible */
void cursorspace(displaymethod * d, world * w, editorinfo * e, char *bigboard, unsigned char paramlist[60][25])
{
	char c, b, f;
	int i = (e->cursorx + e->cursory * 60) * 2;
	c = z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[e->cursorx][e->cursory]]);
	f = c & 0x0f;
	b = (c & 0xf0) >> 4;
	if (f < 8)
		f += 8;
	else
		f -= 8;
	if (f == b)
		c = 7;
	else
		c = (b << 4) + f;
	d->putch(e->cursorx, e->cursory, z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[e->cursorx][e->cursory]], bigboard, e->cursorx, e->cursory), c);
}

/* Update a spot around the cursor */
void drawspot(displaymethod * d, world * w, editorinfo * e, char *bigboard, unsigned char paramlist[60][25])
{
	int x, y, i;
	x = e->cursorx;
	y = e->cursory;

	if (y - 1 >= 0) {
		if (x - 1 >= 0) {
			i = ((x - 1) + (y - 1) * 60) * 2;
			d->putch(x - 1, y - 1,
							 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x - 1][y - 1]], bigboard, x - 1, y - 1),
							 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x - 1][y - 1]])
							);
		}
		i = (x + (y - 1) * 60) * 2;
		d->putch(x, y - 1,
						 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y - 1]], bigboard, x, y - 1),
						 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y - 1]])
						);
		if (x + 1 < 60) {
			i = ((x + 1) + (y - 1) * 60) * 2;
			d->putch(x + 1, y - 1,
							 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x + 1][y - 1]], bigboard, x + 1, y - 1),
							 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x + 1][y - 1]])
							);
		}
	}
	if (x - 1 >= 0) {
		i = ((x - 1) + y * 60) * 2;
		d->putch(x - 1, y,
						 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x - 1][y]], bigboard, x - 1, y),
						 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x - 1][y]])
						);
	}
	i = (x + y * 60) * 2;
	d->putch(x, y,
					 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y]], bigboard, x, y),
					 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y]])
					);
	if (x + 1 < 60) {
		i = ((x + 1) + y * 60) * 2;
		d->putch(x + 1, y,
						 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x + 1][y]], bigboard, x + 1, y),
						 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x + 1][y]])
						);
	}
	if (y + 1 < 25) {
		if (x - 1 >= 0) {
			i = ((x - 1) + (y + 1) * 60) * 2;
			d->putch(x - 1, y + 1,
							 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x - 1][y + 1]], bigboard, x - 1, y + 1),
							 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x - 1][y + 1]])
							);
		}
		i = (x + (y + 1) * 60) * 2;
		d->putch(x, y + 1,
						 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y + 1]], bigboard, x, y + 1),
						 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x][y + 1]])
						);
		if (x + 1 < 60) {
			i = ((x + 1) + (y + 1) * 60) * 2;
			d->putch(x + 1, y + 1,
							 z_getchar(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x + 1][y + 1]], bigboard, x + 1, y + 1),
							 z_getcolour(bigboard[i], bigboard[i + 1], w->board[e->curboard]->params[paramlist[x + 1][y + 1]])
							);
		}
	}
}

stringvector readdirectorytosvector(char* dir, char* extension, int filetypes)
{
	stringvector files;
	DIR *dp;

	initstringvector(&files);

	dp = opendir(dir);
	if (dp == NULL)
		return files;

	while (1) {
		char * fulld_name;
		struct dirent *dirent;

		dirent = readdir(dp);

		if (dirent == NULL)
			break;

		fulld_name = fullpath(dir, dirent->d_name, SLASH_DEFAULT);

		if (access(fulld_name, D_OK)) {
			if (filetypes & FTYPE_FILE) {
				/* The current file is not a directory, check the extension */
				if (extension[0] == '*' ||
						(dirent->d_name[strlen(dirent->d_name) - strlen(extension) - 1]
							 == '.' &&
						str_equ(dirent->d_name + strlen(dirent->d_name) - strlen(extension),
										extension, STREQU_UNCASE))) {
					pushstring(&files, str_dup(dirent->d_name));
				}
			}
		} else if (!str_equ(dirent->d_name, ".", 0)) {
			if (filetypes & FTYPE_DIR) {
				/* Current file is a directory */
				char* dirline = (char*) malloc(sizeof(char) *
																			 (strlen(dirent->d_name)*2 + 5));
				strcpy(dirline, "!");
				strcat(dirline, dirent->d_name);
				strcat(dirline, ";[");
				strcat(dirline, dirent->d_name);
				strcat(dirline, "]");
				pushstring(&files, dirline);
			}
		}

		free(fulld_name);
	}
	closedir(dp);

	inssortstringvector(&files, strcmp);

	return files;
}

char * filedialog(char * dir, char * extension, char * title, int filetypes, displaymethod * mydisplay)
{
	int done = 0;
	char* result = NULL;
	stringvector files;
	char* curdir = strdup(dir);

	while (!done) {
		int response;

		files = readdirectorytosvector(curdir, extension, filetypes);

		response = browsedialog(title, &files, mydisplay);

		switch (response) {
			case EDITBOX_OK:
				if (!(filetypes & FTYPE_FILE) && ishypermessage(files)) {
					/* If files are not to be listed and a directory is chosen,
					 * treat it as a final choice. */
					char* subdir = gethypermessage(files);

					result = fullpath(curdir, subdir, SLASH_DEFAULT);
					free(subdir);

					done = 1;
				}
				/* No break */
			case EDITBOX_FORWARD:
				if (ishypermessage(files)) {
					/* A directory was chosen */
					char* nextdirectory;
					char* subdir = gethypermessage(files);

					nextdirectory = fullpath(curdir, subdir, SLASH_DEFAULT);
					free(curdir);
					curdir = nextdirectory;

					free(subdir);
				} else {
					/* A file was chosen */
					result = fullpath(curdir, files.cur->s, SLASH_DEFAULT);
					done = 1;
				}
				break;

			case EDITBOX_BACKWARD:
			case EDITBOX_BACK:
				{
					char* nextdirectory;

					nextdirectory = fullpath(curdir, "..", SLASH_DEFAULT);
					free(curdir);
					curdir = nextdirectory;
				}
				break;

			case EDITBOX_CANCEL:
				done = 1;
				break;
		}
	}

	deletestringvector(&files);
	free(curdir);

	return result;
}


char *titledialog(char* prompt, displaymethod * d)
{
	char *t;
	int x, y, i = 0;

	/* Display the title box */
	for (y = 12; y < 12 + TITLE_BOX_DEPTH; y++) {
		for (x = 10; x < 10 + TITLE_BOX_WIDTH; x++) {
			d->putch(x, y, TITLE_BOX[i], TITLE_BOX[i + 1]);
			i += 2;
		}
	}

	/* Display the prompt */
	if (strlen(prompt) < 38)
		d->print(30 - (strlen(prompt) / 2), 12, 0x2f, prompt);

	/* Reserve some memory */
	t = (char *) malloc(35);
	memset(t, '\0', 35);

	/* Do the editing */
	line_editor(12, 13, 0x0f, t, 34, LINED_NORMAL, d);
	return t;
}

int boarddialog(world * w, int curboard, int firstnone, char * title,
										 displaymethod * mydisplay)
{
	stringvector boardlist;
	int response;
	int i = 0;

	initstringvector(&boardlist);

	if (firstnone) {
		pushstring(&boardlist, "(none)");
		i = 1;
	} else {
		i = 0;
	}

	for (; i <= w->zhead->boardcount; i++)
		pushstring(&boardlist, w->board[i]->title);

	if (w->zhead->boardcount < 255)
		pushstring(&boardlist, "!;Add New Board");

	svmovetofirst(&boardlist);
	svmoveby(&boardlist, curboard);

	response = scrolldialog(title, &boardlist, mydisplay);
	if (response == EDITBOX_OK) {
		if (boardlist.cur == boardlist.last && w->zhead->boardcount < 255) {
			w->zhead->boardcount++;
			w->board[w->zhead->boardcount] = z_newboard(titledialog("Enter Title", mydisplay));
		}

		curboard = svgetposition(&boardlist);
	}

	/* No dynamic data in this list */
	removestringvector(&boardlist);

	return curboard;
}

int switchboard(world * w, editorinfo * e, displaymethod * mydisplay)
{
	return boarddialog(w, e->curboard, 0, "Switch Boards", mydisplay);
}


int dothepanel_f1(displaymethod * d, editorinfo * e)
{
	int x, y, i = 0;

	for (y = 3; y < 20; y++) {
		for (x = 0; x < 20; x++) {
			d->putch(x + 60, y, PANEL_F1[i], PANEL_F1[i + 1]);
			i += 2;
		}
	}
	d->putch(78, 4, 2, 0x1f);
	d->putch(78, 5, 132, e->defc ? 0x03 : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 6, 157, e->defc ? 0x06 : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 7, 4, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 8, 12, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	if (e->defc == 1)
		d->putch(78, 9, 10, e->forec > 7 ? ((e->forec - 8) << 4) + 0x0f : (e->forec << 4) + 0x0f);
	else
		d->putch(78, 9, 10, (e->backc << 4) + (e->forec));
	d->putch(78, 10, 232, 0x0f);
	if (e->defc == 1)
		d->putch(78, 11, 240, e->forec > 7 ? ((e->forec - 8) << 4) + 0x0f : (e->forec << 4) + 0x0f);
	else
		d->putch(78, 11, 240, (e->backc << 4) + (e->forec));
	d->putch(78, 12, 250, e->defc ? 0x0f : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 13, 11, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 14, 127, e->defc ? 0x05 : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 17, 47, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 18, 92, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));

	while (1) {
		i = d->getch();
		switch (i) {
		case 27:
		case 13:
			return -1;
		case 'Z':
		case 'z':
			return Z_PLAYER;
		case 'A':
		case 'a':
			return Z_AMMO;
		case 'T':
		case 't':
			return Z_TORCH;
		case 'G':
		case 'g':
			return Z_GEM;
		case 'K':
		case 'k':
			return Z_KEY;
		case 'D':
		case 'd':
			return Z_DOOR;
		case 'S':
		case 's':
			return Z_SCROLL;
		case 'P':
		case 'p':
			return Z_PASSAGE;
		case 'U':
		case 'u':
			return Z_DUPLICATOR;
		case 'B':
		case 'b':
			return Z_BOMB;
		case 'E':
		case 'e':
			return Z_ENERGIZER;
		case '1':
			return Z_CWCONV;
		case '2':
			return Z_CCWCONV;
		}
	}
}

int dothepanel_f2(displaymethod * d, editorinfo * e)
{
	int x, y, i = 0;

	for (y = 3; y < 20; y++) {
		for (x = 0; x < 20; x++) {
			d->putch(x + 60, y, PANEL_F2[i], PANEL_F2[i + 1]);
			i += 2;
		}
	}
	d->putch(78, 4, 153,  e->defc ? 0x06 : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 5, 5,  e->defc ? 0x0d : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 6, 1, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 7, '*', (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 8, '^', e->defc ? 0x07 : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));

	while (1) {
		i = d->getch();
		switch (i) {
		case 27:
		case 13:
			return -1;
		case 'O':
		case 'o':
			return Z_OBJECT;
		case 'b':
		case 'B':
			return Z_BEAR;
		case 'R':
		case 'r':
			return Z_RUFFIAN;
		case 'V':
		case 'v':
			return Z_SLIME;
		case 'Y':
		case 'y':
			return Z_SHARK;
		}
	}
}

int dothepanel_f3(displaymethod * d, editorinfo * e)
{
	int x, y, i = 0;

	for (y = 3; y < 20; y++) {
		for (x = 0; x < 20; x++) {
			d->putch(x + 60, y, PANEL_F3[i], PANEL_F3[i + 1]);
			i += 2;
		}
	}
	d->putch(78, 4, 176, e->defc ? 0x9f : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 5, 176, e->defc ? 0x20 : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 6, 219, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 7, 178, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 8, 177, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 9, 254, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 10, 18, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 11, 29, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 12, 178, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 13, 176, (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 16, '*', e->defc ? 0x0a : (e->backc << 4) + (e->forec) + (0x80 * e->blinkmode));
	d->putch(78, 18, 'E', 0x4c);
	while (1) {
		i = d->getch();
		switch (i) {
		case 27:
		case 13:
			return -1;
		case 'W':
		case 'w':
			return Z_WATER;
		case 'F':
		case 'f':
			return Z_FOREST;
		case 'S':
		case 's':
			return Z_SOLID;
		case 'N':
		case 'n':
			return Z_NORMAL;
		case 'B':
		case 'b':
			return Z_BREAKABLE;
		case 'O':
		case 'o':
			return Z_BOULDER;
		case '1':
			return Z_NSSLIDER;
		case '2':
			return Z_EWSLIDER;
		case 'A':
		case 'a':
			return Z_FAKE;
		case 'I':
		case 'i':
			return Z_INVISIBLE;
		case 'R':
		case 'r':
			return Z_RICOCHET;
		case 'E':
		case 'e':
			return Z_EDGE;
		}
	}
}

unsigned char charselect(displaymethod * d, int c)
{
	int z, e, i = 0;
	static int x, y;

	if (c > 255)
		c = 0;

	if(c != -1) {
		y = c / (CHAR_BOX_WIDTH-2);
		x = c % (CHAR_BOX_WIDTH-2);
	}

	for (e = 0; e < CHAR_BOX_DEPTH; e++) {
		for (z = 0; z < CHAR_BOX_WIDTH; z++) {
			d->putch(z + 13, e + 8, CHAR_BOX[i], CHAR_BOX[i + 1]);
			i += 2;
		}
	}
	i = 0;
	while (1) {
		d->cursorgo(14 + x, 9 + y);
		d->putch(14 + x, 9 + y, (x + y * 32), 0x0f);
		e = 0;
		i = d->getch();
		d->putch(14 + x, 9 + y, (x + y * 32), 0x0a);
		if (!i) {
			e = 1;
			i = d->getch();
		}
		if (e == 1 && i == 72) {
			/* Up Arrow */
			if (y > 0)
				y--;
			else
				y = 7;
		}
		if (e == 1 && i == 80) {
			/* Down Arrow */
			if (y < 7)
				y++;
			else
				y = 0;
		}
		if (e == 1 && i == 75) {
			/* Left Arrow */
			if (x > 0)
				x--;
			else
				x = 31;
		}
		if (e == 1 && i == 77) {
			/* Left Arrow */
			if (x < 31)
				x++;
			else
				x = 0;
		}
		if (e == 0 && i == 13) {
			/* Enter */
			i = (x + y * 32);
			break;
		}
		if (e == 0 && i == 27) {
			/* Escape */
			/* Return the char we recieved without doing anything, unless it is -1 */
			return (c != -1)? c : (x + y * 32);
		}
	}

	return i;
}

void colorselectdrawat(displaymethod* d, int x, int y, char ch)
{
	d->putch(x + 13, y + 8, ch, (y << 4) | (x & 0x0F) | ((x & 0x10) << 3));
}

void colorselectdraw(displaymethod* d)
{
	int x, y;

	/* TODO: find a better place to get the cursor out of the way */
	d->cursorgo(0, 0);

	/* Draw the colors */
	for (x = 0; x < 32; x++)
		for (y = 0; y < 8; y++)
			colorselectdrawat(d, x, y, '\xFE');

	/* Draw the corners */
	d->putch(12, 7,  '\xC9', 0x2A);
	d->putch(12, 16, '\xC8', 0x2A);
	d->putch(45, 7,  '\xBB', 0x2A);
	d->putch(45, 16, '\xBC', 0x2A);

	/* Draw the top and bottom borders */
	for (x = 0; x < 32; x++) {
		d->putch(x + 13, 7, '\xCD', 0x2A);
		d->putch(x + 13, 16, '\xCD', 0x2A);
	}

	/* Draw the left and right borders */
	for (y = 0; y < 8; y++) {
		d->putch(12, y + 8, '\xBA', 0x2A);
		d->putch(45, y + 8, '\xBA', 0x2A);
	}
}

void colorselectremovecursor(displaymethod* d, int curx, int cury)
{
	int x, y;

	/* Draw the corners */
	d->putch(12, 7,  '\xC9', 0x2A);
	d->putch(12, 16, '\xC8', 0x2A);
	d->putch(45, 7,  '\xBB', 0x2A);
	d->putch(45, 16, '\xBC', 0x2A);

	/* Draw the top and bottom borders */
	for (x = max(curx - 1, 0); x < 32 && x <= curx + 1; x++) {
		d->putch(x + 13, 7, '\xCD', 0x2A);
		d->putch(x + 13, 16, '\xCD', 0x2A);
	}

	/* Draw the left and right borders */
	for (y = max(cury - 1, 0); y < 8 && y <= cury + 1; y++) {
		d->putch(12, y + 8, '\xBA', 0x2A);
		d->putch(45, y + 8, '\xBA', 0x2A);
	}

	for (x = max(curx - 1, 0); x < 32 && x <= curx + 1; x++)
		for (y = max(cury - 1, 0); y < 8 && y <= cury + 1; y++)
			colorselectdrawat(d, x, y, '\xFE');

	for (x = 0; x < 32; x++)
		colorselectdrawat(d, x, cury, '\xFE');
	for (y = 0; y < 8; y++)
		colorselectdrawat(d, curx, y, '\xFE');
}

void colorselectdrawcursorat(displaymethod* d, int x, int y, char ch)
{
	d->putch(x + 13, y + 8, ch, (y << 4) | (x & 0x0F));
}

void colorselectdrawcursor(displaymethod* d, int curx, int cury)
{
	int x, y;

	/* Draw the arrows */
	d->putch(curx + 13, 7,  '\xCB', 0x2A);
	d->putch(curx + 13, 16, '\xCA', 0x2A);
	d->putch(12, cury + 8,  '\xCC', 0x2A);
	d->putch(45, cury + 8,  '\xB9', 0x2A);

	/* Draw the cursor */
	colorselectdrawcursorat(d, curx + 1, cury - 1, '\xBB');
	colorselectdrawcursorat(d, curx + 1, cury,     '\xCC');
	colorselectdrawcursorat(d, curx + 1, cury + 1, '\xBC');
	colorselectdrawcursorat(d, curx,     cury - 1, '\xCA');
	colorselectdrawcursorat(d, curx,     cury + 1, '\xCB');
	colorselectdrawcursorat(d, curx - 1, cury - 1, '\xC9');
	colorselectdrawcursorat(d, curx - 1, cury,     '\xB9');
	colorselectdrawcursorat(d, curx - 1, cury + 1, '\xC8');

	/* Draw the cross lines */
	for (x = 0; x < curx - 1; x++)
		colorselectdrawcursorat(d, x, cury, '\xCD');
	for (x = curx + 2; x < 32; x++)
		colorselectdrawcursorat(d, x, cury, '\xCD');
	for (y = 0; y < cury - 1; y++)
		colorselectdrawcursorat(d, curx, y, '\xBA');
	for (y = cury + 2; y < 8; y++)
		colorselectdrawcursorat(d, curx, y, '\xBA');

	/* Draw overlaps with the boarders */
	if (curx == 0) {   /* Left side */
		d->putch(12, 8 + cury - 1, '\xCC', 0x2A);
		d->putch(12, 8 + cury,     '\xBA', 0x2A);
		d->putch(12, 8 + cury + 1, '\xCC', 0x2A);
	}
	if (curx == 31) {  /* Right side */
		d->putch(45, 8 + cury - 1, '\xB9', 0x2A);
		d->putch(45, 8 + cury,     '\xBA', 0x2A);
		d->putch(45, 8 + cury + 1, '\xB9', 0x2A);
	}
	if (cury == 0) {   /* Top */
		d->putch(13 + curx - 1, 7, '\xCB', 0x2A);
		d->putch(13 + curx    , 7, '\xCD', 0x2A);
		d->putch(13 + curx + 1, 7, '\xCB', 0x2A);
	}
	if (cury == 7) {   /* Bottom */
		d->putch(13 + curx - 1, 16, '\xCA', 0x2A);
		d->putch(13 + curx    , 16, '\xCD', 0x2A);
		d->putch(13 + curx + 1, 16, '\xCA', 0x2A);
	}

	/* Draw the corners */
	d->putch(12, 7,  '\xC9', 0x2A);
	d->putch(12, 16, '\xC8', 0x2A);
	d->putch(45, 7,  '\xBB', 0x2A);
	d->putch(45, 16, '\xBC', 0x2A);

}

int colorselector(displaymethod * d, int * bg, int * fg, int * blink)
{
	int curx, cury;
	int key;

	curx = *fg | (*blink << 4);
	cury = *bg;

	colorselectdraw(d);

	while (1) {
		/* Draw the cursor */
		colorselectdrawcursor(d, curx, cury);

		/* Get the key */
		key = d->getch();
		if (key == 0)
			key = d->getch() | DDOSKEY_EXT;

		/* Hide the damage done by the cursor */
		colorselectremovecursor(d, curx, cury);

		switch (key) {
			case DKEY_UP:
				if (cury > 0)  cury--; else cury = 7;  break;
			case DKEY_DOWN:
				if (cury < 7)  cury++; else cury = 0;  break;
			case DKEY_LEFT:
				if (curx > 0)  curx--; else curx = 31; break;
			case DKEY_RIGHT:
				if (curx < 31) curx++; else curx = 0;  break;

			case DKEY_ENTER:
				*fg = curx & 0x0F;
				*bg = cury;
				*blink = curx >> 4;
				return 0;

			case DKEY_ESC:
				return 1;
		}
	}
}


int confirmprompt(displaymethod * mydisplay, char * prompt)
{
	int i, x;
	for (i = 3; i < 25; i++) {
		for (x = 0; x < 20; x++) {
			mydisplay->putch(x + 60, i, ' ', 0x1f);
		}
	}

	mydisplay->print(61, 3, 0x1f, prompt);
	mydisplay->print(61, 4, 0x1e, "y/n");

	while (1) {
		i = mydisplay->getch();
		if (i == 'y' || i == 'Y')
			return CONFIRM_YES;
		else if (i == 'n' || i == 'N')
			return CONFIRM_NO;
		else if (i == 27)
			return CONFIRM_CANCEL;
	}
}


