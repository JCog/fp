#include "crash_screen.h"
#include "crash_screen_font.h"
#include "disasm.h"
#include "macros.h"
#include "page.h"
#include "pm64.h"
#include "sys/backtrace.h"
#include "sys/input.h"
#include "util/assert.h"
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
    Backtrace bt;
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
    u32 start = getCP0Count();
    while (osRecvMesg(&siQueue, NULL, OS_MESG_NOBLOCK) == -1) {
        if (getCP0Count() - start > OS_CPU_COUNTER / 100) {
            __osEventStateTab[OS_EVENT_SI] = siEvent;
            pad.valid = FALSE;
            return pad;
        }
    }
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
    u16 *ptr = gCrashScreen.frameBuf + SCREEN_WIDTH * y + x;

    for (s32 i = 0; i < height; i++) {
        for (s32 j = 0; j < width; j++) {
            *ptr++ = color;
        }

        ptr += SCREEN_WIDTH - width;
    }
}

static void crashScreenDrawGlyph(s32 x, s32 y, char c, u16 fg, u16 bg) {
    const u8 *rows;
    s32 i;
    s32 j;

    if (c < CRASH_FONT_GLYPH_MIN || c > CRASH_FONT_GLYPH_MAX) {
        c = '?';
    }
    rows = gCrashScreenFont[c - CRASH_FONT_GLYPH_MIN];

    u16 *ptr = gCrashScreen.frameBuf + SCREEN_WIDTH * y + x;

        for (i = 0; i < CRASH_FONT_HEIGHT; i++) {
            u8 bit = 0x10;
            u8 rowMask = *rows++;

            for (j = 0; j < CRASH_FONT_WIDTH; j++) {
                *ptr++ = (bit & rowMask) ? fg : bg;
                bit >>= 1;
            }

        ptr += SCREEN_WIDTH - CRASH_FONT_WIDTH;
    }
}

static void *crashScreenCopyToBuf(void *dest, const char *src, u32 size) {
    memcpy(dest, src, size);
    return dest + size;
}

static void __attribute__((format(printf, 3, 4))) crashScreenPrintf(s32 x, s32 y, const char *fmt, ...) {
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

    if (causeIndex == 23) {
        causeIndex = 16;
    }

    if (causeIndex == 31) {
        causeIndex = 17;
    }

    u32 badVAddr = ctx->badvaddr;

    switch (causeIndex) {
        case 1: crashScreenPrintf(x, y, "write to bad pointer: 0x%08lx", badVAddr); break;
        case 2:
            if (badVAddr < 0x10000) { // probably null pointer + some struct offset
                crashScreenPrintf(x, y, "load from null pointer: 0x%08lx", badVAddr);
            } else {
                crashScreenPrintf(x, y, "load from bad pointer: 0x%08lx", badVAddr);
            }
            break;
        case 3:
            if (badVAddr < 0x10000) {
                crashScreenPrintf(x, y, "write to null pointer: 0x%08lx", badVAddr);
            } else {
                crashScreenPrintf(x, y, "write to bad pointer: 0x%08lx", badVAddr);
            }
            break;
        case 4: crashScreenPrintf(x, y, "unaligned load: 0x%08lx", badVAddr); break;
        case 5: crashScreenPrintf(x, y, "unaligned store: 0x%08lx", badVAddr); break;
        case 6:
        case 7: crashScreenPrintf(x, y, "address outside ram: 0x%08lx", badVAddr); break;
        case 10: crashScreenPrintf(x, y, "invalid instruction"); break;
        case 12: crashScreenPrintf(x, y, "integer overflow"); break;
        default:
            if (causeIndex < ARRAY_LENGTH(gFaultCauses)) {
                crashScreenPrintf(x, y, "%s", gFaultCauses[causeIndex]);
            break;
            } else {
                crashScreenPrintf(x, y, "cause index: %d", causeIndex);
        }
    }
}

static void crashScreenDrawLine(u32 y) {
    crashScreenFillRect(TEXT_L, y, TEXT_R - TEXT_L, 1, 0);
}

static void crashScreenDrawHeader(__OSThreadContext *ctx, OSId thread) {
    crashScreenPrintCause(ctx, COL0, HDR_Y0);
    crashScreenPrintf(COL0, HDR_Y1, "TH: %ld PC: %08lX SP: %08lX RA: %08lX", thread, ctx->pc, (u32)ctx->sp,
                      (u32)ctx->ra);
    crashScreenDrawLine(RULE_HDR);
}

typedef enum {
    CRASH_PAGE_SUMMARY,
    CRASH_PAGE_DETAIL,
    CRASH_PAGE_FPU,
    CRASH_PAGE_BACKTRACE,
    CRASH_PAGE_STACK,
    CRASH_PAGE_MAX
} CrashPage;

