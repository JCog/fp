#ifndef MENU_H
#define MENU_H
#include "common.h"
#include "sys/gfx.h"
#include <list/list.h>

#define MENU_NOVALUE 0

struct Menu;
struct MenuItem;

enum MenuNavigation {
    MENU_NAVIGATE_UP,
    MENU_NAVIGATE_DOWN,
    MENU_NAVIGATE_LEFT,
    MENU_NAVIGATE_RIGHT,
};

enum MenuCallbackReason {
    MENU_CALLBACK_THINK,
    MENU_CALLBACK_THINK_ACTIVE,
    MENU_CALLBACK_THINK_INACTIVE,
    MENU_CALLBACK_ACTIVATE,
    MENU_CALLBACK_DEACTIVATE,
    MENU_CALLBACK_SWITCH_ON,
    MENU_CALLBACK_SWITCH_OFF,
    MENU_CALLBACK_NAV_UP,
    MENU_CALLBACK_NAV_DOWN,
    MENU_CALLBACK_NAV_LEFT,
    MENU_CALLBACK_NAV_RIGHT,
    MENU_CALLBACK_CHANGED,
};

enum MenuSwitchReason {
    MENU_SWITCH_ENTER,
    MENU_SWITCH_RETURN,
    MENU_SWITCH_SHOW,
    MENU_SWITCH_HIDE,
};

struct MenuDrawParams {
    s32 x;
    s32 y;
    const char *text;
    struct GfxFont *font;
    u32 color;
    u8 alpha;
};

typedef s32 (*MenuGenericCallback)(struct MenuItem *item, enum MenuCallbackReason reason, void *data);
typedef void (*MenuActionCallback)(struct MenuItem *item, void *data);
typedef s32 (*MenuPromptCallback)(s32 optionIndex, void *data);

struct MenuItem {
    struct Menu *owner;
    bool enabled;
    s32 x;
    s32 y;
    s32 pxoffset;
    s32 pyoffset;
    char *text;
    const char *tooltip;
    u32 color;
    bool animateHighlight;
    void *data;
    bool selectable;
    struct Menu *imenu;
    struct MenuItem *chainLinks[4];
    s32 (*enterProc)(struct MenuItem *item, enum MenuSwitchReason reason);
    s32 (*leaveProc)(struct MenuItem *item, enum MenuSwitchReason reason);
    s32 (*thinkProc)(struct MenuItem *item);
    s32 (*drawProc)(struct MenuItem *item, struct MenuDrawParams *drawParams);
    s32 (*navigateProc)(struct MenuItem *item, enum MenuNavigation nav);
    s32 (*activateProc)(struct MenuItem *item);
    s32 (*destroyProc)(struct MenuItem *item);
};

struct Menu {
    s32 cxoffset;
    s32 cyoffset;
    s32 pxoffset;
    s32 pyoffset;
    s32 cellWidth;
    s32 cellHeight;
    struct GfxFont *font;
    f32 alpha;
    struct list items;
    struct MenuItem *selector;
    struct Menu *parent;
    struct Menu *child;
    u32 highlightColorStatic;
    u32 highlightColorAnimated;
    s32 highlightState[3];
};

enum WatchType {
    WATCH_TYPE_U8,
    WATCH_TYPE_S8,
    WATCH_TYPE_X8,
    WATCH_TYPE_U16,
    WATCH_TYPE_S16,
    WATCH_TYPE_X16,
    WATCH_TYPE_U32,
    WATCH_TYPE_S32,
    WATCH_TYPE_X32,
    WATCH_TYPE_F32,
    WATCH_TYPE_MAX,
};

