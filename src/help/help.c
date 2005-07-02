/* help.c  -- hypertext help system
 * $Id: help.c,v 1.3 2005/07/02 21:31:30 kvance Exp $
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

#include "help.h"

#include "themes/theme.h"

#include "structures/svector.h"
#include "texteditor/editbox.h"
#include "hypertxt.h"

#include "helplist.h"
#include "dialogs/files.h"
#include "kevedit/screen.h"

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
		char* buffer = str_dupadd(helpdatapath, 14);
		strcat(buffer, "/kevedit.zml");
		if (fileexists(buffer))
			loadmetafile(buffer);
		else {
			free(buffer);
			buffer = str_dupadd(DATAPATH, 14);
			strcat(buffer, "/kevedit.zml");
			if (fileexists(buffer))
				loadmetafile(buffer);
		}
		free(buffer);
		loaded = 1;
	}
}

/* function local to this file! */
stringvector helploadfile(char* filename)
{
	stringvector
		file = filetosvector(filename, EDITBOX_NOEDIT, EDITBOX_NOEDIT);

	if (file.first == NULL) {
		/* Try adding a .hlp extension */
		char* fullname = str_duplen(filename, strlen(filename) + 5);
		strcat(fullname, ".hlp");
		file = filetosvector(fullname, EDITBOX_NOEDIT, EDITBOX_NOEDIT);
		free(fullname);
	}

	return file;
}

/* Load a help topic if it isn't already available */
void helploadtopic(char* topic)
{
	helploadmetafile();  /* Make sure the metafile is loaded */

	if (findsection(&helplist, topic) == NULL) {
		stringvector file = helploadfile(topic);

		if (file.first == NULL) {
			/* Try loading from the data directory */
			char* fullname = fullpath(helpdatapath, topic, SLASH_DEFAULT);
			file = helploadfile(fullname);
			free(fullname);
		}
		if (file.first == NULL) {
			/* Try loading from the docs subdirectory */
			char* midname = fullpath("docs", topic, SLASH_DEFAULT);
			char* fullname = fullpath(helpdatapath, midname, SLASH_DEFAULT);
			file = helploadfile(fullname);
			free(midname);
			free(fullname);
		}

		/* If we loaded it, add to the list */
		if (file.first != NULL) {
			helpsection* section = (helpsection*) malloc(sizeof(helpsection));
			inithelpsection(section);
			section->title = str_dup(topic);
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
		pushstring(&aboutdialog, str_dup("$KevEdit Version " PACKAGE_VERSION));
		pushstring(&aboutdialog, str_dup("Copyright (C) 2000-2005 Kev Vance, et al."));
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
	drawsidepanel(d, PANEL_HELP);
	
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

	helploadtopic(sectiontitle);

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

