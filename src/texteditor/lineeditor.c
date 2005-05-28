/**@file texteditor/lineeditor.c  Editor for a single line.
 * $Id: lineeditor.c,v 1.2 2005/05/28 03:17:46 bitman Exp $
 * @author Ryan Phillips
 *
 * Copyright (C) 2002 Ryan Phillips <bitman@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either listersion 2 of the License, or
 * (at your option) any later listersion.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "lineeditor.h"

#include "libzzt2/strtools.h"
#include "display/colours.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void lineeditorinsertcharacter(lineeditor * editor, char ch);

/**
 * @relates lineeditor
 * @brief Create a lineeditor.
 *
 * @param x      Screen coordinate.
 * @param y      Screen coordinate.
 * @param line   Line to edit.
 * @param width  Width of the line.
 * @param d      Display method.
 *
 * The line editor will make a its own working copy of line. Do not change
 * editwidth after calling this function. The following default values are used:
 *   - visiblewidth:     Same as width of the line plus one space for the
 *                       cursor a the end of a full line.
 *   - validinputflags:  Normal input.
 *   - pos:              End of the line.
 *   - colour:           White on black.
 *
 * @return A lineeditor.
 **/
lineeditor * createlineeditor(int x, int y, char * line, int width, displaymethod * d)
{
	/* Create the editor. */
	lineeditor * editor = (lineeditor *) malloc(sizeof(lineeditor));

	/* Check for valid pointer. */
	if (editor == NULL)
		return NULL;

	/* Required parameters. */
	editor->x = x; editor->y = y;
	editor->editwidth = width;
	editor->d = d;

	/* Create the line. */
	if (line != NULL)
		editor->line = str_duplen(line, editor->editwidth);
	else
		editor->line = str_duplen("", editor->editwidth);

	/* Default values. */
	editor->validinputflags = LINED_NORMAL;
	editor->visiblewidth = editor->editwidth;
	editor->pos = strlen(editor->line);
	editor->colour = BLACK_B | WHITE_F;

	return editor;
}

/**
 * @relates lineeditor
 * @brief Delete a lineeditor when it is no longer needed.
 **/
void deletelineeditor(lineeditor * editor)
{
	if (editor == NULL)
		return;

	/* Free the line. */
	if (editor->line != NULL) {
		free(editor->line);
		editor->line = NULL;
	}

	/* Free the editor. */
	free(editor);
	editor = NULL;
}

/**
 * @relates lineeditor
 * @brief Perform editing using a lineeditor.
 *
 * @returns Either LINED_OK or LINED_CANCEL.
 **/
int editline(lineeditor * editor)
{
	while (1) {
		int key;

		/* Update the display. */
		lineeditorUpdateDisplay(editor);

		/* Get key from input. */
		key = editor->d->getch();

		/* Handle the key. */
		switch (key) {
			/* Enter means done and result is OK. */
			case DKEY_ENTER:
				return LINED_OK;

			/* Escape means done, but result should be ignored. */
			case DKEY_ESC:
				return LINED_CANCEL;

			/* Handle the key using the lineeditor. */
			default:
				lineeditorHandleKey(editor, key);
				break;
		}
	}
}

/**
 * @relates lineeditor
 * @brief Update the display for a line editor.
 **/
void lineeditorUpdateDisplay(lineeditor * editor)
{
	int offset = 0;
	int i = 0;

	/* When the line may be longer than the visible area, and the cursor reaches
	 * this bound, calculate the draw offset. */
	if (editor->visiblewidth < editor->editwidth &&
	    editor->visiblewidth <= editor->pos) {
		offset = editor->pos - editor->visiblewidth + 1;
	}

	/* Loop through each visible position of the line. */
	i = 0;
	while (i < editor->visiblewidth && i + offset < strlen(editor->line)) {
		editor->d->putch_discrete(editor->x + i, editor->y,
															editor->line[i + offset], editor->colour);
		i++;
	}

	/* Clear the edit area after line. */
	while (i < editor->visiblewidth) {
		editor->d->putch_discrete(editor->x + i, editor->y, ' ', editor->colour);
		i++;
	}

	/* Update the edit area. */
	editor->d->update(editor->x, editor->y, editor->visiblewidth, 1);

	/* Move the cursor */
	editor->d->cursorgo(editor->x + editor->pos - offset, editor->y);
}

