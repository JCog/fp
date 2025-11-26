#ifndef TRAINER_H
#define TRAINER_H
#include "menu/menu.h"

enum PinnedTrainer {
    TRAINER_LZS,
    TRAINER_SPIN,
    TRAINER_ISS,
    TRAINER_ACE,
};

void trainerUpdate(void);
void trainerDrawPinned(s32 x, s32 y, struct GfxFont *font, s32 chWidth, s32 chHeight, u32 color, u8 alpha);
void createTrainerMenu(struct Menu *menu);

#endif
