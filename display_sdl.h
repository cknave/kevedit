#include <stdio.h>
#include "SDL.h"

/* These are in bytes. */
#define TEXT_MODE_VRAM (80 * 25 * 2)

/* This structure has pointers to all important video information */

typedef struct
{
  SDL_Surface *video;            /* Pointer to the video surface in use */
  SDL_Surface *buffer_surface;   /* Everything gets drawn to here first */
  Uint8 *buffer;                 /* Pointer to emualted VRAM            */
  Uint8 *char_set;               /* Pointer to character set            */
  Uint32 *palette;               /* Pointer to palette                  */
  Uint32 write_x;                /* Current x write position in VRAM    */
  Uint32 write_y;                /* Current y write position in VRAM    */
} video_info;

/* Prototypes */
void display_load_charset(Uint8 *dest, Uint8 *name);
void display_load_palette(Uint32 *dest, Uint8 *name);
void display_init(video_info *vdest, Uint32 width, Uint32 height, Uint32
 depth, Uint32 full_screen, Uint32 hw_surface);
void display_end(video_info *vdest);
void display_putch(video_info *vdest, Uint32 x, Uint32 y, Uint8 ch, Uint8 co);
void display_gotoxy(video_info *vdest, Uint32 x, Uint32 y);
void display_redraw(video_info *vdest);
void display_update(video_info *vdest, int x, int y, int width, int height);

/* For linking to KevEdit */
extern displaymethod display_sdl;
