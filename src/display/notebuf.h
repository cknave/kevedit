/* notebuf.h    -- SDL music player note buffer
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

#ifndef NOTEBUF_H
#define NOTEBUF_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    float frequency;
    double duration;
} notebuf_note_t;

typedef struct {
    notebuf_note_t *notes;
    double total_duration;
    size_t capacity;
    size_t length;
} notebuf_t;

typedef enum {
    // The requested duration was subtracted from the front note
    NOTEBUF_SUBTRACTED,

    // The front note was deleted.  The remaining time was returned in *subtracted.
    NOTEBUF_DELETED,
} notebuf_subtract_result_t;

void notebuf_init(notebuf_t *notebuf, size_t capacity);
void notebuf_deinit(notebuf_t *notebuf);
void notebuf_resize(notebuf_t *notebuf, size_t capacity);
void notebuf_clear(notebuf_t *notebuf);

bool notebuf_push_back(notebuf_t *notebuf, notebuf_note_t note);
size_t notebuf_delete_front(notebuf_t *notebuf, size_t count);

// If the duration argument doesn't exceed the duration of the first note, subtract it from the
// first note and return NOTEBUF_SUBTRACTED.  *subtracted will be set to the requested duration.
//
// If the duration argument is longer than the first note, delete the first note and return
// NOTEBUF_DELETED.  *subtracted will be set to the deleted note's duration.
//
// If the buffer is empty, always return NOTEBUF_SUBTRACTED and set *subtracted to the duration
// argument.
notebuf_subtract_result_t notebuf_subtract_from_first(
        notebuf_t *notebuf,
        double duration,
        double *subtracted);

#endif
