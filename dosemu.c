/* dosemu.c		-- Routines for calling dosemu to run ZZT
 * $Id: dosemu.c,v 1.5 2002/12/04 23:53:06 kvance Exp $
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
 * Foundation, Inc., 59 Temple Place Suite 330; Boston, MA 02111-1307, USA.
 */

#include "dosemu.h"

#ifdef DOSEMU

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

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

	/* Make path padded with newlines */
	if(strlen(worldpath) > DEXE_PATH_LENGTH) {
		fprintf(stderr, "ERROR: Path is too long!\n");
		return 0;
	}
	strcpy(zztpath, worldpath);
	for(i = strlen(worldpath); i < DEXE_PATH_LENGTH+1; i++) {
		zztpath[i] = '\r';
		i++;
		if(i >= DEXE_PATH_LENGTH+1)
			break;
		zztpath[i] = '\n';
	}

	zztpath[i] = '\0';

	/* Strip ZZT from world name */
	for(i = 0; i < 8; i++) {
		if(world[i] == '.')
			break;
		chopworld[i] = world[i];
	}
	chopworld[i] = '\0';
	/* Insert newlines after world name */
	strcpy(zztworld, chopworld);
	for(i = strlen(chopworld); i < DEXE_WORLD_LENGTH+1; i++) {
		zztworld[i] = '\r';
		i++;
		if(i >= DEXE_WORLD_LENGTH+1)
			break;
		zztworld[i] = '\n';
	}
	zztworld[i] = '\0';
	
	/* Make a temporary DEXE */
	sprintf(olddexe, "%s/zzt.dexe", datapath);
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
