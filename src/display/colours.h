/* colours.h   -- colour defines
 * $Id: colours.h,v 1.1 2003/11/01 23:45:56 bitman Exp $
 */

#ifndef __COLOURS_H
#define __COLOURS_H

#define BLACK_F   0x00
#define BLUE_F    0x01
#define GREEN_F   0x02
#define RED_F     0x04
#define CYAN_F    0x03
#define MAGENTA_F 0x05
#define YELLOW_F  0x06
#define BROWN_F	  0x06
#define WHITE_F   0x07
#define BRIGHT_F  0x08

#define BLACK_B   0x00
#define BLUE_B    0x10
#define GREEN_B   0x20
#define RED_B     0x40
#define CYAN_B    0x30
#define MAGENTA_B 0x50
#define YELLOW_B  0x60
#define BROWN_B   0x60
#define WHITE_B   0x70
#define BRIGHT_B  0x80

typedef struct textcolor {
	int fg, bg;
	int blink;
} textcolor;

#define encodecolor(tcolor) (((tcolor.fg) & 0x0F) | ((tcolor.bg) << 4) | ((tcolor.blink) << 7))
#define makecolor(fg, bg, blink) (((fg) & 0x0F) | ((bg) << 4) | ((blink) << 7))
#define colorfg(color) ((color) & 0x0F)
#define colorbg(color) (((color) & 0x70) >> 4)
#define colorblink(color) ((color) >> 7)

#endif
