/* zlaunch - zzt launching library (everything necessary to run zzt smoothly) */
/* $id$ */
/* Copyright (c) 2001 Ryan Phillips <bitman@users.sf.net> */

#include "zlaunch.h"

#include "svector.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>   /* For getcwd() */

/* Keyboard buffer stuffing */
#ifdef DOS
#include <dpmi.h>
#endif

#define CPBUFFERSIZE 1024


void  initzlinfo(zlaunchinfo* zli)
{
	zli->copyzztdat   = COPY_TEMPORARY;
	zli->copyhlpfiles = COPY_TEMPORARY;
	zli->stdfont      = str_dup("zzfont.com");
	zli->keystrokes   = str_dup("kc<ENTER>");
}

void  deletezlinfo(zlaunchinfo* zli)
{
	if (zli->stdfont != NULL) free(zli->stdfont);
	if (zli->keystrokes != NULL) free(zli->keystrokes);
}

zlaunchinfo loadzlinfofromfile(char* filename)
{
	char buffer[CPBUFFERSIZE];
	FILE* infofile;
	zlaunchinfo zli;

	initzlinfo(&zli);

	if ((infofile = fopen(filename, "r")) == NULL) {
		return zli;
	}

	fgets(buffer, CPBUFFERSIZE, infofile);
	fclose(infofile);

	if (strlen(buffer) <= 0)
		return zli;

	if (zli.keystrokes != NULL)
		free(zli.keystrokes);

	zli.keystrokes = str_dup(buffer);

	return zli;
}

int
runzzt(char* path, char* world)
{
	zlaunchinfo zli;
	char* configfile = fullpath(path, "zlaunch.cfg", SLASH_DEFAULT);

	zli = loadzlinfofromfile(configfile);
	free(configfile);

	copyfilebydir(path, ".", "zzt.dat");
	run(path, zli.stdfont, "I");

	performkeystrokes(zli.keystrokes);

	if (run(path, "zzt", world))
		if (run(".", "zzt", world))
			if (run("", "zzt", world))
				return 1;

	deletezlinfo(&zli);
	return 0;
}

int
run(char* path, char* program, char* args)
{
	int result;
	char* command = malloc(sizeof(char) *
												 (strlen(path)+strlen(program)+strlen(args)+3));

	strcpy(command, path);
	strcat(command, "/");
	strcat(command, program);
	strcat(command, " ");
	strcat(command, args);

	result = system(command);

	free(command);

	return result;
}


int
copyfilebydir(char* srcdir, char* destdir, char* filename)
{
	int result;
	char* srcname  = malloc(sizeof(char)*(strlen(srcdir) +strlen(filename)+2));
	char* destname = malloc(sizeof(char)*(strlen(destdir)+strlen(filename)+2));

	strcpy(srcname, srcdir);
	strcat(srcname, "/");
	strcat(srcname, filename);

	strcpy(destname, destdir);
	strcat(destname, "/");
	strcat(destname, filename);

	result = copyfile(srcname, destname);

	free(destname);
	free(srcname);

	return result;
}

int
copyfile(char* srcname, char* destname)
{
	FILE* src, * dest;
	size_t readsize;
	int* copybuffer;

	/* Check for existence of destination file */
	dest = fopen(destname, "rb");
	if (dest != NULL) {
		/* The destination file already exists */
		fclose(dest);
		return 0;
	}

	/* Open the source file for reading */
	src = fopen(srcname, "rb");
	if (src == NULL) {
		/* The source file cannot be opened! Error! */
		fprintf(stderr, "Cannot open %s for reading.", srcname);
		return 1;
	}
	
	/* Open the destination file for writing */
	dest = fopen(destname, "wb");
	if (dest == NULL) {
		/* Can't write to this file */
		fprintf(stderr, "Cannot open %s for writing.", destname);

		fclose(src);
		return 1;
	}

	/* Copy src to dest */
	copybuffer = (int*) malloc(sizeof(int) * CPBUFFERSIZE);
	do {
		readsize = fread(copybuffer, sizeof(int), CPBUFFERSIZE, src);
		fwrite(copybuffer, sizeof(int), readsize, dest);
	} while (readsize == CPBUFFERSIZE);
	free(copybuffer);

	/* Close everything down */
	fclose(src);
	fclose(dest);

	return 0;
}

#define CWDMAXLEN 4096
/* Returns a malloc()ed string bearing the location of this program based
 * on main's argv[0]. Not useful for much else. */
char*
locateself(char* argv0)
{
	char* path = (char*) malloc(sizeof(char) * (strlen(argv0) + 1));
	char* cwd = NULL;
	char* fullpathname = NULL;

	pathof(path, argv0, strlen(argv0));

	/* If we find a ':' in the path of this program, it's probably a
	 * DOS full path (anyone know of an exception to this???) */
	if (strchr(path, ':') != NULL)
		return path;

	/* path is a relative path, so we need to prepend the current dir */

	cwd = getcwd(NULL, CWDMAXLEN + 1);
	if (cwd == NULL)
		return path;

	fullpathname = (char*) malloc(sizeof(char) * (strlen(path)+strlen(cwd)+2));
	strcpy(fullpathname, cwd);
	strcat(fullpathname, "/");
	strcat(fullpathname, path);

	free(path);
	free(cwd);
	return fullpathname;
}

/* Extracts the local filename from filename given by fullpath */
char*
fileof(char* buffer, char* fullpath, int buflen)
{
	char* lastslash, * lastbackslash, * start;
	int i;

	lastslash     = strrchr(fullpath, '/');
	lastbackslash = strrchr(fullpath, '\\');

	if (lastslash == NULL && lastbackslash == NULL)
		start = fullpath;
	else if (lastslash > lastbackslash)
		start = lastslash + 1;
	else
		start = lastbackslash + 1;

	for (i = 0; start[i] != '\0' && i < buflen; i++)
		buffer[i] = start[i];
	buffer[i] = '\0';

	return buffer;
}

/* Extracts the path from filename given by fullpath */
char*
pathof(char* buffer, char* fullpath, int buflen)
{
	char* lastslash, * lastbackslash;
	int end;
	int i;

	lastslash     = strrchr(fullpath, '/');
	lastbackslash = strrchr(fullpath, '\\');

	if (lastslash == NULL && lastbackslash == NULL) {
		strcpy(buffer, ".");
		return buffer;
	} else if (lastslash > lastbackslash)
		end = lastslash - fullpath;
	else
		end = lastbackslash - fullpath;

	for (i = 0; i < end && i < buflen; i++)
		buffer[i] = fullpath[i];
	buffer[i] = '\0';

	return buffer;
}

char*
fullpath(char* path, char* file, int slashtype)
{
	char* fullpath = (char*) malloc(sizeof(char)*(strlen(path)+strlen(file)+2));
	strcpy(fullpath, path);
	strcat(fullpath, (slashtype == SLASH_FORWARD ? "/" : "\\"));
	strcat(fullpath, file);

	return fullpath;
}


int
fileexists(char* filename)
{
	FILE* testfile;

	testfile = fopen(filename, "rb");
	if (testfile == NULL) 
		return 0;

	fclose(testfile);
	return 1;
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
			/* TODO: don't assume they used ENTER */
			stuffkbdbuffer(0, '\x0D');
			/* Advance to closing bracket */
			while (keystrokes[i] != '>' && keystrokes[i] != '\x0')
				i++;
		} else {
			stuffkbdbuffer(0, keystrokes[i]);
		}
	}
}

