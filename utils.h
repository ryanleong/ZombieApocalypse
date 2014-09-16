#ifndef _UTILS_H_
#define _UTILS_H_

/** wrapper to malloc that aborts if malloc returns null. */
void * checked_malloc(size_t bytes);

#endif

/** vim: set ts=4 sw=4 et : */
