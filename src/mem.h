#ifndef MEM_H
#define MEM_H
#include "common.h"
#include "menu.h"

void memMenuCreate(struct Menu *menu);
void memGoto(u32 address);
void memOpenWatch(struct Menu *menu, struct Menu *menuMem, u32 address, enum WatchType type);

#endif
