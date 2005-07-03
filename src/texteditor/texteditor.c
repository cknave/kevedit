/**@file texteditor/texteditor.c  Text editor/viewer.
 * $Id: texteditor.c,v 1.6 2005/07/03 00:47:11 kvance Exp $
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
#include "display.h"

#include "kevedit/screen.h"

#include "structures/svector.h"
#include "zzm.h"
#include "libzzt2/zztoop.h"
#include "display/colours.h"

#include "register.h"
#include "select.h"
#include "help/help.h"

#include "themes/theme.h"

#include "synth/synth.h"
#include "synth/zzm.h"

#include "display/display.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int texteditReadyToEdit(texteditor * editor);
void texteditHandleInput(texteditor * editor);

void texteditHandleScrolling(texteditor * editor);
void texteditHandleEditMovement(texteditor * editor);
void texteditHandleEditKey(texteditor * editor);

int texteditGrabTitle(texteditor * editor);
int texteditIgnoreFirstLine(texteditor * editor);
void texteditValidatePosition(texteditor * editor);

void texteditCursorLeft(texteditor * editor);
void texteditCursorRight(texteditor * editor);
void texteditHelpZOC(texteditor * editor);

void texteditZZMPlay(texteditor * editor, int slurflag);
void texteditZZMLoad(texteditor * editor);
void texteditZZMRip(texteditor * editor);

void texteditFileOpen(texteditor * editor, int insertflag);
void texteditFileSave(texteditor * editor, char * prompt);

void texteditInsertSpaces(texteditor * editor, int count);
void texteditInsertNewline(texteditor * editor);

void texteditBackspace(texteditor * editor);
void texteditDelete(texteditor * editor);
void texteditDeleteLine(texteditor * editor);

void texteditInsertASCII(texteditor * editor);
void texteditInsertCharacter(texteditor * editor, int ch);
void texteditInsertCharAndWrap(texteditor * editor, int ch);


/**
 * @relates texteditor
 * @brief Create a text editor.
 *
 * @param title  Dialog title to use by default.
 * @param text   Text to be editted or viewed. Use NULL to start with empty
 *               text.
 * @param d      A display method.
 *
 * Editing or viewing begins on the line indicated by text->cur.
 *
 * You may modify highlightflag, insertflag, and wrapwidth before
 * using the texteditor object.
 *
 * @return a new texteditor.
 **/
texteditor * createtexteditor(char * title, stringvector * text, displaymethod * d)
{
	texteditor * editor = (texteditor *) malloc(sizeof(texteditor));

	if (editor == NULL)
		return NULL;

	editor->d = d;
	editor->title = str_dup(title);

	/* Initialize the OOP drawer. */
	zztoopInitDrawer(&(editor->drawer));
	editor->drawer.display = editor->d;
	editor->drawer.length = TEXTED_MAXWIDTH;

	editor->text = text;

	/* If no text is provided, allocate */
	if (editor->text == NULL) {
		editor->text = (stringvector *) malloc(sizeof(stringvector));
		initstringvector(editor->text);
	}

	/* Remember, text may be NULL, so use editor->text. */
	editor->curline = editor->text->cur;
	editor->pos = 0;

	editor->editflag = 1;
	editor->highlightflag = 1;
	editor->insertflag = 1;

	editor->linewidth = TEXTED_MAXWIDTH + 1;
	editor->wrapwidth = TEXTED_MAXWIDTH;

	editor->updateflags = TUD_ALL;
	editor->exitflag = 0;

	editor->key = DKEY_NONE;

	editor->selectflag = 0;
	editor->selectpos = -1;
	editor->selectlineoffset = 0;

	return editor;
}

/**
 * @relates texteditor
 * @brief Delete a texteditor.
 *
 * Call this function on a texteditor when it is no longer needed.
 * editor->text will not be affected.
 **/
void deletetexteditor(texteditor * editor)
{
	if (editor == NULL)
		return;

	if (editor->title != NULL) {
		free(editor->title);
		editor->title = NULL;
	}

	free(editor);
}

