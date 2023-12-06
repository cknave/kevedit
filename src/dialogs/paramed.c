/* paramed.c  -- Parameter editor
 * $Id: paramed.c,v 1.3 2005/05/28 03:17:45 bitman Exp $
 * Copyright (C) 2000 Ryan Phillips <bitman@scn.org>
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

#include "paramed.h"

#include "kevedit/screen.h"
#include "texteditor/editbox.h"
#include "texteditor/texteditor.h"

#include "libzzt2/zzt.h"
#include "help/help.h"
#include "dialog.h"

#include "display/display.h"
#include "display/colours.h"

#include "themes/theme.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Label colorings */
#define LABEL_COLOR  0x0A
#define OPTION_COLOR 0x0B

/* Option ID's used in the dialog */
#define ID_NONE         0x0100
#define ID_PROGRAM      0x0200
#define ID_DIRECTION    0x0300
#define ID_FIRERATE     0x0400
#define ID_PROJECTILE   0x0500
#define ID_CYCLE        0x0600
#define ID_XSTEP        0x0700
#define ID_YSTEP        0x0800
#define ID_DATA0        0x0900
#define ID_DATA1        0x0A00
#define ID_DATA2        0x0B00
#define ID_BIND         0x0C00
#define ID_INSTRUCTION  0x0D00
#define ID_LEADER       0x0E00
#define ID_FOLLOWER     0x0F00
/* The other IDs come from the ZZT_DATAUSE_* set */

/* Option IDs used in tile info */
#define ID_KIND         1
#define ID_TILEID       2
#define ID_COLOR        3
#define ID_ADDPARAM     4
#define ID_RMPARAM      5
#define ID_EDITPARAM    6
#define ID_UTYPE        7
#define ID_UCOLOR       8

/* Table of direction names based on direction flags */
const char * direction_table[] = {
	"Idle",
	"North",
	"South",
	"", /* North-South */
	"East",
	"North-East",
	"South-East",
	"", /* North-South-East */
	"West",
	"North-West",
	"South-West",
};


int modifyparam(displaymethod * d, ZZTworld * w, int x, int y)
{
	dialog dia;
	int key;

	if (zztTileGet(w, x, y).param == NULL)
		return 0;

	/* Build the dialog */
	dia = buildparamdialog(w, x, y);

	drawsidepanel(d, PANEL_STATS_DIALOG);

	do {
		int rebuild = 0;
		/* Draw the dialog each time around */
		dialogDraw(d, dia);

		key = d->getch();

		switch (key) {
			case DKEY_DOWN: dialogNextOption(&dia); break;
			case DKEY_UP:   dialogPrevOption(&dia); break;
			case DKEY_ENTER:
				rebuild = parameditoption(d, w, x, y, dialogGetCurOption(dia));
				break;
			case DKEY_LEFT:
			case '-':
				rebuild = paramdeltaoption(d, w, x, y, dialogGetCurOption(dia), -1);
				break;
			case DKEY_RIGHT:
			case '+':
				rebuild = paramdeltaoption(d, w, x, y, dialogGetCurOption(dia), 1);
				break;
			case DKEY_F1:
				helpsectiontopic("kstats", "", d);
				break;
		}

		if (rebuild == DKEY_QUIT) {
			key = DKEY_QUIT;
		}
		if (rebuild == 2) {
			/* Redraw the side panel */
			drawsidepanel(d, PANEL_STATS_DIALOG);
		}

		if (rebuild) {
			/* Rebuild param dialog */
			int curoption = dia.curoption;
			dialogFree(&dia);
			dia = buildparamdialog(w, x, y);
			dia.curoption = curoption;
		}
	} while (key != DKEY_ESC && key != DKEY_QUIT);

	dialogFree(&dia);
	return key;
}

stringvector programtosvector(ZZTparam * p, int editwidth)
{
	stringvector sv;    /* list of strings */
	char *str = NULL;   /* temporary string */
	int strpos = 0;     /* position in str */
	int i;

	initstringvector(&sv);

	/* load the vector */
	if ((p->program == NULL) | (p->length <= 0)) {
		/* No data! We need to create an empty node */
		pushstring(&sv, str_dupmin("", editwidth + 1));
		return sv;
	}

	/* Let's fill the node from program! */
	strpos = 0;
	str = (char *) malloc(sizeof(char) * (editwidth + 1));

	for (i = 0; i < p->length; i++) {
		if (p->program[i] == 0x0d) {
			/* end of the line (heh); push the string and start over */
			str[strpos] = 0;
			pushstring(&sv, str);
			strpos = 0;
			str = (char *) malloc(sizeof(char) * (editwidth + 1));
		} else if (strpos > editwidth) {
			/* hmmm... really long line; must not have been made in ZZT... */
			/* let's truncate! */
			str[strpos] = 0;
			pushstring(&sv, str);
			strpos = 0;
			str = (char *) malloc(sizeof(char) * (editwidth + 1));
			/* move to next 0x0d */
			do i++; while (i < p->length && p->program[i] != 0x0d);
		} else {
			/* just your everyday copying... */
			str[strpos++] = p->program[i];
		}
	}

	if (strpos > 0) {
		/* strange... we seem to have an extra line with no CR at the end... */
		str[strpos] = 0;
		pushstring(&sv, str);
	} else {
		/* we grabbed all that RAM for nothing. Darn! */
		free(str);
	}

	return sv;
}

