/* world.c	-- World functions
 * $Id: world.c,v 1.3 2005/06/29 03:20:34 kvance Exp $
 * Copyright (C) 2001 Kev Vance <kvance@kvance.com>
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "strtools.h"
#include "zzt.h"

ZZTworld *zztWorldCreate(char *filename, char *title)
{
	ZZTworld *world;
	int i;

	/* Create this world from scratch */
	world = malloc(sizeof(ZZTworld));
	world->header = malloc(sizeof(ZZTworldinfo));
	/* Zero the header, then set initial values */
	memset(world->header, 0, sizeof(ZZTworldinfo));
	/* Health = 100 */
	zztWorldSetHealth(world, 100);
	/* Title = given or 'name' of filename or UNTITLED */
	if(title != NULL)
		zztWorldSetTitle(world, title);
	else if(filename != NULL) {
		strncpy((char *)world->header->title, filename, 8);
		world->header->title[8] = '\0';
		for(i = 0; i < 8; i++) {
			if(world->header->title[i] == '.')
				world->header->title[i] = '\0';
		}
	} else
		zztWorldSetTitle(world, "UNTITLED");
	/* 1st board = Title Screen */
	world->boards = zztBoardCreate("Title screen");
	world->header->boardcount = 1;
	/* Filename = given or untitled.zzt */
	world->filename = str_dup(filename != NULL ? filename : "untitled.zzt");
	/* Decompress the current/first board */
	world->cur_board = 0;
	zztBoardDecompress(&(world->boards[0]));

	/* Finished, return the world */
	return world;
}

void zztWorldFree(ZZTworld *world)
{
	/* Free all boards */
	while(zztWorldGetBoardcount(world) != 0)
		zztWorldDeleteBoard(world, 0, 0);

	/* Free everything else */
	free(world->header);
	free(world->filename);
	free(world);
}

ZZTworld *zztWorldLoad(char *filename)
{
	ZZTworld *world;
	FILE *fp;
       
	/* Open file */
	fp = fopen(filename, "rb");
	if(fp == NULL)
		return NULL;

	/* Read from file */
	world = zztWorldRead(fp);
	fclose(fp);

	if(world != NULL) {
		free(world->filename);
		world->filename = malloc(strlen(filename)+1);
		strcpy(world->filename, filename);

		/* Decompress the current/first board */
		world->cur_board = 0;
		zztBoardDecompress(&(world->boards[0]));
	}

	/* Done */
	return world;
}

int zztWorldSave(ZZTworld *world)
{
	int result;
	FILE *fp;
	
	/* Open file */
	fp = fopen(world->filename, "wb");
	if(fp == NULL)
		return 0;
	
	/* Commit current board */
	zztBoardCommit(world);
	/* Write to file */
	result = zztWorldWrite(world, fp);
	fclose(fp);

	/* Decompress the current board */
	zztBoardDecompress(&(world->boards[zztBoardGetCurrent(world)]));

	/* Done */
	return result;
}

ZZTworld * zztWorldClear(ZZTworld *world)
{
	zztWorldFree(world);
	return zztWorldCreate(NULL, NULL);
}

void zztWorldSetBoardcount(ZZTworld *world, uint16_t count)
{
	// XXX Don't ever use this, it's just here for completeness
	world->header->boardcount = count;
}
void zztWorldSetAmmo(ZZTworld *world, int16_t ammo)
{
	world->header->ammo = ammo;
}
void zztWorldSetGems(ZZTworld *world, int16_t gems)
{
	world->header->gems = gems;
}
void zztWorldSetKey(ZZTworld *world, int number, uint8_t state)
{
	if(number >= 0 && number < 7)
		world->header->keys[number] = state;
}
void zztWorldSetHealth(ZZTworld *world, int16_t health)
{
	world->header->health = health;
}
void zztWorldSetStartboard(ZZTworld *world, int16_t startboard)
{
	world->header->startboard = startboard;
}
void zztWorldSetTorches(ZZTworld *world, int16_t torches)
{
	world->header->torches = torches;
}
void zztWorldSetTorchcycles(ZZTworld *world, int16_t torchcycles)
{
	world->header->torchcycles = torchcycles;
}
void zztWorldSetEnergizercycles(ZZTworld *world, int16_t energizercycles)
{
	world->header->energizercycles = energizercycles;
}
void zztWorldSetScore(ZZTworld *world, int16_t score)
{
	world->header->score = score;
}
void zztWorldSetTitle(ZZTworld *world, char *title)
{
	strncpy((char *)world->header->title, title, ZZT_WORLD_TITLE_SIZE);
	world->header->title[ZZT_WORLD_TITLE_SIZE] = '\0';
}
void zztWorldSetFlag(ZZTworld *world, int number, char *word)
{
	if(number >= 0 && number < ZZT_MAX_FLAGS) {
		strncpy((char *)world->header->flags[number], word, ZZT_FLAG_SIZE);
		world->header->flags[number][ZZT_FLAG_SIZE] = '\0';
	}
}
void zztWorldSetTimepassed(ZZTworld *world, int16_t time)
{
	world->header->timepassed = time;
}
void zztWorldSetSavegame(ZZTworld *world, uint8_t flag)
{
	world->header->savegame = flag;
}
void zztWorldSetFilename(ZZTworld *world, char *name)
{
	free(world->filename);
	world->filename = malloc(strlen(name)+1);
	strcpy(world->filename, name);
}

uint16_t zztWorldGetBoardcount(ZZTworld *world)
{
	return world->header->boardcount;
}
int16_t zztWorldGetAmmo(ZZTworld *world)
{
	return world->header->ammo;
}
int16_t zztWorldGetGems(ZZTworld *world)
{
	return world->header->gems;
}
uint8_t zztWorldGetKey(ZZTworld *world, int number)
{
	if(number >= 0 && number < 7)
		return world->header->keys[number];
	return -1;
}
int16_t zztWorldGetHealth(ZZTworld *world)
{
	return world->header->health;
}
int16_t zztWorldGetStartboard(ZZTworld *world)
{
	return world->header->startboard;
}
int16_t zztWorldGetTorches(ZZTworld *world)
{
	return world->header->torches;
}
int16_t zztWorldGetTorchcycles(ZZTworld *world)
{
	return world->header->torchcycles;
}
int16_t zztWorldGetEnergizercycles(ZZTworld *world)
{
	return world->header->energizercycles;
}
int16_t zztWorldGetScore(ZZTworld *world)
{
	return world->header->score;
}
uint8_t *zztWorldGetTitle(ZZTworld *world)
{
	return world->header->title;
}
uint8_t *zztWorldGetFlag(ZZTworld *world, int number)
{
	if(number >= 0 && number < ZZT_MAX_FLAGS)
		return world->header->flags[number];
	return NULL;
}
int16_t zztWorldGetTimepassed(ZZTworld *world)
{
	return world->header->timepassed;
}
uint8_t zztWorldGetSavegame(ZZTworld *world)
{
	return world->header->savegame;
}
char *zztWorldGetFilename(ZZTworld *world)
{
	return world->filename;
}

