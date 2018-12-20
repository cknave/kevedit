/* zztoop - zzt oop parser */
/* $Id: zztoop.h,v 1.1 2003/11/01 23:45:57 bitman Exp $ */
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

#ifndef LIBZZT2_ZZTOOP_H
#define LIBZZT2_ZZTOOP_H 1

#include "zzt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A single component of an OOP string */
typedef struct ZZTOOPcomponent {
	struct ZZTOOPcomponent* next;

	int type;    /* Type of component */
	
	int value;   /* Value of the component (meaning varies with type) */
	char * text; /* Text value of the component */

	int pos;     /* Position of the component on the line */
} ZZTOOPcomponent;

typedef struct ZZTOOPparser {
	char * line;   /* A line of ZZT-OOP (hopefully) */
	int flags;     /* Control how parsing is done */

	/* We get results: */
	ZZTOOPcomponent * first, * last;

	/* private: */
	/* Internal tokenizing information */
	char * token;     /* Current token */
	int tokenPos;     /* Pos of token in line */
	int nextTokenPos; /* Pos of next token in line */
	int tokenType;

} ZZTOOPparser;

/* Parser flags */
#define ZOOPFLAG_STRICTZZT  0x1  /* Enforce strict ZZT rules */
#define ZOOPFLAG_HELP       0x2  /* Set when parsing help code */
#define ZOOPFLAG_FIRSTLINE  0x4  /* Set when parsing the first line in a script */

/* Types of tokens */
#define ZOOPTOK_SYMBOL      1
#define ZOOPTOK_SEPARATOR   2  /* semicolon */
#define ZOOPTOK_WHITESPACE  3
#define ZOOPTOK_TEXT        4
#define ZOOPTOK_NONE        5  /* empty token */


/* Component types */
#define ZOOPTYPE_NONE       0  /* Whitespace or unknown */
#define ZOOPTYPE_TEXT       1
#define ZOOPTYPE_SYMBOL     2  /* Symbols like $, #, :, etc. */
#define ZOOPTYPE_COMMENT    3
#define ZOOPTYPE_COMMAND    4
#define ZOOPTYPE_KEYWORD    5  /* Misc keywords (i.e. "then") */
#define ZOOPTYPE_MUSIC      6  /* ZZM music string */
#define ZOOPTYPE_OBJNAME    7
#define ZOOPTYPE_NUMBER     8

/* These types have both standard and non-standard values */
#define ZOOPTYPE_LABEL      9  /* Labels are sent to */
#define ZOOPTYPE_MESSAGE   10  /* Msgs are sent from */
#define ZOOPTYPE_FLAG      11
#define ZOOPTYPE_FLAGMOD   12  /* Most notably: not */

/* These types have a specific set of valid values */
#define ZOOPTYPE_ITEM      13
#define ZOOPTYPE_KIND      14
#define ZOOPTYPE_COLOR     15
#define ZOOPTYPE_DIR       16
#define ZOOPTYPE_DIRMOD    17  /* Direction modifier */

#define ZOOPTYPE_MAX       17


/* Number of built-in values for several types */
#define ZOOPCOMMANDCOUNT    27
#define ZOOPMESSAGECOUNT     5
#define ZOOPFLAGCOUNT        5
#define ZOOPITEMCOUNT        6
#define ZOOPCOLOURCOUNT      7
#define ZOOPDIRCOUNT        15
#define ZOOPDIRMODCOUNT      4

/* Built-in ZZT-OOP commands */
#define ZOOPCMND_BECOME     0
#define ZOOPCMND_BIND       1
#define ZOOPCMND_CHANGE     2
#define ZOOPCMND_CHAR       3
#define ZOOPCMND_CLEAR      4
#define ZOOPCMND_CYCLE      5
#define ZOOPCMND_DIE        6
#define ZOOPCMND_END        7
#define ZOOPCMND_ENDGAME    8
#define ZOOPCMND_GIVE       9
#define ZOOPCMND_GO        10
#define ZOOPCMND_IDLE      11
#define ZOOPCMND_IF        12
#define ZOOPCMND_LOCK      13
#define ZOOPCMND_PLAY      14
#define ZOOPCMND_PUT       15
#define ZOOPCMND_RESTART   16
#define ZOOPCMND_RESTORE   17
#define ZOOPCMND_SEND      18
#define ZOOPCMND_SET       19
#define ZOOPCMND_SHOOT     20
#define ZOOPCMND_TAKE      21
#define ZOOPCMND_THROWSTAR 22
#define ZOOPCMND_TRY       23
#define ZOOPCMND_UNLOCK    24
#define ZOOPCMND_WALK      25
#define ZOOPCMND_ZAP       26

/* Values for the TEXT type */
#define ZOOPTEXT_NORMAL    0
#define ZOOPTEXT_HEADING   1
#define ZOOPTEXT_LABEL     2
#define ZOOPTEXT_HYPERTEXT 3
#define ZOOPTEXT_MAX       3

