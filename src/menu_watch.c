#include "menu.h"
#include "util.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ItemData {
    u32 address;
    enum WatchType type;
};

static s32 watchTypeSize[] = {
    1, 1, 1, 2, 2, 2, 4, 4, 4, 4,
};

static s32 drawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    struct ItemData *data = item->data;
    u32 address = data->address;
    if (address < 0x80000000 || address >= 0x80800000 || data->type < 0 || data->type >= WATCH_TYPE_MAX) {
        return 1;
    }
    address -= address % watchTypeSize[data->type];
    switch (data->type) {
        case WATCH_TYPE_U8: snprintf(item->text, 17, "%" PRIu8, *(u8 *)address); break;
        case WATCH_TYPE_S8: snprintf(item->text, 17, "%" PRIi8, *(s8 *)address); break;
        case WATCH_TYPE_X8: snprintf(item->text, 17, "0x%" PRIx8, *(u8 *)address); break;
        case WATCH_TYPE_U16: snprintf(item->text, 17, "%" PRIu16, *(u16 *)address); break;
        case WATCH_TYPE_S16: snprintf(item->text, 17, "%" PRIi16, *(s16 *)address); break;
        case WATCH_TYPE_X16: snprintf(item->text, 17, "0x%" PRIx16, *(u16 *)address); break;
        case WATCH_TYPE_U32: snprintf(item->text, 17, "%" PRIu32, *(u32 *)address); break;
        case WATCH_TYPE_S32: snprintf(item->text, 17, "%" PRIi32, *(s32 *)address); break;
        case WATCH_TYPE_X32: snprintf(item->text, 17, "0x%" PRIx32, *(u32 *)address); break;
        case WATCH_TYPE_F32: {
            f32 v = *(f32 *)address;
            if (isNan(v)) {
                strcpy(item->text, "nan");
            } else if (v == INFINITY) {
                strcpy(item->text, "inf");
            } else if (v == -INFINITY) {
                strcpy(item->text, "-inf");
            } else {
                if (!isnormal(v)) {
                    v = 0.f;
                }
                snprintf(item->text, 17, "%g", v);
            }
            break;
        }
        default: break;
    }
    return 0;
}

struct MenuItem *menuAddWatch(struct Menu *menu, s32 x, s32 y, u32 address, enum WatchType type) {
    struct ItemData *data = malloc(sizeof(*data));
    data->address = address;
    data->type = type;
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0xC0C0C0);
    item->text = malloc(17);
    item->selectable = FALSE;
    item->data = data;
    item->drawProc = drawProc;
    return item;
}

u32 menuWatchGetAddress(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->address;
}

void menuWatchSetAddress(struct MenuItem *item, u32 address) {
    struct ItemData *data = item->data;
    data->address = address;
}

enum WatchType menuWatchGetType(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->type;
}

void menuWatchSetType(struct MenuItem *item, enum WatchType type) {
    struct ItemData *data = item->data;
    data->type = type;
}
