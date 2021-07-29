//#include <unistd.h>
#define COND 1

#if COND == 1, COND
#define MACRO 42
#else
#define MACRO 24
#endif

MACRO
