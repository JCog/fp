#include "menu.h"
#include "sys/input.h"
#include <stdlib.h>
#include <string.h>

struct ItemData {
    s32 length;
    MenuGenericCallback callbackProc;
    void *callbackData;
    bool active;
    struct Menu *imenu;
    struct MenuItem *item;
    struct MenuItem **chars;
};

static const char charset[] = "_abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "0123456789-.";
static s32 charsetSize = sizeof(charset) - 1;

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
        s32 max = 0;
        for (s32 i = 0; i < data->length; ++i) {
            char c = data->chars[i]->text[0];
            if (c == '_') {
                c = ' ';
            } else {
                max = i + 1;
            }
            data->item->text[i] = c;
        }
        data->item->text[max] = 0;
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
        data->chars[i]->data = NULL;
    }
    menuDestroy(data->imenu);
    free(data->chars);
    return 0;
}

static s32 charNavigateProc(struct MenuItem *item, enum MenuNavigation nav) {
    s32 n = strchr(charset, item->text[0]) - charset;
    s32 d = inputPad().z ? 3 : 1;
    if (nav == MENU_NAVIGATE_UP) {
        n += d;
    } else if (nav == MENU_NAVIGATE_DOWN) {
        n -= d;
    } else {
        return 0;
    }
    n = (n + charsetSize) % charsetSize;
    item->text[0] = charset[n];
    return 1;
}

struct MenuItem *menuAddStrinput(struct Menu *menu, s32 x, s32 y, s32 length, MenuGenericCallback callbackProc,
                                 void *callbackData) {
    struct ItemData *data = malloc(sizeof(*data));
    data->length = length;
    data->callbackProc = callbackProc;
    data->callbackData = callbackData;
    data->active = FALSE;
    data->chars = malloc(sizeof(*data->chars) * length);
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
        struct MenuItem *c = menuItemAdd(data->imenu, i, 0, NULL, data->imenu->highlightColorStatic);
        data->chars[i] = c;
        c->text = malloc(2);
        c->text[0] = '_';
        c->text[1] = 0;
        item->text[i] = ' ';
        c->navigateProc = charNavigateProc;
        c->animateHighlight = TRUE;
        c->data = data;
    }
    data->imenu->selector = data->chars[0];
    return item;
}

void menuStrinputGet(struct MenuItem *item, char *buf) {
    struct ItemData *data = item->data;
    strcpy(buf, data->item->text);
}

void menuStrinputSet(struct MenuItem *item, const char *str) {
    struct ItemData *data = item->data;
    s32 max = 0;
    bool end = FALSE;
    for (s32 i = 0; i < data->length; ++i) {
        char c;
        if (end) {
            c = ' ';
        } else {
            c = str[i];
            if (c == 0) {
                end = TRUE;
                c = ' ';
            } else if (!strchr(charset, c) || c == '_') {
                c = ' ';
            }
        }
        if (c == ' ') {
            data->chars[i]->text[0] = '_';
        } else {
            data->chars[i]->text[0] = c;
            max = i + 1;
        }
        data->item->text[i] = c;
    }
    data->item->text[max] = 0;
}
