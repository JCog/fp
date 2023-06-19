#include "osk.h"
#include "menu/menu.h"
#include "sys/input.h"
#include "sys/resource.h"

static char oskBuf[32];
static OskCallback oskCallbackProc;
static void *oskCallbackData;
static struct Menu oskMenu;
static struct MenuItem *oskText;
static struct MenuItem *oskLetters[26];
static struct MenuItem *oskMinus;
static s32 oskCursorPos;
static bool oskShiftState;

static void insertChar(char c) {
    if (oskCursorPos < sizeof(oskBuf) - 1) {
        memmove(&oskBuf[oskCursorPos + 1], &oskBuf[oskCursorPos], sizeof(oskBuf) - oskCursorPos - 1);
        oskBuf[sizeof(oskBuf) - 1] = 0;
        oskBuf[oskCursorPos++] = c;
    }
}

static void keyProc(struct MenuItem *item, void *data) {
    insertChar(item->text[0]);
}

static void spaceProc(struct MenuItem *item, void *data) {
    insertChar(' ');
}

static s32 navigateProc(struct MenuItem *item, enum MenuNavigation nav) {
    if (inputPad() & BUTTON_Z) {
        switch (nav) {
            case MENU_NAVIGATE_UP: {
                oskShiftState = !oskShiftState;
                for (s32 i = 0; i < 26; ++i) {
                    oskLetters[i]->text[0] += (oskShiftState ? 'A' - 'a' : 'a' - 'A');
                }
                oskMinus->text[0] = (oskShiftState ? '_' : '-');
                break;
            }
            case MENU_NAVIGATE_DOWN: {
                insertChar(' ');
                break;
            }
            case MENU_NAVIGATE_LEFT: {
                if (oskCursorPos > 0) {
                    memmove(&oskBuf[oskCursorPos - 1], &oskBuf[oskCursorPos], sizeof(oskBuf) - oskCursorPos);
                    --oskCursorPos;
                }
                break;
            }
            case MENU_NAVIGATE_RIGHT: {
                if (oskCursorPos < sizeof(oskBuf)) {
                    memmove(&oskBuf[oskCursorPos], &oskBuf[oskCursorPos + 1], sizeof(oskBuf) - oskCursorPos - 1);
                }
                break;
            }
        }
        return 1;
    } else {
        return 0;
    }
}

static s32 activateTextProc(struct MenuItem *item) {
    if (!oskCallbackProc || !oskCallbackProc(oskBuf, oskCallbackData)) {
        menuReturn(&oskMenu);
    }
    return 1;
}

static s32 navigateTextProc(struct MenuItem *item, enum MenuNavigation nav) {
    if (navigateProc(item, nav)) {
        return 1;
    } else {
        switch (nav) {
            case MENU_NAVIGATE_LEFT: {
                if (oskCursorPos > 0) {
                    --oskCursorPos;
                }
                return 1;
            }
            case MENU_NAVIGATE_RIGHT: {
                if (oskCursorPos < strlen(oskBuf)) {
                    ++oskCursorPos;
                }
                return 1;
            }
            default: return 0;
        }
    }
}

static s32 drawTextProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    s32 cw = menuGetCellWidth(item->owner, TRUE);
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    gfxPrintf(drawParams->font, drawParams->x, drawParams->y, "%s", oskBuf);
    gfxPrintf(drawParams->font, drawParams->x + oskCursorPos * cw, drawParams->y + 3, "_");
    return 0;
}

