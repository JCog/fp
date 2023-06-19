#ifndef WATCHLIST_H
#define WATCHLIST_H
#include "common.h"
#include "menu/menu.h"

struct MenuItem *watchlistCreate(struct Menu *menu, struct Menu *menuRelease, s32 x, s32 y);
void watchlistStore(struct MenuItem *item);
void watchlistFetch(struct MenuItem *item);
void watchlistShow(struct MenuItem *item);
void watchlistHide(struct MenuItem *item);
s32 watchlistAdd(struct MenuItem *item, u32 address, enum WatchType type);

#endif
