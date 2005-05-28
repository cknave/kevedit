/**@file texteditor/select.c  Text editor selection, copy, and paste.
 * $Id: select.c,v 1.3 2005/05/28 03:17:46 bitman Exp $
 * @author Ryan Phillips
 *
 * Copyright (C) 2003 Ryan Phillips <bitman@users.sf.net>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "select.h"
#include "register.h"

#include <stdlib.h>
#include <string.h>

selectionBounds texteditGetSelectionBounds(texteditor * editor);

/**
 * @relates texteditor
 * @brief Determine the bounds of the currently selected area.
 *
 * @returns boundaries of the editor's selection.
 **/
selectionBounds texteditGetSelectionBounds(texteditor * editor)
{
	selectionBounds bounds;
	int i;

	/* Start from the current line and move outward. */
	bounds.startLine = bounds.endLine = editor->curline;
	/* Use cursor pos by default for both bounds. */
	bounds.startPos  = bounds.endPos  = editor->pos;

	/* Nothing is selected if selectflag is false. */
	if (!editor->selectflag)
		return bounds;

	if (editor->selectlineoffset > 0) {
		/* The end of selection is below current line. */
		bounds.endPos = editor->selectpos;

		/* Advance endLine by the selectlineoffset. */
		for (i = 0; i < editor->selectlineoffset; i++)
			if (bounds.endLine->next != NULL)
				bounds.endLine = bounds.endLine->next;

	} else if (editor->selectlineoffset < 0) {
		/* The start of selection is above current line. */
		bounds.startPos = editor->selectpos;

		/* Retreat startLine by the negative selectline offset. */
		for (i = 0; i > editor->selectlineoffset; i--)
			if (bounds.startLine->prev != NULL)
				bounds.startLine = bounds.startLine->prev;

	} else {
		/* Selection is only on current line. */
		if (editor->selectpos > editor->pos) {
			/* selectpos comes before pos. */
			bounds.endPos   = editor->selectpos;

		} else {
			/* pos comes before selectpos. */
			bounds.startPos = editor->selectpos;
		}
	}
	
	return bounds;
}

/**
 * @relates texteditor
 * @brief Handle copying of text.
 **/
void texteditHandleCopy(texteditor * editor)
{
	selectionBounds bounds;

	if (editor->key != DKEY_CTRL_C && editor->key != DKEY_CTRL_X)
		return;

	/* Can't copy if nothing is selected */
	/* CONSIDER: copy the current line, maybe? */
	if (!editor->selectflag)
		return;

	bounds = texteditGetSelectionBounds(editor);

	regyank('\"', bounds);
}


/**
 * @relates texteditor
 * @brief Handle text selection.
 *
 * Start or stop selecting text depending on the shift state.
 *
 * Always call this function before any functions that move the cursor when
 * selection begins, such as texteditHandleScrolling() and
 * texteditHandleEditMovement(). Always call after any function that uses the
 * selection and expects the selection to be cleared, such as
 * texteditHandleCopy() and texteditHandleEditKey().
 **/
void texteditHandleSelection(texteditor * editor)
{
	/* Start selecting text if SHIFT key is held down just now. */
	int oldSelectFlag = editor->selectflag;

	/* Selection is based on whether the shift key is down */
	editor->selectflag = editor->d->shift();

	/* Except that shift + ASCII key does not start selection mode */
	if (editor->key < 0x7F)
		editor->selectflag = 0;

	/* If we just started selecting, remember current position */
	if (editor->selectflag && !oldSelectFlag) {
		editor->selectpos = editor->pos;
		editor->selectlineoffset = 0;
	}

	/* If we just stopped selecting, the edit area needs updated */
	if (oldSelectFlag && !editor->selectflag) {
		editor->updateflags |= TUD_EDITAREA;
	}
}


/**
 * @relates texteditor
 * @brief Clear the selected text.
 **/
void texteditClearSelectedText(texteditor * editor)
{
	selectionBounds bounds;

	bounds = texteditGetSelectionBounds(editor);

	if (bounds.startLine == bounds.endLine) {
		/* Selection starts and ends on the same line. */
		int selectionLen = bounds.endPos - bounds.startPos;
		char * line = bounds.startLine->s;
		int i;

		/* Check for badly formed selection: endPos is before startPos. */
		if (selectionLen < 0) selectionLen = -selectionLen;

		/* Copy the end of the line onto the selected text. */
		for (i = bounds.endPos; i < strlen(line); i++) {
			line[i - selectionLen] = line[i];
		}
		line[i - selectionLen] = '\0';
		
		/* Move the cursor to the starting position of the cut. */
		editor->pos = bounds.startPos;

	} else {
		/* Multiple lines are selected. */
		char * endString;
		char * unselectedEndString;

		/* Remove all lines between the start and end lines. */
		editor->text->cur = bounds.startLine->next;
		while (editor->text->cur != bounds.endLine)
			deletestring(editor->text);

		/* Remove the unselected portion of the end line. */
		editor->text->cur = bounds.endLine;
		endString = removestring(editor->text);
		unselectedEndString = endString + bounds.endPos;

		/* Truncate the start line. */
		bounds.startLine->s[bounds.startPos] = '\0';

		/* Wordwrap the unselectedEndString onto the truncated startLine. */
		editor->text->cur = bounds.startLine;
		/* The -1 tells wordwrap to track the cursor position at
		 * the beginning of cutend. Negative tracking values should
		 * be used only by wordwrap for internal purposes, but
		 * necessity warrents in this case.     vv    */
		editor->pos = wordwrap(editor->text, unselectedEndString, bounds.startPos, -1, editor->wrapwidth, editor->linewidth);
		editor->curline = editor->text->cur;

		free(endString);
	}

	editor->updateflags |= TUD_EDITAREA;
}


/**
 * @relates texteditor
 * @brief Paste copied text into the editor.
 */
void texteditPaste(texteditor * editor)
{
	editor->text->cur = editor->curline;

	editor->pos = regput('\"', editor->text, editor->pos, editor->wrapwidth, editor->linewidth);

	editor->curline = editor->text->cur;
	editor->updateflags |= TUD_EDITAREA;
}