ZZTparam svectortoprogram(stringvector sv)
{
	ZZTparam p;
	int pos;

	/* find out how much space we need */
	p.length = 0;
	/* and now for a wierdo for loop... */
	for (sv.cur = sv.first; sv.cur != NULL; sv.cur = sv.cur->next)
		p.length += strlen(sv.cur->s) + 1;		/* + 1 for CR */

	if (p.length <= 1) {
		/* sv holds one empty string (it can happen) */
		p.program = NULL;
		p.length = 0;
		return p;
	}

	/* lets make room for all that program */
	pos = 0;
	p.program = (uint8_t *) malloc(p.length);

	for (sv.cur = sv.first; sv.cur != NULL; sv.cur = sv.cur->next) {
		int i;
		int linelen = strlen(sv.cur->s);	/* I feel efficient today */
		for (i = 0; i < linelen; i++) {
			p.program[pos++] = sv.cur->s[i];
		}
		p.program[pos++] = 0x0d;
	}

	zztParamRehash(&p);

	return p;
}

int editprogram(displaymethod * d, ZZTparam * p)
{
	texteditor * editor;
	stringvector * sv;
	ZZTparam newparam;

	sv = (stringvector *) malloc(sizeof(stringvector));
	*sv = programtosvector(p, EDITBOX_ZZTWIDTH);
	editor = createtexteditor("Program Editor", sv, d);

	/* Now that the node is full, we can edit it. */
	int result = textedit(editor);

	/* Okay, let's put the vector back in program */
	newparam = svectortoprogram(*editor->text);

	/* Free the text editor and text. */
	deletetexteditortext(editor);
	deletetexteditor(editor);

	/* Free the old program. */
	if (p->program != NULL)
		free(p->program);

	p->length = newparam.length;
	p->program = newparam.program;
	p->program_hash = newparam.program_hash;

	return result;
}


int getdirection(short xstep, short ystep)
{
	int dir = 0;

	if (xstep > 0)      dir |= DIR_EAST;
	else if (xstep < 0) dir |= DIR_WEST;
	if (ystep > 0)      dir |= DIR_SOUTH;
	else if (ystep < 0) dir |= DIR_NORTH;

	return dir;
}

void getxystep(short * xstep, short * ystep, int dir)
{
	/* Start with a clear slate */
	*xstep = 0; *ystep = 0;

	if (dir & DIR_EAST) (*xstep)++;
	if (dir & DIR_WEST) (*xstep)--;
	if (dir & DIR_SOUTH) (*ystep)++;
	if (dir & DIR_NORTH) (*ystep)--;
}

int nextdirection(int dir)
{
	/* TODO: less sporatic for non-cardinal directions, please */

	if (dir == 0)
		return 1;
	if (dir & 0x80)
		return 0;
	return dir << 1;
}

char * scalestring(char * buffer, int data)
{
	/* Turn data into a scale string in the form: ..*.|.... */
	int i;
	const char DOTCH = 0xFA, STARCH = '*', CLINE = '|', CSTAR = '+';

	/* Create first four dots */
	for (i = 0; i < 4; i++)
		if (i == data)
			buffer[i] = STARCH;
		else
			buffer[i] = DOTCH;

	/* Create center line */
	if (i == data)
		buffer[i] = CSTAR;
	else
		buffer[i] = CLINE;

	/* Create last four dots */
	for (i++; i < 9; i++)
		if (i == data)
			buffer[i] = STARCH;
		else
			buffer[i] = DOTCH;

	buffer[i] = '\x0';

	return buffer;
}

char * paramdatavaluestring(char * buffer, ZZTtile tile, int which, ZZTworld * w)
{
	uint8_t data = tile.param->data[which];

	switch (zztParamDatauseGet(tile, which)) {
		case ZZT_DATAUSE_FIRERATEMODE:
			if (data > 128) data -= 128;
			/* Continue through... */
		case ZZT_DATAUSE_DUPRATE:
			if (data > 8)
				sprintf(buffer, "%d", data);
			else {
				strcpy(buffer, "Slow ");
				scalestring(buffer + strlen(buffer), data);
				strcat(buffer, " Fast");
			}
			break;
		case ZZT_DATAUSE_SENSITIVITY:
		case ZZT_DATAUSE_INTELLIGENCE:
		case ZZT_DATAUSE_RESTTIME:
		case ZZT_DATAUSE_DEVIANCE:
		case ZZT_DATAUSE_STARTTIME:
		case ZZT_DATAUSE_PERIOD:
			if (data > 8)
				sprintf(buffer, "%d", data);
			else {
				strcpy(buffer, "1 ");
				scalestring(buffer + strlen(buffer), data);
				strcat(buffer, " 9");
			}
			break;
		case ZZT_DATAUSE_SPEED:
			if (data > 8)
				sprintf(buffer, "%d", data);
			else {
				strcpy(buffer, "Fast ");
				scalestring(buffer + strlen(buffer), data);
				strcat(buffer, " Slow");
			}
			break;
		case ZZT_DATAUSE_TIMELEFT:
			if (data == 0) {
				strcpy(buffer, "Inactive");
			} else if (data > 9)
				sprintf(buffer, "%d", data);
			else {
				strcpy(buffer, "1 ");
				scalestring(buffer + strlen(buffer), data - 1);
				strcat(buffer, " 9");
			}
			break;
		case ZZT_DATAUSE_PASSAGEDEST:
			if (data < zztWorldGetBoardcount(w))
				strcpy(buffer, (char *)w->boards[data].title);
			break;
		case ZZT_DATAUSE_CHAR:
			sprintf(buffer, "%c - #char %d - %02Xh", (data != 0 ? data : ' '), data, data);
			break;
		case ZZT_DATAUSE_LOCKED:
			sprintf(buffer, (data == 0 ? "#unlock-ed": "#lock-ed"));
			break;
		case ZZT_DATAUSE_OWNER:
			strcpy(buffer, data == 0 ? "Player" : "Creature");
			break;
		default:
			strcpy(buffer, "");
	}

	return buffer;
}

