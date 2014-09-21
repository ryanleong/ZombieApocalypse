#include <assert.h>
#include <stdlib.h>

#include "utils.h"

/**
 *  Wrapper to malloc that will abort the program if malloc returns null.
 *  We will use assert() to test the return value, so this function will
 *  only catch nulls if NDEBUG is *not* #define'd. See the man page for
 *  assert() for more details on the reasoning behind this design.
 */
void * checked_malloc(size_t bytes) {
	void *mem = malloc(bytes);
	assert(mem != NULL);
	return mem;
}

/** vim: set ts=4 sw=4 et : */
