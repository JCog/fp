#include "crash_screen.h"

typedef struct {
    /* 0x000 */ OSThread thread;
    /* 0x1B0 */ char stack[0x800];
    /* 0x9B0 */ OSMesgQueue queue;
    /* 0x9C8 */ OSMesg mesg;
    /* 0x9CC */ u16 *frameBuf;
    /* 0x9D0 */ u16 width;
    /* 0x9D2 */ u16 height;
} CrashScreen; // size = 0x9D4

static CrashScreen gCrashScreen;

static const char *gFaultCauses[18] = {
    "Interrupt",
    "TLB modification",
    "TLB exception on load",
    "TLB exception on store",
    "Address error on load",
    "Address error on store",
    "Bus error on inst.",
    "Bus error on data",
    "System call exception",
    "Breakpoint exception",
    "Reserved instruction",
    "Coprocessor unusable",
    "Arithmetic overflow",
    "Trap exception",
    "Virtual coherency on inst.",
    "Floating point exception",
    "Watchpoint exception",
    "Virtual coherency on data",
};

static const char *gFPCSRFaultCauses[6] = {
    "Unimplemented operation", "Invalid operation", "Division by zero", "Overflow", "Underflow", "Inexact operation",
};

static void crashScreenSleep(s32 ms) {
    u64 cycles = ms * 1000LL * 46875000LL / 1000000ULL;

    osSetTime(0);

    while (osGetTime() < cycles) {
        // wait
    }
}

static void crashScreenDrawRect(s32 x, s32 y, s32 width, s32 height) {
    u16 *ptr;
    s32 i;
    s32 j;

    if (gCrashScreen.width == 640) {
        x <<= 1;
        y <<= 1;
        width <<= 1;
        height <<= 1;
    }

    ptr = gCrashScreen.frameBuf + gCrashScreen.width * y + x;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            *ptr = ((*ptr & 0xE738) >> 2) | 1;
            ptr++;
        }

        ptr += gCrashScreen.width - width;
    }
}

static void crashScreenDrawGlyph(s32 x, s32 y, char c, u16 fg, u16 bg) {
    u16 width = gCrashScreen.width;
    const u8 *rows;
    s32 i;
    s32 j;

    if (c < CRASH_FONT_GLYPH_MIN || c > CRASH_FONT_GLYPH_MAX) {
        c = '?';
    }
    rows = gCrashScreenFont[c - CRASH_FONT_GLYPH_MIN];

    if (width == 320) {
        u16 *ptr = gCrashScreen.frameBuf + width * y + x;

        for (i = 0; i < CRASH_FONT_HEIGHT; i++) {
            u8 bit = 0x10;
            u8 rowMask = *rows++;

            for (j = 0; j < CRASH_FONT_WIDTH; j++) {
                *ptr++ = (bit & rowMask) ? fg : bg;
                bit >>= 1;
            }

            ptr += width - CRASH_FONT_WIDTH;
        }
    } else if (width == 640) {
        u16 *ptr = gCrashScreen.frameBuf + width * (y * 2) + (x * 2);

        for (i = 0; i < CRASH_FONT_HEIGHT; i++) {
            u8 rowMask = *rows++;
            u8 bit = 0x10;

            for (j = 0; j < CRASH_FONT_WIDTH; j++) {
                u16 temp = (bit & rowMask) ? fg : bg;

                ptr[0] = temp;
                ptr[1] = temp;
                ptr[width] = temp;
                ptr[width + 1] = temp;
                ptr += 2;
                bit >>= 1;
            }

            ptr += (width - CRASH_FONT_WIDTH) * 2;
        }
    }
}

static void *crashScreenCopyToBuf(void *dest, const char *src, u32 size) {
    memcpy(dest, src, size);
    return dest + size;
}

