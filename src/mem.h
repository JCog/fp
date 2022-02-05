#ifndef MEM_H
#define MEM_H
#include "menu.h"

void mem_menu_create(struct menu *menu);
void mem_goto(u32 address);
void mem_open_watch(struct menu *menu, struct menu *menu_mem, u32 address, enum watch_type type);

#endif
