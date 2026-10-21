#include "arena.h"

#include <stdlib.h>
#include <stddef.h>

int arena_init(Arena *arena, size_t capacity)
{
    if (arena == NULL || capacity == 0) {
        return 0;
    }

    arena->buffer = malloc(capacity);
    if (arena->buffer == NULL) {
        arena->capacity = 0;
        arena->used = 0;
        return 0;
    }

    arena->capacity = capacity;
    arena->used = 0;
    return 1;
}

void *arena_alloc(Arena *arena, size_t size)
{
    if (arena == NULL || arena->buffer == NULL || size == 0 ||
        arena->used > arena->capacity) {
        return NULL;
    }

    /*
     * malloc() gives the base block alignment suitable for max_align_t.
     * Keep every returned offset a multiple of that alignment too.
     */
    const size_t alignment = _Alignof(max_align_t);
    const size_t remainder = arena->used % alignment;
    const size_t padding = (alignment - remainder) % alignment;

    /* Subtraction-based checks avoid overflowing used + padding + size. */
    if (padding > arena->capacity - arena->used) {
        return NULL;
    }

    const size_t start = arena->used + padding;
    if (size > arena->capacity - start) {
        return NULL;
    }

    void *result = arena->buffer + start;
    arena->used = start + size;
    return result;
}

void arena_reset(Arena *arena)
{
    if (arena != NULL && arena->buffer != NULL) {
        /* All previous pointers into the arena are now logically invalid. */
        arena->used = 0;
    }
}

void arena_destroy(Arena *arena)
{
    if (arena == NULL) {
        return;
    }

    free(arena->buffer);
    arena->buffer = NULL;
    arena->capacity = 0;
    arena->used = 0;
}
