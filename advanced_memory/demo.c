//demo.c
#include "arena.h"

#include <stdio.h>

typedef struct {
    int x;
    int y;
} Point;

int main(void)
{
    Arena arena = {0};

    if (!arena_init(&arena, 1024)) {
        fputs("Could not create arena\n", stderr);
        return 1;
    }

    /*
     * Both objects come from one backing allocation. No individual free()
     * is needed; the arena releases them together.
     */
    Point *point = arena_alloc(&arena, sizeof *point);
    char *label = arena_alloc(&arena, 64);

    if (point == NULL || label == NULL) {
        fputs("Arena is out of space\n", stderr);
        arena_destroy(&arena);
        return 1;
    }

    point->x = 12;
    point->y = -4;
    snprintf(label, 64, "Point(%d, %d)", point->x, point->y);

    printf("%s\n", label);
    printf("Used %zu of %zu bytes\n", arena.used, arena.capacity);

    /*
     * Reset makes the space reusable, but point and label must no longer
     * be read or written after this call: their contents' lifetime ended.
     */
    arena_reset(&arena);
    printf("After reset: %zu bytes used\n", arena.used);

    arena_destroy(&arena);
    return 0;
}
