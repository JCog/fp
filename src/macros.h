#ifndef _MACROS_H_
#define _MACROS_H_
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

#endif //_MACROS_H_
