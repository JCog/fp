#ifndef ASSERT_H
#define ASSERT_H

#include "types.h"

#define ASSERT_BUFFER_SIZE 200

extern char assertMsg[ASSERT_BUFFER_SIZE];
void __attribute__((noreturn)) panic(const char *msg, const char *file, s32 line);

#ifdef NDEBUG
#define IS_DEBUG_PANIC(msg, file, line) \
    do {                                \
    } while (0)
#else
#define IS_DEBUG_PANIC(msg, file, line) panic(msg, file, line)
#endif

#define PANIC() IS_DEBUG_PANIC("panic!", __FILE__, __LINE__)
#define ASSERT(condition)                                                        \
    do {                                                                         \
        if (!(condition)) {                                                      \
            IS_DEBUG_PANIC("assertion failed: " #condition, __FILE__, __LINE__); \
        }                                                                        \
    } while (0)

#endif