/**
 * @relates texteditor
 * @brief Delete the text used by a text editor.
 *
 * Call this before deletetexteditor() if the text is no longer
 * needed and can be free()ed.
 **/
void deletetexteditortext(texteditor * editor)
{
	if (editor->text == NULL)
		return;

	deletestringvector(editor->text);

	editor->text = NULL;
}

/**
 * @relates texteditor
 * @brief Perform text editing.
 **/
void textedit(texteditor * editor)
{
	if (!texteditReadyToEdit(editor))
		return;

	while (!editor->exitflag) {
		/* Start by updating the display. */
		texteditUpdateDisplay(editor);

		/* Get a key from the display. */
		editor->key = editor->d->getch();

		/* Handle the input. */
		texteditHandleInput(editor);
	}

	editor->text->cur = editor->curline;
}


/**
 * @relates texteditor
 * @brief Prepare an editor before editing starts.
 *
 * @returns true if the editor is ready.
 **/
int texteditReadyToEdit(texteditor * editor)
{
	/* An editor without text is like a party without music. */
	if (editor->text == NULL)
		return 0;

	/* If the text is empty, provide a blank line */
	if (editor->text->cur == NULL)
		pushstring(editor->text, str_duplen("", editor->linewidth));

	/* Try grabbing a title from the text itself */
	if (texteditGrabTitle(editor))
		/* When not editing, make sure curline is not the first line */
		if (!editor->editflag && editor->curline == editor->text->first)
			editor->curline = editor->curline->next;

	/* If the current line is NULL, grab the current line from text. */
	if (editor->curline == NULL)
		editor->curline = editor->text->cur;

	editor->updateflags |= TUD_ALL;

	return 1;
}

/**
 * @relates texteditor
 * @brief Handle all possible types of input.
 **/
void texteditHandleInput(texteditor * editor)
{
	/* Check for exit key. */
	if (editor->key == DKEY_ESC)
		editor->exitflag = 1;

	/* Allow user to copy text. */
	texteditHandleCopy(editor);
	/* Perform editting. */
	texteditHandleEditKey(editor);

	/* Allow user to select text. */
	texteditHandleSelection(editor);

	/* Handle cursor movements. */
	texteditHandleScrolling(editor);
	texteditHandleEditMovement(editor);
}

/**
 * @relates texteditor
 * @brief Handle scrolling keys.
 **/
void texteditHandleScrolling(texteditor * editor)
{
	int i;

	switch (editor->key) {
		case DKEY_UP:  /* Up Arrow */
			if (editor->curline->prev == NULL ||
			    (texteditIgnoreFirstLine(editor) &&
			     editor->curline->prev == editor->text->first))
				return;

			editor->curline = editor->curline->prev;

			texteditValidatePosition(editor);

			/* Selection offset increases */
			if (editor->selectflag)
				editor->selectlineoffset++;

			editor->updateflags |= TUD_EDITAREA;
			return;

		case DKEY_DOWN:  /* Down Arrow */
			if (editor->curline->next == NULL)
				return;

			editor->curline = editor->curline->next;

			texteditValidatePosition(editor);

			/* Selection offset decreases */
			if (editor->selectflag)
				editor->selectlineoffset--;

			editor->updateflags |= TUD_EDITAREA;
			return;

		case DKEY_PAGEUP:  /* Page Up */
			i = 0;
			while (i < TEXTED_PAGELENGTH &&
			       editor->curline->prev != NULL &&
			       !(texteditIgnoreFirstLine(editor) &&
			         editor->curline->prev == editor->text->first)) {
				/* Advance to the next line */
				editor->curline = editor->curline->prev;
				if (editor->selectflag)
					editor->selectlineoffset++;
				i++;
			}

			texteditValidatePosition(editor);

			editor->updateflags |= TUD_EDITAREA;
			return;

		case DKEY_PAGEDOWN:  /* Page Down */
			for (i = 0; i < TEXTED_PAGELENGTH &&
			            editor->curline->next != NULL; i++) {
				/* Return to the previous line */
				editor->curline = editor->curline->next;
				if (editor->selectflag)
					editor->selectlineoffset--;
			}

			texteditValidatePosition(editor);

			editor->updateflags |= TUD_EDITAREA;
			return;
	}
}

