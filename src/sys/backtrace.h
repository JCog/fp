#ifndef BACKTRACE_H
#define BACKTRACE_H
#include "common.h"

#define BACKTRACE_FRAMES_MAX 16

typedef struct {
    u32 pc;
    u32 funcStart;
} BacktraceFrame;

typedef struct {
    BacktraceFrame* frames;
    u32 numFrames;
} Backtrace;

Backtrace recoverBacktrace(u32 pc, u32 ra, u32 sp);

#endif
