#ifndef DEBUG_H_
#define DEBUG_H_
/**
 * Definition of debug_printf macro which expands
 * either to no-op or printing to stderr.
 *
 * The no-op is realized this way to force semicolon
 * after debug_printf in both cases.
 */
#ifdef NDEBUG
#define debug_printf(...) \
	do {} while(0) /* Nothing. */
#else
#define debug_printf(...) \
	fprintf (stderr, "DEBUG: " __VA_ARGS__);
#endif

#endif
