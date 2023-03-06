/* charset.h    -- Defines for character set
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

#ifndef CHARSET_H
#define CHARSET_H 1

#include <stdint.h>

/* Character set buffer size, in bytes. */
#define CHARSET_SIZE 3584

typedef struct {
    char *path;
    uint8_t data[CHARSET_SIZE];
} charset;

extern const charset default_charset;

/* Return a new charset from the file at this path, or NULL if it could not be loaded. */
charset *charset_load(char *path);

/* Free a charset returned from load_charset */
void charset_free(charset *char_set);

#endif
