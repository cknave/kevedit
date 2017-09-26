/* main.c       -- The buck starts here
 * $Id: main.c,v 1.3 2005/06/29 03:20:34 kvance Exp $
 * Copyright (C) 2000-2001 Kev Vance <kvance@kvance.com>
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

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "kevedit.h"
#include "display/display.h"
#include "libzzt2/zzt.h"

#include "help/help.h"
#include "texteditor/register.h"

#include "dialogs/files.h"

ZZTworld * getWorldFromArg(char * arg, char * datapath);
displaymethod * pickdisplay(displaymethod * rootdisplay);
static char *find_datapath(char *argv0);

int main(int argc, char **argv)
{
	keveditor * editor;

	ZZTworld      * myworld;
	displaymethod * mydisplay;
	char          * datapath;

	RegisterDisplays();
	mydisplay = pickdisplay(&display);

	if (!mydisplay->init()) {
		printf("\nFailed to initialize %s display version %s.  Exiting.\n",
		       mydisplay->name, mydisplay->version); 
		return 1;
	}

	/* Search a few places for the data files. */
        char *exe_path = locateself(argv[0]);
	datapath = find_datapath(exe_path);
	inithelpsystem(datapath);

        /* Linux appimage: restore the original working directory. */
        char *owd = getenv("OWD");
        if(owd) {
            chdir(owd);
        }

	myworld = NULL;

	/* Did we get a world on the command line? */
	if (argc > 1)
		myworld = getWorldFromArg(argv[1], exe_path);

	/* Create the blank world */
	if (myworld == NULL)
		myworld = zztWorldCreate(NULL, NULL);

	if (myworld == NULL) {
		fprintf(stderr, "Error: could not create blank world, exiting.\n");
		return 1;
	}

	/* Switch to the start board */
	zztBoardSelect(myworld, zztWorldGetStartboard(myworld));

	/* Create the editor */
	editor = createkeveditor(myworld, mydisplay, datapath);

	kevedit(editor);

	editor->mydisplay->end();

	/* Free the registers used by copy & paste in the ZOC editor */
	deleteregisters();

	/* Free the loaded help system */
	deletehelpsystem();

	/* Free the world */
	zztWorldFree(editor->myworld);

	/* Free the editor */
	deletekeveditor(editor);

	return 0;
}

ZZTworld * getWorldFromArg(char * arg, char * datapath)
{
	int buflen = strlen(arg) + 4;
	char * buffer = str_duplen("", buflen);
	ZZTworld * myworld;

	/* Switch to the directory given within the filename */
	pathof(buffer, arg, buflen);
	chdir(buffer);

	/* Open the file */
	fileof(buffer, arg, buflen - 4);
	myworld = zztWorldLoad(buffer);
	if (myworld == NULL) {
		/* Maybe they left off the .zzt extension? */
		strcat(buffer, ".zzt");
		myworld = zztWorldLoad(buffer);
	}

	if (myworld == NULL) {
		myworld = zztWorldCreate(arg, NULL);
	}

	free(buffer);

	return myworld;
}

displaymethod * pickdisplay(displaymethod * rootdisplay)
{
	int x, i;
	displaymethod* mydisplay = rootdisplay;
	char *string = (char *) malloc(sizeof(char) * 256);

	/* How many display methods? */
	for (x = 1; x < 9; x++) {
		if (mydisplay->next == NULL) {
			break;
		}
		mydisplay = mydisplay->next;
	}

	if (x > 1) {
		/* More than 1 display method available, user must choose */
		printf("Hi.  This seems to be your first time running KevEdit.  What display method\n"
		       "works best on your platform?\n\n");

		mydisplay = rootdisplay;
		for (i = 0; i < x; i++) {
			printf("[%d] %s\n", i + 1, mydisplay->name);
			if (mydisplay->next != NULL)
				mydisplay = mydisplay->next;
		}
		do {
			printf("\nSelect [1-%i]: ", x);
			fgets(string, 255, stdin);
			i = string[0];
		}
		while (i > 49 && i < 51);
		i -= '1';
		printf("\n%d SELECT\n", i);
		mydisplay = rootdisplay;
		for (x = 0; x < i; x++) {
			mydisplay = mydisplay->next;
		}
	} else {
		mydisplay = rootdisplay;
	}

	free(string);
	return mydisplay;
}

char *find_datapath(char *argv0) {
    /* Look for kevedit.zml in a few places relative to the executable's path. */
    char *exe_path = locateself(argv0);
    if(!exe_path) {
        return NULL;
    }

    char *relative_paths[] = {
        "",
        "/share/kevedit",
        "/../Resources",
    };

    char pathbuf[PATH_MAX];
    int i;
    for(i = 0; i < sizeof(relative_paths)/sizeof(*relative_paths); i++) {
        snprintf(pathbuf, PATH_MAX, "%s%s/kevedit.zml", exe_path, relative_paths[i]);
        if(access(pathbuf, F_OK) == 0) {
            /* Return a copy of the final path without the test file. */
            snprintf(pathbuf, PATH_MAX, "%s%s", exe_path, relative_paths[i]);
            return strdup(pathbuf);
        }
    }

    /* Couldn't find it, just return the original path. */
    return exe_path;
}

