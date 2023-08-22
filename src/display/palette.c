/* palette.c    -- Custom color palette
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "palette.h"


palette *palette_load(char *path) {
        FILE *fp = fopen(path, "rb");
        if(!fp) {
                return NULL;
        }

        // Detect the file type by its size
        fseek(fp, 0, SEEK_END);
        size_t file_size = ftell(fp);
        if(file_size == PALETTE_SIZE) {
                // Raw palette data
                fseek(fp, 0, SEEK_SET);
        } else {
                // Unknown size, bail out
                fclose(fp);
                return NULL;
        }

        palette *result = malloc(sizeof(palette));
        result->path = strdup(path);
        if(fread(result->data, 1, PALETTE_SIZE, fp) != PALETTE_SIZE) {
                // Read error
                free(result);
                result = NULL;
        }
        fclose(fp);
        return result;
}

void palette_free(palette *pal) {
        free(pal->path);
        free(pal);
}

const palette default_palette = {
        .path = "(default)",
        .data = {
                '\x00', '\x00', '\x00', '\x00', '\x00', '\x2A', '\x00', '\x2A',
                '\x00', '\x00', '\x2A', '\x2A', '\x2A', '\x00', '\x00', '\x2A',
                '\x00', '\x2A', '\x2A', '\x15', '\x00', '\x2A', '\x2A', '\x2A',
                '\x15', '\x15', '\x15', '\x15', '\x15', '\x3F', '\x15', '\x3F',
                '\x15', '\x15', '\x3F', '\x3F', '\x3F', '\x15', '\x15', '\x3F',
                '\x15', '\x3F', '\x3F', '\x3F', '\x15', '\x3F', '\x3F', '\x3F'
        },
};
