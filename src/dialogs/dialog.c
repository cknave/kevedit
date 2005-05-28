/* dialog.c - general dialog tools
 * $Id: dialog.c,v 1.2 2005/05/28 03:17:45 bitman Exp $
 * Copyright (C) 2001 Ryan Phillips <bitman@users.sourceforge.net>
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
 * Foundation, Inc., 59 Temple Place Suite 330; Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "dialog.h"

#include "structures/svector.h"

#include "kevedit/screen.h"

#include "display/display.h"

#include <stdlib.h>
#include <string.h>

/* Bounds of the dialog */
#define TITLE_Y 4
#define START_X 9
#define START_Y 6
#define END_X 49
#define END_Y 20

/* dialogCompListFree(complist, size)
 * Frees a list of components (including the list itself)
 * Helper for dialogFree()
 */
void dialogCompListFree(dialogComponent * complist, int size)
{
	int i;

	for (i = 0; i < size; i++)
		if (complist[i].text != NULL)
			free(complist[i].text);
	free(complist);
}

/* dialogCompListGrow(complist, size)
 * Grow the given list by one from given original size
 * Returns pointer to the new list
 * Helper for dialogAddComponent
 */
dialogComponent * dialogCompListGrow(dialogComponent * complist, int size)
{
	dialogComponent * newlist;
	newlist = (dialogComponent *) malloc(sizeof(dialogComponent) * (size + 1));
	if (complist != NULL && newlist != NULL) {
		memcpy(newlist, complist, sizeof(dialogComponent) * size);
		free(complist);
	}

	return newlist;
}

/* dialogCompontentDraw(display, component)
 * Draw a component list to a displaymethod
 */
void dialogComponentDraw(displaymethod * mydisplay, dialogComponent component)
{
	char * buffer;
	/* Don't display options out of bounds */
	if (START_Y + component.y > END_Y)
		return;
	if (component.text == NULL)
		return;

	/* Protect against writing past right boundry */
	buffer = str_duplen(component.text, END_X - (START_X + component.x) + 1);
	if (strlen(component.text) > strlen(buffer)) {
		int i;
		for (i = 0; i < 3 && strlen(buffer) - 1 - i >= 0; i++)
			buffer[strlen(buffer) - 1 - i] = '.';
	}

	switch (component.type) {
		case DIALOG_COMP_TITLE:
			/* Display title in title-bar */
			mydisplay->print_discrete(30 - (strlen(buffer) / 2), TITLE_Y, component.color, buffer);
			break;
		case DIALOG_COMP_HEADING:
			mydisplay->print_discrete(30 - (strlen(buffer) / 2), START_Y + component.y, component.color, buffer);
			break;
		default:
			mydisplay->print_discrete(START_X + component.x, START_Y + component.y,
											 component.color, buffer);
	}

	free(buffer);
}

void dialogEraseComponent(displaymethod * mydisplay, dialogComponent * component)
{
	int i;

	switch (component->type) {
		case DIALOG_COMP_TITLE:
		case DIALOG_COMP_HEADING:
			/* TODO: implement */
			break;
		default:
			for (i = 0; i < strlen(component->text) && START_X + component->x + i <= END_X; i++)
				mydisplay->putch_discrete(START_X + component->x + i, START_Y + component->y, ' ', 0x0F);
			mydisplay->update(START_X + component->x, START_Y + component->y, i, 1);
	}
}


void dialogInit(dialog * dia)
{
	dia->components = NULL;
	dia->options = NULL;

	dia->componentcount = dia->optioncount = dia->curoption = 0;
}

void dialogFree(dialog * dia)
{
	/* Free the component lists */
	dialogCompListFree(dia->components, dia->componentcount);
	dialogCompListFree(dia->options, dia->optioncount);

	/* Re-initialize */
	dialogInit(dia);
}

dialogComponent dialogComponentMake(int type, int x, int y, unsigned char color, char * text, int id)
{
	dialogComponent comp = { type, x, y, color, text, id };
	return comp;
}

