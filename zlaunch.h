/* zlaunch - zzt launching library (everything necessary to run zzt smoothly) */
/* $id$ */
/* Copyright (c) 2001 Ryan Phillips <bitman@users.sf.net> */

#include "svector.h"

/* Default config file to use */
#define DEFAULTCONFIG "default.zln"

/* Actions to be performed by the launcher */
#define ZL_NONE         0
#define ZL_RUN          1
#define ZL_KEYSTROKES   2
#define ZL_COPY         3
#define ZL_PERMCOPY     4
#define ZL_PERMCOPYOVER 5

/* Action structure */
typedef struct zlauchaction {
	char* name;              /* name of the action */
	int optional;            /* false will force this action to be performed */

	int type;                /* Type of action (one of above defines) */
	char* command;           /* Action command */

	struct zlauchaction* next;    /* Next action in the list */
} zlaunchaction;

/* Configuration settings for the zlauncher */
typedef struct zlaunchinfo {
	stringvector actionstoperform;   /* list of action names to actually use */
	stringvector paramlist;          /* list of parameters */
	char* bindir;                    /* location of binaries */
	char* datadir;                   /* location of datafiles */
	stringvector filestoremove;      /* list of files to delete when finished */

	zlaunchaction* actions;    /* Linked list of actions */
} zlaunchinfo;


/* Info operations */
void initzlinfo(zlaunchinfo* zli);
void deletezlinfo(zlaunchinfo* zli);

stringvector defaultzlinfo(void);
zlaunchinfo loadzlinfofromsvector(stringvector info);

/* Load info from a number of sources */
stringvector loadinfo(char* datapath, char* worldfile);

/* zlaunching */
void zlaunchact(zlaunchinfo* zli);
void zlaunchcleanup(zlaunchinfo* zli);

/* Keboard buffer stuffing routines */
void performkeystrokes(char* keystrokes);
void stuffkbdbuffer(unsigned char scancode, unsigned char ch);
