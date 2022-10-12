#include "input.h"
#include "menu.h"
#include "resource.h"
#include "settings.h"
#include <stdarg.h>
#include <stdlib.h>

static s8 joyX;
static s8 joyY;
static u16 pad;
static s32 buttonTime[16];
static u16 padPressedRaw;
static u16 padPressed;
static u16 padReleased;
static u16 padReserved;
static s32 buttonReserveCount[16];
static s32 bindComponentState[COMMAND_MAX];
static s32 bindTime[COMMAND_MAX];
static bool bindPressedRaw[COMMAND_MAX];
static bool bindPressed[COMMAND_MAX];
static bool bindDisable[COMMAND_MAX];
static bool bindOverride[COMMAND_MAX];
static bool inputEnabled = TRUE;

static s32 bitmaskButtonIndex(u16 bitmask) {
    for (s32 i = 0; i < 16; ++i) {
        if (bitmask & (1 << i)) {
            return i;
        }
    }
    return -1;
}

u16 bindMake(s32 length, ...) {
    u16 bind = 0;
    va_list vl;
    va_start(vl, length);
    for (s32 i = 0; i < length; ++i) {
        bind |= bitmaskButtonIndex(va_arg(vl, int)) << (i * 4);
    }
    va_end(vl);
    if (length < 4) {
        bind |= BIND_END << (length * 4);
    }
    return bind;
}

s32 bindGetComponent(u16 bind, s32 index) {
    return (bind >> (4 * index)) & 0x000F;
}

u16 bindGetBitmask(u16 bind) {
    u16 p = 0;
    for (s32 i = 0; i < 4; ++i) {
        s32 c = bindGetComponent(bind, i);
        if (c == BIND_END) {
            break;
        }
        p |= 1 << c;
    }
    return p;
}

const u32 inputButtonColor[] = {
    0xFFA000, 0xFFA000, 0xFFA000, 0xFFA000, 0xC0C0C0, 0xC0C0C0, 0x000000, 0x000000,
    0xC8C8C8, 0xC8C8C8, 0xC8C8C8, 0xC8C8C8, 0xC80000, 0xC0C0C0, 0x009600, 0x5A5AFF,
};

void inputUpdate(void) {
    OSContPad *cont = &pm_D_8009A5B8;
    nuContDataGet(cont, 0);

    joyX = cont->stick_x;
    joyY = cont->stick_y;
    if (joyX > 0) {
        joyX -= 4;
        if (joyX < 0) {
            joyX = 0;
        }
    }
    if (joyX < 0) {
        joyX += 4;
        if (joyX > 0) {
            joyX = 0;
        }
    }
    if (joyY > 0) {
        joyY -= 4;
        if (joyY < 0) {
            joyY = 0;
        }
    }
    if (joyY < 0) {
        joyY += 4;
        if (joyY > 0) {
            joyY = 0;
        }
    }

    u16 pmPad = cont->button;
    padPressedRaw = (pad ^ pmPad) & pmPad;
    padReleased = (pad ^ pmPad) & ~pmPad;
    pad = pmPad;
    padPressed = 0;
    for (s32 i = 0; i < 16; ++i) {
        u16 p = 1 << i;
        if (pad & p) {
            ++buttonTime[i];
        } else {
            buttonTime[i] = 0;
        }
        if ((padPressedRaw & p) || buttonTime[i] >= INPUT_REPEAT_DELAY) {
            padPressed |= p;
        }
    }
    u16 bindPad[SETTINGS_BIND_MAX];
    bool bindState[SETTINGS_BIND_MAX];
    for (s32 i = 0; i < SETTINGS_BIND_MAX; ++i) {
        u16 *b = &settings->binds[i];
        bindPad[i] = bindGetBitmask(*b);
        s32 *cs = &bindComponentState[i];
        s32 j;
        u16 c;
        if (!inputEnabled || bindDisable[i] || (!bindOverride[i] && (padReserved & bindPad[i]))) {
            *cs = 0;
        } else {
            s32 css = *cs;
            for (j = 0; j < 4; ++j) {
                c = bindGetComponent(*b, j);
                if (c == BIND_END) {
                    break;
                }
                u8 csm = 1 << j;
                if (*cs & csm) {
                    if (pad & (1 << c)) {
                        continue;
                    } else {
                        if (*cs & ~((1 << (j + 1)) - 1)) {
                            *cs = 0;
                        } else {
                            *cs &= ~csm;
                        }
                        break;
                    }
                }
                if ((padReleased & (1 << c)) || (css != 0 && (padPressedRaw & ~bindPad[i]))) {
                    *cs = 0;
                    break;
                } else if (padPressedRaw & (1 << c)) {
                    *cs |= csm;
                } else {
                    break;
                }
            }
        }
        bindState[i] = (*cs && (j == 4 || c == BIND_END));
    }
    for (s32 i = 0; i < SETTINGS_BIND_MAX; ++i) {
        u16 pi = bindPad[i];
        for (s32 j = 0; bindState[i] && j < SETTINGS_BIND_MAX; ++j) {
            if (!bindState[j]) {
                continue;
            }
            u16 pj = bindPad[j];
            if (pi != pj && (pi & pj) == pi) {
                bindComponentState[i] = 0;
                bindState[i] = FALSE;
            }
        }
        bindPressedRaw[i] = (bindTime[i] == 0 && bindState[i]);
        if (!bindState[i]) {
            bindTime[i] = 0;
        } else {
            ++bindTime[i];
        }
        bindPressed[i] = (bindPressedRaw[i] || bindTime[i] >= INPUT_REPEAT_DELAY);
    }
}

