#include "menu.h"
#include "util.h"
#include <math.h>
#include <stdlib.h>

struct ItemData {
    s32 sigPrecis;
    s32 expPrecis;
    MenuGenericCallback callbackProc;
    void *callbackData;
    f32 value;
    bool active;
    struct Menu *imenu;
    struct MenuItem *item;
    struct MenuItem *sigSign;
    struct MenuItem **sigDigits;
    struct MenuItem *expSign;
    struct MenuItem **expDigits;
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
        s32 sig = 0;
        s32 exp = 0;
        char *p = data->item->text;
        s32 sigSign;
        s32 expSign;
        s32 mul;

        if (data->sigSign->text[0] == '+') {
            sigSign = 1;
        } else {
            sigSign = -1;
            *p++ = '-';
        }
        data->item->text[0] = data->sigSign->text[0];
        mul = 1;
        for (s32 i = data->sigPrecis - 1; i >= 0; --i) {
            s32 x = i;
            if (i > 0) {
                ++x;
            }
            s32 n = data->sigDigits[i]->text[0];
            sig += charToInt(n) * mul;
            mul *= 10;
            p[x] = n;
        }
        p[1] = '.';
        p += data->sigPrecis + 1;
        *p++ = 'e';

        for (s32 i = 0; i < data->expPrecis; ++i) {
            if (data->expDigits[i]->text[0] != '0') {
                break;
            }
            if (i == data->expPrecis - 1) {
                data->expSign->text[0] = '+';
            }
        }
        if (data->expSign->text[0] == '+') {
            expSign = 1;
        } else {
            expSign = -1;
            *p++ = '-';
        }
        data->item->text[3 + data->sigPrecis] = data->expSign->text[0];
        mul = 1;
        for (s32 i = data->expPrecis - 1; i >= 0; --i) {
            s32 n = data->expDigits[i]->text[0];
            exp += charToInt(n) * mul;
            mul *= 10;
            p[i] = n;
        }
        p[data->expPrecis] = 0;

        f32 expMul = pow(10., exp * expSign - (data->sigPrecis - 1));
        data->value = sig * sigSign * expMul;

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
    data->sigSign->data = NULL;
    for (s32 i = 0; i < data->sigPrecis; ++i) {
        data->sigDigits[i]->data = NULL;
    }
    data->expSign->data = NULL;
    for (s32 i = 0; i < data->expPrecis; ++i) {
        data->expDigits[i]->data = NULL;
    }
    menuDestroy(data->imenu);
    free(data->sigDigits);
    free(data->expDigits);
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
    s32 value = charToInt(item->text[0]);
    if (nav == MENU_NAVIGATE_UP) {
        ++value;
    } else if (nav == MENU_NAVIGATE_DOWN) {
        --value;
    } else {
        return 0;
    }
    value = (value + 10) % 10;
    item->text[0] = intToChar(value);
    return 1;
}

struct MenuItem *menuAddFloatinput(struct Menu *menu, s32 x, s32 y, s32 sigPrecis, s32 expPrecis,
                                   MenuGenericCallback callbackProc, void *callbackData) {
    struct ItemData *data = malloc(sizeof(*data));
    data->sigPrecis = sigPrecis;
    data->expPrecis = expPrecis;
    data->callbackProc = callbackProc;
    data->callbackData = callbackData;
    data->value = 0.;
    data->active = FALSE;
    data->sigDigits = malloc(sizeof(*data->sigDigits) * sigPrecis);
    data->expDigits = malloc(sizeof(*data->expDigits) * expPrecis);
    data->imenu = malloc(sizeof(*data->imenu));
    menuInit(data->imenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    data->imenu->parent = menu;
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0xFFFFFF);
    data->item = item;
    item->text = malloc(sigPrecis + expPrecis + 5);
    item->text[1] = '.';
    item->text[1 + sigPrecis] = 'e';
    item->text[sigPrecis + expPrecis + 2] = 0;
    item->data = data;
    item->thinkProc = thinkProc;
    item->drawProc = drawProc;
    item->navigateProc = navigateProc;
    item->activateProc = activateProc;
    item->destroyProc = destroyProc;
    u32 color = data->imenu->highlightColorStatic;
    menuItemAdd(data->imenu, 2, 0, ".", color)->selectable = FALSE;
    menuItemAdd(data->imenu, 2 + sigPrecis, 0, "e", color)->selectable = FALSE;
    for (s32 i = 0; i < 2; ++i) {
        s32 x = i * (3 + sigPrecis);
        struct MenuItem *sign = menuItemAdd(data->imenu, x, 0, NULL, color);
        sign->text = malloc(2);
        sign->text[0] = '+';
        sign->text[1] = 0;
        sign->navigateProc = signNavigateProc;
        sign->animateHighlight = TRUE;
        sign->data = data;
        if (i == 0) {
            data->sigSign = sign;
        } else {
            data->expSign = sign;
        }
    }
    for (s32 i = 0; i < sigPrecis + expPrecis; ++i) {
        s32 x = 1 + i;
        s32 tx = i;
        if (i >= sigPrecis) {
            x += 3;
            tx += 2;
        } else if (i > 0) {
            ++x;
            ++tx;
        }
        struct MenuItem *digit = menuItemAdd(data->imenu, x, 0, NULL, color);
        digit->text = malloc(2);
        digit->text[0] = '0';
        item->text[tx] = '0';
        digit->text[1] = 0;
        digit->navigateProc = digitNavigateProc;
        digit->animateHighlight = TRUE;
        digit->data = data;
        if (i < sigPrecis) {
            data->sigDigits[i] = digit;
        } else {
            data->expDigits[i - sigPrecis] = digit;
        }
    }
    data->imenu->selector = data->sigDigits[0];
    return item;
}

f32 menuFloatinputGet(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->value;
}

void menuFloatinputSet(struct MenuItem *item, f32 value) {
    if (isNan(value) || !isnormal(value)) {
        value = 0.f;
    }
    struct ItemData *data = item->data;
    data->value = value;

    s32 sigSign = signbit(value) ? -1 : 1;
    value = fabsf(value);
    s32 exp = value == 0.f ? 0.f : floorf(log10f(value));
    s32 sig = value / pow(10., exp - (data->sigPrecis - 1)) + 0.5;
    s32 expSign = exp < 0 ? -1 : 1;
    exp *= expSign;
    char *p = data->item->text;

    if (sigSign == 1) {
        data->item->text[0] = data->sigSign->text[0] = '+';
    } else {
        data->item->text[0] = data->sigSign->text[0] = '-';
        *p++ = '-';
    }
    for (s32 i = data->sigPrecis - 1; i >= 0; --i) {
        s32 x = i;
        if (i > 0) {
            ++x;
        }
        s32 c = intToChar(sig % 10);
        sig /= 10;
        data->sigDigits[i]->text[0] = c;
        p[x] = c;
    }
    p[1] = '.';
    p += data->sigPrecis + 1;
    *p++ = 'e';

    if (expSign == 1) {
        data->item->text[3 + data->sigPrecis] = data->expSign->text[0] = '+';
    } else {
        data->item->text[3 + data->sigPrecis] = data->expSign->text[0] = '-';
        *p++ = '-';
    }
    for (s32 i = data->expPrecis - 1; i >= 0; --i) {
        s32 c = intToChar(exp % 10);
        exp /= 10;
        data->expDigits[i]->text[0] = c;
        p[i] = c;
    }
    p[data->expPrecis] = 0;
}
