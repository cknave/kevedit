#include <assert.h>

#include "notebuf.h"

notebuf_t nb;

void push_tests() {
        // Push success
        {
                double expected_duration = 0;
                for(int i = 1; i <= nb.capacity; i++) {
                        bool res = notebuf_push_back(&nb, (notebuf_note_t) {
                                .duration = (double)i,
                                .frequency = (float)i,
                        });
                        assert(res);
                        expected_duration += (double)i;
                        assert(nb.total_duration == expected_duration);
                        assert(nb.length == i);
                }
                for(int i = 0; i < nb.capacity; i++) {
                        assert(nb.notes[i].frequency == i + 1);
                        assert(nb.notes[i].duration == i + 1);
                }
        }

        // Push failed
        {
                assert(nb.length == nb.capacity);
                bool res = notebuf_push_back(&nb, (notebuf_note_t) {
                        .duration = 100,
                        .frequency = 100,
                });
                assert(!res);
                assert(nb.length == nb.capacity);
                assert(nb.total_duration == 1+2+3+4+5);
        }
}

void resize_tests() {
        // Resize larger
        {
                notebuf_resize(&nb, 6);
                assert(nb.capacity == 6);
                assert(nb.length == 5);
                bool res = notebuf_push_back(&nb, (notebuf_note_t) {
                        .duration = 6,
                        .frequency = 6,
                });
                assert(res);
                assert(nb.length == 6);
                assert(nb.total_duration == 1+2+3+4+5+6);
                for(int i = 0; i < nb.capacity; i++) {
                        assert(nb.notes[i].frequency == i + 1);
                        assert(nb.notes[i].duration == i + 1);
                }
        }

        // Resize smaller
        {
                notebuf_resize(&nb, 3);
                assert(nb.capacity == 3);
                assert(nb.length == 3);
                assert(nb.total_duration == 1+2+3);
                for(int i = 0; i < nb.capacity; i++) {
                        assert(nb.notes[i].frequency == i + 1);
                        assert(nb.notes[i].duration == i + 1);
                }
        }
}

void clear_tests() {
        notebuf_resize(&nb, 3);
        notebuf_clear(&nb);
        assert(nb.capacity == 3);
        assert(nb.length == 0);
        assert(nb.total_duration == 0);
}

void delete_tests() {
        notebuf_resize(&nb, 3);

        // Delete less than length
        {
                for(int i = 1; i <= nb.capacity; i++) {
                        notebuf_push_back(&nb, (notebuf_note_t) {
                                .frequency = (float)i,
                                .duration = (double)i,
                        });
                }
                size_t res = notebuf_delete_front(&nb, 2);
                assert(res == 2);
                assert(nb.capacity == 3);
                assert(nb.length == 1);
                assert(nb.total_duration == 3);
                assert(nb.notes[0].frequency == 3);
                assert(nb.notes[0].duration == 3);
        }

        // Delete more than length
        {
                size_t res = notebuf_delete_front(&nb, 2);
                assert(res == 1);
                assert(nb.capacity == 3);
                assert(nb.length == 0);
                assert(nb.total_duration == 0);
        }
}

void subtract_tests() {
        // Prepare for subtract tests
        {
                notebuf_resize(&nb, 3);
                for(int i = 1; i <= 4; i++) {
                        notebuf_push_back(&nb, (notebuf_note_t) {
                                .frequency = (float)i,
                                .duration = (double)i,
                        });
                }
        }

        // Subtract exact note duration
        {
                double subtracted;
                notebuf_subtract_result_t res = notebuf_subtract_from_first(
                        &nb, 1.0f, &subtracted);
                assert(res == NOTEBUF_DELETED);
                assert(subtracted == 1.0f);
                assert(nb.length == 2);
                assert(nb.total_duration == 2+3);
                assert(nb.notes[0].duration == 2.0f);
                assert(nb.notes[1].duration == 3.0f);
        }

        // Subtract less than note duration
        {
                double subtracted;
                notebuf_subtract_result_t res = notebuf_subtract_from_first(
                        &nb, 1.0f, &subtracted);
                assert(res == NOTEBUF_SUBTRACTED);
                assert(subtracted == 1.0f);
                assert(nb.length == 2);
                assert(nb.total_duration == 1+3);
                assert(nb.notes[0].duration == 1.0f);
                assert(nb.notes[1].duration == 3.0f);
        }

        // Subtract more than note duration
        {
                double subtracted;
                notebuf_subtract_result_t res = notebuf_subtract_from_first(
                        &nb, 9.0f, &subtracted);
                assert(res == NOTEBUF_DELETED);
                assert(subtracted == 1.0f);
                assert(nb.length == 1);
                assert(nb.total_duration == 3.0f);
                assert(nb.notes[0].duration == 3.0f);
        }

        // Subtract last note
        {
                double subtracted;
                notebuf_subtract_result_t res = notebuf_subtract_from_first(
                        &nb, 999.0f, &subtracted);
                assert(res == NOTEBUF_DELETED);
                assert(subtracted == 3.0f);
                assert(nb.length == 0);
                assert(nb.total_duration == 0);
        }

        // Subtract from empty notebuf
        {
                double subtracted;
                notebuf_subtract_result_t res = notebuf_subtract_from_first(
                        &nb, 0.9f, &subtracted);
                assert(res == NOTEBUF_SUBTRACTED);
                assert(subtracted == 0.9f);
                assert(nb.length == 0);
                assert(nb.total_duration == 0);
        }
}

int main() {
        // Initialization
        notebuf_init(&nb, 5);
        assert(nb.total_duration == 0);
        assert(nb.length == 0);
        assert(nb.capacity == 5);

        push_tests();
        resize_tests();
        clear_tests();
        delete_tests();
        subtract_tests();

        // Deinit
        notebuf_deinit(&nb);
        assert(nb.notes == NULL);
        assert(nb.capacity == 0);
        assert(nb.length == 0);
        assert(nb.total_duration == 0);

        return 0;
}
