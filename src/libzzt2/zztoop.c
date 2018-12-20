/* zztoop - zzt oop parser */
/* $Id: zztoop.c,v 1.4 2006/11/26 21:44:00 kvance Exp $ */
/* Copyright (C) 2002 Ryan Phillips <bitman@users.sourceforge.net>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "zztoop.h"
#include "strtools.h"

#include <stdlib.h>
#include <string.h>

/**** Parser creation/destruction *****/

ZZTOOPparser * zztoopCreateParser(char * line)
{
	ZZTOOPparser * parser = (ZZTOOPparser *) malloc(sizeof(ZZTOOPparser));

	parser->line  = line;
	parser->flags = ZOOPFLAG_STRICTZZT;

	parser->first = parser->last = NULL;

	parser->token = NULL;
	parser->nextTokenPos = 0;

	return parser;
}

void zztoopDeleteParser(ZZTOOPparser * parser)
{
	if (parser == NULL)
		return;

	/* Free the token if it's still there */
	if (parser->token != NULL) {
		free(parser->token);
		parser->token = NULL;
	}

	/* Free the component chain unless it has been removed or does not exist. */
	if (parser->first != NULL) {
		zztoopDeleteComponentChain(parser->first);
		parser->first = parser->last = NULL;
	}

	free(parser);
	parser = NULL;
}

ZZTOOPcomponent * zztoopRemoveComponents(ZZTOOPparser * parser)
{
	ZZTOOPcomponent * chain = parser->first;

	parser->first = parser->last = NULL;
	return chain;
}

int zztoopAddComponent(ZZTOOPparser * parser, ZZTOOPcomponent * component)
{
	/* Assume component->next == NULL */

	if (parser->first == NULL) {
		/* Add component as first and last item */
		parser->first = component;
		parser->last  = component;
		return 1;
	} else if (parser->last != NULL) {
		/* Add component to last and advance last */
		parser->last->next = component;
		parser->last = component;
		return 1;
	} else {
		return 0;
	}
}

int zztoopAddToken(ZZTOOPparser * parser, int type, int value)
{
	ZZTOOPcomponent *component = zztoopCreateComponent(type, value, parser->token, parser->tokenPos);
	if(!zztoopAddComponent(parser, component)) {
		free(component);
		return 0;
	}
	zztoopNextToken(parser);
	return 1;
}

int zztoopAddRemainder(ZZTOOPparser * parser, int type, int value)
{
	/* TODO: consider stopping at newline characters */
	char * remainder = parser->line + parser->tokenPos;
	ZZTOOPcomponent *component = zztoopCreateComponent(type, value, remainder, parser->tokenPos);
	if(!zztoopAddComponent(parser, component)) {
		free(component);
		return 0;
	}

	/* Flush the token */
	parser->nextTokenPos = strlen(parser->line);
	zztoopNextToken(parser);
	return 1;
}

int zztoopAddWhitespace(ZZTOOPparser * parser)
{
	if (parser->tokenType != ZOOPTOK_WHITESPACE) {
		return 0;
	}
	ZZTOOPcomponent *component = zztoopCreateComponent(ZOOPTYPE_NONE, 0, parser->token, parser->tokenPos);
	if(!zztoopAddComponent(parser, component)) {
		free(component);
		return 0;
	}
	zztoopNextToken(parser);
	return 1;
}

/**** Component creation/destruction **/

ZZTOOPcomponent * zztoopCreateComponent(int type, int value, char * text, int pos)
{
	ZZTOOPcomponent * component = (ZZTOOPcomponent *) malloc(sizeof(ZZTOOPcomponent));

	component->type  = type;

	component->value = value;
	component->text  = str_dup(text);

	component->pos   = pos;

	component->next  = NULL;

	return component;
}

void zztoopDeleteComponentChain(ZZTOOPcomponent * components)
{
	if (components == NULL)
		return;

	/* Free data in the current component */
	if (components->text != NULL) {
		free(components->text);
		components->text = NULL;
	}

	/* Delete the next component in the chain */
	zztoopDeleteComponentChain(components->next);
	components->next = NULL;

	/* Free the current component */
	free(components);
}


/******** Parsing ********************/

/* NOTE: strspn(string, set) finds next char not in set */

