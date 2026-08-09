#include "assert.h"
#include "pm64.h"
#include <stdio.h>

char assertMsg[ASSERT_BUFFER_SIZE] = {0};

void __attribute__((noreturn)) panic(const char *msg, const char *file, s32 line) {
    osSyncPrintf("file:%s line:%ld  %s\n", file, line, msg);
    snprintf(assertMsg, sizeof(assertMsg), "file:%s line:%ld\n%s", file, line, msg);
    asm volatile("break");
    __builtin_unreachable();
}
