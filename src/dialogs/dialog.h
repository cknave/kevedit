/* dialog.h - general dialog tools
 * $Id: dialog.h,v 1.1 2003/11/01 23:45:56 bitman Exp $
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

#ifndef _DIALOG_H
#define _DIALOG_H 1

#include "display/display.h"

/* Component types */
#define DIALOG_COMP_TITLE 0
#define DIALOG_COMP_HEADING 1
#define DIALOG_COMP_LABEL 2
#define DIALOG_COMP_OPTION 3

typedef struct dialogComponent {
	int type;
	int x, y;  /* Position in dialog */
	unsigned char color;

	char * text;

	int id;  /* Unique identifier */
} dialogComponent;

typedef struct dialog {
	dialogComponent * components;  /* Static components */
	dialogComponent * options;     /* Selectable options */

	int componentcount;
	int optioncount;

	int curoption; /* Current option */

} dialog;

/* dialogInit(dialog)
 * Initialize empty dialog
 */
void dialogInit(dialog * dia);

/* dialogFree(dialog)
 * Free and empty a dialog
 */
void dialogFree(dialog * dia);

/* dialogComponentMake(type, x, y, color, text, id)
 * Generate a component from the given values
 * Memory is shared in the case of "text"
 */
dialogComponent dialogComponentMake(int type, int x, int y, unsigned char color, char * text, int id);

/* dialogAddComponent(dialog, component)
 * Add a component to a dialog
 * Component is copied completely
 */
void dialogAddComponent(dialog * dia, dialogComponent component);

/* dialogAddCompList(dialog, complist, size)
 * Add a list of components to a dialog
 */
void dialogAddCompList(dialog * dia, dialogComponent * complist, int size);

/* dialogDraw(display, dialog)
 * Draw a dialog to the given display
 */
void dialogDraw(displaymethod * mydisplay, dialog dia);

/* dialogEraseComponent(display, component)
 * Erase the given component from the screen (rarely useful) */
void dialogEraseComponent(displaymethod * mydisplay, dialogComponent * component);

/* dialogComponentEdit(display, component, editwidth, linedflags)
 * Edit the current component in the line_editor
 * using the given flags and editwidth */
int dialogComponentEdit(displaymethod * mydisplay, dialogComponent * comp, int editwidth, int linedflags);

/* dialogGetCurOption(dialog)
 * Returns a pointer to the current option
 */
dialogComponent * dialogGetCurOption(dialog dia);

/* dialogNextOption(dialog)
 * dialogPrevOption(dialog)
 * Switches the currently selected option
 */
void dialogNextOption(dialog * dia);
void dialogPrevOption(dialog * dia);


/* Testing code */
void dialogTest(displaymethod * mydisplay);

#endif