/**
 * @relates texteditor
 * @brief Handle edit movement.
 **/
void texteditHandleEditMovement(texteditor * editor)
{
	if (!editor->editflag)
		return;

	switch (editor->key) {
		case DKEY_LEFT:  /* Left Arrow */
			texteditCursorLeft(editor);
			break;

		case DKEY_RIGHT:    /* Right Arrow */
			texteditCursorRight(editor);
			break;

		case DKEY_HOME: /* Home */
			editor->pos = 0;
			break;

		case DKEY_END: /* End */
			editor->pos = strlen(editor->curline->s);
			break;
	}
}

/**
 * @relates texteditor
 * @brief Handle an edit keypress.
 **/
void texteditHandleEditKey(texteditor * editor)
{
	if (!editor->editflag)
		return;

	switch (editor->key) {

		/********** Insert & Delete ***********/

		case DKEY_INSERT:
			/* Insert */
			editor->insertflag = !editor->insertflag;
			editor->updateflags |= TUD_PANEL;
			break;

		case DKEY_DELETE:
			texteditDelete(editor);
			break;

		/****** Help dialog ******/

		case DKEY_F1: /* F1: help dialog */
			texteditHelpZOC(editor);
			break;

		/********* ZZM Testing ********************/
		case DKEY_CTRL_T:
			/* Play slurred music */
			texteditZZMPlay(editor, 1);
			break;

		case DKEY_ALT_T:
			/* Play music with slight break between notes */
			texteditZZMPlay(editor, 0);
			break;

		/********* File access operations *********/
		case DKEY_ALT_O: /* alt+o: open file */
			texteditFileOpen(editor, 0);
			break;

		case DKEY_ALT_I: /* alt+i: insert file */
			texteditFileOpen(editor, 1);
			break;

		case DKEY_ALT_S: /* alt-s: save to file */
			texteditFileSave(editor, "Save Object Code As");
			break;

		case DKEY_ALT_M: /* alt-m: load .zzm music */
			texteditZZMLoad(editor);
			break;

		case DKEY_CTRL_R: /* ctrl-r: rip music */
			texteditZZMRip(editor);
			break;

		/******** Cut operation *********/

		case DKEY_CTRL_DELETE:    /* ctrl-delete: clear selected text */
		case DKEY_CTRL_X:         /* ctrl-x: cut selected text */
			texteditClearSelectedText(editor);
			break;

		case DKEY_CTRL_V:     /* ctrl-v: paste register */
			texteditPaste(editor);
			break;

		case DKEY_TAB: /* Tab */
			texteditInsertSpaces(editor, 4);
			break;

		case DKEY_ENTER: /* Enter */
			texteditInsertNewline(editor);
			break;

		case DKEY_BACKSPACE: /* Backspace */
			texteditBackspace(editor);
			break;

		case DKEY_CTRL_Y: /* ctrl-y: delete line */
			texteditDeleteLine(editor);
			break;

		case DKEY_ESC: /* escape when done */
			editor->exitflag = 1;
			break;

		case DKEY_CTRL_A: /* ctrl-a: insert ascii char/decimal-value */
			texteditInsertASCII(editor);
			break;

		default:
			/* Insert keyboard character */
			if (editor->key < 0x7F && editor->key > 0x1F)
				texteditInsertCharacter(editor, editor->key);
			break;
	}
}

/**
 * @relates texteditor
 * @brief Grab the title using zzt syntax.
 *
 * @returns true if a title is grabbed.
 **/
int texteditGrabTitle(texteditor * editor)
{
	stringnode* first = editor->text->first;

	/* Look for @title on first line */
	if (first != NULL &&
	    first->s[0] == '@' &&
			first->s[1] != '\x0') {
		/* Use the first line as the title */
		if (editor->title != NULL)
			free(editor->title);
		editor->title = str_dup(first->s + 1);

		editor->updateflags |= TUD_TITLE;

		return 1;
	}

	return 0;
}

/**
 * @relates texteditor
 * @brief Check whether first line should be ignored.
 *
 * @returns true if the first line should be ignored.
 */
