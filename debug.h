#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG
#define dprintf(...) do {fprintf( stderr, __VA_ARGS__ );} while(false)
#else
#define dprintf(...) do {} while (false)
#endif

#endif
