#ifndef PI_H
#define PI_H
#include "common.h"
#include <n64/pi.h>

void piWriteLocked(u32 devAddr, const void *src, size_t size);
void piReadLocked(u32 devAddr, void *dst, size_t size);
void piWrite(u32 devAddr, const void *src, size_t size);
void piRead(u32 devAddr, void *dst, size_t size);

static inline void __piWait(void) { // NOLINT
    while (pi_regs.status & (PI_STATUS_DMA_BUSY | PI_STATUS_IO_BUSY)) {}
}

static inline u32 __piReadRaw(u32 devAddr) { // NOLINT
    __piWait();
    return *(volatile u32 *)devAddr;
}

static inline void __piWriteRaw(u32 devAddr, u32 value) { // NOLINT
    __piWait();
    *(volatile u32 *)devAddr = value;
}

#endif
