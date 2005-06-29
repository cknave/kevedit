/* dosbox.c		-- Routines for calling DOSBOX to run ZZT
 * $Id: dosbox.c,v 1.2 2005/06/29 04:00:35 kvance Exp $
 * Copyright (C) 2005 Kev Vance <kvance@kvance.com>
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

/* Only do this if we want DOSBox support */
#ifdef DOSBOX

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>

#define DOSBOX_EXE_NAME		"dosbox"
#define DOSBOX_CONF_NAME	"kevedos.cfg"
#define DOSBOX_ISO_NAME		"kevedos.iso"
#define DOSBOX_TEMPLATE		"kevedos-XXXXXX"

#define IMGMOUNT_C		"imgmount c "
#define IMGMOUNT_ISO		" -t iso\n"
#define MOUNT_D			"mount d \""

#define RUNSKIP_CMD		"COPY C:\\*.* D:\nD:\nSKIP\n"
#define RUNZZT_CMD		"ZZT "
#define EXIT_CMD		"EXIT\n"

/* Copy from fd_in to fd_out */
int _dosbox_copy_file(int fd_in, int fd_out)
{
	char buffer[1024];
	ssize_t size;

	while((size = read(fd_in, buffer, sizeof(buffer))) > 0) {
		write(fd_out, buffer, size);
	}
	
	if(size < 0)
		return -1;	/* An error occured :( */
	return 0;
}

/* Prepare the temporary directory with a custom configfile and the test world
 * file */
int _dosbox_prep_tempdir(char *tmpdir, char *datapath, char *worldpath,
		char *world)
{
	/* Generate the filename of the default KevEdit DOSBOX config */
	char *config_in = malloc(
			strlen(datapath)+
			strlen("/")+
			strlen(DOSBOX_CONF_NAME)+
			1);
	char *config_out = malloc(
			strlen(tmpdir)+
			strlen("/")+
			strlen(DOSBOX_CONF_NAME)+
			1);
	sprintf(config_in,  "%s%s%s", datapath, "/", DOSBOX_CONF_NAME);
	sprintf(config_out, "%s%s%s", tmpdir, "/", DOSBOX_CONF_NAME);

	/* Make a copy of the default KevEdit DOSBOX config */
	int fd_in, fd_out;
	fd_in = open(config_in, O_RDONLY);
	if(fd_in < 0) {
		free(config_in);
		free(config_out);
		return fd_in;
	}
	fd_out = creat(config_out, 0600);
	if(fd_out < 0) {
		close(fd_in);
		free(config_in);
		free(config_out);
		return fd_out;
	}
	if(_dosbox_copy_file(fd_in, fd_out) < 0) {
		close(fd_in);
		close(fd_out);
		free(config_in);
		free(config_out);
		return -1;
	}
	close(fd_in);

	/* Append our lines */
	char *imgmount_line = malloc(
			strlen(IMGMOUNT_C)+
			strlen(datapath)+
			strlen("/")+
			strlen(DOSBOX_ISO_NAME)+
			strlen(IMGMOUNT_ISO)+
			1);
	sprintf(imgmount_line, "%s%s%s%s%s", IMGMOUNT_C, datapath, "/",
			DOSBOX_ISO_NAME, IMGMOUNT_ISO);

	char *mount_line = malloc(
			strlen(MOUNT_D)+
			strlen(tmpdir)+
			strlen("\"\n")+
			1);
	sprintf(mount_line, "%s%s%s", MOUNT_D, tmpdir, "\"\n");

	char *runzzt_line = malloc(
			strlen(RUNZZT_CMD)+
			strlen(world)+
			strlen("\n")+
			1);
	sprintf(runzzt_line, "%s%s%s", RUNZZT_CMD, world, "\n");

	write(fd_out, imgmount_line, strlen(imgmount_line));
	write(fd_out, mount_line, strlen(mount_line));
	write(fd_out, RUNSKIP_CMD, strlen(RUNSKIP_CMD));
	write(fd_out, runzzt_line, strlen(runzzt_line));
	write(fd_out, EXIT_CMD, strlen(EXIT_CMD));
	close(fd_out);

	/* Clean up */
	free(config_in);
	free(config_out);
	free(imgmount_line);
	free(mount_line);
	free(runzzt_line);

	/* Create a copy of the current world */
	char *world_in = malloc(
			strlen(worldpath)+
			strlen("/")+
			strlen(world)+
			1);
	char *world_out = malloc(
			strlen(tmpdir)+
			strlen("/")+
			strlen(world)+
			1);
	sprintf(world_in, "%s%s%s", worldpath, "/", world);
	sprintf(world_out, "%s%s%s", tmpdir, "/", world);

	fd_in = open(world_in, O_RDONLY);
	if(fd_in < 0) {
		free(world_in);
		free(world_out);
		return fd_in;
	}
	fd_out = creat(world_out, 0600);
	if(fd_out < 0) {
		close(fd_in);
		free(world_in);
		free(world_out);
		return fd_out;
	}
	if(_dosbox_copy_file(fd_in, fd_out) < 0) {
		close(fd_in);
		close(fd_out);
		free(world_in);
		free(world_out);
		return -1;
	}
	close(fd_in);
	close(fd_out);

	/* Clean up */
	free(world_in);
	free(world_out);
	return 0;
}

/* Remove all files in a directory.  Note: this does not erase directories! */
void _dosbox_rm_star(char *path)
{
	DIR *dir = opendir(path);
	struct dirent *ent;
	char filename[PATH_MAX];
	
	if(dir == NULL)
		return;
	
	while((ent = readdir(dir)) != NULL) {
		sprintf(filename, "%s/%s", path, ent->d_name);
		unlink(filename);
	}

	closedir(dir);
}

int dosbox_launch(char *datapath, char *worldpath, char *world)
{
	/* Create a temporary directory for all this stuff */
	char *tmpdir = malloc(
			strlen(P_tmpdir)+
			strlen("/")+
			strlen(DOSBOX_TEMPLATE)+
			1);
	sprintf(tmpdir, "%s%s%s", P_tmpdir, "/", DOSBOX_TEMPLATE);
	if(mkdtemp(tmpdir) == NULL) {
		free(tmpdir);
		return -1;
	}

	/* Fill the temp dir with the necessary files */
	if(_dosbox_prep_tempdir(tmpdir, datapath, worldpath, world) < 0) {
		free(tmpdir);
		return -1;
	}

	/* Generate the DOSBOX commandline */
	char *commandline = malloc(
			strlen(DOSBOX_EXE_NAME)+
			strlen(" -conf ")+
			strlen(tmpdir)+
			strlen("/")+
			strlen(DOSBOX_CONF_NAME)+
			1);
	sprintf(commandline, "%s%s%s%s%s", DOSBOX_EXE_NAME, " -conf ", tmpdir,
			"/", DOSBOX_CONF_NAME);

	/* EXECUTE! */
	system(commandline);

	/* Clean up */
	_dosbox_rm_star(tmpdir);
	rmdir(tmpdir);
	free(tmpdir);
	free(commandline);
	return 0;
}

#endif /* DOSBOX */
