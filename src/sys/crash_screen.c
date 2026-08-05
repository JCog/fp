#include "crash_screen.h"
#include "crash_screen_font.h"
#include "disasm.h"
#include "page.h"
#include "pm64.h"
#include "sys/input.h"
#include "util/util.h"

typedef struct {
    bool valid;
    u16 buttons;
    s8 stickX;
    s8 stickY;
    u8 err;
} CrashScreenPad;

typedef struct {
    char stack[0x800];
    OSThread thread;
    OSMesgQueue queue;
    OSMesg mesg;
    u16 *frameBuf;
    u16 width;
    u16 height;
} CrashScreen;

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
    u32 start = getCP0Count();
    u32 cycles = ms * (OS_CPU_COUNTER / 1000U);

    while (getCP0Count() - start < cycles) {}
}

static CrashScreenPad crashScreenReadPad(void) {
    static OSMesgQueue siQueue;
    static OSMesg siMesg;
    static bool siQueueReady = FALSE;
    __OSEventState siEvent;
    CrashScreenPad pad = {0};
    OSContPad contPad[4];

    if (!siQueueReady) {
        osCreateMesgQueue(&siQueue, &siMesg, 1);
        siQueueReady = TRUE;
    }

    siEvent = __osEventStateTab[OS_EVENT_SI];
    __osEventStateTab[OS_EVENT_SI].messageQueue = &siQueue;
    __osEventStateTab[OS_EVENT_SI].message = NULL;

    osContStartReadData(&siQueue);
    osRecvMesg(&siQueue, NULL, OS_MESG_BLOCK);
    osContGetReadData(contPad);

    __osEventStateTab[OS_EVENT_SI] = siEvent;

    pad.err = contPad[0].errno;
    pad.valid = (contPad[0].errno == 0);
    pad.buttons = contPad[0].button;
    pad.stickX = contPad[0].stick_x;
    pad.stickY = contPad[0].stick_y;

    return pad;
}