void menuInit(struct Menu *menu, s32 cellWidth, s32 cellHeight, struct GfxFont *font);
void menuImitate(struct Menu *dest, struct Menu *src);
void menuDestroy(struct Menu *menu);
s32 menuGetCxoffset(struct Menu *menu, bool inherit);
void menuSetCxoffset(struct Menu *menu, s32 cxoffset);
s32 menuGetCyoffset(struct Menu *menu, bool inherit);
void menuSetCyoffset(struct Menu *menu, s32 cyoffset);
s32 menuGetPxoffset(struct Menu *menu, bool inherit);
void menuSetPxoffset(struct Menu *menu, s32 pxoffset);
s32 menuGetPyoffset(struct Menu *menu, bool inherit);
void menuSetPyoffset(struct Menu *menu, s32 pyoffset);
s32 menuGetCellWidth(struct Menu *menu, bool inherit);
void menuSetCellWidth(struct Menu *menu, s32 cellWidth);
s32 menuGetCellHeight(struct Menu *menu, bool inherit);
void menuSetCellHeight(struct Menu *menu, s32 cellHeight);
struct GfxFont *menuGetFont(struct Menu *menu, bool inherit);
void menuSetFont(struct Menu *menu, struct GfxFont *font);
f32 menuGetAlpha(struct Menu *menu, bool inherit);
u8 menuGetAlphaI(struct Menu *menu, bool inherit);
void menuSetAlpha(struct Menu *menu, f32 alpha);
s32 menuCellScreenX(struct Menu *menu, s32 cellX);
s32 menuCellScreenY(struct Menu *menu, s32 cellY);
struct MenuItem *menuGetSelector(struct Menu *menu);
struct Menu *menuGetTop(struct Menu *menu);
struct Menu *menuGetFront(struct Menu *menu);
s32 menuThink(struct Menu *menu);
void menuDraw(struct Menu *menu);
void menuNavigate(struct Menu *menu, enum MenuNavigation nav);
void menuActivate(struct Menu *menu);
void menuEnter(struct Menu *menu, struct Menu *submenu);
struct Menu *menuReturn(struct Menu *menu);
void menuSelect(struct Menu *menu, struct MenuItem *item);
void menuSignalEnter(struct Menu *menu, enum MenuSwitchReason reason);
void menuSignalLeave(struct Menu *menu, enum MenuSwitchReason reason);
void menuNavigateTop(struct Menu *menu, enum MenuNavigation nav);
void menuActivateTop(struct Menu *menu);
void menuEnterTop(struct Menu *menu, struct Menu *submenu);
struct Menu *menuReturnTop(struct Menu *menu);
void menuSelectTop(struct Menu *menu, struct MenuItem *item);

struct MenuItem *menuItemAdd(struct Menu *menu, s32 x, s32 y, const char *text, u32 color);
void menuItemEnable(struct MenuItem *item);
void menuItemDisable(struct MenuItem *item);
void menuItemTransfer(struct MenuItem *item, struct Menu *menu);
void menuItemRemove(struct MenuItem *item);
void menuItemAddChainLink(struct MenuItem *fromItem, struct MenuItem *toItem, enum MenuNavigation direction);
void menuItemCreateChain(struct MenuItem *items[], s32 itemsSize, enum MenuNavigation navDirection, bool loop,
                         bool reverseChain);
s32 menuItemScreenX(struct MenuItem *item);
s32 menuItemScreenY(struct MenuItem *item);
struct MenuItem *menuAddStatic(struct Menu *menu, s32 x, s32 y, const char *text, u32 color);
struct MenuItem *menuAddStaticIcon(struct Menu *menu, s32 x, s32 y, struct GfxTexture *texture, s32 textureTile,
                                   u32 color, f32 scale);
struct MenuItem *menuAddStaticCustom(struct Menu *menu, s32 x, s32 y,
                                     s32 (*drawProc)(struct MenuItem *item, struct MenuDrawParams *drawParams),
                                     const char *text, u32 color);
struct MenuItem *menuAddTooltip(struct Menu *menu, s32 x, s32 y, struct Menu *toolMenu, u32 color);
struct MenuItem *menuAddImenu(struct Menu *menu, s32 x, s32 y, struct Menu **pImenu);
struct MenuItem *menuAddTab(struct Menu *menu, s32 x, s32 y, struct Menu *tabs, s32 nTabs);
void menuTabGoto(struct MenuItem *item, s32 tabIndex);
void menuTabPrevious(struct MenuItem *item);
void menuTabNext(struct MenuItem *item);
s32 menuTabGetCurrentTab(struct MenuItem *item);

struct MenuItem *menuAddIntinput(struct Menu *menu, s32 x, s32 y, s32 base, s32 length,
                                 MenuGenericCallback callbackProc, void *callbackData);
u32 menuIntinputGet(struct MenuItem *item);
s32 menuIntinputGets(struct MenuItem *item);
void menuIntinputSet(struct MenuItem *item, u32 value);
struct MenuItem *menuAddFloatinput(struct Menu *menu, s32 x, s32 y, s32 sigPrecis, s32 expPrecis,
                                   MenuGenericCallback callbackProc, void *callbackData);
