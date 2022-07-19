#ifndef IO_H
#define IO_H
#include "common.h"

s32 io_init(void);

s32 disk_init(void);
s32 disk_read(size_t lba, size_t n_blocks, void *dst);
s32 disk_write(size_t lba, size_t n_blocks, const void *dst);

s32 fifo_poll(void);
s32 fifo_read(void *dst, size_t n_blocks);
s32 fifo_write(const void *src, size_t n_blocks);

unsigned clock_ticks(void);
unsigned clock_freq(void);

void cpu_reset(void);

static inline unsigned msec_from_now(unsigned msec) {
    return clock_ticks() + clock_freq() / 1000 * msec;
}

static inline s32 clock_after(unsigned ticks) {
    return (s32)clock_ticks() - (s32)ticks > 0;
}

#endif