int zztoopNextToken(ZZTOOPparser * parser)
{
	int tokenLen;
	char * tokenStart = parser->line + parser->nextTokenPos;
	char * tokenEnd;

	/* Free the old token if it's there */
	if (parser->token != NULL) {
		free(parser->token);
		parser->token = NULL;
	}

	/* End where we started and start moving forward */
	tokenEnd = tokenStart;

	/* Find the end of the next token */
	if (tokenStart[0] != '\x0') {

		/* Grab whitespace if there is any */
		tokenEnd += strspn(tokenStart, " \t");

		if (tokenEnd != tokenStart) {
			/* If tokenEnd advanced, we have whitespace */
			parser->tokenType = ZOOPTOK_WHITESPACE;
		} else {
			/* There were no spaces: look for other things */
			if (strchr("#:?/!\'$@", tokenStart[0]) != NULL) {
				/* Next character is a symbol - take it */
				tokenEnd = tokenStart + 1;
				parser->tokenType = ZOOPTOK_SYMBOL;
			} else if (strchr(";", tokenStart[0]) != NULL) {
				tokenEnd = tokenStart + 1;
				parser->tokenType = ZOOPTOK_SEPARATOR;
			} else {
				/* Grab everything until we hit symbols, spaces, or endlines */
				tokenEnd = tokenStart + strcspn(tokenStart, "#:?/!\'$@; \t\r\n");
				parser->tokenType = ZOOPTOK_TEXT;
			}
		}
	}

	/* Length of the token is the distance from start to finish */
	tokenLen = tokenEnd - tokenStart;

	/* If we grabbed nothing then we have nothing */
	if (tokenLen == 0)
		parser->tokenType = ZOOPTOK_NONE;

	/* Copy the token to the parser */
	parser->token = str_duplen(tokenStart, tokenLen);
	parser->tokenPos = parser->nextTokenPos;
	parser->nextTokenPos += tokenLen;

	/* Done */
	return tokenLen;
}


/**
 * @brief Grow the current token by appending the next token.
 *
 * @return the new token length.
 **/
int zztoopGrowToken(ZZTOOPparser * parser)
{
	char * newToken = NULL;
	char * oldToken = NULL;
	int oldTokenPos = 0;
	int tokenLen = 0;

	/* Copy the old token. */
	oldToken = str_dup(parser->token);
	tokenLen = strlen(oldToken);
	oldTokenPos = parser->tokenPos;

	/* Grab the next token. */
	tokenLen += zztoopNextToken(parser);

	newToken = str_duplen(oldToken, tokenLen);
	strcat(newToken, parser->token);

	free(oldToken);
	free(parser->token);
	parser->token = newToken;
	parser->tokenPos = oldTokenPos;

	return tokenLen;
}

/**
 * @brief Grow the current token until the next token will be a symbol.
 *
 * This function is useful when a token is expected to contain whitespace.
 *
 * @return the new token length.
 **/
int zztoopGrowTokenUntilSymbol(ZZTOOPparser * parser)
{
	int tokenLen = strlen(parser->token);

	/* Grow token to collect extra spaces. */
	while (strchr("#:?/!\'$@;\n\r", parser->line[parser->nextTokenPos]) == NULL)
		tokenLen = zztoopGrowToken(parser);

	return tokenLen;
}

ZZTOOPcomponent * zztoopParseLine(ZZTOOPparser * parser)
{
	/* Grab a token to start with */
	zztoopNextToken(parser);

	/* Begin parsing */
	zztoopParseRoot(parser);

	/* If there's more, I don't know what it is. */
	while (parser->tokenType != ZOOPTOK_NONE) {
		if (parser->flags & ZOOPFLAG_STRICTZZT)
			/* Strict ZZT doesn't allow other data at the end of the line */
			zztoopAddRemainder(parser, ZOOPTYPE_NONE, 0);
		else
			/* There may be comments at the end of a line */
			zztoopParseRoot(parser);
	}

	/* Return the component chain */
	return parser->first;
}

void zztoopParseRoot(ZZTOOPparser * parser)
{
	/* Find out what we're dealing with from the token type */
	switch (parser->tokenType) {
		case ZOOPTOK_SYMBOL:
			/* Deal with symbols */
			zztoopParseSymbol(parser);
			break;

		case ZOOPTOK_WHITESPACE:
			if (parser->flags & ZOOPFLAG_HELP) {
				/* Help oop can have leading whitespace to prevent parsing */
				zztoopAddWhitespace(parser);
				zztoopParseRoot(parser);
			} else {
				/* Leading whitespace means normal text for non-help */
				zztoopAddRemainder(parser, ZOOPTYPE_TEXT, ZOOPTEXT_NORMAL);
			}
			break;

		case ZOOPTOK_TEXT:
		case ZOOPTOK_SEPARATOR:
			/* It's just text (so is an initial separator). */
			zztoopAddRemainder(parser, ZOOPTYPE_TEXT, ZOOPTEXT_NORMAL);
			break;

		case ZOOPTOK_NONE:
			/* We got nothing, so add nothing. */
			break;
	}
}