int texteditIgnoreFirstLine(texteditor * editor)
{
	/* Ignore the first line when not editing and
	 * when the first line starts with an @ */
	return !editor->editflag && editor->text->first->s[0] == '@';
}

/**
 * @relates texteditor
 * @brief Make sure that pos is valid for the current line.
 **/
void texteditValidatePosition(texteditor * editor)
{
	if (editor->pos > strlen(editor->curline->s))
		editor->pos = strlen(editor->curline->s);
}

/**
 * @relates texteditor
 * @brief Move the cursor left, wrapping as necessary.
 **/
void texteditCursorLeft(texteditor * editor)
{
	if (editor->pos > 0)
		editor->pos--;
	else {
		/* Move to end of previous line (or current line) */
		if (editor->curline->prev != NULL) {
			editor->curline = editor->curline->prev;
			editor->updateflags |= TUD_EDITAREA;

			/* Update selectlineoffset */
			if (editor->selectflag)
				editor->selectlineoffset++;
		}
		editor->pos = strlen(editor->curline->s);
	}
}

/**
 * @relates texteditor
 * @brief Move the cursor right, wrapping as necessary.
 **/
void texteditCursorRight(texteditor * editor)
{
	if (editor->pos < strlen(editor->curline->s))
		editor->pos++;
	else {
		/* Move to begining of next line (or current line) */
		if (editor->curline->next != NULL) {
			editor->curline = editor->curline->next;
			editor->updateflags |= TUD_EDITAREA;

			/* Update selectlineoffset */
			if (editor->selectflag)
				editor->selectlineoffset--;
		}
		editor->pos = 0;
	}
}

/**
 * @relates texteditor
 * @brief Open the ZZT-OOP help dialog. If the current line contains a ZZT-OOP
 * command, look up that command.
 **/
void texteditHelpZOC(texteditor * editor)
{
	int i;

	/* Look for #command on current line for lookup in help */
	i = editor->pos;
	while (i > 0 && editor->curline->s[i] != '#')
		i--;

	if (editor->curline->s[i] == '#') {
		/* Lookup the command in the language reference */
		char * command;

		command = str_dup(editor->curline->s + i + 1);
		for (i = 0; command[i] != ' ' && command[i] != '\0'; i++)
			;
		command[i] = '\0';

		if (zztoopFindCommand(command) == -1) {
			/* If it's not a valid command, don't bother looking for it */
			command[0] = '\0';
		}

		/* Display the help file with the command as the topic */
		helpsectiontopic("langref", command, editor->d);
		
		free(command);
	} else {
		/* Display the oop help file */
		helpsectiontopic("langref", NULL, editor->d);
	}

	editor->updateflags |= TUD_ALL;
}

/**
 * @relates texteditor
 * @brief Play any ZZM music found in the editor.
 *
 * @param slurflag  Slur notes together when true (ZZT-style).
 */