static void crashScreenPrintf(s32 x, s32 y, const char *fmt, ...) {
    u8 *ptr;
    s32 size;
    u8 buf[0x100];
    va_list args;

    va_start(args, fmt);

    size = _Printf(crashScreenCopyToBuf, buf, fmt, args);

    if (size > 0) {
        ptr = buf;

        while (size > 0) {
            crashScreenDrawGlyph(x, y, (char)*ptr, 0xFFFF, 1);

            x += 6;
            size--;
            ptr++;
        }
    }

    va_end(args);
}

static void crashScreenPrintFpr(s32 x, s32 y, s32 regNum, void *addr) {
    u32 bits = *(u32 *)addr;
    s32 exponent = ((bits & 0x7F800000U) >> 0x17) - 0x7F;

    if ((exponent >= -0x7E && exponent <= 0x7F) || bits == 0) {
        crashScreenPrintf(x, y, "F%02d:%+.3e", regNum, *(f32 *)addr);
    } else {
        crashScreenPrintf(x, y, "F%02d:---------", regNum);
    }
}

static void crashScreenPrintFpcsr(u32 value) {
    s32 i;
    u32 flag = 0x20000;

    crashScreenPrintf(30, 155, "FPCSR:%08XH", value);

    for (i = 0; i < 6;) {
        if (value & flag) {
            crashScreenPrintf(132, 155, "(%s)", gFPCSRFaultCauses[i]);
            break;
        }

        i++;
        flag >>= 1;
    }
}

static void crashScreenDraw(OSThread *faultedThread) {
    s16 causeIndex;
    __OSThreadContext *ctx;

    ctx = &faultedThread->context;
    causeIndex = ((faultedThread->context.cause >> 2) & 0x1F);

    if (causeIndex == 23) {
        causeIndex = 16;
    }

    if (causeIndex == 31) {
        causeIndex = 17;
    }

    osWritebackDCacheAll();

    crashScreenDrawRect(25, 20, 270, 25);
    crashScreenPrintf(30, 25, "THREAD:%d  (%s)", faultedThread->id, gFaultCauses[causeIndex]);
    crashScreenPrintf(30, 35, "PC:%08XH   SR:%08XH   VA:%08XH", ctx->pc, ctx->sr, ctx->badvaddr);

    crashScreenSleep(2000);

    osViBlack(0);
    osViRepeatLine(0);
    osViSwapBuffer(gCrashScreen.frameBuf);

    crashScreenDrawRect(25, 45, 270, 185);

    crashScreenPrintf(30, 50, "AT:%08XH   V0:%08XH   V1:%08XH", (u32)ctx->at, (u32)ctx->v0, (u32)ctx->v1);
    crashScreenPrintf(30, 60, "A0:%08XH   A1:%08XH   A2:%08XH", (u32)ctx->a0, (u32)ctx->a1, (u32)ctx->a2);
    crashScreenPrintf(30, 70, "A3:%08XH   T0:%08XH   T1:%08XH", (u32)ctx->a3, (u32)ctx->t0, (u32)ctx->t1);
    crashScreenPrintf(30, 80, "T2:%08XH   T3:%08XH   T4:%08XH", (u32)ctx->t2, (u32)ctx->t3, (u32)ctx->t4);
    crashScreenPrintf(30, 90, "T5:%08XH   T6:%08XH   T7:%08XH", (u32)ctx->t5, (u32)ctx->t6, (u32)ctx->t7);
    crashScreenPrintf(30, 100, "S0:%08XH   S1:%08XH   S2:%08XH", (u32)ctx->s0, (u32)ctx->s1, (u32)ctx->s2);
    crashScreenPrintf(30, 110, "S3:%08XH   S4:%08XH   S5:%08XH", (u32)ctx->s3, (u32)ctx->s4, (u32)ctx->s5);
    crashScreenPrintf(30, 120, "S6:%08XH   S7:%08XH   T8:%08XH", (u32)ctx->s6, (u32)ctx->s7, (u32)ctx->t8);
    crashScreenPrintf(30, 130, "T9:%08XH   GP:%08XH   SP:%08XH", (u32)ctx->t9, (u32)ctx->gp, (u32)ctx->sp);
    crashScreenPrintf(30, 140, "S8:%08XH   RA:%08XH", (u32)ctx->s8, (u32)ctx->ra);

    crashScreenPrintFpcsr(ctx->fpcsr);

    crashScreenPrintFpr(30, 170, 0, &ctx->fp32[0]);
    crashScreenPrintFpr(120, 170, 2, &ctx->fp32[2]);
    crashScreenPrintFpr(210, 170, 4, &ctx->fp32[4]);
    crashScreenPrintFpr(30, 180, 6, &ctx->fp32[6]);
    crashScreenPrintFpr(120, 180, 8, &ctx->fp32[8]);
    crashScreenPrintFpr(210, 180, 10, &ctx->fp32[10]);
    crashScreenPrintFpr(30, 190, 12, &ctx->fp32[12]);
    crashScreenPrintFpr(120, 190, 14, &ctx->fp32[14]);
    crashScreenPrintFpr(210, 190, 16, &ctx->fp32[16]);
    crashScreenPrintFpr(30, 200, 18, &ctx->fp32[18]);
    crashScreenPrintFpr(120, 200, 20, &ctx->fp32[20]);
    crashScreenPrintFpr(210, 200, 22, &ctx->fp32[22]);
    crashScreenPrintFpr(30, 210, 24, &ctx->fp32[24]);
    crashScreenPrintFpr(120, 210, 26, &ctx->fp32[26]);
    crashScreenPrintFpr(210, 210, 28, &ctx->fp32[28]);
    crashScreenPrintFpr(30, 220, 30, &ctx->fp32[30]);

    crashScreenSleep(500);

    crashScreenPrintf(210, 140, "MM:%08XH", *(u32 *)ctx->pc);
}

