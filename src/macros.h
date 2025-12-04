#ifndef MACROS_H
#define MACROS_H
#include "pm64.h"
#include "types.h"

#define TRUE  1
#define FALSE 0

#ifdef NDEBUG
#define PRINTF(...) ((void)0)
#else
#define PRINTF(...) (osSyncPrintf(__VA_ARGS__))
#endif

#define ARRAY_LENGTH(arr) (s32)(sizeof(arr) / sizeof(arr[0]))

#define SQ(x)             ((x) * (x))

#define STRINGIFY(S)      STRINGIFY_H(S)
#define STRINGIFY_H(S)    #S

#endif // MACROS_H