void texteditZZMPlay(texteditor * editor, int slurflag)
{
	/** @TODO: do a damn good job of testing music */
	/* Idea: create a copy of *editor so that we can mess around with curline and
	 * pos and such, using existing functions to do the bulk of the display. */
#if 0
	editor->text->cur = editor->curline;
	testMusic(editor->text, slurflag, editor->linewidth, flags, editor->d);
#endif

	/* Create a new view of the editor data. This allows us to move the cursor
	 * and change display settings for the new view without affecting editor. */
	const char* playString = "#play ";
	const int playStringLen = 6;

	texteditor editorCopy = *editor;
	texteditor* view = &editorCopy;

	int done;

#ifdef SDL
	SDL_AudioSpec spec;
#endif

	/* Display everything, in case the editor has not been displayed recently. */
	view->updateflags |= TUD_ALL;

#ifdef SDL
	/* IF opening the audio device fails, return now before we crash something. */
	if (OpenSynth(&spec))
		return;
#endif

	done = 0;

	/* Loop through the stringvector looking for #play statements */
	while (view->curline != NULL && !done) {
		char* tune = strstr(view->curline->s, "#");
		if (tune != NULL && str_equ(tune, playString, STREQU_UNCASE | STREQU_RFRONT)) {
			/* Current note and settings */
			musicalNote note = zzmGetDefaultNote();
			musicSettings settings = zzmGetDefaultSettings();

			int xoffset = tune - view->curline->s + playStringLen;
			tune += playStringLen;  /* Advance to notes! */

			/* Change the slur setting */
			note.slur = slurflag;

			while (note.src_pos < strlen(tune) && !done) {
				if (view->d->getkey() != DKEY_NONE)
					done = 1;

				/* Move the cursor and re-display before playing note. */
				view->pos = note.src_pos + xoffset;
				texteditUpdateDisplay(view);

				note = zzmGetNote(tune, note);

#ifdef DOS
				pcSpeakerPlayNote(note, settings);
#elif defined SDL
				SynthPlayNote(spec, note, settings);
#endif
			}
		}
		view->curline = view->curline->next;

		/* Re-display edit area since the current line has changed. */
		view->updateflags |= TUD_EDITAREA;
	}

#ifdef SDL
	/* TODO: instead of just sitting here, display the progress of playback */
	/* Wait until the music is done or the user presses a key */
	while (!IsSynthBufferEmpty() && view->d->getkey() == DKEY_NONE)
		;

	CloseSynth();
#elif defined DOS
	pcSpeakerFinish();
#endif

	/* No need to free the view, it only exists on the stack. */

	/* The edit area needs to be redisplayed now. */
	editor->updateflags |= TUD_EDITAREA;
}

/**
 * @relates texteditor
 * @brief Load music from a ZZM file.
 **/
void texteditZZMLoad(texteditor * editor)
{
	char* filename;
	filename = filedialog(".", "zzm", "Choose ZZT Music (ZZM) File",
												FTYPE_ALL, editor->d);

	if (filename != NULL) {
		stringvector zzmv;
		zzmv = filetosvector(filename, 80, 80);
		if (zzmv.first != NULL) {
			stringvector song;
			song = zzmpullsong(&zzmv, zzmpicksong(&zzmv, editor->d));
			if (song.first != NULL) {
				/* copy song into editor->text */
				editor->text->cur = editor->curline;
				for (song.cur = song.first; song.cur != NULL; song.cur = song.cur->next) {
					char * insline = str_duplen("#play ", editor->linewidth);
					strncat(insline, song.cur->s, editor->linewidth - 6);

					preinsertstring(editor->text, insline);
				}
				deletestringvector(&song);
			}
			deletestringvector(&zzmv);
		}
	}
	free(filename);

	editor->updateflags |= TUD_EDITAREA | TUD_TITLE | TUD_PANEL;
}

/**
 * @relates texteditor
 * @brief Rip music and browse it.
 *
 * This allows ZZM music files to be created from object code.
 *
 * @TODO: Document this feature and make it more usable.
 */
void texteditZZMRip(texteditor * editor)
{
	/* This is mostly worthless just now */
	stringvector ripped;

	editor->text->cur = editor->curline;

	ripped = zzmripsong(editor->text, 4);
	scrolldialog("Ripped Music", &ripped, editor->d);

	deletestringvector(&ripped);
	editor->updateflags |= TUD_ALL;
}

/**
 * @relates texteditor
 * @brief Open a file for editting.
 *
 * @param insertflag  Insert file at cursor when true; otherwise overwrite all
 *                    existing text.
 **/