void dialogAddComponent(dialog * dia, dialogComponent component)
{
	if (component.type != DIALOG_COMP_OPTION) {
		dia->components = dialogCompListGrow(dia->components, dia->componentcount);

		/* Copy onto the new component */
		dia->components[dia->componentcount] = component;
		if (component.text != NULL) {
			dia->components[dia->componentcount].text = str_dup(component.text);
		}
		/* Increase component count to reflect the change */
		dia->componentcount++;
	} else {
		/* If I wasn't so lazy this code would be in a seperate function */
		dia->options = dialogCompListGrow(dia->options, dia->optioncount);

		/* Copy onto the new component */
		dia->options[dia->optioncount] = component;
		if (component.text != NULL) {
			dia->options[dia->optioncount].text = str_dup(component.text);
		}
		/* Increase component count to reflect the change */
		dia->optioncount++;
	}
}

void dialogAddCompList(dialog * dia, dialogComponent * complist, int size)
{
	int i;
	for (i = 0; i < size; i++)
		dialogAddComponent(dia, complist[i]);
}

void dialogDraw(displaymethod * mydisplay, dialog dia)
{
	int i;

	/* Draw the scrollbox without pointers */
	drawscrollbox(mydisplay, 0, 0, 0);
	mydisplay->putch_discrete( 7, 13, ' ', 0x00);
	mydisplay->putch_discrete(51, 13, ' ', 0x00);

	/* Draw the components */
	if (dia.components != NULL)
		for (i = 0; i < dia.componentcount; i++)
			dialogComponentDraw(mydisplay, dia.components[i]);

	/* Draw the options */
	if (dia.options != NULL)
		for (i = 0; i < dia.optioncount; i++)
			dialogComponentDraw(mydisplay, dia.options[i]);

	/* Draw pointer to current option */
	if (dia.options != NULL) {
		dialogComponent curopt = dia.options[dia.curoption];
		mydisplay->putch_discrete(START_X - 2, START_Y + curopt.y, '\xAF', 0x02);
		mydisplay->putch_discrete(END_X + 2,   START_Y + curopt.y, '\xAE', 0x02);
		mydisplay->cursorgo(START_X + curopt.x, START_Y + curopt.y);
	}

	/* Update the display */
	mydisplay->update(3, 3, 52, 19);
}

int dialogComponentEdit(displaymethod * mydisplay, dialogComponent * comp, int editwidth, int linedflags)
{
	int result;
	char * buffer = str_duplen(comp->text, editwidth);

	result = line_editor(START_X + comp->x, START_Y + comp->y, comp->color, buffer, editwidth, linedflags, mydisplay);

	if (result == LINED_OK) {
		free(comp->text);
		comp->text = buffer;
	} else {
		free(buffer);
	}

	return result;
}

dialogComponent * dialogGetCurOption(dialog dia)
{
	/* Sneaky sneaky sneaky */
	return dia.options + dia.curoption;
}

void dialogNextOption(dialog * dia)
{
	if (++(dia->curoption) >= dia->optioncount)
		dia->curoption = 0;
}

void dialogPrevOption(dialog * dia)
{
	if (--(dia->curoption) < 0)
		dia->curoption = dia->optioncount - 1;
}

void dialogTest(displaymethod * mydisplay)
{
	dialog dia;

	dialogComponent comps[] = {
		{ DIALOG_COMP_TITLE,   0, 0, 0x0F, "Dialog Test", 0 },
		{ DIALOG_COMP_HEADING, 0, 0, 0x0a, "Here be the Test", 0 },
		{ DIALOG_COMP_LABEL,   0, 2, 0x03, "Label:", 0 },
		{ DIALOG_COMP_OPTION, 10, 2, 0x0b, "option1", 1 },
		{ DIALOG_COMP_LABEL,   0, 3, 0x03, "Whoa:", 0 },
		{ DIALOG_COMP_OPTION, 10, 3, 0x0b, "option2", 2 },
		{ DIALOG_COMP_LABEL,   0, 4, 0x03, "Foo:", 0 },
		{ DIALOG_COMP_OPTION, 10, 4, 0x0b, "bar", 3 },
	};

	dialogInit(&dia);

	dialogAddCompList(&dia, comps, 8);
	dialogAddComponent(&dia, dialogComponentMake(DIALOG_COMP_LABEL, 0, 6, 0x0F, "Hello", 0));

	dialogPrevOption(&dia);

	dialogDraw(mydisplay, dia);

	mydisplay->putch(END_X, END_Y, 'x', 0x02);
	mydisplay->getch();

	dialogFree(&dia);
}

