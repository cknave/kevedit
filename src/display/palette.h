/* charset.h    -- Defines for custom color palette
 * Copyright (C) 2023 Kev Vance <kvance@kvance.com>
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

#ifndef PALETTE_H
#define PALETTE_H 1

#include <stdint.h>

/* Palette size, in bytes. */
#define PALETTE_SIZE (48)

typedef struct {
    char *path;
    uint8_t data[PALETTE_SIZE];
} palette;

extern const palette default_palette;

/* Return a new palette from the file at this path, or NULL if it could not be loaded. */
palette *palette_load(char *path);

/* Free a palette returned from load_palette */
void palette_free(palette *char_set);

#endif
