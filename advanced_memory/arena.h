#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

/*
 * A tiny arena (bump) allocator for learning.
 *
 * The arena owns one large block. Individual allocations are taken from
 * that block; they are NOT freed separately. Call arena_reset() to reuse
 * the whole block, or arena_destroy() to release it.
 *
 * This allocator guarantees alignment suitable for ordinary C types
 * (up to _Alignof(max_align_t)). It is not a general-purpose replacement
 * for malloc(), and it is not thread-safe.
 */
typedef struct {
    unsigned char *buffer;
    size_t capacity;
    size_t used;
} Arena;

/* Returns 1 on success, 0 on failure. A capacity of zero is rejected. */
int arena_init(Arena *arena, size_t capacity);

/* Returns aligned storage, or NULL if size is zero or space is exhausted. */
void *arena_alloc(Arena *arena, size_t size);

/* Invalidates every allocation made from this arena, but keeps its storage. */
void arena_reset(Arena *arena);

/* Releases the backing block and clears the arena fields. */
void arena_destroy(Arena *arena);

#endif
