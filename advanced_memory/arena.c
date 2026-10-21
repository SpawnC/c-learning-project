//arena.c
#include "arena.h"
//Includes our own header file.
//Quotes are used for a project header.
//This provides the Arena structure and function
//declarations that we implement in this file.

#include <stdlib.h>
//Required for malloc() and free().
//malloc() requests memory from the system's
//allocation mechanism.
//free() releases previously allocated memory.

#include <stddef.h>
//Provides size_t, NULL and max_align_t.

/*
  This function creates the arena's backing block.

  It returns 1 on success and 0 on failure.
*/
int arena_init(Arena *arena, size_t capacity)
{
    /*
      arena is a pointer to an Arena structure.

      We check whether the pointer is NULL before
      accessing any of its fields.

      If arena is NULL, there is no structure to
      initialize, so accessing arena->buffer would
      be invalid.

      capacity == 0 is also rejected because this
      implementation does not support empty arenas.

      || means logical OR: if either condition is
      true, the entire condition is true.
    */
    if (arena == NULL || capacity == 0) {
        return 0;
        //Report failure to the caller.
    }

    /*
      malloc() requests capacity bytes of memory.

      It returns a pointer to the beginning of the
      allocated block, or NULL if allocation fails.

      We store that address in arena->buffer.

      The -> operator accesses a structure member
      through a pointer.

      For example:
          arena->buffer

      is equivalent to:
          (*arena).buffer
    */
    arena->buffer = malloc(capacity);

    /*
      malloc() can fail, so its result must be checked.

      If the returned pointer is NULL, no usable
      memory block was obtained.

      We reset the size fields to zero before
      returning failure.

      IMPORTANT:
      This initialization function expects a fresh
      or already-destroyed Arena. Calling it on an
      arena that already owns memory would overwrite
      its old pointer and could leak that allocation.
    */
    if (arena->buffer == NULL) {
        arena->capacity = 0;
        arena->used = 0;
        return 0;
    }

    /*
      Memory allocation succeeded.

      capacity records the total size of the block.

      used starts at zero because no arena allocations
      have been made yet.

      The first allocation will begin at the start
      of the block, subject to alignment requirements.
    */
    arena->capacity = capacity;
    arena->used = 0;

    return 1;
    //Report successful initialization.
}

/*
  Allocates memory from the arena.

  Unlike malloc(), this function does not request
  a new block from the system on every call.

  It moves a position inside the existing block.
*/
void *arena_alloc(Arena *arena, size_t size)
{
    /*
      Check every basic condition before doing
      arithmetic or accessing the memory.

      arena == NULL:
          There is no Arena structure to use.

      arena->buffer == NULL:
          The backing memory does not exist.

      size == 0:
          This implementation rejects zero-byte requests.

      arena->used > arena->capacity:
          The current offset is invalid because it
          lies beyond the declared capacity.

      Returning NULL tells the caller that the
      allocation could not be completed.
    */
    if (arena == NULL || arena->buffer == NULL || size == 0 ||
        arena->used > arena->capacity) {
        return NULL;
    }

    /*
      C types can have alignment requirements.

      For example, an object of a particular type
      may need to begin at an address divisible
      by a specific number.

      _Alignof(max_align_t) gives the alignment
      required by max_align_t, which is suitable
      for ordinary types with fundamental alignment.

      malloc() provides a suitably aligned base
      address. We must also align the offset of
      each allocation inside that block.

      This does not support every possible extended
      alignment requirement for over-aligned types.
    */
    const size_t alignment = _Alignof(max_align_t);

    /*
      % is the remainder operator.

      remainder tells us what remains after dividing
      the current offset by the alignment.

      If used is 16 and alignment is 16:
          remainder = 0

      If used is 19 and alignment is 16:
          remainder = 3

      In the second case, the current offset is
      three bytes past an aligned boundary.
    */
    const size_t remainder = arena->used % alignment;

    /*
      Calculate how many extra bytes are needed
      to reach the next aligned position.

      If remainder is zero, no padding is needed.

      Otherwise, padding moves the position forward
      to the next multiple of alignment.

      The second % alignment ensures that padding
      is zero when the current offset is already
      correctly aligned.
    */
    const size_t padding = (alignment - remainder) % alignment;

    /*
      Check whether the padding fits in the arena.

      arena->capacity - arena->used represents the
      number of bytes remaining before adding padding.

      We check padding against that remaining space
      instead of calculating used + padding first.

      This subtraction-based approach helps avoid
      integer overflow in size calculations.

      The earlier check guarantees that used is not
      greater than capacity, so this subtraction
      is safe.
    */
    if (padding > arena->capacity - arena->used) {
        return NULL;
        //There is not enough room even for alignment.
    }

    /*
      Calculate the aligned starting offset.

      For example:
          used = 19
          padding = 13
          start = 32

      With an alignment of 16, offset 32 is aligned.

      The + operator adds the padding to the current
      offset to calculate the allocation's start.
    */
    const size_t start = arena->used + padding;

    /*
      Check whether the requested object fits.

      capacity - start is the number of bytes left
      from the aligned starting position.

      If size is greater than that remaining amount,
      the requested allocation would exceed the block.

      We use subtraction instead of checking
      start + size > capacity because adding size
      first could overflow size_t.
    */
    if (size > arena->capacity - start) {
        return NULL;
        //The requested memory does not fit.
    }

    /*
      Calculate the returned address.

      arena->buffer points to the first byte.

      Adding start moves the pointer forward by
      start bytes because buffer is an unsigned char
      pointer.

      Pointer arithmetic on unsigned char pointers
      lets us work in individual bytes.

      The result points to the beginning of the
      allocated region inside the backing block.
    */
    void *result = arena->buffer + start;

    /*
      Advance the used counter to the byte immediately
      after the newly allocated region.

      start + size is safe here because the previous
      check established that size fits in capacity
      - start.

      Any padding before this allocation is included
      in the updated used offset.

      The next allocation will begin searching
      from this new offset.
    */
    arena->used = start + size;

    return result;
    //Give the caller the address of the new region.
}

/*
  Makes the entire arena available for reuse.

  This does not release the backing allocation.
*/
void arena_reset(Arena *arena)
{
    /*
      First check that the structure exists and
      that it owns a backing block.

      If either condition is false, there is nothing
      to reset.
    */
    if (arena != NULL && arena->buffer != NULL) {

        /*
          Setting used to zero makes the next
          allocation begin at the start of the block,
          with alignment applied as usual.

          The old bytes are not erased.

          The old objects' logical lifetimes have
          ended, so their previous pointers must
          not be used to access them.

          New allocations may overwrite the same bytes.
        */
        arena->used = 0;
    }
}

/*
  Releases the backing memory and clears the structure.
*/
void arena_destroy(Arena *arena)
{
    /*
      A NULL pointer cannot be used to access a
      structure, so return immediately if necessary.
    */
    if (arena == NULL) {
        return;
    }

    /*
      free() releases the block previously obtained
      from malloc().

      The Arena structure itself is not freed here.
      Only the memory referenced by arena->buffer
      is released.

      Calling free(NULL) is valid in C, so this
      operation is also safe if buffer is NULL.
    */
    free(arena->buffer);

    /*
      Clear the pointer after releasing its memory.

      Otherwise, it would retain a dangling pointer:
      an address referring to memory that is no
      longer allocated to this arena.
    */
    arena->buffer = NULL;

    /*
      Clear the size information too.

      The structure now represents an empty,
      destroyed arena rather than an active one.
    */
    arena->capacity = 0;
    arena->used = 0;
}
//This marks the end of arena.c.
//The functions above implement the public interface
//declared in arena.h.
