/* zlaunch - zzt launching library (everything necessary to run zzt smoothly) */
/* $id$ */
/* Copyright (c) 2001 Ryan Phillips <bitman@users.sf.net> */

/* Configuration settings for the zlauncher */
typedef struct zlaunchinfo {
	int copyzztdat;
	int copyhlpfiles;
	char* stdfont;       /* Name of standard font program */
	char* keystrokes;    /* Keystrokes to be "performed" when running zzt */
} zlaunchinfo;

/* Copy types */
#define COPY_DONT 0       /* Never copy */
#define COPY_PERMINANT 1  /* Copy and leave there */
#define COPY_TEMPORARY 2  /* Copy and remove when done */
#define COPY_OVERWRITE 3  /* Overwrite existing files and leave there */

/* Types of slashes */
#define SLASH_DEFAULT 0
#define SLASH_FORWARD 0
#define SLASH_BACK    1

/* Info operations */
void  initzlinfo(zlaunchinfo* zli);
void  deletezlinfo(zlaunchinfo* zli);
zlaunchinfo loadzlinfofromfile(char* filename);

/* Run zzt on the given world, all bells and whistles handled perfectly */
int   runzzt(char* path, char* world);

/* Run a program with given path and arguments in current directory */
int   run(char* path, char* program, char* args);

/* File copying */
int   copyfilebydir(char* srcdir, char* destdir, char* filename);
int   copyfile(char* srcname, char* destname);

/* Determine path of self from main's argv[0] */
char* locateself(char* argv0);

/* Deduce filename or path from a file's full path */
char* fileof(char* buffer, char* fullpath, int buflen);
char* pathof(char* buffer, char* fullpath, int buflen);
char* fullpath(char* path, char* file, int slashtype);

/* Keboard buffer stuffing routines */
void performkeystrokes(char* keystrokes);
void stuffkbdbuffer(unsigned char scancode, unsigned char ch);
