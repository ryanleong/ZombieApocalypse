#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef NDEBUG
#define debug_printf(...)       /** Nothing. */
#else
#define debug_printf(...)       fprintf (stderr, "DEBUG: " __VA_ARGS__);
#endif

#endif
