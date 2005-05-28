/**@file texteditor/display.c  Text editor display functions.
 * $Id: display.c,v 1.4 2005/05/28 03:17:45 bitman Exp $
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

#include "texteditor.h"

#include "display/colours.h"
#include "themes/theme.h"

#include "kevedit/screen.h"

#include <string.h>

/* in texteditor.c */
int texteditIgnoreFirstLine(texteditor * editor);

void texteditDisplayLine(texteditor * editor, int offset, char * line);
void texteditDisplayText(texteditor * editor);
void texteditDisplaySelection(texteditor * editor);
void texteditDisplayPanel(texteditor * editor);

/** The X and Y positions on the display of the leftmost cursor position. */
const int baseX = 9, baseY = 13;
/** The number of lines in each of the bottom and top text areas. */
const int btLineCount = 7;
/** Lead text: line displayed at the beginning and end of the edit text. */
char leadText[] =
	"\x07    \x07    \x07    \x07    \x07    \x07    \x07    \x07    \x07";


/**
 * @relates texteditor
 * @brief Update the display for a texteditor.
 **/
void texteditUpdateDisplay(texteditor * editor)
{
	/** @TODO: Phase out drawscrollbox(). */
	texteditDisplayPanel(editor);

	/* Clear the entire scrollbox. */
	if (editor->updateflags == TUD_ALL) {
		drawscrollbox(editor->d, 0, 0, 1);
	} else {
		/* Clear the scrollbox in pieces. */
		if (editor->updateflags & TUD_TOP)
			drawscrollbox(editor->d, 3, 9, 0);
		if (editor->updateflags & TUD_CENTER)
			drawscrollbox(editor->d, 10, 8, 0);
		if (editor->updateflags & TUD_BOTTOM)
			drawscrollbox(editor->d, 11, 1, 0);
	}

	/* Update title if needed. */
	if (editor->updateflags & TUD_TITLE) {
		drawscrollbox(editor->d, 1, 17, 0);
		editor->d->print_discrete(30 - (strlen(editor->title) / 2), 4, 0x0a, editor->title);
	}

	if (editor->updateflags & TUD_CENTER) {
		/* Draw the center */
		texteditDisplayLine(editor, 0, editor->curline->s);
	}

	texteditDisplayText(editor);

	/* Clear the update flags now that everything is updated. */
	editor->updateflags = TUD_NONE;
}


/**
 * @relates texteditor
 * @brief Display a texteditor's text.
 **/
void texteditDisplayText(texteditor * editor)
{
	stringvector * text = editor->text;
	int i;

	/* Draw bottom half. */
	if (editor->updateflags & TUD_BOTTOM) {
		text->cur = editor->curline->next;
		for (i = 1; i <= btLineCount && text->cur != NULL; i++, text->cur = text->cur->next)
			texteditDisplayLine(editor, i, text->cur->s);

		if (i <= btLineCount)
			editor->d->print_discrete(baseX, i + baseY, 0x07, leadText);
	}

	/* Draw top half. */
	if (editor->updateflags & TUD_TOP) {
		text->cur = editor->curline->prev;
		for (i = -1; i >= -btLineCount && text->cur != NULL; i--, text->cur = text->cur->prev)
			texteditDisplayLine(editor, i, text->cur->s);

		/* If the first line is being ignored, move back a step. */
		if (text->cur == NULL && texteditIgnoreFirstLine(editor))
			i++;

		/* Write the leader. If the first line was displayed erroneously, this
		 * will overwrite it. */
		if (i >= -btLineCount)
			editor->d->print_discrete(baseX, i + baseY, 0x07, leadText);
	}

	texteditDisplaySelection(editor);

	editor->d->cursorgo(baseX + editor->pos, baseY);
	editor->d->update(3, 4, 51, 19);
}


/**
 * @relates texteditor
 * @brief Display a line of text in the text editor box.
 *
 * @param offset  Offset of this line from the cursor line.
 * @param line    The line of text.
 **/