dialog buildparamdialog(ZZTworld * w, int x, int y)
{
	ZZTtile tile = zztTileGet(w, x, y);
	uint8_t properties = zztParamGetProperties(tile);
	dialog dia;

	int i;
	char buffer[100];

	dialogComponent label  = dialogComponentMake(DIALOG_COMP_LABEL,   0, 1, LABEL_COLOR,  NULL, ID_NONE);
	dialogComponent option = dialogComponentMake(DIALOG_COMP_OPTION, 20, 1, OPTION_COLOR, NULL, ID_NONE);

	/* Handy macros for using template label & option */
#define _addlabel(TEXT)      { label.text  = (TEXT); dialogAddComponent(&dia, label); label.y++; }
#define _addoption(TEXT, ID) { option.text = (TEXT); option.id = (ID); dialogAddComponent(&dia, option); option.y++; }

	/* Initialize the dialog */
	dialogInit(&dia);

	/* Generate the title */
	dialogAddComponent(&dia, dialogComponentMake(DIALOG_COMP_TITLE, 0, 0, 0x0F, (char *) zztTileGetName(tile), ID_NONE));


	/* Add "edit program" option */
	if (properties & ZZT_PROPERTY_PROGRAM) {
		_addlabel(tile.type == ZZT_OBJECT ? "Program" : "Text");
		_addoption("Edit", ID_PROGRAM);
	}

	/* Direction option */
	if (properties & ZZT_PROPERTY_STEP) {
		_addlabel("Direction");
		_addoption((char *) direction_table[getdirection(tile.param->xstep, tile.param->ystep)], ID_DIRECTION);
	}

	for (i = 0; i < 3; i++) {
		int datause = zztParamDatauseGet(tile, i);
		if (datause == ZZT_DATAUSE_FIRERATEMODE) {
			/* Confounded special case */

			/* Remove the projectile-type component for printing */
			int rate = tile.param->data[i] & 0x7F;
			_addlabel("Firing Rate");
			if (rate > 8)
				sprintf(buffer, "%d", rate);
			else {
				strcpy(buffer, "1 ");
				scalestring(buffer + strlen(buffer), rate);
				strcat(buffer, " 9");
			}
			_addoption(buffer, ID_FIRERATE);

			_addlabel("Projectile");
			_addoption((tile.param->data[i] & 0x80) ? "Throwstars" : "Bullets", ID_PROJECTILE);
		} else if (datause != ZZT_DATAUSE_NONE) {
			char * usename = (char *) zztParamDatauseGetName(tile, i);
			_addlabel(usename);
			paramdatavaluestring(buffer, tile, i, w);
			_addoption(buffer, zztParamDatauseGet(tile, i));
		}
	}

	if (properties & ZZT_PROPERTY_CYCLE) {
		/* Add a blank line before cycle if not the first item */
		if (label.y > 1) { label.y++; option.y++; }

		_addlabel("Cycle");
		sprintf(buffer, "%d", tile.param->cycle);
		_addoption(buffer, ID_CYCLE);
	}

	/* Advanced configuration */
	label.y = option.y += 2;
	label.x = 0; option.x = 8;

	dialogAddComponent(&dia, dialogComponentMake(DIALOG_COMP_HEADING, 0, label.y - 1, 0x0F, "Advanced Tweaking", ID_NONE));

	/* If cycle is not a normal property, add it as an advanced tweak */
	if (!(properties & ZZT_PROPERTY_CYCLE)) {
		_addlabel("Cycle");
		sprintf(buffer, "%d", tile.param->cycle);
		_addoption(buffer, ID_CYCLE);
		label.y++; option.y++;
	}

	_addlabel("X Step");
	_addlabel("Y Step");
	_addlabel("Data 1");
	_addlabel("Data 2");
	_addlabel("Data 3");

	sprintf(buffer, "%d", (short) tile.param->xstep); _addoption(buffer, ID_XSTEP);
	sprintf(buffer, "%d", (short) tile.param->ystep); _addoption(buffer, ID_YSTEP);
	sprintf(buffer, "%d", tile.param->data[0]); _addoption(buffer, ID_DATA0);
	sprintf(buffer, "%d", tile.param->data[1]); _addoption(buffer, ID_DATA1);
	sprintf(buffer, "%d", tile.param->data[2]); _addoption(buffer, ID_DATA2);

	if (properties & ZZT_PROPERTY_PROGRAM || tile.param->length != 0 || tile.param->bindindex != 0) {
		char * bindname;
		if (tile.param->bindindex != 0)
			bindname = buildparamdescription(zztBoardGetBlock(w), tile.param->bindindex);
		else
			bindname = str_dup("(none)");

		_addlabel("Bind to");
		_addoption(bindname, ID_BIND);
		free(bindname);

		option.x = 20;
		_addlabel("Program Instruction");
		sprintf(buffer, "%d", (int16_t) tile.param->instruction); _addoption(buffer, ID_INSTRUCTION);
	}

	if (properties & ZZT_PROPERTY_LEADER) {
		/* Edit follower/leader values */
		char * name;

		label.y++; option.y++;
		option.x++;

		/* Add the leader */
		_addlabel("Leader");
		if (tile.param->leaderindex < -1) {
			sprintf(buffer, "%d", tile.param->leaderindex);
			_addoption(buffer, ID_LEADER);
		} else {
			if (tile.param->leaderindex != -1)
				name = buildparamdescription(zztBoardGetBlock(w), tile.param->leaderindex);
			else
				name = str_dup("(none)");
			_addoption(name, ID_LEADER);
			free(name);
		}

		/* Add the follower */
		_addlabel("Follower");
		if (tile.param->followerindex < -1) {
			sprintf(buffer, "%d", tile.param->followerindex);
			_addoption(buffer, ID_FOLLOWER);
		} else {
			if (tile.param->followerindex != -1)
				name = buildparamdescription(zztBoardGetBlock(w), tile.param->followerindex);
			else
				name = str_dup("(none)");
			_addoption(name, ID_FOLLOWER);
			free(name);
		}
	}

	return dia;
}

