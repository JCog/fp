#ifndef PI_H
#define PI_H
#include "common.h"
#include <n64/pi.h>

void pi_write_locked(u32 dev_addr, const void *src, size_t size);
void pi_read_locked(u32 dev_addr, void *dst, size_t size);
void pi_write(u32 dev_addr, const void *src, size_t size);
void pi_read(u32 dev_addr, void *dst, size_t size);

static inline void __pi_wait(void) {
    while (pi_regs.status & (PI_STATUS_DMA_BUSY | PI_STATUS_IO_BUSY)) {}
}

static inline u32 __pi_read_raw(u32 dev_addr) {
    __pi_wait();
    return *(volatile u32 *)dev_addr;
}

static inline void __pi_write_raw(u32 dev_addr, u32 value) {
    __pi_wait();
    *(volatile u32 *)dev_addr = value;
}

#endif
