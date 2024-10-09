#include "menu.h"
#include <stdlib.h>
#include <string.h>
#include <vector/vector.h>

struct ItemData {
    struct vector options;
    MenuGenericCallback callbackProc;
    void *callbackData;
    s32 value;
    bool active;
};

static s32 thinkProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
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

static s32 navigateProc(struct MenuItem *item, enum MenuNavigation nav) {
    struct ItemData *data = item->data;
    if (data->callbackProc && data->callbackProc(item, MENU_CALLBACK_NAV_UP + nav, data->callbackData)) {
        return 1;
    }
    s32 value = data->value;
    switch (nav) {
        case MENU_NAVIGATE_UP: value += 1; break;
        case MENU_NAVIGATE_DOWN: value -= 1; break;
        case MENU_NAVIGATE_LEFT: value -= 3; break;
        case MENU_NAVIGATE_RIGHT: value += 3; break;
    }
    value %= (s32)data->options.size;
    if (value < 0) {
        value += (s32)data->options.size;
    }
    data->value = value;
    char **option = vector_at(&data->options, data->value);
    item->text = *option;
    if (data->callbackProc) {
        data->callbackProc(item, MENU_CALLBACK_CHANGED, data->callbackData);
    }
    return 1;
};

static s32 activateProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    if (data->active) {
        if (data->callbackProc && data->callbackProc(item, MENU_CALLBACK_DEACTIVATE, data->callbackData)) {
            return 1;
        }
        item->navigateProc = NULL;
        item->animateHighlight = FALSE;
    } else {
        if (data->callbackProc && data->callbackProc(item, MENU_CALLBACK_ACTIVATE, data->callbackData)) {
            return 1;
        }
        item->navigateProc = navigateProc;
        item->animateHighlight = TRUE;
    }
    data->active = !data->active;
    return 1;
}

static s32 destroyProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    item->text = NULL;
    for (size_t i = 0; i < data->options.size; ++i) {
        char **option = vector_at(&data->options, i);
        free(*option);
    }
    vector_destroy(&data->options);
    return 0;
}

struct MenuItem *menuAddOption(struct Menu *menu, s32 x, s32 y, const char *options, MenuGenericCallback callbackProc,
                               void *callbackData) {
    struct ItemData *data = malloc(sizeof(*data));
    vector_init(&data->options, sizeof(char *));
    for (const char *option = options; *option;) {
        size_t optionLength = strlen(option);
        char *newOption = malloc(optionLength + 1);
        strcpy(newOption, option);
        vector_push_back(&data->options, 1, &newOption);
        option += optionLength + 1;
    }
    data->callbackProc = callbackProc;
    data->callbackData = callbackData;
    data->value = 0;
    data->active = FALSE;
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0xFFFFFF);
    char **option = vector_at(&data->options, data->value);
    item->text = *option;
    item->data = data;
    item->thinkProc = thinkProc;
    item->activateProc = activateProc;
    item->destroyProc = destroyProc;
    return item;
}

s32 menuOptionGet(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->value;
}

void menuOptionSet(struct MenuItem *item, s32 value) {
    struct ItemData *data = item->data;
    data->value = value;
    if (value >= (s32)data->options.size) {
        return;
    }
    char **option = vector_at(&data->options, data->value);
    item->text = *option;
}