int parameditoption(displaymethod * d, ZZTworld * w, int x, int y, dialogComponent * opt)
{
	ZZTtile tile = zztTileGet(w, x, y);
	int num;   /* General use number */

	switch (opt->id) {
		case ID_PROGRAM:
			if (editprogram(d, tile.param) == DKEY_QUIT)
				return DKEY_QUIT;
			return 2;
		case ZZT_DATAUSE_PASSAGEDEST: {
			int result = boarddialog(w, tile.param->data[2], "Passage Destination", 0, d);
			if (result == DKEY_QUIT)
				return DKEY_QUIT;
			tile.param->data[2] = result;
			return 2;
		}
		case ZZT_DATAUSE_CHAR:
			num = charselect(d, tile.param->data[0]);
			if (num == DKEY_QUIT)
				return DKEY_QUIT;
			if (num != -1)
				tile.param->data[0] = num;
			return 1;
		case ZZT_DATAUSE_LOCKED:
			tile.param->data[1] = !tile.param->data[1];
			return 1;
		/* 8-bit numbers */
		case ZZT_DATAUSE_TIMELEFT:
			/* For values under ten, toggle between 0 and 9 */
			if (tile.param->data[zztParamDatauseLocate(opt->id)] < 10) {
				if (tile.param->data[zztParamDatauseLocate(opt->id)] == 0)
					tile.param->data[zztParamDatauseLocate(opt->id)] = 9;
				else
					tile.param->data[zztParamDatauseLocate(opt->id)] = 0;
				return 1;
			}
		case ZZT_DATAUSE_DUPRATE:
		case ZZT_DATAUSE_SENSITIVITY:
		case ZZT_DATAUSE_INTELLIGENCE:
		case ZZT_DATAUSE_RESTTIME:
		case ZZT_DATAUSE_SPEED:
		case ZZT_DATAUSE_DEVIANCE:
		case ZZT_DATAUSE_STARTTIME:
		case ZZT_DATAUSE_PERIOD:
			/* Require user to inc/dec for values under 9 */
			if (tile.param->data[zztParamDatauseLocate(opt->id)] < 9)
				return 0;
		case ID_CYCLE: {
			/* zero's are special */
			if (str_equ(opt->text, "0", 0)) opt->text[0] = '\x0';
			int result = dialogComponentEdit(d, opt, 6, LINED_SNUMBER);
			if (result == LINED_OK) {
				sscanf(opt->text, "%d", &num);
				/* No exceeding the bounds of a signed 16-bit number */
				if (num > 32767) num = 32767;
				else if (num < -32768) num = -32768;
				/* zero's are special */
				if (opt->text[0] == '\x0') num = 0;
				tile.param->cycle = num;
			} else if (result == LINED_QUIT) {
				return DKEY_QUIT;
			}
			return 1;
		}
		case ID_DATA0:
		case ID_DATA1:
		case ID_DATA2: {
			/* zero's are special */
			if (str_equ(opt->text, "0", 0)) opt->text[0] = '\x0';
			int result = dialogComponentEdit(d, opt, 3, LINED_NUMBER);
			if (result == LINED_OK) {
				sscanf(opt->text, "%d", &num);
				/* No exceeding the bounds of an 8-bit number */
				if (num > 255) num = 255;
				/* zero's are special */
				if (opt->text[0] == '\x0') num = 0;

				/* Is this lame, or what? */
				/* We could put the above in a function and use
				 * the top level switch only, but why bother? */
				switch (opt->id) {
					case ID_CYCLE:
						tile.param->cycle = num; break;
					case ID_DATA0:
						tile.param->data[0] = num; break;
					case ID_DATA1:
						tile.param->data[1] = num; break;
					case ID_DATA2:
						tile.param->data[2] = num; break;
					default:
						tile.param->data[zztParamDatauseLocate(opt->id)] = num;
						break;
				}
			} else if (result == LINED_QUIT) {
				return DKEY_QUIT;
			}
			return 1;
		}
		/* signed 8-bit values -- ack! */
		case ID_XSTEP:
		case ID_YSTEP: {
			/* almost like regular 8-bits... */
			/* zero's are special */
			if (str_equ(opt->text, "0", 0)) opt->text[0] = '\x0';
			int result = dialogComponentEdit(d, opt, 4, LINED_SNUMBER);
			if (result == LINED_OK) {
				sscanf(opt->text, "%d", &num);
				/* No exceeding the bounds of a signed 16-bit number */
				if (num > 32767) num = 32767;
				else if (num < -32768) num = -32768;
				/* zero's are special */
				if (opt->text[0] == '\x0') num = 0;
				if (opt->id == ID_XSTEP) tile.param->xstep = num;
				else tile.param->ystep = num;
			} else if (result == LINED_QUIT) {
				return DKEY_QUIT;
			}
			return 1;
		}
		case ID_FIRERATE: {
			if (str_equ(opt->text, "0", 0)) opt->text[0] = '\x0';
			int result = dialogComponentEdit(d, opt, 3, LINED_NUMBER);
			if (result == LINED_OK) {
				int firerateindex = zztParamDatauseLocate(ZZT_DATAUSE_FIRERATEMODE);
				sscanf(opt->text, "%d", &num);
				/* No exceeding the bounds of a 7-bit number */
				if (num > 127) num = 127;
				/* zero's are special */
				if (opt->text[0] == '\x0') num = 0;

				tile.param->data[firerateindex] &= 0x80;
				tile.param->data[firerateindex] |= num;
			} else if (result == LINED_QUIT) {
				return DKEY_QUIT;
			}
			return 1;
		}
		case ID_BIND: {
			num = tile.param->bindindex;
			if (num == 0)
				num = -1;  /* For #bind, 0 means none */

			int result = paramlistdialog(d, zztBoardGetBlock(w), num, "Select object to bind with");
			if (result == DKEY_QUIT)
				return DKEY_QUIT;

			num = result;
			if (num == -1)
				num = 0;
			tile.param->bindindex = num;
			return 1;
		}
		case ID_INSTRUCTION: {
			/* zero's are special */
			if (str_equ(opt->text, "0", 0)) opt->text[0] = '\x0';
			int result = dialogComponentEdit(d, opt, 6, LINED_SNUMBER);
			if (result == LINED_OK) {
				sscanf(opt->text, "%d", &num);
				/* zero's are special */
				if (opt->text[0] == '\x0') num = 0;
					tile.param->instruction = num;
			} else if (result == LINED_QUIT) {
				return DKEY_QUIT;
			}
			return 1;
		}
		case ID_LEADER:
		case ID_FOLLOWER: {
			if (opt->id == ID_LEADER)
				num = tile.param->leaderindex;
			else
				num = tile.param->followerindex;

			int result = paramlistdialog(d, zztBoardGetBlock(w), num, "Select object to bind with");
			if (result == DKEY_QUIT) {
				return DKEY_QUIT;
			}

			num = result;
			if (opt->id == ID_LEADER)
				tile.param->leaderindex = num;
			else
				tile.param->followerindex = num;
			return 1;
		}
		case ID_PROJECTILE:
			tile.param->data[zztParamDatauseLocate(ZZT_DATAUSE_FIRERATEMODE)] ^= 0x80;
			return 1;
		case ZZT_DATAUSE_OWNER:
			tile.param->data[zztParamDatauseLocate(opt->id)] = !tile.param->data[zztParamDatauseLocate(opt->id)];
			return 1;
		case ID_DIRECTION:
			{
				short xstep, ystep;
				num = getdirection(tile.param->xstep, tile.param->ystep);
				num = nextdirection(num);
				getxystep(&xstep, &ystep, num);
				tile.param->xstep = xstep;
				tile.param->ystep = ystep;
			}
			return 1;
	}

	/* No change occured if we reach this point */
	return 0;
}

