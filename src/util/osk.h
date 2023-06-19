#ifndef OSK_H
#define OSK_H
#include "common.h"
#include "menu/menu.h"

typedef s32 (*OskCallback)(const char *str, void *data);

void menuGetOskString(struct Menu *menu, const char *dflt, OskCallback callbackProc, void *callbackData);

#endif