static void drawShortcut(struct MenuItem *item, struct MenuDrawParams *drawParams, u16 bind, const char *desc) {
    struct GfxTexture *texture = resourceGet(RES_ICON_BUTTONS);
    s32 cw = menuGetCellWidth(item->owner, TRUE);
    s32 x = drawParams->x + (cw - texture->tileWidth) / 2;
    s32 y = drawParams->y - (gfxFontXheight(drawParams->font) + texture->tileHeight + 1) / 2;

    s32 n;
    for (n = 0; n < 4; ++n) {
        u16 c = bindGetComponent(bind, n);
        if (c == BIND_END) {
            break;
        }
        struct GfxSprite sprite = {
            texture, c, 0, x + n * 10, y, 1.f, 1.f,
        };
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(inputButtonColor[c], drawParams->alpha));
        gfxSpriteDraw(&sprite);
    }

    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    gfxPrintf(drawParams->font, drawParams->x + (n + 1) * 10, drawParams->y, "%s", desc);
}

static s32 drawTooltipProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    s32 ch = menuGetCellHeight(item->owner, TRUE);

    drawShortcut(item, drawParams, bindMake(2, BUTTON_Z, BUTTON_D_UP), "shift");
    drawParams->y += ch;
    drawShortcut(item, drawParams, bindMake(2, BUTTON_Z, BUTTON_D_LEFT), "backspace");
    drawParams->y += ch;
    drawShortcut(item, drawParams, bindMake(2, BUTTON_Z, BUTTON_D_DOWN), "space");
    drawParams->y += ch;
    drawShortcut(item, drawParams, bindMake(2, BUTTON_Z, BUTTON_D_RIGHT), "delete");

    if (item->owner->selector == oskText) {
        drawParams->y += ch;
        drawShortcut(item, drawParams, bindMake(1, BUTTON_D_LEFT), "cursor left");
        drawParams->y += ch;
        drawShortcut(item, drawParams, bindMake(1, BUTTON_D_RIGHT), "cursor right");
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
        drawParams->y += ch;
        gfxPrintf(drawParams->font, drawParams->x, drawParams->y, "press to confirm");
    }

    return 1;
}

