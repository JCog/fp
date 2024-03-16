#ifndef UTIL_H
#define UTIL_H
#include "common.h"
#include <mips.h>
#include <n64.h>
#include <startup.h>

/* set irq bit and return previous value */
static inline s32 setIrqf(s32 irqf) {
    u32 sr;

    __asm__("mfc0    %[sr], $12;" : [sr] "=r"(sr));
    s32 oldIrqf = sr & MIPS_STATUS_IE;

    sr = (sr & ~MIPS_STATUS_IE) | (irqf & MIPS_STATUS_IE);
    __asm__("mtc0    %[sr], $12;" ::[sr] "r"(sr));

    return oldIrqf;
}

static inline s32 getIrqf(void) {
    u32 sr;

    __asm__("mfc0    %[sr], $12;" : [sr] "=r"(sr));

    return sr & MIPS_STATUS_IE;
}

static inline void dcacheInv(const void *ptr, size_t len) {
    uintptr_t p = (uintptr_t)ptr & ~0xF;
    uintptr_t e = (uintptr_t)ptr + len;
    while (p < e) {
        __asm__("cache   0x11, 0x0000(%[p]);" ::[p] "r"(p));
        p += 0x10;
    }
}

static inline void dcacheWbinv(const void *ptr, size_t len) {
    uintptr_t p = (uintptr_t)ptr & ~0xF;
    uintptr_t e = (uintptr_t)ptr + len;
    while (p < e) {
        __asm__("cache   0x15, 0x0000(%[p]);" ::[p] "r"(p));
        p += 0x10;
    }
}

static inline void dcacheWb(const void *ptr, size_t len) {
    uintptr_t p = (uintptr_t)ptr & ~0xF;
    uintptr_t e = (uintptr_t)ptr + len;
    while (p < e) {
        __asm__("cache   0x19, 0x0000(%[p]);" ::[p] "r"(p));
        p += 0x10;
    }
}

/* safe (non-signaling) nan check */
static inline bool isNan(f32 f) {
    u32 expMask = 0b01111111100000000000000000000000;
    u32 sigMask = 0b00000000011111111111111111111111;
    union {
        u32 w;
        f32 f;
    } pun;
    pun.f = f;
    return (pun.w & expMask) == expMask && (pun.w & sigMask) != 0;
}

static inline void maybeInitGp(void) {
#ifndef NO_GP
    init_gp();
#endif
}

#endif