void texteditDisplayLine(texteditor * editor, int offset, char * line)
{
	if (!editor->highlightflag) {
		char defColour = editor->drawer.textcolours[0];
		editor->d->print_discrete(baseX, baseY + offset, defColour, line);
	} else {
		ZZTOOPparser * parser = zztoopCreateParser(line);

		/* Use help parsing (non-strict) if the drawer expects it. */
		if (editor->drawer.helpformatting) {
			parser->flags = ZOOPFLAG_HELP;
		}

		/* The first line is parsed differently. */
		if (line == editor->text->first->s)
			parser->flags |= ZOOPFLAG_FIRSTLINE;

		/* Set up drawer coords. */
		editor->drawer.x = baseX;
		editor->drawer.y = baseY + offset;

		/* Parse and draw the line. */
		zztoopDraw(editor->drawer, zztoopParseLine(parser));

		zztoopDeleteParser(parser);
	}
}


/**
 * @relates texteditor
 * @brief Display any selected text.
 **/
void texteditDisplaySelection(texteditor * editor)
{
	stringvector * text = editor->text;
	int startPos = 0, endPos = 0;
	int lineoffset = 0, i = 0;

	const int highlightColour = BLACK_F | WHITE_B;

	/* Only highlight if a selection exists. */
	if (!editor->selectflag)
		return;

	/* Determine where to start and stop highlighting on the current line. */
	if (editor->selectlineoffset > 0) {
		/* Highlight from pos to end of line. */
		startPos = editor->pos;
		endPos = strlen(editor->curline->s);
	} else if (editor->selectlineoffset < 0) {
		/* Highlight from pos to beginning of line. */
		startPos = 0;
		endPos = editor->pos;
	} else {
		/* Selection is entirely on this line; highlight from selectpos to pos, or
		 * visa versa depending on which comes first. */
		if (editor->selectpos > editor->pos) {
			startPos = editor->pos;
			endPos = editor->selectpos;
		} else {
			startPos = editor->selectpos;
			endPos = editor->pos;
		}
	}

	/* Draw the highlighting for the current line. */
	for (i = startPos; i < endPos; i++)
		editor->d->putch_discrete(baseX + i, baseY, editor->curline->s[i], highlightColour);

	/* If only the current line is highlighted, we are done. */
	if (editor->selectlineoffset == 0)
		return;

	/* Draw meat lines -- lines that are fully highlighted. */
	if (editor->selectlineoffset > 0) {
		/* Start above the current line and move up until we reach either the line
		 * before the end of the selection, the end of the viewing area, or the end
		 * of the text. */
		for (lineoffset = 1, text->cur = editor->curline->next;
				 lineoffset < editor->selectlineoffset && lineoffset <= btLineCount && text->cur != NULL;
				 lineoffset++, text->cur = text->cur->next)
			editor->d->print_discrete(baseX, baseY + lineoffset, highlightColour, text->cur->s);

	} else {
		/* Same as above, except going down. */
		for (lineoffset = -1, text->cur = editor->curline->prev;
				 lineoffset > editor->selectlineoffset && lineoffset >= -btLineCount && text->cur != NULL;
				 lineoffset--, text->cur = text->cur->prev)
			editor->d->print_discrete(baseX, baseY + lineoffset, highlightColour, text->cur->s);
	}

	/* Draw farthest line from centerstr. */
	if (lineoffset <= btLineCount && lineoffset >= -btLineCount && text->cur != NULL && text->cur->s != NULL) {
		/* Determine start and end positions. */
		if (editor->selectlineoffset < 0) {
			startPos = editor->selectpos;
			endPos = strlen(text->cur->s);
		} else if (editor->selectlineoffset > 0) {
			startPos = 0;
			endPos = editor->selectpos;
		}

		/* Draw them characters, yeah. */
		for (i = startPos; i < endPos; i++)
			editor->d->putch_discrete(baseX + i, baseY + lineoffset, text->cur->s[i], highlightColour);
	}
}

void texteditDisplayPanel(texteditor * editor)
{
	char buf[10] = "";

	/* Only update the panel if necessary. */
	if (!(editor->updateflags & TUD_PANEL) || !editor->editflag)
		return;

	drawsidepanel(editor->d, PANEL_EDIT);
	
	editor->d->print(76, 6,  YELLOW_F | BRIGHT_F | BLUE_B, (editor->insertflag ? "on" : "off"));

	sprintf(buf, "%d", editor->wrapwidth);

	if (editor->wrapwidth != 0)
		editor->d->print(76, 8, YELLOW_F | BRIGHT_F | BLUE_B, buf);
	else
		editor->d->print(72, 8, YELLOW_F | BRIGHT_F | BLUE_B, "off");
}


