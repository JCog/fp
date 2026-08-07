#include "backtrace.h"
#include "disasm.h"
#include "macros.h"

#define ADDIU_SP_SP(x)     (((x) >> 16) == 0x27BD)
#define SW_RA_SP(x)        (((x) >> 16) == 0xAFBF)
#define BACKTRACE_SCAN_MAX 0x1000
#define FUNC_SIZE_MAX      0x8000

static bool isCall(u32 inst) {
    // jal or jalr
    return OP(inst) == 3 || (OP(inst) == 0 && FUNCT(inst) == 9);
}

static bool isCallSite(const u32 *pc) {
    return VALID_ADDR(pc) && isCall(*pc);
}

static void findFuncStart(u32 pc, u32 callAddr, u32 *funcStart) {
    if (!VALID_ADDR(pc) || !VALID_ADDR(callAddr)) {
        return;
    }

    u32 call = *(u32 *)callAddr;
    if (OP(call) != 3) { // not jal
        return;
    }

    u32 target = TARGET(call, callAddr + 4);
    if (!VALID_ADDR(target) || target > pc || pc - target >= FUNC_SIZE_MAX) {
        return;
    }

    u32 prologues = 0;
    for (u32 *p = (u32 *)target; p < (u32 *)pc; p++) {
        if (ADDIU_SP_SP(*p) && (IMM(*p) & 0x8000)) {
            prologues++;
        }
    }

    if (prologues > 1) {
        return;
    }

    *funcStart = target;
}

Backtrace recoverBacktrace(u32 pc, u32 ra, u32 sp) {
    static BacktraceFrame frames[BACKTRACE_FRAMES_MAX];
    u32 frameID = 0;

    while (frameID < ARRAY_LENGTH(frames) && VALID_ADDR(pc)) {
        u32 idx = frameID++;
        bool leaf = FALSE;
        s32 frameSize = -1;
        s32 raOffset = -1;

        frames[idx].pc = pc;
        frames[idx].funcStart = 0;

        if (idx == 0) {
            findFuncStart(pc, ra - 8, &frames[0].funcStart);
        }

        u32 *scan = (u32 *)pc;
        for (u32 i = 0; i < BACKTRACE_SCAN_MAX; i++) {
            if (!VALID_ADDR(scan)) {
                break;
            }

            // reached the start of the function with no prologue
            if (frames[idx].funcStart != 0 && (u32)scan < frames[idx].funcStart) {
                leaf = TRUE;
                break;
            }

            u32 inst = *scan;
            if (SW_RA_SP(inst)) { // sw ra, raOffset(sp)
                raOffset = IMM(inst);
            } else if (ADDIU_SP_SP(inst) && IMM(inst) & 0x8000) { // addiu sp, sp, -frameSize
                frameSize = -IMM(inst);
                break;
            }

            scan--;
        }

        if (leaf) {
            frameSize = 0;
            raOffset = -1;
        } else if (frameSize < 0) {
            break;
        }

        if (raOffset < 0) {
            if (idx != 0) {
                break;
            }
            if (!isCallSite((u32 *)(ra - 8))) {
                break;
            }
            // we probably crashed in a function with no stack frame so start scanning from call site
            pc = ra - 8;
        } else {
            u32 *slot = (u32 *)(sp + raOffset);

            if (!VALID_ADDR(slot)) {
                break;
            }

            u32 retAddr = *slot;
            if (!isCallSite((u32 *)(retAddr - 8))) {
                break;
            }

            pc = retAddr - 8;
        }

        sp += frameSize;
    }

    for (u32 i = 0; i + 1 < frameID; i++) {
        if (frames[i].funcStart == 0) {
            findFuncStart(frames[i].pc, frames[i + 1].pc, &frames[i].funcStart);
        }
    }

    Backtrace b;
    b.frames = frames;
    b.numFrames = frameID;

    return b;
}
