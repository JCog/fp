#include "menu.h"
#include <stdlib.h>

struct ItemData {
    struct MenuItem *address;
    struct MenuItem *type;
    struct MenuItem *watch;
};

static s32 addressProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    struct ItemData *itemData = data;
    if (reason == MENU_CALLBACK_CHANGED) {
        menuWatchSetAddress(itemData->watch, menuIntinputGet(item));
    }
    return 0;
}

static s32 typeProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    struct ItemData *itemData = data;
    if (reason == MENU_CALLBACK_CHANGED) {
        menuWatchSetType(itemData->watch, menuOptionGet(item));
    }
    return 0;
}

struct MenuItem *menuAddUserwatch(struct Menu *menu, s32 x, s32 y, u32 address, enum WatchType type) {
    struct Menu *imenu;
    struct MenuItem *item = menuAddImenu(menu, x, y, &imenu);
    struct ItemData *data = malloc(sizeof(*data));
    data->address = menuAddIntinput(imenu, 0, 0, 16, 8, addressProc, data);
    menuIntinputSet(data->address, address);
    data->type = menuAddOption(imenu, 9, 0,
                               "u8\0"
                               "s8\0"
                               "x8\0"
                               "u16\0"
                               "s16\0"
                               "x16\0"
                               "u32\0"
                               "s32\0"
                               "x32\0"
                               "f32\0",
                               typeProc, data);
    menuOptionSet(data->type, type);
    data->watch = menuAddWatch(imenu, 13, 0, address, type);
    item->data = data;
    return item;
}

struct MenuItem *menuUserwatchAddress(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->address;
}

struct MenuItem *menuUserwatchType(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->type;
}

struct MenuItem *menuUserwatchWatch(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->watch;
}
