#ifndef BOSSES_H
#define BOSSES_H
#include "menu/menu.h"

#define BOSSES_DUMMY_ID (s16)0xDEAD

extern pm_Npc bossesDummyNpc;

void battleUpdateWarps(void);

void createBossesMenu(struct Menu *menu);

#endif