void texteditFileOpen(texteditor * editor, int insertflag)
{
	stringvector filetypelist;
	char* filename = NULL;
	
	initstringvector(&filetypelist);

	pushstring(&filetypelist, "*.zoc");
	pushstring(&filetypelist, "*.txt");
	pushstring(&filetypelist, "*.hlp");
	pushstring(&filetypelist, "*.zzm");
	pushstring(&filetypelist, "*.*");
	if (editbox("Select A File Type", &filetypelist, 0, 1, editor->d) == 27) {
		editor->updateflags |= TUD_EDITAREA | TUD_TITLE;
		return;
	}

	if (filetypelist.cur != NULL)
		filename =
			filedialog(".", filetypelist.cur->s + 2,
								 (!insertflag ?
									 "Open ZZT Object Code (ZOC) File" :
									 "Insert ZZT Object Code (ZOC) File"),
								 FTYPE_ALL, editor->d);

	if (filename != NULL && strlen(filename) != 0) {
		stringvector filetext;
		filetext = filetosvector(filename, editor->wrapwidth, editor->linewidth);
		if (filetext.first != NULL) {

			/* TODO: remember the filename for future reference */

			if (!insertflag) {
				/* erase & replace editor->text */
				deletestringvector(editor->text);
				*editor->text = filetext;
				editor->curline = editor->text->first;
				editor->pos = 0;
			} else {
				/* insert filetext before editor->curline */
				editor->text->cur = editor->curline;

				/* TODO: this code should be in an svector function */
				if (editor->text->cur == editor->text->first) {
					/* first node */
					editor->text->first = filetext.first;
					editor->text->cur->prev = filetext.last;
					filetext.last->next = editor->text->cur;
					editor->curline = filetext.first;
				} else if (editor->text->cur->prev != NULL) {
					/* middle/end node */
					filetext.first->prev = editor->text->cur->prev;
					editor->text->cur->prev->next = filetext.first;
					filetext.last->next = editor->text->cur;
					editor->text->cur->prev = filetext.last;
					editor->curline = filetext.first;
				} else {
					/* this code should be unreachable */
					deletestringvector(&filetext);
				}
			} /* esle */
		}	/* fi file selected */
	}		/* fi not empty */

	free(filename);
	removestringvector(&filetypelist);

	editor->updateflags |= TUD_EDITAREA | TUD_TITLE | TUD_PANEL;
}

/**
 * @relates texteditor
 * @brief Save text to a file.
 * @param prompt  Prompt to give user.
 */
void texteditFileSave(texteditor * editor, char * prompt)
{
	char* filename;

	/* TODO: Use a stored filename, rather than empty by default */
	filename = filenamedialog("", "", prompt, 1, editor->d);

	if (filename != NULL) {
		/* Save to the file */
		svectortofile(editor->text, filename);

		/* TODO: Remember the file name */
		free(filename);
	}
	editor->updateflags |= TUD_EDITAREA | TUD_PANEL | TUD_PANEL;
}

/**
 * @relates texteditor
 * @brief Insert a number of spaces at the cursor position.
 *
 * @param count  Number of spaces to insert.
 **/
void texteditInsertSpaces(texteditor * editor, int count)
{
	int i;

	/* Make sure the spaces will fit on the line */
	count = count - (editor->pos % count);

	for (i = 0; i < count; i++)
		texteditInsertCharacter(editor, ' ');
}

/**
 * @relates texteditor
 * @brief Insert a newline at the cursor.
 **/
void texteditInsertNewline(texteditor * editor)
{
	char * nextline;

	/* Copy everything after the cursor */
	nextline = str_duplen(editor->curline->s + editor->pos, editor->linewidth);

#if 0
	nextline = (char*) malloc(editor->linewidth + 2);
	for (i = editor->pos, j = 0; i < strlen(editor->curline->s); i++, j++)
		nextline[j] = editor->curline->s[i];
	tmpstr[j] = 0;
#endif

	/* Truncate the current line */
	editor->curline->s[editor->pos] = '\x0';

	/* Insert nextline */
	editor->text->cur = editor->curline;
	insertstring(editor->text, nextline);

	/* Advance to the new line */
	editor->curline = editor->curline->next;
	editor->pos = 0;

	editor->updateflags |= TUD_EDITAREA;
}

/**
 * @relates texteditor
 * @brief Delete the character before the cursor.
 */
