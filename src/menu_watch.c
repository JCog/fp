#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <stdint.h>
#include <inttypes.h>
#include "menu.h"
#include "util.h"

struct item_data {
    u32 address;
    enum watch_type type;
};

// clang-format off
static s32 watch_type_size[] = {
    1, 1, 1, 2, 2, 2, 4, 4, 4, 4,
};
// clang-format on

static s32 draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    struct item_data *data = item->data;
    u32 address = data->address;
    if (address < 0x80000000 || address >= 0x80800000 || data->type < 0 || data->type >= WATCH_TYPE_MAX) {
        return 1;
    }
    address -= address % watch_type_size[data->type];
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
            if (is_nan(v)) {
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

struct menu_item *menu_add_watch(struct menu *menu, s32 x, s32 y, u32 address, enum watch_type type) {
    struct item_data *data = malloc(sizeof(*data));
    data->address = address;
    data->type = type;
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0xC0C0C0);
    item->text = malloc(17);
    item->selectable = 0;
    item->data = data;
    item->draw_proc = draw_proc;
    return item;
}

u32 menu_watch_get_address(struct menu_item *item) {
    struct item_data *data = item->data;
    return data->address;
}

void menu_watch_set_address(struct menu_item *item, u32 address) {
    struct item_data *data = item->data;
    data->address = address;
}

enum watch_type menu_watch_get_type(struct menu_item *item) {
    struct item_data *data = item->data;
    return data->type;
}

void menu_watch_set_type(struct menu_item *item, enum watch_type type) {
    struct item_data *data = item->data;
    data->type = type;
}