static void crashScreenDrawFooter(u8 page) {
    static const char *pageNames[] = {"SUMMARY", "DETAIL", "FPU", "BACKTRACE", "STACK"};

    crashScreenDrawLine(RULE_FOOT);
    crashScreenPrintf(COL0, FOOT_Y, "L/R %d/%d  %s", page + 1, CRASH_PAGE_MAX, pageNames[page]);
    crashScreenPrintf(TEXT_R - 60, FOOT_Y, "Z HIDE");
}

static void printBacktrace(s32 x, s32 y, s32 count) {
    BacktraceFrame *frames = gCrashScreen.frames;
    if (count > BACKTRACE_FRAMES_MAX) {
        count = BACKTRACE_FRAMES_MAX;
    }

    for (s32 i = 0; i < count; i++) {
        if (frames[i].pc == 0) {
            return;
        }

        if (frames[i].funcStart != 0) {
            crashScreenPrintf(x, y, "%08lX  <%08lX+0x%lX>", frames[i].pc, frames[i].funcStart,
                              frames[i].pc - frames[i].funcStart);
        } else {
            crashScreenPrintf(x, y, "%08lX", frames[i].pc);
        }

        y += LINE;
    }
}

static void crashScreenDrawSummary(__OSThreadContext *ctx) {
    u32 addr = ctx->pc - (4 * 4);
    s32 row = 1;

    if (!VALID_ADDR(addr)) {
        return;
    }

    crashScreenPrintf(COL0, ROW(0), "DISASSEMBLY");

    for (s32 i = 0; i < 8; i++) {
        const char *inst = disasmInstruction(*(u32 *)addr, addr);
        if (addr == ctx->pc) {
            crashScreenPrintf(COL0, ROW(row++), "-> %08lx: %s", addr, inst);
        } else {
            crashScreenPrintf(COL0 + 18, ROW(row++), "%08lx: %s", addr, inst);
        }

        addr += 4;
        if (!VALID_ADDR(addr)) {
            return;
        }
    }

    row++;

    crashScreenPrintf(COL0, ROW(row++), "CALL STACK");
    printBacktrace(COL0, ROW(row++), 5);
}

static void crashScreenPrintAssertMsg(s32 x, s32 y) {
    char mmsg[ASSERT_BUFFER_SIZE] = {0};
    strncpy(mmsg, assertMsg, ARRAY_LENGTH(mmsg));
    char *msg = mmsg;
    char *p = msg;
    for (s32 i = 0; i < ASSERT_BUFFER_SIZE; i++) {
        if (*p == 0) {
            crashScreenPrintf(x, y, "%s", msg);
            break;
        }

        if (*p == '\n') {
            *p++ = '\0';
            crashScreenPrintf(x, y, "%s", msg);
            y += LINE;
            msg = p;
            continue;
        }
        p++;
    }
}

static void crashScreenDrawDetail(__OSThreadContext *ctx) {
    crashScreenPrintf(COL0, ROW(0), "AT:%08lX     V0:%08lX     V1:%08lX", (u32)ctx->at, (u32)ctx->v0, (u32)ctx->v1);
    crashScreenPrintf(COL0, ROW(1), "A0:%08lX     A1:%08lX     A2:%08lX", (u32)ctx->a0, (u32)ctx->a1, (u32)ctx->a2);
    crashScreenPrintf(COL0, ROW(2), "A3:%08lX     T0:%08lX     T1:%08lX", (u32)ctx->a3, (u32)ctx->t0, (u32)ctx->t1);
    crashScreenPrintf(COL0, ROW(3), "T2:%08lX     T3:%08lX     T4:%08lX", (u32)ctx->t2, (u32)ctx->t3, (u32)ctx->t4);
    crashScreenPrintf(COL0, ROW(4), "T5:%08lX     T6:%08lX     T7:%08lX", (u32)ctx->t5, (u32)ctx->t6, (u32)ctx->t7);
    crashScreenPrintf(COL0, ROW(5), "S0:%08lX     S1:%08lX     S2:%08lX", (u32)ctx->s0, (u32)ctx->s1, (u32)ctx->s2);
    crashScreenPrintf(COL0, ROW(6), "S3:%08lX     S4:%08lX     S5:%08lX", (u32)ctx->s3, (u32)ctx->s4, (u32)ctx->s5);
    crashScreenPrintf(COL0, ROW(7), "S6:%08lX     S7:%08lX     T8:%08lX", (u32)ctx->s6, (u32)ctx->s7, (u32)ctx->t8);
    crashScreenPrintf(COL0, ROW(8), "T9:%08lX     GP:%08lX     S8:%08lX", (u32)ctx->t9, (u32)ctx->gp, (u32)ctx->s8);
    crashScreenPrintf(COL0, ROW(10), "SR:%08lX   CAUSE:%08lX", ctx->sr, ctx->cause);
    crashScreenPrintf(COL0, ROW(11), "HI:%08lX   LO:%08lX", (u32)ctx->hi, (u32)ctx->lo);

    crashScreenPrintAssertMsg(COL0, ROW(13));
}