void texteditBackspace(texteditor * editor)
{
	int i;

	if (editor->selectflag) {
		texteditClearSelectedText(editor);
		return;
	}

	if (editor->pos > 0) {
		/* Slide everything at or after the cursor back */
		for (i = editor->pos - 1; i < strlen(editor->curline->s); i++)
			editor->curline->s[i] = editor->curline->s[i+1];

		/* Cursor moves back too */
		editor->pos--;
		editor->updateflags |= TUD_CENTER;
	}
	else if (editor->curline->prev != NULL) {
		if (strlen(editor->curline->s) == 0) {
			/* remove current line & move up & to eol */
			editor->text->cur = editor->curline;
			editor->curline = editor->curline->prev;
			editor->pos = strlen(editor->curline->s);

			deletestring(editor->text);

			editor->updateflags |= TUD_TOP | TUD_CENTER;
		}
		else if (strlen(editor->curline->prev->s) == 0) {
			/* remove previous line */
			editor->text->cur = editor->curline->prev;
			deletestring(editor->text);

			/* update center too, in case @ line has moved to top now */
			editor->updateflags |= TUD_TOP | TUD_CENTER;
		}
		else if (strlen(editor->curline->prev->s) + 1 < editor->wrapwidth) {
			/* merge lines; wordwrap */
			char * prevline;

			i = strlen(editor->curline->prev->s);

			/* Previous line must have a space at the end */
			if (editor->curline->prev->s[i-1] != ' ' && editor->curline->s[0] != ' ') {
				/* add a space at the end */
				editor->curline->prev->s[i]     = ' ';
				editor->curline->prev->s[i + 1] = 0;
			}

			/* Grab the previous line */
			editor->text->cur = editor->curline->prev;
			prevline = removestring(editor->text);
			editor->text->cur = editor->curline;

			/* Wrap the previous line onto the beginning of the current line */
			editor->pos = wordwrap(editor->text, prevline, 0, 0, editor->wrapwidth, editor->linewidth);
			editor->curline = editor->text->cur;

			free(prevline);
			editor->updateflags |= TUD_EDITAREA;
		}
	}
}

/**
 * @relates texteditor
 * @brief Delete the character under the cursor.
 **/
void texteditDelete(texteditor * editor)
{
	int i;

	if (editor->selectflag) {
		texteditClearSelectedText(editor);
		return;
	}

	if (editor->pos < strlen(editor->curline->s)) {
		/* Slide everything after the cursor backward */
		for (i = editor->pos; i < strlen(editor->curline->s); i++)
			editor->curline->s[i] = editor->curline->s[i+1];
		editor->updateflags |= TUD_CENTER;
	}
	else if (strlen(editor->curline->s) == 0 && !(editor->text->first == editor->text->last)) {
		/* This string is empty: destroy */
		editor->text->cur = editor->curline;
		deletestring(editor->text);
		editor->curline = editor->text->cur;
		editor->pos = strlen(editor->curline->s);
		editor->updateflags |= TUD_EDITAREA;
	}
	else if (editor->curline->next != NULL) {
		if (strlen(editor->curline->next->s) == 0) {
			/* Next string is empty: destroy */
			editor->text->cur = editor->curline->next;
			deletestring(editor->text);
			editor->updateflags |= TUD_BOTTOM;
		}
		else if (strlen(editor->curline->s) + 1 < editor->wrapwidth) {
			char * nextline;

			/* merge lines; wordwrap */
			i = strlen(editor->curline->s);
			if (editor->curline->s[i-1] != ' ' && editor->curline->next->s[0] != ' ') {
				/* add a space at the end */
				editor->curline->s[i]   = ' ';
				editor->curline->s[++i] = 0;
			}

			/* Remove the next line */
			editor->text->cur = editor->curline->next;
			nextline = removestring(editor->text);
			editor->text->cur = editor->curline;

			/* Wordwrap the next line onto the end of the current line */
			editor->pos = wordwrap(editor->text, nextline, i, -1, editor->wrapwidth, editor->linewidth);
			editor->curline = editor->text->cur;

			free(nextline);
			editor->updateflags |= TUD_CENTER | TUD_BOTTOM | TUD_TOP;
		}
	}
}


/**
 * @relates texteditor
 * @brief Delete the current line.
 **/
