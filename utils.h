#ifndef _UTILS_H
#define _UTILS_H

/** wrapper to malloc that aborts if malloc returns null. */
void * checked_malloc (size_t bytes);

#endif

/** vim: set ts=4 sw=4 et : */