static void crashScreenPrintFpcsr(u32 value) {
    crashScreenPrintf(COL0, ROW(0), "FPCSR:%08lX", value);

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
        crashScreenPrintf(x, y, "F%02ld:%+.3e", regNum, *(f32 *)addr);
    } else {
        crashScreenPrintf(x, y, "F%02ld:---------", regNum);
    }
}

static void crashScreenDrawFpu(__OSThreadContext *ctx) {
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

static void crashScreenDrawBacktrace(void) {
    printBacktrace(COL0, ROW(0), BACKTRACE_FRAMES_MAX);
}

static void crashScreenDrawStack(__OSThreadContext *ctx) {
    u32 *sp = (u32 *)(u32)ctx->sp;

    crashScreenPrintf(TEXT_L, ROW(0), "    00:       04:       08:       0C:");
    for (u32 i = 0; i < 15; i++) {
        if (!VALID_ADDR(sp) || !VALID_ADDR(sp + 3)) {
            return;
        }
        crashScreenPrintf(TEXT_L, ROW(i + 1), "%02ld: %08lX  %08lX  %08lX  %08lX", i, *sp, *(sp + 1), *(sp + 2),
                          *(sp + 3));
        sp += 4;
    }
}

static void crashScreenDrawPage(OSThread *faultedThread, CrashPage page) {
    __OSThreadContext *ctx = &faultedThread->context;

    osWritebackDCacheAll();

    crashScreenFillRect(PAGE_X, PAGE_Y, PAGE_W, PAGE_H, 1);

    crashScreenDrawHeader(ctx, faultedThread->id);
    crashScreenDrawFooter(page);

    switch (page) {
        case CRASH_PAGE_SUMMARY: crashScreenDrawSummary(ctx); break;
        case CRASH_PAGE_DETAIL: crashScreenDrawDetail(ctx); break;
        case CRASH_PAGE_FPU: crashScreenDrawFpu(ctx); break;
        case CRASH_PAGE_BACKTRACE: crashScreenDrawBacktrace(); break;
        case CRASH_PAGE_STACK: crashScreenDrawStack(ctx); break;
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

    OSThread *thread = __osGetActiveQueue();

    while (thread->priority != -1) {
        if (thread->priority > 0 && thread->priority < 0x7F && thread->id != gCrashScreen.thread.id) {
            osStopThread(thread);
        }
        thread = thread->tlnext;
    }
    __osSiRelAccess();

    PRINTF("drawing crash screen\n");

    void *fb = (void *)MIPS_KSEG0_TO_KSEG1(osViGetCurrentFramebuffer());

    osViBlack(0);
    osViRepeatLine(0);
    osViSwapBuffer(gCrashScreen.frameBuf);

    u8 page = CRASH_PAGE_SUMMARY;
    u16 lastButtons = 0;
    bool redraw = TRUE;
    bool drawingCrashScreen = TRUE;

    __OSThreadContext *ctx = &faultedThread->context;
    gCrashScreen.frames = recoverBacktrace(ctx->pc, (u32)ctx->ra, (u32)ctx->sp);

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

void crashScreenSetDrawInfoCustom(u16 *frameBufPtr) {
    gCrashScreen.frameBuf = (u16 *)MIPS_KSEG0_TO_KSEG1(frameBufPtr);
}

void crashScreenInit(void) {
    gCrashScreen.frameBuf = (u16 *)(MIPS_KSEG0_TO_KSEG1(osMemSize) - ((SCREEN_WIDTH * SCREEN_HEIGHT) * 2));
    osCreateMesgQueue(&gCrashScreen.queue, &gCrashScreen.mesg, 1);
#if PM64_VERSION == US
    OSThread *thread = __osGetActiveQueue();
    while (thread->priority != -1) {
        if (thread->id == 2) {
            osDestroyThread(thread);
            break;
        }

        thread = thread->tlnext;
    }
#endif
    osCreateThread(&gCrashScreen.thread, 2, crashScreenThreadEntry, NULL,
                   gCrashScreen.stack + sizeof(gCrashScreen.stack), 0x80);
    osStartThread(&gCrashScreen.thread);
    PRINTF("crash thread started %8X\n", crashScreenThreadEntry);
}
