/* help.c  -- hypertext help system
 * $Id: help.c,v 1.3 2001/11/10 22:06:07 bitman Exp $
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "help.h"

#include "svector.h"
#include "editbox.h"
#include "hypertxt.h"
#include "panel_hl.h"
#include "helplist.h"
#include "files.h"

#include <stdlib.h>
#include <string.h>

static helpsection helplist;
static char* helpdatapath;

int inithelpsystem(char* datapath)
{
	helpdatapath = str_dup(datapath);
	inithelpsection(&helplist);
	return 0;
}

void deletehelpsystem(void)
{
	deletesectionlist(&helplist);
	free(helpdatapath);
}

/* function local to this file! */
int loadmetafile(char* metafilename)
{
	stringvector meta;

	meta = filetosvector(metafilename, EDITBOX_NOEDIT, EDITBOX_NOEDIT);
	if (meta.first == NULL)  /* could not load metafile */
		return 1;

	return loadhelpmeta(&helplist, &meta); 
}

/* function local to this file! */
void helploadmetafile(void)
{
	static int loaded = 0;
	if (!loaded) {
		char* buffer = str_duplen(helpdatapath, strlen(helpdatapath) + 14);
		strcat(buffer, "/kevedit.zml");
		loadmetafile(buffer);
		free(buffer);
		loaded = 1;
	}
}

/* function local to this file! */
void helploadfile(char* filename)
{
	helploadmetafile();  /* Make sure the metafile is loaded */

	/* Make sure the file isn't already available */
	if (findsection(&helplist, filename) == NULL) {
		stringvector
			file = filetosvector(filename, EDITBOX_NOEDIT, EDITBOX_NOEDIT);

		if (file.first == NULL) {
			/* Try adding a .hlp extension */
			char* fullname = str_duplen(filename, strlen(filename) + 5);
			strcat(fullname, ".hlp");
			file = filetosvector(fullname, EDITBOX_NOEDIT, EDITBOX_NOEDIT);
			free(fullname);
		}

		/* If we loaded it, add to the list */
		if (file.first != NULL) {
			helpsection* section = (helpsection*) malloc(sizeof(helpsection));
			inithelpsection(section);
			section->title = str_dup(filename);
			section->sv = file;

			appendsection(&helplist, section);
		}
	}
}


void help(displaymethod* d)
{
	helploadmetafile();  /* Load the metafile if it hasn't already been loaded */
	if (findsection(&helplist, "index") != NULL) {
		helpsectiontopic("index", NULL, d);
	} else {
		stringvector aboutdialog;
		initstringvector(&aboutdialog);

		pushstring(&aboutdialog, str_dup("@About KevEdit"));
		pushstring(&aboutdialog, str_dup("$KevEdit Version " VERSION));
		pushstring(&aboutdialog, str_dup("Copyright (C) 2000-2001 Kev Vance, et al."));
		pushstring(&aboutdialog, str_dup("Distribute under the terms of the GNU GPL"));
		editbox("", &aboutdialog, 0, EDITBOX_ZOCMODE | EDITBOX_MOVEMENT, d);

		deletestringvector(&aboutdialog);
	}
}

int
helptopic(stringvector section, char* topic, displaymethod* d)
{
	int retcode;

	/* Try searching for the topic if one is provided, otherwise look for "top" */
	if (topic != NULL && topic[0] != '\0')
		findhypermessage(topic, &section);
	else
		findhypermessage("top", &section);

	/* draw the help panel for all the world to see */
	drawpanelhelp(d);
	
	while (1) {
		retcode = editbox("Help", &section, 0, EDITBOX_ZOCMODE | EDITBOX_MOVEMENT, d);

		if (retcode == EDITBOX_OK || retcode == EDITBOX_FORWARD) {
			if (ishypermessage(section)) {
				int keepbrowsing;
				char* msg = gethypermessage(section);

				if (ishypersection(msg)) {
					/* Load a (potentially) different section */
					char* hypersection, * hypermsg;

					hypersection = gethypersection(msg);
					hypermsg     = gethypersectionmessage(msg);

					keepbrowsing = helpsectiontopic(hypersection, hypermsg, d);

					free(hypersection);
					free(hypermsg);
				} else {
					keepbrowsing = helptopic(section, msg, d);
				}
				free(msg);

				/* If we are done browsing, return as such */
				if (!keepbrowsing)
					return 0;
			}
		} else if (retcode == EDITBOX_BACK || retcode == EDITBOX_BACKWARD) {
			/* Go back to the previous dialog and go again */
			return 1;
		} else if (retcode == EDITBOX_CANCEL) {
			/* Finished */
			return 0;
		}
	}
}


int
helpsectiontopic(char* sectiontitle, char* topic, displaymethod* d)
{
	helpsection* sectionnode;
	stringvector section;

	helploadfile(sectiontitle);

	sectionnode = findsection(&helplist, sectiontitle);
	if (sectionnode != NULL) {
		/* Grab the section svector from the section node and use it */
		section = sectionnode->sv;
		if (section.first != NULL) {
			section.cur = section.first;
			return helptopic(section, topic, d);
		}
	}

	return 1;
}


/* Help panel */

void drawpanelhelp(displaymethod* d)
{
	int x, y, i = 0;

	for (y = 0; y < PANEL_HELP_DEPTH + 0; y++) {
		for (x = 0; x < PANEL_HELP_WIDTH; x++) {
			d->putch(x + 60, y + 3, PANEL_HELP[i], PANEL_HELP[i + 1]);
			i += 2;
		}
	}
}