int paramdeltaoption(displaymethod * d, ZZTworld * w, int x, int y, dialogComponent * opt, int delta)
{
	ZZTtile tile = zztTileGet(w, x, y);
	int dul = zztParamDatauseLocate(opt->id);

	switch (opt->id) {
		case ZZT_DATAUSE_DUPRATE:
		case ZZT_DATAUSE_SENSITIVITY:
		case ZZT_DATAUSE_INTELLIGENCE:
		case ZZT_DATAUSE_RESTTIME:
		case ZZT_DATAUSE_SPEED:
		case ZZT_DATAUSE_DEVIANCE:
		case ZZT_DATAUSE_STARTTIME:
		case ZZT_DATAUSE_PERIOD:
			/* These can all slide as high as 8 */
			if (tile.param->data[dul] < -delta)
				tile.param->data[dul] = 0;
			else if (tile.param->data[dul] < 9 && tile.param->data[dul] > 8 - delta)
				tile.param->data[dul] = 8;
			else
				tile.param->data[dul] += delta;
			return 1;
		case ZZT_DATAUSE_TIMELEFT:
			/* This can slide as high as 9 */
			if (tile.param->data[dul] < -delta)
				tile.param->data[dul] = 0;
			else if (tile.param->data[dul] < 10 && tile.param->data[dul] > 9 - delta)
				tile.param->data[dul] = 9;
			else
				tile.param->data[dul] += delta;
			return 1;
		case ZZT_DATAUSE_CHAR:
			tile.param->data[dul] += delta;
			return 1;
		case ZZT_DATAUSE_LOCKED:
			tile.param->data[1] = !tile.param->data[1];
			return 1;

		case ID_CYCLE: tile.param->cycle += delta; return 1;
		case ID_XSTEP: tile.param->xstep += delta; return 1;
		case ID_YSTEP: tile.param->ystep += delta; return 1;

		case ID_DATA0: tile.param->data[0] += delta; return 1;
		case ID_DATA1: tile.param->data[1] += delta; return 1;
		case ID_DATA2: tile.param->data[2] += delta; return 1;

		case ZZT_DATAUSE_PASSAGEDEST:
			if (tile.param->data[2] + delta >= zztWorldGetBoardcount(w) )
				tile.param->data[2] = zztWorldGetBoardcount(w) - 1;
			else if(tile.param->data[2] + delta < 0)
				tile.param->data[2] = 0;
			else
				tile.param->data[2] += delta;
			return 1;

		case ID_PROJECTILE:
			if (delta != 0)
				tile.param->data[1] ^= 0x80;
			return 1;

		case ID_FIRERATE: {
			int rate = tile.param->data[1] & 0x7F;
			if (rate < -delta)
				rate = 0;
			else if (rate < 9 && rate > 8 - delta)
				rate = 8;
			else
				rate += delta;
			tile.param->data[1] = (tile.param->data[1] & 0x80) | (rate & 0x7F);
		} return 1;

		case ID_INSTRUCTION:
		case ID_BIND:
			/* TODO: implement */
			return 0;
	}

	return 0;
}