/* Values for the FLAG type */
#define ZOOPFLAG_ALLIGNED  0
#define ZOOPFLAG_CONTACT   1
#define ZOOPFLAG_BLOCKED   2
#define ZOOPFLAG_ENERGIZED 3
#define ZOOPFLAG_ANY       4

/* Command argument syntax:
 * Each type of argument to a command is given a letter and stored
 * in a string in zztcommandargs[], which corresponds to that same
 * numbered element in zztcommands */

#define ZOOPARG_KIND        'k'
#define ZOOPARG_OBJECTNAME  'o'
#define ZOOPARG_NUMBER      'n'
#define ZOOPARG_FLAG        'f'
#define ZOOPARG_ITEM        'i'
#define ZOOPARG_DIRECTION   'd'
#define ZOOPARG_THENMESSAGE 't'
#define ZOOPARG_MUSIC       's'
#define ZOOPARG_MESSAGE     'm'

/* Create a new parser
 * Do not free or modify "line" until you are done with the parser. */
ZZTOOPparser * zztoopCreateParser(char * line);

/* Delete a parser */
void zztoopDeleteParser(ZZTOOPparser * parser);

/* Parse the line */
ZZTOOPcomponent * zztoopParseLine(ZZTOOPparser * parser);

/* Remove the component chain from a parser. Call this function
 * before deleting the parser if you want to keep using the
 * generated components. */
ZZTOOPcomponent * zztoopRemoveComponents(ZZTOOPparser * parser);

/* private: */

/* Add a component to the parser */
int zztoopAddComponent(ZZTOOPparser * parser, ZZTOOPcomponent * component);

/* Add the current token to the parser and move on */
int zztoopAddToken(ZZTOOPparser * parser, int type, int value);

/* Add remainder of the line (including token) as a component with given type and value */
int zztoopAddRemainder(ZZTOOPparser * parser, int type, int value);

/* Add the next token if it is whitespace and advance */
int zztoopAddWhitespace(ZZTOOPparser * parser);


/* Create a new component
 * Note: text will be free()ed when component is deleted. */
ZZTOOPcomponent * zztoopCreateComponent(int type, int value, char * text, int pos);

/* Free a chain of components */
void zztoopDeleteComponentChain(ZZTOOPcomponent * components);


/* Parsing subroutines */
void zztoopParseRoot(ZZTOOPparser * parser);  /* First level */
void zztoopParseSymbol(ZZTOOPparser * parser);
void zztoopParseCommand(ZZTOOPparser * parser);
void zztoopParseLabel(ZZTOOPparser * parser);
void zztoopParseDirection(ZZTOOPparser * parser);
void zztoopParseMessage(ZZTOOPparser * parser);
void zztoopParseHypermessage(ZZTOOPparser * parser);
void zztoopParseCommandArgs(ZZTOOPparser * parser, int command);

/* Grab the next token
 * Returns the length of the token */
int zztoopNextToken(ZZTOOPparser * parser);

/* Give a textual description of a token type */
#define zztoopTypeDescription(type) (type <= ZOOPTYPE_MAX ? zztooptypes[(type)] : zztooptypes[0])

/* Token identifying macros (return -1 on failure) */
#define zztoopFindCommand(token) lookupString(zztoopcommands, ZOOPCOMMANDCOUNT, (token), STREQU_UNCASE)
#define zztoopFindMessage(token) lookupString(zztoopmessages, ZOOPMESSAGECOUNT, (token), STREQU_UNCASE)
#define zztoopFindFlag(token)    lookupString(zztoopflags,    ZOOPFLAGCOUNT,    (token), STREQU_UNCASE)
#define zztoopFindItem(token)    lookupString(zztoopitems,    ZOOPITEMCOUNT,    (token), STREQU_UNCASE)
#define zztoopFindDir(token)     lookupString(zztoopdirs,     ZOOPDIRCOUNT,     (token), STREQU_UNCASE)
#define zztoopFindDirMod(token)  lookupString(zztoopdirmods,  ZOOPDIRMODCOUNT,  (token), STREQU_UNCASE)
#define zztoopFindColour(token)  lookupString(zztoopcolours,  ZOOPCOLOURCOUNT,  (token), STREQU_UNCASE)
#define zztoopFindKind(token)    lookupString(_zzt_type_kind_table, ZZT_MAX_TYPE, (token), STREQU_UNCASE)

/* Lookup tables */
extern const char * zztooptypes[ZOOPTYPE_MAX + 1];

extern const char * zztoopcommands[ZOOPCOMMANDCOUNT];
extern const char * zztoopcommandargs[ZOOPCOMMANDCOUNT];
extern const char * zztoopmessages[ZOOPMESSAGECOUNT];
extern const char * zztoopflags[ZOOPFLAGCOUNT];
extern const char * zztoopitems[ZOOPITEMCOUNT];
extern const char * zztoopdirs[ZOOPDIRCOUNT];
extern const char * zztoopdirmods[ZOOPDIRMODCOUNT];
extern const char * zztoopcolours[ZOOPCOLOURCOUNT];

#ifdef __cplusplus
}
#endif

#endif /* LIBZZT2_ZZTOOP_H */
