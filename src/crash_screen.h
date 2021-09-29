#ifndef CRASH_SCREEN_H
#define CRASH_SCREEN_H

#include "fp.h"

typedef struct {
    /* 0x000 */ OSThread thread;
    /* 0x1B0 */ char stack[0x800];
    /* 0x9B0 */ OSMesgQueue queue;
    /* 0x9C8 */ OSMesg mesg;
    /* 0x9CC */ u16* frameBuf;
    /* 0x9D0 */ u16 width;
    /* 0x9D2 */ u16 height;
} CrashScreen; // size = 0x9D4

void crash_screen_init(void);
void crash_screen_set_draw_info(u16* frameBufPtr, s16 width, s16 height);

#endif