static void crashScreenFillRect(s32 x, s32 y, s32 width, s32 height, u16 color) {
    if (gCrashScreen.width == 640) {
        x <<= 1;
        y <<= 1;
        width <<= 1;
        height <<= 1;
    }

    u16 *ptr = gCrashScreen.frameBuf + gCrashScreen.width * y + x;

    for (s32 i = 0; i < height; i++) {
        for (s32 j = 0; j < width; j++) {
            *ptr++ = color;
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

static void crashScreenPrintCause(__OSThreadContext *ctx, s32 x, s32 y) {
    s16 causeIndex = ((ctx->cause >> 2) & 0x1F);
    u32 badVAddr = ctx->badvaddr;

    switch (causeIndex) {
        case 1: crashScreenPrintf(x, y, "write to bad pointer: 0x%08x", badVAddr); break;
        case 2:
            if (badVAddr < 0x10000) { // probably null pointer + some struct offset
                crashScreenPrintf(x, y, "load from null pointer: 0x%08x", badVAddr);
            } else {
                crashScreenPrintf(x, y, "load from bad pointer: 0x%08x", badVAddr);
            }
            break;
        case 3:
            if (badVAddr < 0x10000) {
                crashScreenPrintf(x, y, "write to null pointer: 0x%08x", badVAddr);
            } else {
                crashScreenPrintf(x, y, "write to bad pointer: 0x%08x", badVAddr);
            }
            break;
        case 4: crashScreenPrintf(x, y, "unaligned load: 0x%08x", badVAddr); break;
        case 5: crashScreenPrintf(x, y, "unaligned store: 0x%08x", badVAddr); break;
        case 6:
        case 7: crashScreenPrintf(x, y, "address outside ram: 0x%08x", badVAddr); break;
        case 10: crashScreenPrintf(x, y, "invalid instruction"); break;
        case 12: crashScreenPrintf(x, y, "integer overflow"); break;
        case 15: {
            break;
        }
        default: crashScreenPrintf(x, y, gFaultCauses[causeIndex]); break;
    }
}

static void crashScreenDrawLine(u32 y) {
    for (u32 i = TEXT_L; i < TEXT_R; i++) {
        crashScreenPrintf(i, y, "-");
    }
}

static void crashScreenDrawHeader(__OSThreadContext *ctx, OSId thread) {
    crashScreenPrintCause(ctx, COL0, HDR_Y0);
    crashScreenPrintf(COL0, HDR_Y1, "TH: %d PC: %08X SP: %08X RA: %08X", thread, ctx->pc, (u32)ctx->sp, (u32)ctx->ra);
    crashScreenDrawLine(RULE_HDR);
}

typedef enum {
    CRASH_PAGE_SUMMARY,
    CRASH_PAGE_DETAIL,
    CRASH_PAGE_FPU,
    CRASH_PAGE_BACKTRACE,
    CRASH_PAGE_MAX
} CrashPage;

static void crashScreenDrawFooter(u8 page) {
    crashScreenDrawLine(RULE_FOOT);
    crashScreenPrintf(COL0, FOOT_Y, "L/R %d/%d", page + 1, CRASH_PAGE_MAX);
    crashScreenPrintf(COL2, FOOT_Y, "Z HIDE");
}

static void crashScreenDrawSummary(OSThread *faultedThread) {
    __OSThreadContext *ctx = &faultedThread->context;

    if (!VALID_ADDR(ctx->pc)) {
        return;
    }

    crashScreenPrintf(COL0, ROW(0), "DISASSEMBLY");

    u32 addr = ctx->pc - (4 * 4);
    s32 row = 1;

    for (s32 i = 0; i < 8; i++, row++) {
        const char *inst = disasmInstruction(*(u32 *)addr, addr);
        if (addr == ctx->pc) {
            crashScreenPrintf(COL0, ROW(row), "-> %08x: %s", addr, inst);
        } else {
            crashScreenPrintf(COL0 + 18, ROW(row), "%08x: %s", addr, inst);
        }

        addr += 4;
    }

    row += 2;

    crashScreenPrintf(COL0, ROW(row), "CALL STACK");
    for (s32 i = 0; i < 4; i++, row++) {}
}

static void crashScreenDrawDetail(OSThread *faultedThread) {
    __OSThreadContext *ctx = &faultedThread->context;

    s16 causeIndex = ((faultedThread->context.cause >> 2) & 0x1F);

    if (causeIndex == 23) {
        causeIndex = 16;
    }

    if (causeIndex == 31) {
        causeIndex = 17;
    }

    crashScreenPrintf(COL0, ROW(0), "AT:%08XH   V0:%08XH   V1:%08XH", ctx->at, ctx->v0, ctx->v1);
    crashScreenPrintf(COL0, ROW(1), "A0:%08XH   A1:%08XH   A2:%08XH", ctx->a0, ctx->a1, ctx->a2);
    crashScreenPrintf(COL0, ROW(2), "A3:%08XH   T0:%08XH   T1:%08XH", ctx->a3, ctx->t0, ctx->t1);
    crashScreenPrintf(COL0, ROW(3), "T2:%08XH   T3:%08XH   T4:%08XH", ctx->t2, ctx->t3, ctx->t4);
    crashScreenPrintf(COL0, ROW(4), "T5:%08XH   T6:%08XH   T7:%08XH", ctx->t5, ctx->t6, ctx->t7);
    crashScreenPrintf(COL0, ROW(5), "S0:%08XH   S1:%08XH   S2:%08XH", ctx->s0, ctx->s1, ctx->s2);
    crashScreenPrintf(COL0, ROW(6), "S3:%08XH   S4:%08XH   S5:%08XH", ctx->s3, ctx->s4, ctx->s5);
    crashScreenPrintf(COL0, ROW(7), "S6:%08XH   S7:%08XH   T8:%08XH", ctx->s6, ctx->s7, ctx->t8);
    crashScreenPrintf(COL0, ROW(8), "T9:%08XH   GP:%08XH   S8:%08XH", ctx->t9, ctx->gp, ctx->s8);
    crashScreenPrintf(COL0, ROW(10), "SR:%08XH   CAUSE:%08XH", ctx->sr, ctx->cause);
    crashScreenPrintf(COL0, ROW(11), "HI:%08XH   LO:%08XH", ctx->hi, ctx->lo);
}

static void crashScreenPrintFpcsr(u32 value) {
    crashScreenPrintf(COL0, ROW(0), "FPCSR:%08XH", value);

    u32 flag = 0x20000;
    for (s32 i = 0; i < 6; i++) {
        if (value & flag) {
            crashScreenPrintf(COL1, ROW(0), "(%s)", gFPCSRFaultCauses[i]);
            break;
        }

        flag >>= 1;
    }
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

static void crashScreenDrawFpu(OSThread *faultedThread) {
    __OSThreadContext *ctx = &faultedThread->context;

    crashScreenPrintFpcsr(ctx->fpcsr);

    crashScreenPrintFpr(COL0, ROW(2), 0, &ctx->fp32[0]);
    crashScreenPrintFpr(COL1, ROW(2), 2, &ctx->fp32[2]);
    crashScreenPrintFpr(COL2, ROW(2), 4, &ctx->fp32[4]);
    crashScreenPrintFpr(COL0, ROW(3), 6, &ctx->fp32[6]);
    crashScreenPrintFpr(COL1, ROW(3), 8, &ctx->fp32[8]);
    crashScreenPrintFpr(COL2, ROW(3), 10, &ctx->fp32[10]);
    crashScreenPrintFpr(COL0, ROW(4), 12, &ctx->fp32[12]);
    crashScreenPrintFpr(COL1, ROW(4), 14, &ctx->fp32[14]);
    crashScreenPrintFpr(COL2, ROW(4), 16, &ctx->fp32[16]);
    crashScreenPrintFpr(COL0, ROW(5), 18, &ctx->fp32[18]);
    crashScreenPrintFpr(COL1, ROW(5), 20, &ctx->fp32[20]);
    crashScreenPrintFpr(COL2, ROW(5), 22, &ctx->fp32[22]);
    crashScreenPrintFpr(COL0, ROW(6), 24, &ctx->fp32[24]);
    crashScreenPrintFpr(COL1, ROW(6), 26, &ctx->fp32[26]);
    crashScreenPrintFpr(COL2, ROW(6), 28, &ctx->fp32[28]);
    crashScreenPrintFpr(COL0, ROW(7), 30, &ctx->fp32[30]);
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

static void crashScreenDrawPage(OSThread *faultedThread, CrashPage page) {
    __OSThreadContext *ctx = &faultedThread->context;

    osWritebackDCacheAll();

    crashScreenFillRect(PAGE_X, PAGE_Y, PAGE_W, PAGE_H, 1);

    crashScreenDrawHeader(ctx, faultedThread->id);
    crashScreenDrawFooter(page);

    switch (page) {
        case CRASH_PAGE_SUMMARY: crashScreenDrawSummary(faultedThread); break;
        case CRASH_PAGE_DETAIL: crashScreenDrawDetail(faultedThread); break;
        case CRASH_PAGE_FPU: crashScreenDrawFpu(faultedThread); break;
        case CRASH_PAGE_MAX: break;
    }
}

static void crashScreenThreadEntry(void *unused) {
    OSMesg mesg;
    OSThread *faultedThread;

    init_gp();

    osSetEventMesg(OS_EVENT_CPU_BREAK, &gCrashScreen.queue, (OSMesg)1);
    osSetEventMesg(OS_EVENT_FAULT, &gCrashScreen.queue, (OSMesg)2);

    do {
        osRecvMesg(&gCrashScreen.queue, &mesg, 1);
        faultedThread = crashScreenGetFaultedThread();

    } while (faultedThread == NULL);

    osStopThread(faultedThread);
    PRINTF("drawing crash screen\n");

    void *fb = (void *)MIPS_KSEG0_TO_KSEG1(osViGetCurrentFramebuffer());

    osViBlack(0);
    osViRepeatLine(0);
    osViSwapBuffer(gCrashScreen.frameBuf);

    u8 page = CRASH_PAGE_SUMMARY;
    u16 lastButtons = 0;
    bool redraw = TRUE;
    bool drawingCrashScreen = TRUE;

    while (1) {
        CrashScreenPad pad;
        pad = crashScreenReadPad();
        u16 pressed = 0;

        if (pad.valid) {
            pressed = pad.buttons & ~lastButtons;
            lastButtons = pad.buttons;
        }

        if (pressed & BUTTON_R) {
            page = (page + 1) % CRASH_PAGE_MAX;
            redraw = TRUE;
        } else if (pressed & BUTTON_L) {
            page = (page + CRASH_PAGE_MAX - 1) % CRASH_PAGE_MAX;
            redraw = TRUE;
        } else if (pressed & BUTTON_Z) {
            if (drawingCrashScreen) {
                drawingCrashScreen = FALSE;
                osViSwapBuffer(fb);
                redraw = FALSE;
            } else {
                drawingCrashScreen = TRUE;
                osViSwapBuffer(gCrashScreen.frameBuf);
                redraw = TRUE;
            }
        }

        if (redraw) {
            crashScreenDrawPage(faultedThread, page);
            redraw = FALSE;
        }

        crashScreenSleep(33);
    }
}

void crashScreenSetDrawInfoCustom(u16 *frameBufPtr, s16 width, s16 height) {
    gCrashScreen.frameBuf = (u16 *)MIPS_KSEG0_TO_KSEG1(frameBufPtr);
    gCrashScreen.width = width;
    gCrashScreen.height = height;
}

void crashScreenInit(void) {
    gCrashScreen.width = SCREEN_WIDTH;
    gCrashScreen.height = 16;
    gCrashScreen.frameBuf = (u16 *)(MIPS_KSEG0_TO_KSEG1(osMemSize) - ((SCREEN_WIDTH * SCREEN_HEIGHT) * 2));
    osCreateMesgQueue(&gCrashScreen.queue, &gCrashScreen.mesg, 1);
    osCreateThread(&gCrashScreen.thread, 2, crashScreenThreadEntry, NULL,
                   gCrashScreen.stack + sizeof(gCrashScreen.stack), 0x80);
    osStartThread(&gCrashScreen.thread);
    PRINTF("crash thread started %8X\n", crashScreenThreadEntry);
}