s8 inputX(void) {
    return joyX;
}

s8 inputY(void) {
    return joyY;
}

u16 inputPad(void) {
    if (inputEnabled) {
        return pad;
    } else {
        return 0;
    }
}

u16 inputPressedRaw(void) {
    if (inputEnabled) {
        return padPressedRaw;
    } else {
        return 0;
    }
}

u16 inputPressed(void) {
    if (inputEnabled) {
        return padPressed;
    } else {
        return 0;
    }
}

u16 inputReleased(void) {
    if (inputEnabled) {
        return padReleased;
    } else {
        return 0;
    }
}

void inputReserve(u16 bitmask) {
    for (s32 i = 0; i < 16; ++i) {
        if (bitmask & (1 << i)) {
            ++buttonReserveCount[i];
        }
    }
    padReserved |= bitmask;
}

void inputFree(u16 bitmask) {
    for (s32 i = 0; i < 16; ++i) {
        u16 b = 1 << i;
        if (bitmask & b) {
            --buttonReserveCount[i];
            if (buttonReserveCount[i] == 0) {
                padReserved &= ~b;
            }
        }
    }
}

void inputBindSetDisable(s32 index, bool value) {
    bindDisable[index] = value;
}

void inputBindSetOverride(s32 index, bool value) {
    bindOverride[index] = value;
}

bool inputBindHeld(s32 index) {
    return bindTime[index] > 0;
}

bool inputBindPressedRaw(s32 index) {
    return bindPressedRaw[index];
}

bool inputBindPressed(s32 index) {
    return bindPressed[index];
}

struct ItemData {
    s32 bindIndex;
    s32 state;
};

static s32 thinkProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    u16 *b = &settings->binds[data->bindIndex];
    if (data->state == 1) {
        if (!pad) {
            data->state = 2;
        } else if (buttonTime[bitmaskButtonIndex(BUTTON_L)] >= INPUT_REPEAT_DELAY) {
            *b = bindMake(0);
            item->animateHighlight = FALSE;
            data->state = 0;
            inputEnabled = TRUE;
        }
    }
    if (data->state == 2) {
        if (pad) {
            *b = bindMake(0);
            data->state = 3;
        }
    }
    if (data->state == 3) {
        u16 p = bindGetBitmask(*b);
        if (pad == 0) {
            item->animateHighlight = FALSE;
            data->state = 0;
            inputEnabled = TRUE;
        } else {
            u16 pp = padPressedRaw & ~p;
            for (s32 i = 0; pp && i < 4; ++i) {
                s32 c = bindGetComponent(*b, i);
                if (c != BIND_END) {
                    continue;
                }
                c = bitmaskButtonIndex(pp);
                *b = (*b & ~(0x000F << (i * 4))) | (c << (i * 4));
                if (i < 3) {
                    *b = (*b & ~(0x000F << ((i + 1) * 4))) | (BIND_END << ((i + 1) * 4));
                }
                pp &= ~(1 << c);
            }
        }
    }
    return 0;
}

static s32 drawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    struct ItemData *data = item->data;
    struct GfxTexture *texture = resourceGet(RES_ICON_BUTTONS);
    s32 cw = menuGetCellWidth(item->owner, TRUE);
    s32 x = drawParams->x + (cw - texture->tileWidth) / 2;
    s32 y = drawParams->y - (gfxFontXheight(drawParams->font) + texture->tileHeight + 1) / 2;
    u16 b = settings->binds[data->bindIndex];
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    for (s32 i = 0; i < 4; ++i) {
        u16 c = bindGetComponent(b, i);
        if (c == BIND_END) {
            if (i == 0) {
                gfxPrintf(drawParams->font, drawParams->x, drawParams->y, "unbound");
            }
            break;
        }
        struct GfxSprite sprite = {
            texture, c, 0, x + i * 10, y, 1.f, 1.f,
        };
        if (item->owner->selector != item) {
            gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(inputButtonColor[c], drawParams->alpha));
        }
        gfxSpriteDraw(&sprite);
    }
    return 1;
}

static s32 activateProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    if (data->state == 0) {
        item->animateHighlight = TRUE;
        data->state = 1;
        inputEnabled = FALSE;
    }
    return 1;
}

struct MenuItem *binderCreate(struct Menu *menu, s32 x, s32 y, s32 bindIndex) {
    struct ItemData *data = malloc(sizeof(*data));
    data->bindIndex = bindIndex;
    data->state = 0;
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0xFFFFFF);
    item->data = data;
    item->text = malloc(12);
    item->thinkProc = thinkProc;
    item->drawProc = drawProc;
    item->activateProc = activateProc;
    return item;
}
