/* dosemu.c		-- Routines for calling dosemu to run ZZT
 * $Id: dosemu.c,v 1.2 2002/09/24 01:05:37 kvance Exp $
 * Copyright (C) 2002 Kev Vance <kev@kvance.com>
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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "dosemu.h"

#ifdef DOSEMU

int dosemu_launch(char *datapath, char *worldpath, char *world)
{
	char zztpath[DEXE_PATH_LENGTH+1];
	char chopworld[DEXE_WORLD_LENGTH+1];
	char zztworld[DEXE_WORLD_LENGTH+1];
	char olddexe[PATH_MAX];
	char newdexe[PATH_MAX];
	char cmdline[PATH_MAX];
	FILE *fp;
	int i, j;

	struct stat statbuf;

	/* Make path padded with spaces */
	if(strlen(worldpath) > DEXE_PATH_LENGTH) {
		fprintf(stderr, "ERROR: Path is too long!\n");
		return 0;
	}
	for(i = 0, j = strlen(worldpath); j < DEXE_PATH_LENGTH; i++, j++)
		zztpath[i] = ' ';
	for(j = 0; i < DEXE_PATH_LENGTH+1; i++, j++)
		zztpath[i] = worldpath[j];
	zztpath[i] = '\0';

	/* Strip ZZT from world name */
	for(i = 0; i < 8; i++) {
		if(world[i] == '.')
			break;
		chopworld[i] = world[i];
	}
	chopworld[i] = '\0';
	/* Insert whitespace before world name */
	for(i = 0, j = strlen(chopworld); j < DEXE_WORLD_LENGTH; i++, j++)
		zztworld[i] = ' ';
	for(j = 0; i < DEXE_WORLD_LENGTH+1; i++, j++)
		zztworld[i] = chopworld[j];
	zztworld[i] = '\0';
	
	/* Make a temporary DEXE */
	sprintf(olddexe, "%s/dexe/zzt.dexe", datapath);
	sprintf(newdexe, "/tmp/zzt%i.dexe", getpid());
	if(stat(olddexe, &statbuf) != 0) {
		fprintf(stderr, "ERROR: %s not found!\n", olddexe);
		return 0;
	}
	sprintf(cmdline, "cp %s %s", olddexe, newdexe);
	system(cmdline);

	/* Apply changes to DEXE */
	fp = fopen(newdexe, "r+b");
	fseek(fp, DEXE_PATH_LOCATION, SEEK_SET);
	fputs(zztpath, fp);
	fseek(fp, DEXE_WORLD_LOCATION, SEEK_SET);
	fputs(zztworld, fp);
	fclose(fp);

	/* Run DEXE */
	sprintf(cmdline, "dosexec %s", newdexe);
	system(cmdline);

	/* Remove temporary DEXE */
	unlink(newdexe);

	/* Done */
	return 1;
}

#endif /* DOSEMU */
