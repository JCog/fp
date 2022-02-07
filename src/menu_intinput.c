#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "menu.h"

struct item_data {
    _Bool signed_;
    s32 base;
    s32 length;
    menu_generic_callback callback_proc;
    void *callback_data;
    u32 value;
    _Bool active;
    struct menu *imenu;
    struct menu_item *item;
    struct menu_item **digits;
};

static inline s32 char_to_int(s32 x) {
    if (x >= '0' && x <= '9') {
        return x - ('0' - 0x0);
    } else if (x >= 'a' && x <= 'f') {
        return x - ('a' - 0xA);
    }
    return -1;
}

static inline s32 int_to_char(s32 x) {
    if (x >= 0x0 && x <= 0x9) {
        return x + ('0' - 0x0);
    } else if (x >= 0xA && x <= 0xF) {
        return x + ('a' - 0xA);
    }
    return -1;
}

static s32 think_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    if (data->active) {
        s32 r = menu_think(data->imenu);
        if (r) {
            return r;
        }
    }
    if (data->callback_proc) {
        s32 r = data->callback_proc(item, MENU_CALLBACK_THINK, data->callback_data);
        if (r) {
            return r;
        }
        if (data->active) {
            r = data->callback_proc(item, MENU_CALLBACK_THINK_ACTIVE, data->callback_data);
        } else {
            r = data->callback_proc(item, MENU_CALLBACK_THINK_INACTIVE, data->callback_data);
        }
        return r;
    }
    return 0;
}

static s32 draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    struct item_data *data = item->data;
    if (data->active) {
        data->imenu->cxoffset = item->x;
        data->imenu->cyoffset = item->y;
        menu_draw(data->imenu);
    }
    return data->active;
}

static s32 navigate_proc(struct menu_item *item, enum menu_navigation nav) {
    struct item_data *data = item->data;
    if (data->active) {
        menu_navigate(data->imenu, nav);
    }
    return data->active;
}

static s32 activate_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    if (data->active) {
        if (data->callback_proc && data->callback_proc(item, MENU_CALLBACK_DEACTIVATE, data->callback_data)) {
            return 1;
        }
        u32 value = 0;
        u32 mul = 1;
        s32 sign = data->signed_ && data->digits[0]->text[0] == '-' ? -1 : 1;
        for (s32 i = data->length - 1; i >= 0; --i) {
            s32 n = data->digits[i]->text[0];
            if (data->signed_ && i == 0) {
                if (value == 0 && sign == -1) {
                    sign = 1;
                    data->digits[i]->text[0] = '+';
                    data->item->text[0] = '0';
                    data->item->text[data->length - 1] = ' ';
                }
            } else {
                value += char_to_int(n) * mul;
                mul *= data->base;
            }
            if (data->signed_ && sign == 1) {
                if (i > 0) {
                    data->item->text[i - 1] = n;
                }
                if (i == data->length - 1) {
                    data->item->text[i] = ' ';
                }
            } else {
                data->item->text[i] = n;
            }
        }
        data->value = value * sign;
        if (data->callback_proc) {
            data->callback_proc(item, MENU_CALLBACK_CHANGED, data->callback_data);
        }
    } else {
        if (data->callback_proc && data->callback_proc(item, MENU_CALLBACK_ACTIVATE, data->callback_data)) {
            return 1;
        }
    }
    data->active = !data->active;
    return 1;
}

static s32 destroy_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    for (s32 i = 0; i < data->length; ++i) {
        data->digits[i]->data = NULL;
    }
    menu_destroy(data->imenu);
    free(data->digits);
    return 0;
}

static s32 sign_navigate_proc(struct menu_item *item, enum menu_navigation nav) {
    if (nav != MENU_NAVIGATE_UP && nav != MENU_NAVIGATE_DOWN) {
        return 0;
    }
    item->text[0] = '+' + '-' - item->text[0];
    return 1;
}

static s32 digit_navigate_proc(struct menu_item *item, enum menu_navigation nav) {
    struct item_data *data = item->data;
    s32 value = char_to_int(item->text[0]);
    if (nav == MENU_NAVIGATE_UP) {
        ++value;
    } else if (nav == MENU_NAVIGATE_DOWN) {
        --value;
    } else {
        return 0;
    }
    value = (value + data->base) % data->base;
    item->text[0] = int_to_char(value);
    return 1;
}

struct menu_item *menu_add_intinput(struct menu *menu, s32 x, s32 y, s32 base, s32 length,
                                    menu_generic_callback callback_proc, void *callback_data) {
    struct item_data *data = malloc(sizeof(*data));
    if (base < 0) {
        data->signed_ = 1;
        base = -base;
    } else {
        data->signed_ = 0;
    }
    data->base = base;
    data->length = length;
    data->callback_proc = callback_proc;
    data->callback_data = callback_data;
    data->value = 0;
    data->active = 0;
    data->digits = malloc(sizeof(*data->digits) * length);
    data->imenu = malloc(sizeof(*data->imenu));
    menu_init(data->imenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    data->imenu->parent = menu;
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0xFFFFFF);
    data->item = item;
    item->text = malloc(length + 1);
    item->text[length] = 0;
    item->data = data;
    item->think_proc = think_proc;
    item->draw_proc = draw_proc;
    item->navigate_proc = navigate_proc;
    item->activate_proc = activate_proc;
    item->destroy_proc = destroy_proc;
    for (s32 i = 0; i < length; ++i) {
        u32 color = data->imenu->highlight_color_static;
        struct menu_item *digit = menu_item_add(data->imenu, i, 0, NULL, color);
        data->digits[i] = digit;
        digit->text = malloc(2);
        if (data->signed_ && i == 0) {
            digit->text[0] = '+';
            digit->navigate_proc = sign_navigate_proc;
        } else {
            digit->text[0] = '0';
            digit->navigate_proc = digit_navigate_proc;
        }
        if (data->signed_ && i == data->length - 1) {
            item->text[i] = ' ';
        } else {
            item->text[i] = '0';
        }
        digit->text[1] = 0;
        digit->animate_highlight = 1;
        digit->data = data;
    }
    data->imenu->selector = data->digits[length - 1];
    return item;
}

u32 menu_intinput_get(struct menu_item *item) {
    struct item_data *data = item->data;
    return data->value;
}

s32 menu_intinput_gets(struct menu_item *item) {
    struct item_data *data = item->data;
    return data->value;
}

void menu_intinput_set(struct menu_item *item, u32 value) {
    struct item_data *data = item->data;
    data->value = value;
    s32 sign = data->signed_ && (s32)value < 0 ? -1 : 1;
    value *= sign;
    for (s32 i = data->length - 1; i >= 0; --i) {
        s32 c;
        if (data->signed_ && i == 0) {
            c = '+' - (sign - 1);
        } else {
            c = int_to_char(value % data->base);
        }
        value /= data->base;
        data->digits[i]->text[0] = c;
        if (data->signed_ && sign == 1) {
            if (i > 0) {
                data->item->text[i - 1] = c;
            }
            if (i == data->length - 1) {
                data->item->text[i] = ' ';
            }
        } else {
            data->item->text[i] = c;
        }
    }
}