/* Tile info dialog */

int tileinfo(displaymethod * d, ZZTworld * w, int x, int y)
{
	dialog dia;
	int key;

	dia = buildtileinfodialog(w, x, y);

	drawsidepanel(d, PANEL_TILE_INFO);

	do {
		int rebuild = 0;

		/* Draw the dialog */
		dialogDraw(d, dia);

		key = d->getch();

		switch (key) {
			case DKEY_DOWN: dialogNextOption(&dia); break;
			case DKEY_UP:   dialogPrevOption(&dia); break;
			case DKEY_ENTER:
				rebuild = tileinfoeditoption(d, w, x, y, dialogGetCurOption(dia));
				break;
			case DKEY_F1:
				helpsectiontopic("ktileinf", "", d);
				break;
		}

		if (rebuild == DKEY_QUIT) {
			key = DKEY_QUIT;
			break;
		}
		if (rebuild) {
			/* Rebuild dialog */
			int curoption = dia.curoption;
			dialogFree(&dia);
			dia = buildtileinfodialog(w, x, y);
			if(curoption < dia.optioncount) {
				dia.curoption = curoption;
			}
		}
	} while (key != DKEY_ESC && key != DKEY_QUIT);

	dialogFree(&dia);
	return key;
}

const char * _color_name_table[] = {
	"Black",
	"Blue",
	"Green",
	"Cyan",
	"Red",
	"Purple",
	"Brown",
	"Light Grey",
	"Dark Grey",
	"Light Blue",
	"Light Green",
	"Light Cyan",
	"Light Red",
	"Light Purple",
	"Yellow",
	"White"
};

char * colorname(char * buf, int color)
{
	buf[0] = '\x0';

	/* Number form */
	sprintf(buf, "%02X: ", color);

	/* Text form */
	if (color & 0x80) strcat(buf, "Blinking ");
	strcat(buf, _color_name_table[color & 0x0F]);
	strcat(buf, " on ");
	strcat(buf, _color_name_table[(color & 0x70) >> 4]);

	return buf;
}

int selectcolor(displaymethod * d, int color)
{
	textcolor tcolor;

	tcolor.fg    = colorfg(color);
	tcolor.bg    = colorbg(color);
	tcolor.blink = colorblink(color);

	int result = colorselector(d, &tcolor);
	if (result == DKEY_QUIT)
		return DKEY_QUIT;
	if (!result)
		return encodecolor(tcolor);
	return -1;
}

int selecttiletype(displaymethod * d, int type)
{
	stringvector list;
	int i;

	/* List all available types */
	initstringvector(&list);
	for (i = 0; i <= ZZT_MAX_TYPE; i++) {
		pushstring(&list, (char *) _zzt_type_name_table[i]);
	}

	/* Move to the type of the current tile */
	svmoveby(&list, type);

	/* Make user select a type */
	int result = scrolldialog("Change Tile Type", &list, d);
	if (result == EDITBOX_OK) {
		/* Retrieve the new type */
		type = svgetposition(&list);
	} else if (result == EDITBOX_QUIT) {
		type = DKEY_QUIT;
	} else {
		type = -1;
	}

	/* Remove (not delete!) the list */
	removestringvector(&list);

	return type;
}

