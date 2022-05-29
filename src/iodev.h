#ifndef IODEV_H
#define IODEV_H
#include "pm64.h"

struct iodev {
    s32 (*probe)(void);

    s32 (*disk_init)(void);
    s32 (*disk_read)(size_t lba, size_t n_blocks, void *dst);
    s32 (*disk_write)(size_t lba, size_t n_blocks, const void *src);

    s32 (*fifo_poll)(void);
    s32 (*fifo_read)(void *dst, size_t n_blocks);
    s32 (*fifo_write)(const void *src, size_t n_blocks);

    u32 (*clock_ticks)(void);
    u32 (*clock_freq)(void);
    void (*cpu_reset)(void);
};

#endif
