/* Image Saved In ACiDDRAW v1.1 */
/* $Id: cbox.c,v 1.1 2003/11/01 23:45:57 bitman Exp $ */

#include "src/display/textblock.h"

#define CHAR_BOX_WIDTH 34
#define CHAR_BOX_DEPTH 10
#define CHAR_BOX_LENGTH 680
unsigned char CHAR_BOX[] =
{
	'\xda', 0x2F, '\xc4', 0x2F, '\xc4', 0x2F, '\xc4', 0x2F, '\xc4', 0x2F, '\xc4', 0x2A, '\xc4', 0x2A,
	'\xc4', 0x2A, 'S', 0x2F, 'e', 0x2F, 'l', 0x2F, 'e', 0x2F, 'c', 0x2F, 't', 0x2F,
	' ', 0x2F, 'A', 0x2F, ' ', 0x2F, 'C', 0x2F, 'h', 0x2F, 'a', 0x2F, 'r', 0x2F,
	'a', 0x2F, 'c', 0x2F, 't', 0x2F, 'e', 0x2F, 'r', 0x2F, '\xc4', 0x2A, '\xc4', 0x2A,
	'\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xbf', 0x2A, '\xb3', 0x2F,
	0, 0x0A, 1, 0x0A, 2, 0x0A, 3, 0x0A, 4, 0x0A, 5, 0x0A, 6, 0x0A,
	7, 0x0A, 8, 0x0A, 9, 0x0A, 10, 0x0A, 11, 0x0A, 12, 0x0A, 13, 0x0A,
    14, 0x0A, 15, 0x0A, 16, 0x0A, 17, 0x0A, 18, 0x0A, 19, 0x0A, 20, 0x0A,
    21, 0x0A, 22, 0x0A, 23, 0x0A, 24, 0x0A, 25, 0x0A, 26, 0x0A, 27, 0x0A,
 28, 0x0A, 29, 0x0A, 30, 0x0A, 31, 0x0A, '\xb3', 0x2A, '\xb3', 0x2A, ' ', 0x0A,
	'!', 0x0A, '"', 0x0A, '#', 0x0A, '$', 0x0A, '%', 0x0A, '&', 0x0A, '\'', 0x0A,
	'(', 0x0A, ')', 0x0A, '*', 0x0A, '+', 0x0A, ',', 0x0A, '-', 0x0A, '.', 0x0A,
	'/', 0x0A, '0', 0x0A, '1', 0x0A, '2', 0x0A, '3', 0x0A, '4', 0x0A, '5', 0x0A,
	'6', 0x0A, '7', 0x0A, '8', 0x0A, '9', 0x0A, ':', 0x0A, ';', 0x0A, '<', 0x0A,
	'=', 0x0A, '>', 0x0A, '?', 0x0A, '\xb3', 0x2A, '\xb3', 0x2A, '@', 0x0A, 'A', 0x0A,
	'B', 0x0A, 'C', 0x0A, 'D', 0x0A, 'E', 0x0A, 'F', 0x0A, 'G', 0x0A, 'H', 0x0A,
	'I', 0x0A, 'J', 0x0A, 'K', 0x0A, 'L', 0x0A, 'M', 0x0A, 'N', 0x0A, 'O', 0x0A,
	'P', 0x0A, 'Q', 0x0A, 'R', 0x0A, 'S', 0x0A, 'T', 0x0A, 'U', 0x0A, 'V', 0x0A,
	'W', 0x0A, 'X', 0x0A, 'Y', 0x0A, 'Z', 0x0A, '[', 0x0A, '\\', 0x0A, ']', 0x0A,
	'^', 0x0A, '_', 0x0A, '\xb3', 0x2A, '\xb3', 0x2A, '`', 0x0A, 'a', 0x0A, 'b', 0x0A,
	'c', 0x0A, 'd', 0x0A, 'e', 0x0A, 'f', 0x0A, 'g', 0x0A, 'h', 0x0A, 'i', 0x0A,
	'j', 0x0A, 'k', 0x0A, 'l', 0x0A, 'm', 0x0A, 'n', 0x0A, 'o', 0x0A, 'p', 0x0A,
	'q', 0x0A, 'r', 0x0A, 's', 0x0A, 't', 0x0A, 'u', 0x0A, 'v', 0x0A, 'w', 0x0A,
	'x', 0x0A, 'y', 0x0A, 'z', 0x0A, '{', 0x0A, '|', 0x0A, '}', 0x0A, '~', 0x0A,
	127, 0x0A, '\xb3', 0x2A, '\xb3', 0x2A, 128, 0x0A, 129, 0x0A, 130, 0x0A, 131, 0x0A,
	132, 0x0A, 133, 0x0A, 134, 0x0A, 135, 0x0A, 136, 0x0A, 137, 0x0A, 138, 0x0A,
	139, 0x0A, 140, 0x0A, 141, 0x0A, 142, 0x0A, 143, 0x0A, 144, 0x0A, 145, 0x0A,
	146, 0x0A, 147, 0x0A, 148, 0x0A, 149, 0x0A, 150, 0x0A, 151, 0x0A, 152, 0x0A,
	153, 0x0A, 154, 0x0A, 155, 0x0A, 156, 0x0A, 157, 0x0A, 158, 0x0A, 159, 0x0A,
	'\xb3', 0x2A, '\xb3', 0x2A, 160, 0x0A, 161, 0x0A, 162, 0x0A, 163, 0x0A, 164, 0x0A,
	165, 0x0A, 166, 0x0A, 167, 0x0A, 168, 0x0A, 169, 0x0A, 170, 0x0A, 171, 0x0A,
	172, 0x0A, 173, 0x0A, 174, 0x0A, 175, 0x0A, 176, 0x0A, 177, 0x0A, 178, 0x0A,
	179, 0x0A, 180, 0x0A, 181, 0x0A, 182, 0x0A, 183, 0x0A, 184, 0x0A, 185, 0x0A,
	186, 0x0A, 187, 0x0A, 188, 0x0A, 189, 0x0A, 190, 0x0A, 191, 0x0A, '\xb3', 0x2A,
	'\xb3', 0x2A, 192, 0x0A, 193, 0x0A, 194, 0x0A, 195, 0x0A, 196, 0x0A, 197, 0x0A,
	198, 0x0A, 199, 0x0A, 200, 0x0A, 201, 0x0A, 202, 0x0A, 203, 0x0A, 204, 0x0A,
	205, 0x0A, 206, 0x0A, 207, 0x0A, 208, 0x0A, 209, 0x0A, 210, 0x0A, 211, 0x0A,
	212, 0x0A, 213, 0x0A, 214, 0x0A, 215, 0x0A, 216, 0x0A, 217, 0x0A, 218, 0x0A,
	219, 0x0A, 220, 0x0A, 221, 0x0A, 222, 0x0A, 223, 0x0A, '\xb3', 0x2A, '\xb3', 0x2A,
	224, 0x0A, 225, 0x0A, 226, 0x0A, 227, 0x0A, 228, 0x0A, 229, 0x0A, 230, 0x0A,
	231, 0x0A, 232, 0x0A, 233, 0x0A, 234, 0x0A, 235, 0x0A, 236, 0x0A, 237, 0x0A,
	238, 0x0A, 239, 0x0A, 240, 0x0A, 241, 0x0A, 242, 0x0A, 243, 0x0A, 244, 0x0A,
	245, 0x0A, 246, 0x0A, 247, 0x0A, 248, 0x0A, 249, 0x0A, 250, 0x0A, 251, 0x0A,
	252, 0x0A, 253, 0x0A, 254, 0x0A, 255, 0x0A, '\xb3', 0x2F, '\xc0', 0x2A, '\xc4', 0x2A,
	'\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A,
	'\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A,
	'\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A,
	'\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2A, '\xc4', 0x2F,
	'\xc4', 0x2F, '\xc4', 0x2F, '\xc4', 0x2F, '\xd9', 0x2F};

/* Buffer for restoring what's underneath when displaying
   a character selection box. */

textDatum charBoxBuffer[textBlockDataSize(CHAR_BOX_WIDTH, CHAR_BOX_DEPTH)];

textBlock charBoxBackup = {
	CHAR_BOX_WIDTH,
	CHAR_BOX_DEPTH,
	charBoxBuffer
};