/**
 * @relates lineeditor
 * @brief Handle a lineeditor keystroke.
 *
 * @param key  A key that the user has pressed.
 **/
void lineeditorHandleKey(lineeditor * editor, int key)
{
	int i;

	switch (key) {
		case DKEY_LEFT:  if (editor->pos > 0)                    editor->pos--; break;
		case DKEY_RIGHT: if (editor->pos < strlen(editor->line)) editor->pos++; break;
		case DKEY_HOME:  editor->pos = 0;                     break;
		case DKEY_END:   editor->pos = strlen(editor->line);  break;

		case DKEY_BACKSPACE:
			/* Move everything after editor->pos backward */
			if (editor->pos > 0) {
				for (i = editor->pos - 1; i < strlen(editor->line); i++)
					editor->line[i] = editor->line[i + 1];
				editor->pos--;
			}
			break;

		case DKEY_DELETE:
			if (editor->pos < strlen(editor->line)) {
				for (i = editor->pos; i < strlen(editor->line); i++)
					editor->line[i] = editor->line[i+1];
			}
			break;

		case DKEY_CTRL_Y:
			/* Clear line */
			editor->pos = 0;
			editor->line[0] = '\0';
			break;

		case DKEY_TAB:
			/* Insert 4 spaces */
			if (strlen(editor->line) > (editor->editwidth - 4))
				break;

			for (i = strlen(editor->line) + 4; i > editor->pos; i--)
				editor->line[i] = editor->line[i-4];
			for (i = 0; i < 4; i++)
				editor->line[editor->pos++] = ' ';
			break;

		default:
			/* Keys outside the standard ASCII range are ignored. */
			if (key < 0x20 || key > 0x7E)
				break;

			lineeditorinsertcharacter(editor, key);
			break;
	}
}

/**
 * @relates lineeditor
 * @brief Insert a character at the cursor position if it is legal.
 **/
void lineeditorinsertcharacter(lineeditor * editor, char ch)
{
	int flags = editor->validinputflags;
	int i;

	/* Be sure we have room */
	if (strlen(editor->line) >= editor->editwidth)
		return;

	/* Act on flags */
	if ((flags & LINED_NOLOWER) && (flags & LINED_NOUPPER) &&
			((ch >= 0x41 && ch <= 0x5A) || (ch >= 0x61 && ch <= 0x7A))) return;
	if ((flags & LINED_NODIGITS) && (ch >= 0x30 && ch <= 0x39)) return;
	if ((flags & LINED_NOPUNCT) && ((ch >= 0x21 && ch <= 0x2F) ||
																	(ch >= 0x3A && ch <= 0x40) ||
																	(ch >= 0x5A && ch <= 0x60) ||
																	(ch >= 0x7B && ch <= 0x7E))) return;
	if ((flags & LINED_NOSPACES) && (ch == ' ')) return;
	if ((flags & LINED_NOPERIOD) && (ch == '.')) return;
	if ((flags & LINED_FILENAME) && (ch == '\"' || ch == '?' || ch == '*' ||
																	 ch == '<'  || ch == '>' || ch == '|')) return;
	if ((flags & LINED_NOPATH)   && (ch == '\\' || ch == '/' || ch == ':')) return;

	if (flags & LINED_NOUPPER) ch = tolower(ch);
	if (flags & LINED_NOLOWER) ch = toupper(ch);

	/* Insert character */
	for (i = strlen(editor->line) + 1; i > editor->pos; i--)
		editor->line[i] = editor->line[i-1];
	editor->line[editor->pos++] = ch;
}

