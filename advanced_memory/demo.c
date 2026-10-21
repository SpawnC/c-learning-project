//demo.c
#include "arena.h"
//Includes our arena interface so we can use
//Arena and its four public functions.

#include <stdio.h>
//Required for printf(), snprintf() and fputs().

/*
  Define our own structure representing a point
  in a two-dimensional coordinate system.
*/
typedef struct {
    int x;
    /*
      The horizontal coordinate.

      int is an integer type, so it stores whole
      numbers such as 12, -4 or 0.
    */

    int y;
    /*
      The vertical coordinate.

      Together, x and y describe the point's position.
    */
} Point;
/*
  typedef lets us use Point as a type name.

  For example:
      Point p;

  creates a variable containing two integer fields.
*/

int main(void)
{
    /*
      Create an Arena structure.

      {0} initializes all structure members to zero.

      Therefore:
          buffer = NULL
          capacity = 0
          used = 0

      This gives us a known initial state before
      calling arena_init().

      void inside main(void) means that main takes
      no arguments in this declaration.
    */
    Arena arena = {0};

    /*
      Request a backing block of 1024 bytes.

      arena_init() returns 1 if successful and 0
      if initialization fails.

      The ! operator means logical NOT.

      Therefore:
          !1 is false
          !0 is true

      The if body runs when initialization fails.
    */
    if (!arena_init(&arena, 1024)) {

        /*
          &arena means the address of the arena
          variable.

          arena_init() needs this address so it can
          modify the original structure's members.

          fputs() writes the error message to stderr,
          the standard error output stream.
        */
        fputs("Could not create arena\n", stderr);

        return 1;
        //A nonzero return value conventionally signals
        //that the program ended because of an error.
    }

    /*
      Allocate enough space for one Point object.

      sizeof is an operator that returns the size
      of a type or object in bytes.

      sizeof *point means the size of the object
      that point will point to, namely Point.

      This is safer than manually writing a number
      because the compiler calculates the size.

      arena_alloc() returns void *, a generic object
      pointer. C allows it to be assigned to Point *.

      Point * means that point is a pointer to Point,
      not a Point object stored directly in this
      local variable.
    */
    Point *point = arena_alloc(&arena, sizeof *point);

    /*
      Allocate another 64 bytes for a text label.

      char is the type used for individual characters.

      A string in C is a sequence of characters
      ending with the special null character '\0'.

      64 bytes gives snprintf() room to store the
      label, provided the final text fits.
    */
    char *label = arena_alloc(&arena, 64);

    /*
      Either allocation may fail if the arena has
      insufficient space or if the request is invalid.

      We must check both pointers before dereferencing
      them or writing into the allocated regions.

      The || operator means that failure of either
      allocation makes the entire condition true.
    */
    if (point == NULL || label == NULL) {

        fputs("Arena is out of space\n", stderr);

        /*
          Release the backing block before returning.

          This is important because initialization
          already succeeded, so the arena owns memory.
        */
        arena_destroy(&arena);

        return 1;
        //Stop because the demonstration cannot continue.
    }

    /*
      Access the Point object through its pointer.

      The -> operator accesses a structure field
      through a pointer.

      These assignments store:
          x = 12
          y = -4

      The memory for these fields came from the arena,
      not from separate malloc() calls.
    */
    point->x = 12;
    point->y = -4;

    /*
      snprintf() formats text and writes it into
      the destination buffer.

      label is the destination address.

      64 is the maximum number of bytes it may write,
      including the final '\0' character.

      The format string contains %d placeholders.
      Each %d is replaced with the corresponding int.

      For x = 12 and y = -4, the result is:
          Point(12, -4)

      Unlike an unchecked sprintf(), snprintf()
      limits how much it writes to the buffer.

      If the text is too long, snprintf() truncates
      the result while ensuring termination when the
      buffer size is greater than zero.
    */
    snprintf(label, 64, "Point(%d, %d)", point->x, point->y);

    /*
      printf() writes formatted output to stdout.

      %s means that the corresponding argument is
      a string.

      label points to the text created above.

      \n is an escape sequence representing a newline,
      so the next output begins on another line.
    */
    printf("%s\n", label);

    /*
      %zu is the correct printf format specifier
      for size_t.

      arena.used tells us how many bytes have been
      consumed, including any alignment padding.

      arena.capacity tells us the total arena size.

      The exact used count depends on the platform's
      alignment and the size of Point.
    */
    printf("Used %zu of %zu bytes\n", arena.used, arena.capacity);

    /*
      Reset the arena.

      arena_reset() sets used to zero, allowing future
      allocations to reuse the backing block.

      It does not call free() and does not erase
      the previous bytes.

      However, point and label must no longer be used
      to access their former objects after reset.

      Their memory may be reused by later allocations.
    */
    arena_reset(&arena);

    /*
      Print the new used count.

      Since reset set used to zero, this prints:
          After reset: 0 bytes used
    */
    printf("After reset: %zu bytes used\n", arena.used);

    /*
      Destroy the arena.

      This calls free() on the backing block and
      clears the Arena structure.

      All pointers into the released block are now
      invalid and must not be dereferenced.
    */
    arena_destroy(&arena);

    return 0;
    /*
      Returning zero conventionally indicates
      successful program termination.
    */
}
//This marks the end of main().
//
//The opening { begins the function body, and the
//closing } ends it.
//
//The program initializes an arena, allocates memory,
//stores and prints a Point label, resets the arena,
//and finally releases the backing allocation.