static void createOskMenu(void) {
    static bool ready = FALSE;
    if (!ready) {
        menuInit(&oskMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

        oskText = menuItemAdd(&oskMenu, 0, 0, NULL, 0xFFFFFF);
        oskMenu.selector = oskText;
        oskText->activateProc = activateTextProc;
        oskText->drawProc = drawTextProc;
        oskText->navigateProc = navigateTextProc;

        struct MenuItem *item;
        item = menuAddButton(&oskMenu, 0, 2, "1", keyProc, NULL);
        item->navigateProc = navigateProc;
        item = menuAddButton(&oskMenu, 2, 2, "2", keyProc, NULL);
        item->navigateProc = navigateProc;
        item = menuAddButton(&oskMenu, 4, 2, "3", keyProc, NULL);
        item->navigateProc = navigateProc;
        item = menuAddButton(&oskMenu, 6, 2, "4", keyProc, NULL);
        item->navigateProc = navigateProc;
        item = menuAddButton(&oskMenu, 8, 2, "5", keyProc, NULL);
        item->navigateProc = navigateProc;
        item = menuAddButton(&oskMenu, 10, 2, "6", keyProc, NULL);
        item->navigateProc = navigateProc;
        item = menuAddButton(&oskMenu, 12, 2, "7", keyProc, NULL);
        item->navigateProc = navigateProc;
        item = menuAddButton(&oskMenu, 14, 2, "8", keyProc, NULL);
        item->navigateProc = navigateProc;
        item = menuAddButton(&oskMenu, 16, 2, "9", keyProc, NULL);
        item->navigateProc = navigateProc;
        item = menuAddButton(&oskMenu, 18, 2, "0", keyProc, NULL);
        item->navigateProc = navigateProc;

        oskLetters[0] = menuAddButton(&oskMenu, 0, 4, "q", keyProc, NULL);
        oskLetters[1] = menuAddButton(&oskMenu, 2, 4, "w", keyProc, NULL);
        oskLetters[2] = menuAddButton(&oskMenu, 4, 4, "e", keyProc, NULL);
        oskLetters[3] = menuAddButton(&oskMenu, 6, 4, "r", keyProc, NULL);
        oskLetters[4] = menuAddButton(&oskMenu, 8, 4, "t", keyProc, NULL);
        oskLetters[5] = menuAddButton(&oskMenu, 10, 4, "y", keyProc, NULL);
        oskLetters[6] = menuAddButton(&oskMenu, 12, 4, "u", keyProc, NULL);
        oskLetters[7] = menuAddButton(&oskMenu, 14, 4, "i", keyProc, NULL);
        oskLetters[8] = menuAddButton(&oskMenu, 16, 4, "o", keyProc, NULL);
        oskLetters[9] = menuAddButton(&oskMenu, 18, 4, "p", keyProc, NULL);

        oskLetters[10] = menuAddButton(&oskMenu, 0, 6, "a", keyProc, NULL);
        oskLetters[11] = menuAddButton(&oskMenu, 2, 6, "s", keyProc, NULL);
        oskLetters[12] = menuAddButton(&oskMenu, 4, 6, "d", keyProc, NULL);
        oskLetters[13] = menuAddButton(&oskMenu, 6, 6, "f", keyProc, NULL);
        oskLetters[14] = menuAddButton(&oskMenu, 8, 6, "g", keyProc, NULL);
        oskLetters[15] = menuAddButton(&oskMenu, 10, 6, "h", keyProc, NULL);
        oskLetters[16] = menuAddButton(&oskMenu, 12, 6, "j", keyProc, NULL);
        oskLetters[17] = menuAddButton(&oskMenu, 14, 6, "k", keyProc, NULL);
        oskLetters[18] = menuAddButton(&oskMenu, 16, 6, "l", keyProc, NULL);
        oskMinus = menuAddButton(&oskMenu, 18, 6, "-", keyProc, NULL);
        oskMinus->navigateProc = navigateProc;

        oskLetters[19] = menuAddButton(&oskMenu, 2, 8, "z", keyProc, NULL);
        oskLetters[20] = menuAddButton(&oskMenu, 4, 8, "x", keyProc, NULL);
        oskLetters[21] = menuAddButton(&oskMenu, 6, 8, "c", keyProc, NULL);
        oskLetters[22] = menuAddButton(&oskMenu, 8, 8, "v", keyProc, NULL);
        oskLetters[23] = menuAddButton(&oskMenu, 10, 8, "b", keyProc, NULL);
        oskLetters[24] = menuAddButton(&oskMenu, 12, 8, "n", keyProc, NULL);
        oskLetters[25] = menuAddButton(&oskMenu, 14, 8, "m", keyProc, NULL);
        item = menuAddButton(&oskMenu, 16, 8, ",", keyProc, NULL);
        item->navigateProc = navigateProc;
        item = menuAddButton(&oskMenu, 18, 8, ".", keyProc, NULL);
        item->navigateProc = navigateProc;

        for (s32 i = 0; i < 26; ++i) {
            oskLetters[i]->navigateProc = navigateProc;
        }

        struct GfxTexture *oskIcons = resourceGet(RES_ICON_OSK);
        item = menuAddButtonIcon(&oskMenu, 0, 8, oskIcons, 0, 0, 0xFFFFFF, 1.0f, spaceProc, NULL);
        item->navigateProc = navigateProc;

        item = menuItemAdd(&oskMenu, 0, 10, NULL, 0xC0C0C0);
        item->drawProc = drawTooltipProc;
        item->selectable = FALSE;

        ready = TRUE;
    }
}

void menuGetOskString(struct Menu *menu, const char *dflt, OskCallback callbackProc, void *callbackData) {
    memset(oskBuf, 0, sizeof(oskBuf));
    if (dflt) {
        strncpy(oskBuf, dflt, sizeof(oskBuf) - 1);
    }
    oskCallbackProc = callbackProc;
    oskCallbackData = callbackData;
    oskCursorPos = 0;
    createOskMenu();
    menuEnter(menu, &oskMenu);
}
