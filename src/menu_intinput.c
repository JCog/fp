#include "menu.h"
#include <stdlib.h>

struct ItemData {
    bool signed_;
    s32 base;
    s32 length;
    MenuGenericCallback callbackProc;
    void *callbackData;
    u32 value;
    bool active;
    struct Menu *imenu;
    struct MenuItem *item;
    struct MenuItem **digits;
};

static inline s32 charToInt(s32 x) {
    if (x >= '0' && x <= '9') {
        return x - ('0' - 0x0);
    } else if (x >= 'a' && x <= 'f') {
        return x - ('a' - 0xA);
    }
    return -1;
}

static inline s32 intToChar(s32 x) {
    if (x >= 0x0 && x <= 0x9) {
        return x + ('0' - 0x0);
    } else if (x >= 0xA && x <= 0xF) {
        return x + ('a' - 0xA);
    }
    return -1;
}

static s32 thinkProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    if (data->active) {
        s32 r = menuThink(data->imenu);
        if (r) {
            return r;
        }
    }
    if (data->callbackProc) {
        s32 r = data->callbackProc(item, MENU_CALLBACK_THINK, data->callbackData);
        if (r) {
            return r;
        }
        if (data->active) {
            r = data->callbackProc(item, MENU_CALLBACK_THINK_ACTIVE, data->callbackData);
        } else {
            r = data->callbackProc(item, MENU_CALLBACK_THINK_INACTIVE, data->callbackData);
        }
        return r;
    }
    return 0;
}

static s32 drawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    struct ItemData *data = item->data;
    if (data->active) {
        data->imenu->cxoffset = item->x;
        data->imenu->cyoffset = item->y;
        menuDraw(data->imenu);
    }
    return data->active;
}

static s32 navigateProc(struct MenuItem *item, enum MenuNavigation nav) {
    struct ItemData *data = item->data;
    if (data->active) {
        menuNavigate(data->imenu, nav);
    }
    return data->active;
}

static s32 activateProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    if (data->active) {
        if (data->callbackProc && data->callbackProc(item, MENU_CALLBACK_DEACTIVATE, data->callbackData)) {
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
                value += charToInt(n) * mul;
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
        if (data->callbackProc) {
            data->callbackProc(item, MENU_CALLBACK_CHANGED, data->callbackData);
        }
    } else {
        if (data->callbackProc && data->callbackProc(item, MENU_CALLBACK_ACTIVATE, data->callbackData)) {
            return 1;
        }
    }
    data->active = !data->active;
    return 1;
}

static s32 destroyProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    for (s32 i = 0; i < data->length; ++i) {
        data->digits[i]->data = NULL;
    }
    menuDestroy(data->imenu);
    free(data->digits);
    return 0;
}

static s32 signNavigateProc(struct MenuItem *item, enum MenuNavigation nav) {
    if (nav != MENU_NAVIGATE_UP && nav != MENU_NAVIGATE_DOWN) {
        return 0;
    }
    item->text[0] = '+' + '-' - item->text[0];
    return 1;
}

static s32 digitNavigateProc(struct MenuItem *item, enum MenuNavigation nav) {
    struct ItemData *data = item->data;
    s32 value = charToInt(item->text[0]);
    if (nav == MENU_NAVIGATE_UP) {
        ++value;
    } else if (nav == MENU_NAVIGATE_DOWN) {
        --value;
    } else {
        return 0;
    }
    value = (value + data->base) % data->base;
    item->text[0] = intToChar(value);
    return 1;
}

struct MenuItem *menuAddIntinput(struct Menu *menu, s32 x, s32 y, s32 base, s32 length,
                                 MenuGenericCallback callbackProc, void *callbackData) {
    struct ItemData *data = malloc(sizeof(*data));
    if (base < 0) {
        data->signed_ = TRUE;
        base = -base;
    } else {
        data->signed_ = FALSE;
    }
    data->base = base;
    data->length = length;
    data->callbackProc = callbackProc;
    data->callbackData = callbackData;
    data->value = 0;
    data->active = FALSE;
    data->digits = malloc(sizeof(*data->digits) * length);
    data->imenu = malloc(sizeof(*data->imenu));
    menuInit(data->imenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    data->imenu->parent = menu;
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0xFFFFFF);
    data->item = item;
    item->text = malloc(length + 1);
    item->text[length] = 0;
    item->data = data;
    item->thinkProc = thinkProc;
    item->drawProc = drawProc;
    item->navigateProc = navigateProc;
    item->activateProc = activateProc;
    item->destroyProc = destroyProc;
    for (s32 i = 0; i < length; ++i) {
        u32 color = data->imenu->highlightColorStatic;
        struct MenuItem *digit = menuItemAdd(data->imenu, i, 0, NULL, color);
        data->digits[i] = digit;
        digit->text = malloc(2);
        if (data->signed_ && i == 0) {
            digit->text[0] = '+';
            digit->navigateProc = signNavigateProc;
        } else {
            digit->text[0] = '0';
            digit->navigateProc = digitNavigateProc;
        }
        if (data->signed_ && i == data->length - 1) {
            item->text[i] = ' ';
        } else {
            item->text[i] = '0';
        }
        digit->text[1] = 0;
        digit->animateHighlight = TRUE;
        digit->data = data;
    }
    data->imenu->selector = data->digits[length - 1];
    return item;
}

u32 menuIntinputGet(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->value;
}

s32 menuIntinputGets(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->value;
}

void menuIntinputSet(struct MenuItem *item, u32 value) {
    struct ItemData *data = item->data;
    data->value = value;
    s32 sign = data->signed_ && (s32)value < 0 ? -1 : 1;
    value *= sign;
    for (s32 i = data->length - 1; i >= 0; --i) {
        s32 c;
        if (data->signed_ && i == 0) {
            c = '+' - (sign - 1);
        } else {
            c = intToChar(value % data->base);
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