void zztoopParseSymbol(ZZTOOPparser * parser)
{
	/* Parse current token as a symbol */
	char symbol = parser->token[0];

	if (parser->tokenType == ZOOPTOK_NONE)
		return;

	/* Add symbol component */
	zztoopAddToken(parser, ZOOPTYPE_SYMBOL, symbol);

	/* What kind of symbol is it? */
	switch (symbol) {
		case '#':
			/* Command or label */
			zztoopParseCommand(parser);
			break;

		case ':':
			/* label */
			zztoopParseLabel(parser);
			break;

		case '?':
		case '/':
			/* movement */
			/* Add a dummy movement command */
			zztoopAddComponent(parser, zztoopCreateComponent(ZOOPTYPE_COMMAND, (symbol == '?' ? ZOOPCMND_TRY : ZOOPCMND_GO), "", parser->tokenPos - 1));

			/* Grab the direction */
			zztoopParseDirection(parser);

			/* Piggy-back commands: continue parsing from the top */
			zztoopParseRoot(parser);
			break;

		case '!':
			/* hypermessage */
			zztoopParseHypermessage(parser);
			break;

		case '\'':
			/* comment */
			zztoopAddRemainder(parser, ZOOPTYPE_COMMENT, 0);
			break;

		case '$':
			/* heading */
			zztoopAddRemainder(parser, ZOOPTYPE_TEXT, ZOOPTEXT_HEADING);
			break;

		case '@':
			/* Under strict ZZT, object names are only valid on the first line */
			if ((parser->flags & ZOOPFLAG_STRICTZZT) &&
					!(parser->flags & ZOOPFLAG_FIRSTLINE)) {
				zztoopAddRemainder(parser, ZOOPTYPE_NONE, 0);
			} else {
				zztoopAddRemainder(parser, ZOOPTYPE_OBJNAME, 0);
				/* TODO: Non-strict parsing should search for comments */
			}
			break;
	}
}

void zztoopParseCommand(ZZTOOPparser * parser)
{
	int index;

	if (parser->tokenType == ZOOPTOK_NONE)
		return;

	/* Determine which command it is */
	index = zztoopFindCommand(parser->token);
	if (index != -1) {
		/* It's a command, so add a component for it */
		zztoopAddToken(parser, ZOOPTYPE_COMMAND, index);

		/* If the next token is whitespace, add it as such */
		zztoopAddWhitespace(parser);

		/* Parse the command arguments based on the command's index */
		zztoopParseCommandArgs(parser, index);

	} else {
		/* If it's not a valid command, treat it as a #send */

		/* Create a dummy send command for easy interpreting */
		zztoopAddComponent(parser, zztoopCreateComponent(ZOOPTYPE_COMMAND, ZOOPCMND_SEND, "", parser->tokenPos - 1));

		/* Parse the remainder of the line as arguments to the send command */
		zztoopParseCommandArgs(parser, ZOOPCMND_SEND);
	}
}

void zztoopParseLabel(ZZTOOPparser * parser)
{
	/* Label time! */
	if (parser->tokenType == ZOOPTOK_NONE)
		return;

	/* Under strict ZZT, the first line cannot be a label */
	if ((parser->flags & ZOOPFLAG_STRICTZZT) &&
	    (parser->flags & ZOOPFLAG_FIRSTLINE)) {
		/* Add the remainder as a NONE type */
		zztoopAddRemainder(parser, ZOOPTYPE_NONE, 0);

		return;
	}

	/* Grow token to collect extra spaces. */
	zztoopGrowTokenUntilSymbol(parser);

	/* Store the token as a label */
	zztoopAddToken(parser, ZOOPTYPE_LABEL, zztoopFindMessage(parser->token));

	/* In help mode, when the label is followed by a semicolon, the remainder of
	 * the line is text */
	if ((parser->flags & ZOOPFLAG_HELP) && (parser->tokenType == ZOOPTOK_SEPARATOR)) {
		zztoopAddToken(parser, ZOOPTYPE_SYMBOL, ';');

		zztoopAddRemainder(parser, ZOOPTYPE_TEXT, ZOOPTEXT_LABEL);

		return;
	}

	/* Grab any whitespace */
	zztoopAddWhitespace(parser);

	/* If label is trailed by a comment, go back to the root and parse it. */
	/* Also return to the root if we are not being strict */
	if (parser->token[0] == '\'' || !(parser->flags & ZOOPFLAG_STRICTZZT)) {
		zztoopParseRoot(parser);
	}
}

