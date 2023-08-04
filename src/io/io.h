#ifndef IO_H
#define IO_H
#include "common.h"

s32 ioInit(void);

s32 diskInit(void);
s32 diskRead(size_t lba, size_t nBlocks, void *dst);
s32 diskWrite(size_t lba, size_t nBlocks, const void *dst);

s32 fifoPoll(void);
s32 fifoRead(void *dst, size_t nBlocks);
s32 fifoWrite(const void *src, size_t nBlocks);

unsigned clockTicks(void);
unsigned clockFreq(void);

void cpuReset(void);

static inline unsigned msecFromNow(unsigned msec) {
    return clockTicks() + clockFreq() / 1000 * msec;
}

static inline s32 clockAfter(unsigned ticks) {
    return (s32)clockTicks() - (s32)ticks > 0;
}

#endif