f32 menuFloatinputGet(struct MenuItem *item);
void menuFloatinputSet(struct MenuItem *item, f32 value);
struct MenuItem *menuAddStrinput(struct Menu *menu, s32 x, s32 y, s32 length, MenuGenericCallback callbackProc,
                                 void *callbackData);
void menuStrinputGet(struct MenuItem *item, char *buf);
void menuStrinputSet(struct MenuItem *item, const char *str);
struct MenuItem *menuAddOption(struct Menu *menu, s32 x, s32 y, const char *options, MenuGenericCallback callbackProc,
                               void *callbackData);
s32 menuOptionGet(struct MenuItem *item);
void menuOptionSet(struct MenuItem *item, s32 value);
void menuPrompt(struct Menu *menu, const char *prompt, const char *options, s32 defaultOption,
                MenuPromptCallback callbackProc, void *callbackData);
struct MenuItem *menuAddWatch(struct Menu *menu, s32 x, s32 y, u32 address, enum WatchType type);
u32 menuWatchGetAddress(struct MenuItem *item);
void menuWatchSetAddress(struct MenuItem *item, u32 address);
enum WatchType menuWatchGetType(struct MenuItem *item);
void menuWatchSetType(struct MenuItem *item, enum WatchType type);
struct MenuItem *menuAddUserwatch(struct Menu *menu, s32 x, s32 y, u32 address, enum WatchType type);
struct MenuItem *menuUserwatchAddress(struct MenuItem *item);
struct MenuItem *menuUserwatchType(struct MenuItem *item);
struct MenuItem *menuUserwatchWatch(struct MenuItem *item);
struct MenuItem *menuAddSubmenu(struct Menu *menu, s32 x, s32 y, struct Menu *submenu, const char *name);
struct MenuItem *menuAddSubmenuIcon(struct Menu *menu, s32 x, s32 y, struct Menu *submenu, struct GfxTexture *texture,
                                    s32 textureTile, s8 texturePalette, f32 scale);
struct MenuItem *menuAddSwitch(struct Menu *menu, s32 x, s32 y, struct GfxTexture *textureOn, s32 textureTileOn,
                               s8 texturePaletteOn, u32 colorOn, struct GfxTexture *textureOff, s32 textureTileOff,
                               s8 texturePaletteOff, u32 colorOff, f32 scale, bool disableShadow,
                               MenuGenericCallback callbackProc, void *callbackData);
void menuSwitchSet(struct MenuItem *item, bool state);
bool menuSwitchGet(struct MenuItem *item);
void menuSwitchToggle(struct MenuItem *item);
struct MenuItem *menuAddButton(struct Menu *menu, s32 x, s32 y, const char *name, MenuActionCallback callbackProc,
                               void *callbackData);
struct MenuItem *menuAddButtonIcon(struct Menu *menu, s32 x, s32 y, struct GfxTexture *texture, s32 textureTile,
                                   s8 texturePalette, u32 color, f32 scale, MenuActionCallback callbackProc,
                                   void *callbackData);
struct MenuItem *menuAddPositioning(struct Menu *menu, s32 x, s32 y, MenuGenericCallback callbackProc,
                                    void *callbackData);
struct MenuItem *menuAddCheckbox(struct Menu *menu, s32 x, s32 y, MenuGenericCallback callbackProc, void *callbackData);
bool menuCheckboxGet(struct MenuItem *item);
void menuCheckboxSet(struct MenuItem *item, bool state);
struct MenuItem *menuAddCycle(struct Menu *menu, s32 x, s32 y, s32 cycleCount, struct GfxTexture **textures,
                              s32 *textureTiles, s8 *texturePalettes, u32 *colors, f32 scale, bool disableShadow,
                              MenuGenericCallback callbackProc, void *callbackData);
void menuCycleSet(struct MenuItem *item, s32 state);
s32 menuCycleGet(struct MenuItem *item);

s32 menuByteModProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data);
s32 menuByteOptionmodProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data);
s32 menuByteSwitchToggleProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data);
s32 menuByteCheckboxProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data);
s32 menuByteCycleProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data);
s32 menuHalfwordModProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data);
s32 menuWordOptionmodProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data);
void menuFuncProc(struct MenuItem *item, void *data);
void menuTabPrevProc(struct MenuItem *item, void *data);
void menuTabNextProc(struct MenuItem *item, void *data);

#endif