void zztoopParseDirection(ZZTOOPparser * parser)
{
	int index;

	if (parser->tokenType == ZOOPTOK_NONE)
		return;

	do {
		index = zztoopFindDirMod(parser->token);

		if (index != -1) {
			/* We found a modifier; add it */
			zztoopAddToken(parser, ZOOPTYPE_DIRMOD, index);

			zztoopAddWhitespace(parser);
		}
	} while (index != -1);

	/* Determine which direction it is */
	index = zztoopFindDir(parser->token);

	/* Add this token whether it's a valid direction or not. */
	zztoopAddToken(parser, ZOOPTYPE_DIR, index);
}

void zztoopParseMessage(ZZTOOPparser * parser)
{
	if (parser->line[parser->nextTokenPos] == ':') {
		/* Message is preceeded by an object name and colon */
		zztoopAddToken(parser, ZOOPTYPE_OBJNAME, 0);

		zztoopAddToken(parser, ZOOPTYPE_SYMBOL, parser->token[0]);
	}

	/* Grow token to collect extra spaces. */
	zztoopGrowTokenUntilSymbol(parser);

	/* Add the message */
	zztoopAddToken(parser, ZOOPTYPE_MESSAGE, zztoopFindMessage(parser->token));
}

void zztoopParseHypermessage(ZZTOOPparser * parser)
{
	if (parser->tokenType == ZOOPTOK_NONE)
		return;

	/* Check for leading '-' symbol, indicating that the next token in a
	 * filename. */
	if (parser->token[0] == '-') {
		char * newToken = NULL;
		zztoopAddComponent(parser, zztoopCreateComponent(ZOOPTYPE_SYMBOL, '-', "-", parser->tokenPos));

		/* Advance the token by one character. */
		newToken = str_dup(parser->token + 1);
		free(parser->token);
		parser->token = newToken;
		parser->tokenPos++;
	}

	if (parser->flags & ZOOPFLAG_STRICTZZT) {
		/* Collect tokens until ";", newline, or NULL begins next token. */
		while (strchr(";\n\r", parser->line[parser->nextTokenPos]) == NULL)
			zztoopGrowToken(parser);

		/* Add the hypermessage */
		zztoopAddToken(parser, ZOOPTYPE_MESSAGE, zztoopFindMessage(parser->token));
	} else {
		/* Hypermessages can have object:message form */
		zztoopParseMessage(parser);
	}

	/* Add the semicolon and hypertext */
	if (parser->tokenType == ZOOPTOK_SEPARATOR) {
		zztoopAddToken(parser, ZOOPTYPE_SYMBOL, ';');

		zztoopAddRemainder(parser, ZOOPTYPE_TEXT, ZOOPTEXT_HYPERTEXT);
	}
}

void zztoopParseKind(ZZTOOPparser * parser)
{
	/* Consider kind and colour */
	int index;

	/* Check for a color preceeding the kind */
	index = zztoopFindColour(parser->token);
	if (index != -1) {
		zztoopAddToken(parser, ZOOPTYPE_COLOR, index);
		zztoopAddWhitespace(parser);
	}

	/* Add the kind */
	index = zztoopFindKind(parser->token);
	zztoopAddToken(parser, ZOOPTYPE_KIND, index);
}