void texteditDeleteLine(texteditor * editor)
{
	/* Move cursor to the beginning of the line */
	editor->pos = 0;
	editor->text->cur = editor->curline;

	if (editor->curline->next != NULL) {
		/* Delete the current line, moving curline forward */
		editor->curline = editor->curline->next;
		deletestring(editor->text);
		editor->updateflags |= TUD_CENTER | TUD_BOTTOM;
	}
	else if (editor->curline->prev != NULL) {
		/* Delete the current line, moving curline backward */
		editor->curline = editor->curline->prev;
		deletestring(editor->text);
		editor->updateflags |= TUD_TOP | TUD_CENTER;
	}
	else {
		/* Clear the current (and only) line */
		editor->curline->s[0] = 0;
		editor->updateflags |= TUD_CENTER;
	}
}

/**
 * @relates texteditor
 * @brief Prompt the user for an ASCII character to insert. If the current line
 * begins with a #char command, modify the argument to #char instead.
 **/
void texteditInsertASCII(texteditor * editor)
{
	static int selChar; /* TODO: static isn't the way to go, or is it? */
	int choice;
 
	editor->updateflags |= TUD_EDITAREA;

	/* TODO: let #char be anywhere on the line */

	if (str_equ(editor->curline->s, "#char", STREQU_UNCASE | STREQU_RFRONT)) {
		/* Change character number for a #char command */
		char * number;

		/* append decimal value for ascii char */

		sscanf(editor->curline->s + 5, "%d", &selChar);
		choice = charselect(editor->d, selChar);
		if (choice == -1)
			return;

		editor->curline->s[5] = ' ';
		editor->curline->s[6] = '\x0';

		/* change the character to a string */
		number = str_duplen("", 64);
		sprintf(number, "%d", choice);

		strcat(editor->curline->s, number);

		free(number);

		texteditValidatePosition(editor);
		editor->updateflags |= TUD_EDITAREA;
	} else {
		/* insert ascii char */

		choice = charselect(editor->d, selChar);
		if (choice == -1)
			return;

		texteditInsertCharacter(editor, choice);
	}

	/* Remember the choice for future reference */
	selChar = choice;
}

/**
 * @relates texteditor
 * @brief Insert a character at the cursor position.
 *
 * @param ch  character to insert.
 **/
void texteditInsertCharacter(texteditor * editor, int ch)
{
	int i;

	ch = ch & 0xFF;  /* Clear all but the first 8 bits */

	/* Don't insert end-of-line character. */
	if (ch == 0)
		return;

	if (editor->insertflag) {
		/* insert */
		if (strlen(editor->curline->s) < (editor->wrapwidth?editor->wrapwidth:editor->linewidth)) {
			/* insert if there is room */
			for (i = strlen(editor->curline->s) + 1; i > editor->pos; i--)
				editor->curline->s[i] = editor->curline->s[i-1];

			editor->curline->s[editor->pos++] = ch;
			editor->updateflags |= TUD_CENTER;
		}
		else if (editor->wrapwidth) {
			/* no room; wordwrap */
			texteditInsertCharAndWrap(editor, ch);
		}
	}
	else {
		/* easy replace */
		if (editor->curline->s[editor->pos] == 0) {
			/* Insert at the end of the line; not so easy. */
			if (strlen(editor->curline->s) < (editor->wrapwidth?editor->wrapwidth:editor->linewidth)) {
				editor->curline->s[editor->pos+1] = 0;
				editor->curline->s[editor->pos++] = ch;
				editor->updateflags |= TUD_CENTER;
			}
			else if (editor->wrapwidth) {
				/* no room; wordwrap */
				texteditInsertCharAndWrap(editor, ch);
			}
		}
		else {
			editor->curline->s[editor->pos++] = ch;
			editor->updateflags |= TUD_CENTER;
		}
	}
}

/**
 * @relates texteditor
 * @brief Insert a character onto a full line using wordwrap.
 *
 * @param ch  Character to insert.
 **/
void texteditInsertCharAndWrap(texteditor * editor, int ch)
{
	char * inserttext = str_duplen("", 2);
	inserttext[0] = ch;
	inserttext[1] = 0;

	editor->text->cur = editor->curline;

	editor->pos = wordwrap(editor->text, inserttext, editor->pos, editor->pos, editor->wrapwidth, editor->linewidth);

	editor->curline = editor->text->cur;

	free(inserttext);
	editor->updateflags |= TUD_EDITAREA;
}


