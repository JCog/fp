#ifndef IODEV_H
#define IODEV_H
#include "common.h"

struct Iodev {
    s32 (*probe)(void);

    s32 (*diskInit)(void);
    s32 (*diskRead)(size_t lba, size_t nBlocks, void *dst);
    s32 (*diskWrite)(size_t lba, size_t nBlocks, const void *src);

    s32 (*fifoPoll)(void);
    s32 (*fifoRead)(void *dst, size_t nBlocks);
    s32 (*fifoWrite)(const void *src, size_t nBlocks);

    u32 (*clockTicks)(void);
    u32 (*clockFreq)(void);
    void (*cpuReset)(void);
};

#endif
