#include "menu.h"
#include "util.h"
#include <math.h>
#include <stdlib.h>

struct item_data {
    s32 sig_precis;
    s32 exp_precis;
    menu_generic_callback callback_proc;
    void *callback_data;
    f32 value;
    _Bool active;
    struct menu *imenu;
    struct menu_item *item;
    struct menu_item *sig_sign;
    struct menu_item **sig_digits;
    struct menu_item *exp_sign;
    struct menu_item **exp_digits;
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
        s32 sig = 0;
        s32 exp = 0;
        char *p = data->item->text;
        s32 sig_sign;
        s32 exp_sign;
        s32 mul;

        if (data->sig_sign->text[0] == '+') {
            sig_sign = 1;
        } else {
            sig_sign = -1;
            *p++ = '-';
        }
        data->item->text[0] = data->sig_sign->text[0];
        mul = 1;
        for (s32 i = data->sig_precis - 1; i >= 0; --i) {
            s32 x = i;
            if (i > 0) {
                ++x;
            }
            s32 n = data->sig_digits[i]->text[0];
            sig += char_to_int(n) * mul;
            mul *= 10;
            p[x] = n;
        }
        p[1] = '.';
        p += data->sig_precis + 1;
        *p++ = 'e';

        for (s32 i = 0; i < data->exp_precis; ++i) {
            if (data->exp_digits[i]->text[0] != '0') {
                break;
            }
            if (i == data->exp_precis - 1) {
                data->exp_sign->text[0] = '+';
            }
        }
        if (data->exp_sign->text[0] == '+') {
            exp_sign = 1;
        } else {
            exp_sign = -1;
            *p++ = '-';
        }
        data->item->text[3 + data->sig_precis] = data->exp_sign->text[0];
        mul = 1;
        for (s32 i = data->exp_precis - 1; i >= 0; --i) {
            s32 n = data->exp_digits[i]->text[0];
            exp += char_to_int(n) * mul;
            mul *= 10;
            p[i] = n;
        }
        p[data->exp_precis] = 0;

        f32 exp_mul = pow(10., exp * exp_sign - (data->sig_precis - 1));
        data->value = sig * sig_sign * exp_mul;

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
    data->sig_sign->data = NULL;
    for (s32 i = 0; i < data->sig_precis; ++i) {
        data->sig_digits[i]->data = NULL;
    }
    data->exp_sign->data = NULL;
    for (s32 i = 0; i < data->exp_precis; ++i) {
        data->exp_digits[i]->data = NULL;
    }
    menu_destroy(data->imenu);
    free(data->sig_digits);
    free(data->exp_digits);
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
    s32 value = char_to_int(item->text[0]);
    if (nav == MENU_NAVIGATE_UP) {
        ++value;
    } else if (nav == MENU_NAVIGATE_DOWN) {
        --value;
    } else {
        return 0;
    }
    value = (value + 10) % 10;
    item->text[0] = int_to_char(value);
    return 1;
}

struct menu_item *menu_add_floatinput(struct menu *menu, s32 x, s32 y, s32 sig_precis, s32 exp_precis,
                                      menu_generic_callback callback_proc, void *callback_data) {
    struct item_data *data = malloc(sizeof(*data));
    data->sig_precis = sig_precis;
    data->exp_precis = exp_precis;
    data->callback_proc = callback_proc;
    data->callback_data = callback_data;
    data->value = 0.;
    data->active = 0;
    data->sig_digits = malloc(sizeof(*data->sig_digits) * sig_precis);
    data->exp_digits = malloc(sizeof(*data->exp_digits) * exp_precis);
    data->imenu = malloc(sizeof(*data->imenu));
    menu_init(data->imenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    data->imenu->parent = menu;
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0xFFFFFF);
    data->item = item;
    item->text = malloc(sig_precis + exp_precis + 5);
    item->text[1] = '.';
    item->text[1 + sig_precis] = 'e';
    item->text[sig_precis + exp_precis + 2] = 0;
    item->data = data;
    item->think_proc = think_proc;
    item->draw_proc = draw_proc;
    item->navigate_proc = navigate_proc;
    item->activate_proc = activate_proc;
    item->destroy_proc = destroy_proc;
    u32 color = data->imenu->highlight_color_static;
    menu_item_add(data->imenu, 2, 0, ".", color)->selectable = 0;
    menu_item_add(data->imenu, 2 + sig_precis, 0, "e", color)->selectable = 0;
    for (s32 i = 0; i < 2; ++i) {
        s32 x = i * (3 + sig_precis);
        struct menu_item *sign = menu_item_add(data->imenu, x, 0, NULL, color);
        sign->text = malloc(2);
        sign->text[0] = '+';
        sign->text[1] = 0;
        sign->navigate_proc = sign_navigate_proc;
        sign->animate_highlight = 1;
        sign->data = data;
        if (i == 0) {
            data->sig_sign = sign;
        } else {
            data->exp_sign = sign;
        }
    }
    for (s32 i = 0; i < sig_precis + exp_precis; ++i) {
        s32 x = 1 + i;
        s32 tx = i;
        if (i >= sig_precis) {
            x += 3;
            tx += 2;
        } else if (i > 0) {
            ++x;
            ++tx;
        }
        struct menu_item *digit = menu_item_add(data->imenu, x, 0, NULL, color);
        digit->text = malloc(2);
        digit->text[0] = '0';
        item->text[tx] = '0';
        digit->text[1] = 0;
        digit->navigate_proc = digit_navigate_proc;
        digit->animate_highlight = 1;
        digit->data = data;
        if (i < sig_precis) {
            data->sig_digits[i] = digit;
        } else {
            data->exp_digits[i - sig_precis] = digit;
        }
    }
    data->imenu->selector = data->sig_digits[0];
    return item;
}

f32 menu_floatinput_get(struct menu_item *item) {
    struct item_data *data = item->data;
    return data->value;
}

void menu_floatinput_set(struct menu_item *item, f32 value) {
    if (is_nan(value) || !isnormal(value)) {
        value = 0.f;
    }
    struct item_data *data = item->data;
    data->value = value;

    s32 sig_sign = signbit(value) ? -1 : 1;
    value = fabsf(value);
    s32 exp = value == 0.f ? 0.f : floorf(log10f(value));
    s32 sig = value / pow(10., exp - (data->sig_precis - 1)) + 0.5;
    s32 exp_sign = exp < 0 ? -1 : 1;
    exp *= exp_sign;
    char *p = data->item->text;

    if (sig_sign == 1) {
        data->item->text[0] = data->sig_sign->text[0] = '+';
    } else {
        data->item->text[0] = data->sig_sign->text[0] = '-';
        *p++ = '-';
    }
    for (s32 i = data->sig_precis - 1; i >= 0; --i) {
        s32 x = i;
        if (i > 0) {
            ++x;
        }
        s32 c = int_to_char(sig % 10);
        sig /= 10;
        data->sig_digits[i]->text[0] = c;
        p[x] = c;
    }
    p[1] = '.';
    p += data->sig_precis + 1;
    *p++ = 'e';

    if (exp_sign == 1) {
        data->item->text[3 + data->sig_precis] = data->exp_sign->text[0] = '+';
    } else {
        data->item->text[3 + data->sig_precis] = data->exp_sign->text[0] = '-';
        *p++ = '-';
    }
    for (s32 i = data->exp_precis - 1; i >= 0; --i) {
        s32 c = int_to_char(exp % 10);
        exp /= 10;
        data->exp_digits[i]->text[0] = c;
        p[i] = c;
    }
    p[data->exp_precis] = 0;
}
