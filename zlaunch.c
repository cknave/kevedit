/* zlaunch - zzt launching library (everything necessary to run zzt smoothly) */
/* $id$ */
/* Copyright (c) 2001 Ryan Phillips <bitman@users.sf.net> */

#include "zlaunch.h"

#include "svector.h"
#include "files.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

/* Keyboard buffer stuffing */
#ifdef DOS
#include <dpmi.h>
#endif

/* Default configuration info */
#define DEFAULTZLINFOLEN 14
const char* defaultzlinfodata[] = {
	"\' zlaunch configuration file",
	"",
	"\' List of optional actions to perform:",
	"\' (remove \"zztkeys\" to disable virtual keystrokes)",
	"perform zztkeys",
	"",
	"\' List of actions and optional actions:",
	"action copydat copy zzt.dat",
	"action copyhlp copy *.hlp",
	"optional screenthief run st/st.exe",
	"optional zzfont run zzfont.com i",
	"optional zztkeys keystrokes kc<ENTER>",
	"optional play keystrokes p",
	"action zzt run zzt %1"
};

/****** Local functions **************/

/* Action operations */
void initaction(zlaunchaction* action);
void deleteactions(zlaunchaction* action);
void addaction(zlaunchinfo* zli, zlaunchaction* action);

void removefiles(stringvector files);

/****** Action creation/destruction ***************/
void initaction(zlaunchaction* action)
{
	action->name = NULL;
	action->optional = 0;

	action->type = ZL_NONE;
	action->command = 0;
	
	action->next = NULL;
}

void deleteactions(zlaunchaction* action)
{
	zlaunchaction* next;

	if (action == NULL)
		return;

	next = action->next;

	/* Cleanup */
	if (action->name != NULL) {
		free(action->name);
		action->name = NULL;
	}

	if (action->command != NULL) {
		free(action->command);
		action->command = NULL;
	}

	/* Recurse */
	deleteactions(next);
}

void addaction(zlaunchinfo* zli, zlaunchaction* action)
{
	zlaunchaction* node = zli->actions;

	if (zli->actions == NULL) {
		zli->actions = action;
		return;
	}

	/* Advance to end of list */
	while (node->next != NULL)
		node = node->next;

	node->next = action;
}


/*********** Info creation/destruction ***************/
void  initzlinfo(zlaunchinfo* zli)
{
	initstringvector(&(zli->actionstoperform));
	initstringvector(&(zli->paramlist));
	initstringvector(&(zli->filestoremove));

	zli->bindir  = NULL;
	zli->datadir = NULL;

	zli->actions = NULL;
}

void  deletezlinfo(zlaunchinfo* zli)
{
	deletestringvector(&(zli->actionstoperform));
	deletestringvector(&(zli->paramlist));
	deletestringvector(&(zli->filestoremove));

	if (zli->bindir  != NULL) { free(zli->bindir);  zli->bindir  = NULL; }
	if (zli->datadir != NULL) { free(zli->datadir); zli->datadir = NULL; }
	
	deleteactions(zli->actions);
	zli->actions = NULL;
}


/******** Stringvector to zlinfo ******************/
stringvector defaultzlinfo(void)
{
	stringvector info;
	int i;

	initstringvector(&info);

	for (i = 0; i < DEFAULTZLINFOLEN; i++) {
	pushstring(&info, str_dup((char *)defaultzlinfodata[i]));
	}

	return info;
}

zlaunchinfo loadzlinfofromsvector(stringvector info)
{
	zlaunchinfo zli;
	stringnode* cur;

	char token[256];
	int pos = 0;

	initzlinfo(&zli);

	for (cur = info.first; cur != NULL; cur = cur->next) {
		/* Get the first token on this line, space seperated */
		pos = 0;
		tokenadvance(token, cur->s, &pos);

		if (str_equ(token, "action",   STREQU_UNCASE) ||
				str_equ(token, "optional", STREQU_UNCASE)) {
			zlaunchaction* action = (zlaunchaction*) malloc(sizeof(zlaunchaction));

			initaction(action);

			/* set optional flag if it is an optional command */
			if (str_equ(token, "optional", STREQU_UNCASE)) 
				action->optional = 1;

			/* get the action's title */
			if (tokenadvance(token, cur->s, &pos))
				action->name = str_dup(token);

			/* get the action's type */
			if (tokenadvance(token, cur->s, &pos)) {
				if      (str_equ(token, "run", STREQU_UNCASE))
					action->type = ZL_RUN;
				else if (str_equ(token, "keystrokes", STREQU_UNCASE))
					action->type = ZL_KEYSTROKES;
				else if (str_equ(token, "copy", STREQU_UNCASE))
					action->type = ZL_COPY;
				else if (str_equ(token, "permcopy", STREQU_UNCASE))
					action->type = ZL_PERMCOPY;
				else if (str_equ(token, "permcopyover", STREQU_UNCASE))
					action->type = ZL_PERMCOPYOVER;
			}

			/* get the action's command */
			while (cur->s[pos] == ' ') pos++;
			action->command = str_dup(cur->s + pos);

			/* Add the action to the list */
			addaction(&zli, action);

		} else if (str_equ(token, "perform", STREQU_UNCASE)) {
			while (tokenadvance(token, cur->s, &pos))
				pushstring(&(zli.actionstoperform), str_dup(token));
		}
	}

	return zli;
}