dialog buildtileinfodialog(ZZTworld * w, int x, int y)
{
	dialog dia;
	ZZTtile tile = zztTileGet(w, x, y);
	char buf[256];

	dialogComponent label = dialogComponentMake(DIALOG_COMP_LABEL,  0, 1, LABEL_COLOR,  NULL, ID_NONE);
	dialogComponent option = dialogComponentMake(DIALOG_COMP_OPTION, 13, 1, OPTION_COLOR, NULL, ID_NONE);

	/* Also use _addlabel() and _addoption() from buildparamdialog() */
#define _addvalue(TEXT)      { option.text  = (TEXT); option.type = DIALOG_COMP_LABEL; dialogAddComponent(&dia, option); option.y++; option.type = DIALOG_COMP_OPTION; }

	/* Initialize the dialog */
	dialogInit(&dia);

	/* Generate the title */
	dialogAddComponent(&dia, dialogComponentMake(DIALOG_COMP_TITLE, 0, 0, 0x0F, (char *) zztTileGetName(tile), ID_NONE));

	/* Common information to all types */
	_addlabel("ZZT-OOP Kind");
	_addlabel("Id Number");
	_addlabel(zztTileIsText(tile) ? "Character" : "Color");
	_addlabel("Coordinate");

	_addoption((char *) zztTileGetKind(tile), ID_KIND);

	sprintf(buf, "%d, %02Xh", tile.type, tile.type);
	_addoption(buf, ID_TILEID);

	if(zztTileIsText(tile)) {
		sprintf(buf, "%02X: '%c'", tile.color, tile.color);
		_addoption(buf, ID_COLOR);
	} else {
		_addoption(colorname(buf, tile.color), ID_COLOR);
	}

	sprintf(buf, "(%d, %d)", x + 1, y + 1);
	_addvalue(buf);

	/* Advance option position */
	option.y += 2; option.x = 0;

	/* Stats vs. No Stats */
	if (tile.param == NULL) {
		dialogAddComponent(&dia, dialogComponentMake(DIALOG_COMP_HEADING, 0, option.y - 1, 0x0F, "No Stats", ID_NONE));
		_addoption("Add Stats", ID_ADDPARAM);
	} else {
		ZZTtile under = { 0, 0, NULL};

		dialogAddComponent(&dia, dialogComponentMake(DIALOG_COMP_HEADING, 0, option.y - 1, 0x0F, "Stats", ID_NONE));
		_addoption("Edit Stats", ID_EDITPARAM);
		_addoption("Remove Stats", ID_RMPARAM);

		/* Advance the label/value templates */
		option.y++; option.x = 13;
		label.y = option.y;

		/* Stat index */
		_addlabel("Stat Index");
		sprintf(buf, "%d", tile.param->index);
		_addoption(buf, ID_NONE);

		/* Tile under info */
		under.type = tile.param->utype;
		under.color = tile.param->ucolor;

		_addlabel("Under Type");
		_addlabel("Under Color");

		sprintf(buf, "%d, %02X: %s", under.type, under.type, zztTileGetName(under));
		_addoption(buf, ID_UTYPE);
		_addoption(colorname(buf, under.color), ID_UCOLOR);

		if (tile.param->program != NULL) {
			int i, instr = tile.param->instruction, plen = tile.param->length;

			_addlabel("Code Length");
			_addlabel("Current Code");

			sprintf(buf, "%d", plen);
			_addvalue(buf);

			for (i = 0; instr + i < plen && tile.param->program[instr + i] != '\r' && i < 255; i++)
				buf[i] = tile.param->program[instr + i];
			buf[i] = '\x0';

			if ((int16_t) instr == -1)
				strcpy(buf, "(Program Execution #end-ed)");

			_addvalue(buf);
		}
	}

	return dia;
}

int tileinfoeditoption(displaymethod * d, ZZTworld * w, int x, int y, dialogComponent * opt)
{
	int i;
	ZZTtile t = zztTileGet(w, x, y);

	switch (opt->id) {
		case ID_KIND:
			i = selecttiletype(d, t.type);

			if (i == -1) return 0;
			if (i == DKEY_QUIT) return DKEY_QUIT;

			t.type = i;
			/* Update changes if the new type is valid */
			if (t.type <= ZZT_MAX_TYPE) {
				/* zztPlot will free the current param, so duplicate it first.
				 * This is slightly inefficient, but we don't have to
				 * manipulate the zzt world directly this way. */
				t.param = zztParamDuplicate(t.param);
				zztPlot(w, x, y, t);

				/* TODO: consider allowing the player's type to be changed */
			}
			return 1;

		case ID_TILEID:
			/* Erase the long component name */
			dialogEraseComponent(d, opt);

			/* Edit the type in decimal */
			sprintf(opt->text, "%d", t.type);
			if(dialogComponentEdit(d, opt, 3, LINED_NUMBER) == LINED_QUIT) {
				return DKEY_QUIT;
			}
			sscanf(opt->text, "%d", &i);

			/* Max value of 255 */
			if (i > 255) i = 255;
			t.type = i;

			/* Plot changes */
			t.param = zztParamDuplicate(t.param);
			zztPlot(w, x, y, t);

			return 1;

		case ID_COLOR:
			if(zztTileIsText(t)) {
				// Character is stored in the color field
				i = charselect(d, t.color);
			} else {
				i = selectcolor(d, t.color);
			}
			if (i == DKEY_QUIT)
				return DKEY_QUIT;
			if (i == -1)
				return 0;

			t.color = i;

			/* Plot changes */
			t.param = zztParamDuplicate(t.param);
			zztPlot(w, x, y, t);
			return 1;

		case ID_ADDPARAM:
			t.param = zztParamCreate(t);
			/* If this is usually a non-param type, force param creation */
			if (t.param == NULL) t.param = zztParamCreateBlank();
			zztPlot(w, x, y, t);
			return 1;

		case ID_RMPARAM:
			t.param = NULL;
			zztPlot(w, x, y, t);
			return 1;

		case ID_EDITPARAM:
			if (modifyparam(d, w, x, y) == DKEY_QUIT)
				return DKEY_QUIT;
			drawsidepanel(d, PANEL_TILE_INFO);
			return 1;

		case ID_UTYPE:
			i = selecttiletype(d, t.param->utype);
			if (i == DKEY_QUIT) return DKEY_QUIT;
			if (i == -1) return 0;

			/* Directly modify value */
			t.param->utype = i;
			return 1;

		case ID_UCOLOR:
			i = selectcolor(d, t.param->ucolor);
			if (i == DKEY_QUIT) return DKEY_QUIT;
			if (i == -1)
				return 0;

			/* Directly modify value */
			t.param->ucolor = i;
			return 1;

		default:
			return 0;
	}
}