void zztoopParseCommandArgs(ZZTOOPparser * parser, int command)
{
	char * cmdargs;  /* Command argument type list */
	int argindex;    /* Index of current arg */

	if (command < 0 || command >= ZOOPCOMMANDCOUNT || parser->tokenType == ZOOPTOK_NONE)
		return;

	cmdargs = (char *) zztoopcommandargs[command];

	/* Handle earch argument individually */
	for (argindex = 0; argindex < strlen(cmdargs); argindex++) {
		/* Act on token based on expected argument type */
		switch (cmdargs[argindex]) {
			case ZOOPARG_OBJECTNAME:
				/* Object name is always the last argument */
				zztoopAddRemainder(parser, ZOOPTYPE_OBJNAME, 0);
				break;

			case ZOOPARG_NUMBER:
				/* TODO: should we find the integer value of the number? */
				zztoopAddToken(parser, ZOOPTYPE_NUMBER, 0);
				break;

			case ZOOPARG_FLAG:
				/* Check for the presence of flag modifiers (not is the only one at present): */
				if (str_equ(parser->token, "not", STREQU_UNCASE)) {
					zztoopAddToken(parser, ZOOPTYPE_FLAGMOD, 0);
					zztoopAddWhitespace(parser);
				}

				/* Add the flag */
				zztoopAddToken(parser, ZOOPTYPE_FLAG, zztoopFindFlag(parser->token));

				/* The flags "blocked" and "any" require more arguments */
				if (parser->last->value == ZOOPFLAG_BLOCKED) {
					zztoopAddWhitespace(parser);
					zztoopParseDirection(parser);
				} else if (parser->last->value == ZOOPFLAG_ANY) {
					zztoopAddWhitespace(parser);
					zztoopParseKind(parser);
				}

				break;

			case ZOOPARG_ITEM:
				zztoopAddToken(parser, ZOOPTYPE_ITEM, zztoopFindItem(parser->token));

			case ZOOPARG_THENMESSAGE:
				if (str_equ(parser->token, "then", STREQU_UNCASE)) {
					zztoopAddToken(parser, ZOOPTYPE_KEYWORD, 0);
					zztoopAddWhitespace(parser);
				}

				if (strchr("#/?!", parser->token[0])) {
					/* Remainder of args is #command or movement, parse from the top */
					zztoopParseRoot(parser);
				} else {
					/* Remainder is either command or message, # sign omitted */
					zztoopParseCommand(parser);
				}
				break;

			case ZOOPARG_MESSAGE:
				zztoopParseMessage(parser);
				break;

			case ZOOPARG_MUSIC:
				zztoopAddRemainder(parser, ZOOPTYPE_MUSIC, 0);
				break;

			case ZOOPARG_KIND:
				zztoopParseKind(parser);
				break;

			case ZOOPARG_DIRECTION:
				zztoopParseDirection(parser);
				break;
		}

		zztoopAddWhitespace(parser);
	}
}


/***** Lookup tables *************/

const char * zztooptypes[ZOOPTYPE_MAX + 1] =
{
	/* ZOOPTYPE_NONE       */  "Whitespace/Unknown",
	/* ZOOPTYPE_TEXT       */  "Text",
	/* ZOOPTYPE_SYMBOL     */  "Symbol",
	/* ZOOPTYPE_COMMENT    */  "Comment",
	/* ZOOPTYPE_COMMAND    */  "Command",
	/* ZOOPTYPE_KEYWORD    */  "Misc Keyword",
	/* ZOOPTYPE_MUSIC      */  "ZZM Music String",
	/* ZOOPTYPE_OBJNAME    */  "Object Name",
	/* ZOOPTYPE_NUMBER     */  "Number",

/* These types have both standard and non-standard values */
	/* ZOOPTYPE_LABEL      */  "Label",
	/* ZOOPTYPE_MESSAGE    */  "Message",
	/* ZOOPTYPE_FLAG       */  "Flag",
	/* ZOOPTYPE_FLAGMOD    */  "Flag Modifier",

/* These types have a specific set of valid values */
	/* ZOOPTYPE_ITEM       */  "Item",
	/* ZOOPTYPE_KIND       */  "Kind",
	/* ZOOPTYPE_COLOR      */  "Colour",
	/* ZOOPTYPE_DIR        */  "Direction",
	/* ZOOPTYPE_DIRMOD     */  "Direction Modifier"
};

const char * zztoopcommands[ZOOPCOMMANDCOUNT] =
{
	"become",  "bind",    "change", "char",
	"clear",   "cycle",   "die",    "end",
	"endgame", "give",    "go",     "idle",
	"if",      "lock",    "play",   "put",
	"restart", "restore", "send",   "set",
	"shoot",   "take",    "throwstar",
	"try",     "unlock",  "walk",   "zap"
};


const char * zztoopcommandargs[ZOOPCOMMANDCOUNT] =
{
	"k",  "o",   "kk", "n",
	"f",  "n",   "",   "",
	"",   "in",  "d",  "",
	"ft", "",    "s",  "dk",
	"",   "m",   "m",  "f",
	"d",  "int", "d",
	"dt", "",    "d",  "m"
};

const char * zztoopmessages[ZOOPMESSAGECOUNT] =
{
	"touch", "shot", "bombed", "thud", "energize"
};

const char * zztoopflags[ZOOPFLAGCOUNT] =
{
	"alligned", "contact", "blocked", "energized", "any"
};

const char * zztoopitems[ZOOPITEMCOUNT] =
{
	"ammo", "gems", "torches", "health", "score", "time"
};

const char * zztoopcolours[ZOOPCOLOURCOUNT] =
{
	"blue", "green", "red", "cyan", "purple", "yellow", "white"
};

const char * zztoopdirs[ZOOPDIRCOUNT] =
{
	"north", "south", "east", "west", "idle",
	"seek", "flow", "rnd", "rndns", "rndne",
	"n", "s", "e", "w", "i"
};

const char * zztoopdirmods[ZOOPDIRMODCOUNT] =
{
	"cw", "ccw", "rndp", "opp"
};

