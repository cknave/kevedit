/* notebuf.c    -- SDL music player note buffer
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

#include <stdlib.h>
#include <string.h>

#include "notebuf.h"

void notebuf_init(notebuf_t *notebuf, size_t capacity) {
        notebuf->notes = malloc(sizeof(notebuf_note_t) * capacity);
        notebuf->capacity = capacity;
        notebuf->length = 0;
        notebuf->total_duration = 0;
}

void notebuf_deinit(notebuf_t *notebuf) {
        free(notebuf->notes);
        notebuf->notes = NULL;
        notebuf->capacity = 0;
        notebuf->length = 0;
        notebuf->total_duration = 0;
}

void notebuf_resize(notebuf_t *notebuf, size_t capacity) {
        notebuf->notes = realloc(notebuf->notes, sizeof(notebuf_note_t) * capacity);
        notebuf->capacity = capacity;
        if(notebuf->length > capacity) {
                notebuf->length = capacity;
        }
        notebuf->total_duration = 0;
        for(int i = 0; i < notebuf->length; i++) {
                notebuf->total_duration += (double)notebuf->notes[i].duration;
        }
}

void notebuf_clear(notebuf_t *notebuf) {
        notebuf->length = 0;
        notebuf->total_duration = 0;
}

bool notebuf_push_back(notebuf_t *notebuf, notebuf_note_t note) {
        if(notebuf->length >= notebuf->capacity) {
                return false;
        }
        notebuf->notes[notebuf->length] = note;
        notebuf->length++;
        notebuf->total_duration += note.duration;
        return true;
}

size_t notebuf_delete_front(notebuf_t *notebuf, size_t count) {
        if(count >= notebuf->length) {
                size_t res = notebuf->length;
                notebuf->length = 0;
                notebuf->total_duration = 0;
                return res;
        }
        for(unsigned int i = 0; i < count; i++) {
                notebuf->total_duration -= notebuf->notes[i].duration;
        }
        if(notebuf->total_duration < 0) {
                notebuf->total_duration = 0;  // in case of floating point error
        }
        memmove(notebuf->notes,
                &notebuf->notes[count],
                (notebuf->length - count) * sizeof(notebuf_note_t));
        notebuf->length -= count;
        return count;
}

notebuf_subtract_result_t notebuf_subtract_from_first(notebuf_t *notebuf, double duration, double *subtracted) {
        if(notebuf->length == 0) {
                *subtracted = duration;
                return NOTEBUF_SUBTRACTED;
        }
        if(duration < notebuf->notes[0].duration) {
                notebuf->notes[0].duration -= duration;
                notebuf->total_duration -= duration;
                *subtracted = duration;
                return NOTEBUF_SUBTRACTED;
        }
        *subtracted = notebuf->notes[0].duration;
        notebuf_delete_front(notebuf, 1);
        return NOTEBUF_DELETED;
}