/* Paramlist dialog */

char * getobjectname(ZZTtile object)
{
	char * program;
	char * endpos;

	if (object.param == NULL)
		return NULL;

	program = (char *) object.param->program;

	if (program == NULL)
		return NULL;

	/* If the first line doesn't start with an @, then the obj
	 * has no name */
	if (program[0] != '@')
		return NULL;

	/* Find the end of the first line */
	endpos = strchr(program, '\x0d');

	if (endpos == NULL)
		return NULL;

	return str_duplen(program + 1, endpos - program - 1);
}

char * buildparamdescription(ZZTblock * block, int index)
{
	ZZTparam * param = NULL;
	char * tileName    = NULL;
	char * description = NULL;

	if (index < 0 || index >= block->paramcount) {
		description = str_create(30);
		sprintf(description, "(missing stat %d)", index);
		return description;
	}

	param = block->params[index];

	if (param->x < block->width && param->y < block->height) {
		ZZTtile tile = zztTileAt(block, param->x, param->y);

		/* If it's an object, try getting its name */
		if (tile.type == ZZT_OBJECT)
			tileName = getobjectname(tile);

		/* Fall back on the general name of the tile */
		if (tileName == NULL)
			tileName = str_dup((char *) zztTileGetName(tile));

	} else {
		/* This tile's not on the board (it's probably a messenger) */
		tileName = str_dup("(no type)");
	}

	description = str_create(strlen(tileName) + 20);

	sprintf(description, "(%2d, %2d): ", (char) param->x + 1, (char) param->y + 1);
	strcat(description, tileName);

	/* QUICKHACK: if the name is too long, truncate */
	if (strlen(description) > 42)
		strcpy(description + 39, "...");

	free(tileName);
	return description;
}

/* Returns the index of the stat the given stat is bound to, or 0
   for none. */
int bound_to(ZZTblock * block, int index) {
	ZZTparam * param = NULL;

	if (index < 0 || index >= block->paramcount) {
		return 0;
	}

	param = block->params[index];

	return param->bindindex;
}

stringvector buildparamlist(ZZTblock * block)
{
	stringvector list;
	int i;

	/* Create an empty list */
	initstringvector(&list);

	if (block == NULL || block->params == NULL)
		return list;

	for (i = 0; i < block->paramcount; i++)
	{
		pushstring(&list, buildparamdescription(block, i));
	}

	return list;
}

int paramlistdialog(displaymethod * d, ZZTblock * block, int curparam, char * title)
{
	stringvector paramlist;
	int response;

	/* Build the param list and move to the beginning */
	paramlist = buildparamlist(block);
	svmovetofirst(&paramlist);
	preinsertstring(&paramlist, str_dup("(none)"));
	
	/* Move to the current param */
	svmoveby(&paramlist, curparam);

	/* Scrool through the params */
	response = scrolldialog(title, &paramlist, d);

	/* Change the current param if the user okay */
	if (response == EDITBOX_OK) {
		curparam = svgetposition(&paramlist) - 1;
	} else if (response == EDITBOX_QUIT) {
		curparam = DKEY_QUIT;
	}

	/* Free up all that memory we used */
	deletestringvector(&paramlist);

	return curparam;
}

int statsinfo(displaymethod * d, ZZTworld * w)
{
	ZZTblock * block = zztBoardGetBlock(w);
	stringvector paramlist;
	int response;

	/* Build the param list */
	paramlist = buildparamlist(block);

	do {
		response = scrolldialog("Stats Info", &paramlist, d);

		if (response == EDITBOX_OK && block->params != NULL) {
			int curparam = svgetposition(&paramlist);
			ZZTparam * param = block->params[curparam];

			if (param == NULL)
				continue;

			/* Modify the param */
			if(modifyparam(d, w, param->x, param->y) == DKEY_QUIT)
				response = EDITBOX_QUIT;
		}
	} while (response != EDITBOX_CANCEL && response != EDITBOX_QUIT);

	/* Free up all that memory we used */
	deletestringvector(&paramlist);

	return response;
}