static OSThread *crashScreenGetFaultedThread(void) {
    OSThread *thread = __osGetActiveQueue();

    while (thread->priority != -1) {
        if (thread->priority > 0 && thread->priority < 0x7F && (thread->flags & 3)) {
            PRINTF("faulted thread detected\n");
            return thread;
        }

        thread = thread->tlnext;
    }

    return NULL;
}

static void crashScreenThreadEntry(void *unused) {
    OSMesg mesg;
    OSThread *faultedThread;

    osSetEventMesg(OS_EVENT_CPU_BREAK, &gCrashScreen.queue, (OSMesg)1);
    osSetEventMesg(OS_EVENT_FAULT, &gCrashScreen.queue, (OSMesg)2);

    do {
        osRecvMesg(&gCrashScreen.queue, &mesg, 1);
        faultedThread = crashScreenGetFaultedThread();

    } while (faultedThread == NULL);

    osStopThread(faultedThread);
    PRINTF("drawing crash screen\n");
    crashScreenDraw(faultedThread);

    while (1) {}
}

void crashScreenSetDrawInfoCustom(u16 *frameBufPtr, s16 width, s16 height) {
    gCrashScreen.frameBuf = (u16 *)((u32)frameBufPtr | 0xA0000000);
    gCrashScreen.width = width;
    gCrashScreen.height = height;
}

void crashScreenInit(void) {
    gCrashScreen.width = SCREEN_WIDTH;
    gCrashScreen.height = 16;
    gCrashScreen.frameBuf = (u16 *)((osMemSize | 0xA0000000) - ((SCREEN_WIDTH * SCREEN_HEIGHT) * 2));
    osCreateMesgQueue(&gCrashScreen.queue, &gCrashScreen.mesg, 1);
    osCreateThread(&gCrashScreen.thread, 2, crashScreenThreadEntry, NULL,
                   gCrashScreen.stack + sizeof(gCrashScreen.stack), 0x80);
    osStartThread(&gCrashScreen.thread);
    PRINTF("crash thread started %8X\n", crashScreenThreadEntry);
}
