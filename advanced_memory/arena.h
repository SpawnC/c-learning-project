//arena.h
#ifndef ARENA_H
#define ARENA_H
/*
  This is an include guard.

  It prevents this header file from being included
  more than once in the same source file.

  If ARENA_H has not been defined yet, #ifndef allows
  the code below to be processed.

  #define ARENA_H defines the name so that if this
  file is included again, its contents will be skipped.

  This prevents duplicate declarations and other
  problems caused by including the same header twice.
*/

#include <stddef.h>
//Important for size_t and NULL.
//size_t is an unsigned integer type used for sizes
//and memory-related measurements.
//NULL represents a null pointer, meaning that a
//pointer does not point to a valid object.

/*
  An arena allocator is a memory management technique.

  Instead of allocating every object separately with
  malloc(), we allocate one large block of memory.

  Individual objects are then placed inside that block.

  This is sometimes called a bump allocator because
  a position inside the block moves forward whenever
  we allocate more memory.

  Individual objects cannot be freed separately.
  Instead, arena_reset() makes the entire block
  reusable, and arena_destroy() releases the block.
*/

typedef struct {
    unsigned char *buffer;
    /*
      This is a pointer to the beginning of our memory.

      unsigned char represents a byte-sized unit of
      storage. It is useful when working directly
      with raw memory.

      The * means that buffer is a pointer, not an
      ordinary unsigned char variable.

      We use this pointer to access different positions
      inside the allocated block.
    */

    size_t capacity;
    /*
      The total number of bytes available in the arena.

      For example, if capacity is 1024, the arena owns
      a block that can hold up to 1024 bytes.

      This number does not change when objects are
      allocated. It describes the total block size.
    */

    size_t used;
    /*
      The current offset inside the arena.

      It tells us how many bytes at the beginning of
      the arena have been consumed, including padding
      inserted to align allocations.

      If used is 100, the next allocation starts
      searching at offset 100, before alignment.

      This is not a pointer. It is a byte count.
    */
} Arena;
/*
  typedef gives a type another name.

  struct defines a group of related variables.

  Arena is now the name we use for this structure.

  For example:
      Arena arena;

  creates an arena variable containing the pointer
  and the two size values defined above.
*/

/*
  Initializes an arena by allocating one large block.

  arena is a pointer to the Arena structure that
  should be initialized.

  capacity specifies the number of bytes requested.

  Returns 1 if initialization succeeds.
  Returns 0 if initialization fails.

  A capacity of zero is rejected because this
  implementation requires a non-empty memory block.
*/
int arena_init(Arena *arena, size_t capacity);

/*
  Allocates a region of memory inside the arena.

  size specifies how many bytes the caller requests.

  The returned pointer refers to memory inside
  the arena's existing block.

  Returns NULL if the request is invalid or if
  there is not enough available space.

  The returned address is aligned for ordinary C
  types up to the alignment of max_align_t.
*/
void *arena_alloc(Arena *arena, size_t size);

/*
  Resets the arena's used counter to zero.

  The allocated block itself is NOT freed.

  This allows the same memory to be reused.

  However, every previous allocation is logically
  invalid after reset. Old pointers must not be
  used to access their former objects.
*/
void arena_reset(Arena *arena);

/*
  Releases the arena's backing memory using free().

  After destruction, the pointer is set to NULL
  and the size fields are reset to zero.

  This prevents the structure from retaining its
  old address and size information.
*/
void arena_destroy(Arena *arena);

#endif
//This marks the end of the header's include guard.
//The declarations above can now be included safely
//by source files that need to use the Arena API.