stringvector loadinfo(char* datapath, char* worldfile)
{
	stringvector info;
	char* filename = NULL;
	char* search;

	if (worldfile != NULL) {
		/* Try the first filename with an .zln extension tacked on */
		filename = str_dupadd(worldfile, 4);
		strcat(filename, ".zln");
		info = filetosvector(filename, 0, 0);
		free(filename);
		
		if (info.first != NULL)
			return info;

		/* Try replacing the extension */
		filename = str_dupadd(worldfile, 4);
		if ((search = strrchr(filename, '.')) != NULL) {
			strcpy(search, ".zln");
			info = filetosvector(filename, 0, 0);
		}
		free(filename);

		if (info.first != NULL)
			return info;
	}

	/* Try the default config file in the current directory */
	info = filetosvector(DEFAULTCONFIG, 0, 0);

	if (info.first != NULL)
		return info;

	/* Try the default config in the datapath */
	filename = fullpath(datapath, DEFAULTCONFIG, SLASH_DEFAULT);
	info = filetosvector(filename, 0, 0);

	if (info.first != NULL) {
		free(filename);
		return info;
	}

	/* Get the defaults */
	info = defaultzlinfo();

	/* Create a file containing the default info */
	svectortofile(&info, filename);

	free(filename);
	return info;
}


void zlaunchact(zlaunchinfo* zli)
{
	zlaunchaction* curaction = zli->actions;

	while (curaction != NULL) {
		int cmsize = strlen(curaction->command);
		int i = 0, j = 0;
		char* command;

		/* prevent options not in the perform list from running */
		if (curaction->optional) {
			int perform = 0;
			for (svmovetofirst(&(zli->actionstoperform));
					 zli->actionstoperform.cur != NULL;
					 zli->actionstoperform.cur = zli->actionstoperform.cur->next) {
				if (str_equ(zli->actionstoperform.cur->s, curaction->name,
										STREQU_UNCASE))
					perform = 1;
			}
			svmovetofirst(&(zli->actionstoperform));
			if (!perform) {
				curaction = curaction->next;
				continue;
			}
		}

		/* parse the command for %1, %2 etc */
		command = str_dupmin("", cmsize);
		while (i < strlen(curaction->command) && j < cmsize) {
			switch (curaction->command[i]) {
				case '%':
					{
						int pos = curaction->command[++i] - 0x31;
						svmovetofirst(&(zli->paramlist));
						if (svmoveby(&(zli->paramlist), pos) == pos) {
							/* Grow the command to hold the param */
							char* oldcm = command;
							cmsize += strlen(zli->paramlist.cur->s);
							command = str_dupmin(command, cmsize);
							free(oldcm);
							strcat(command, zli->paramlist.cur->s);
							j += strlen(zli->paramlist.cur->s);
						}
						i++;
					}
				default:
					command[j++] = curaction->command[i++];
			}
		}
		command[j] = '\x0';

		switch (curaction->type) {
			case ZL_RUN:
				run(zli->bindir, command, "");
				break;
			case ZL_KEYSTROKES:
				performkeystrokes(command);
				break;
			case ZL_COPY:
				copyfilepatternbydir(zli->datadir, ".", command, COPY_NOOVERWRITE,
														 &(zli->filestoremove));
				break;
			case ZL_PERMCOPY:
				copyfilepatternbydir(zli->datadir, ".", command, COPY_NOOVERWRITE, NULL);
				break;
			case ZL_PERMCOPYOVER:
				copyfilepatternbydir(zli->datadir, ".", command, COPY_OVERWRITE, NULL);
				break;
		}

		/* Advance and take another spin */
		curaction = curaction->next;
		free(command);
	}
}

void zlaunchcleanup(zlaunchinfo* zli)
{
	stringvector* files = &(zli->filestoremove);

	/* Start from the top */
	svmovetofirst(files);

	while (files->cur != NULL) {
		/* Delete the file */
		remove(files->cur->s);

		/* Delete the string */
		deletestring(files);
	}
}


void
stuffkbdbuffer(unsigned char scancode, unsigned char ch)
{
#ifdef DOS
	__dpmi_regs r;

	r.x.ax = 0x0500;
	r.x.cx = ch | ((int)scancode << 8);
	__dpmi_int(0x16, &r);
#endif
}

void
performkeystrokes(char* keystrokes)
{
	int i;

	for (i = 0; i < strlen(keystrokes); i++) {
		if (keystrokes[i] == '<') {
			if (str_equ(keystrokes + i + 1, "enter", STREQU_FRONT | STREQU_UNCASE))
				stuffkbdbuffer(0, '\x0D');
			if (str_equ(keystrokes + i + 1, "esc", STREQU_FRONT | STREQU_UNCASE))
				stuffkbdbuffer(0x01, '\x1B');
			if (str_equ(keystrokes + i + 1, "left", STREQU_FRONT | STREQU_UNCASE))
				stuffkbdbuffer(0x4B, 0);
			/* Advance to closing bracket */
			while (keystrokes[i] != '>' && keystrokes[i] != '\x0')
				i++;
		} else {
			stuffkbdbuffer(0, keystrokes[i]);
		}
	}
}

