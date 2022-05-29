#ifndef MENU_H
#define MENU_H
#include <stdint.h>
#include <list/list.h>
#include "gfx.h"

#define MENU_NOVALUE 0

struct menu;
struct menu_item;

enum menu_navigation {
    MENU_NAVIGATE_UP,
    MENU_NAVIGATE_DOWN,
    MENU_NAVIGATE_LEFT,
    MENU_NAVIGATE_RIGHT,
};

enum menu_callback_reason {
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

enum menu_switch_reason {
    MENU_SWITCH_ENTER,
    MENU_SWITCH_RETURN,
    MENU_SWITCH_SHOW,
    MENU_SWITCH_HIDE,
};

struct menu_draw_params {
    s32 x;
    s32 y;
    const char *text;
    struct gfx_font *font;
    u32 color;
    u8 alpha;
};

typedef s32 (*menu_generic_callback)(struct menu_item *item, enum menu_callback_reason reason, void *data);
typedef void (*menu_action_callback)(struct menu_item *item, void *data);
typedef s32 (*menu_prompt_callback)(s32 option_index, void *data);

struct menu_item {
    struct menu *owner;
    _Bool enabled;
    s32 x;
    s32 y;
    s32 pxoffset;
    s32 pyoffset;
    char *text;
    const char *tooltip;
    u32 color;
    _Bool animate_highlight;
    void *data;
    _Bool selectable;
    struct menu *imenu;
    s32 (*enter_proc)(struct menu_item *item, enum menu_switch_reason reason);
    s32 (*leave_proc)(struct menu_item *item, enum menu_switch_reason reason);
    s32 (*think_proc)(struct menu_item *item);
    s32 (*draw_proc)(struct menu_item *item, struct menu_draw_params *draw_params);
    s32 (*navigate_proc)(struct menu_item *item, enum menu_navigation nav);
    s32 (*activate_proc)(struct menu_item *item);
    s32 (*destroy_proc)(struct menu_item *item);
};

struct menu {
    s32 cxoffset;
    s32 cyoffset;
    s32 pxoffset;
    s32 pyoffset;
    s32 cell_width;
    s32 cell_height;
    struct gfx_font *font;
    f32 alpha;
    struct list items;
    struct menu_item *selector;
    struct menu *parent;
    struct menu *child;
    u32 highlight_color_static;
    u32 highlight_color_animated;
    s32 highlight_state[3];
};

enum watch_type {
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

void menu_init(struct menu *menu, s32 cell_width, s32 cell_height, struct gfx_font *font);
void menu_imitate(struct menu *dest, struct menu *src);
void menu_destroy(struct menu *menu);
s32 menu_get_cxoffset(struct menu *menu, _Bool inherit);
void menu_set_cxoffset(struct menu *menu, s32 cxoffset);
s32 menu_get_cyoffset(struct menu *menu, _Bool inherit);
void menu_set_cyoffset(struct menu *menu, s32 cyoffset);
s32 menu_get_pxoffset(struct menu *menu, _Bool inherit);
void menu_set_pxoffset(struct menu *menu, s32 pxoffset);
s32 menu_get_pyoffset(struct menu *menu, _Bool inherit);
void menu_set_pyoffset(struct menu *menu, s32 pyoffset);
s32 menu_get_cell_width(struct menu *menu, _Bool inherit);
void menu_set_cell_width(struct menu *menu, s32 cell_width);
s32 menu_get_cell_height(struct menu *menu, _Bool inherit);
void menu_set_cell_height(struct menu *menu, s32 cell_height);
struct gfx_font *menu_get_font(struct menu *menu, _Bool inherit);
void menu_set_font(struct menu *menu, struct gfx_font *font);
f32 menu_get_alpha(struct menu *menu, _Bool inherit);
u8 menu_get_alpha_i(struct menu *menu, _Bool inherit);
void menu_set_alpha(struct menu *menu, f32 alpha);
s32 menu_cell_screen_x(struct menu *menu, s32 cell_x);
s32 menu_cell_screen_y(struct menu *menu, s32 cell_y);
struct menu_item *menu_get_selector(struct menu *menu);
struct menu *menu_get_top(struct menu *menu);
struct menu *menu_get_front(struct menu *menu);
s32 menu_think(struct menu *menu);
void menu_draw(struct menu *menu);
void menu_navigate(struct menu *menu, enum menu_navigation nav);
void menu_activate(struct menu *menu);
void menu_enter(struct menu *menu, struct menu *submenu);
struct menu *menu_return(struct menu *menu);
void menu_select(struct menu *menu, struct menu_item *item);
void menu_signal_enter(struct menu *menu, enum menu_switch_reason reason);
void menu_signal_leave(struct menu *menu, enum menu_switch_reason reason);
void menu_navigate_top(struct menu *menu, enum menu_navigation nav);
void menu_activate_top(struct menu *menu);
void menu_enter_top(struct menu *menu, struct menu *submenu);
struct menu *menu_return_top(struct menu *menu);
void menu_select_top(struct menu *menu, struct menu_item *item);

struct menu_item *menu_item_add(struct menu *menu, s32 x, s32 y, const char *text, u32 color);
void menu_item_enable(struct menu_item *item);
void menu_item_disable(struct menu_item *item);
void menu_item_transfer(struct menu_item *item, struct menu *menu);
void menu_item_remove(struct menu_item *item);
s32 menu_item_screen_x(struct menu_item *item);
s32 menu_item_screen_y(struct menu_item *item);
struct menu_item *menu_add_static(struct menu *menu, s32 x, s32 y, const char *text, u32 color);
struct menu_item *menu_add_static_icon(struct menu *menu, s32 x, s32 y, struct gfx_texture *texture, s32 texture_tile,
                                       u32 color, f32 scale);
struct menu_item *menu_add_static_custom(struct menu *menu, s32 x, s32 y,
                                         s32 (*draw_proc)(struct menu_item *item, struct menu_draw_params *draw_params),
                                         const char *text, u32 color);
struct menu_item *menu_add_tooltip(struct menu *menu, s32 x, s32 y, struct menu *tool_menu, u32 color);
struct menu_item *menu_add_imenu(struct menu *menu, s32 x, s32 y, struct menu **p_imenu);
struct menu_item *menu_add_tab(struct menu *menu, s32 x, s32 y, struct menu *tabs, s32 n_tabs);
void menu_tab_goto(struct menu_item *item, s32 tab_index);
void menu_tab_previous(struct menu_item *item);
void menu_tab_next(struct menu_item *item);

struct menu_item *menu_add_intinput(struct menu *menu, s32 x, s32 y, s32 base, s32 length,
                                    menu_generic_callback callback_proc, void *callback_data);
u32 menu_intinput_get(struct menu_item *item);
s32 menu_intinput_gets(struct menu_item *item);
void menu_intinput_set(struct menu_item *item, u32 value);
struct menu_item *menu_add_floatinput(struct menu *menu, s32 x, s32 y, s32 sig_precis, s32 exp_precis,
                                      menu_generic_callback callback_proc, void *callback_data);
f32 menu_floatinput_get(struct menu_item *item);
void menu_floatinput_set(struct menu_item *item, f32 value);
struct menu_item *menu_add_strinput(struct menu *menu, s32 x, s32 y, s32 length, menu_generic_callback callback_proc,
                                    void *callback_data);
void menu_strinput_get(struct menu_item *item, char *buf);
void menu_strinput_set(struct menu_item *item, const char *str);
struct menu_item *menu_add_option(struct menu *menu, s32 x, s32 y, const char *options,
                                  menu_generic_callback callback_proc, void *callback_data);
s32 menu_option_get(struct menu_item *item);
void menu_option_set(struct menu_item *item, s32 value);
void menu_prompt(struct menu *menu, const char *prompt, const char *options, s32 default_option,
                 menu_prompt_callback callback_proc, void *callback_data);
struct menu_item *menu_add_watch(struct menu *menu, s32 x, s32 y, u32 address, enum watch_type type);
u32 menu_watch_get_address(struct menu_item *item);
void menu_watch_set_address(struct menu_item *item, u32 address);
enum watch_type menu_watch_get_type(struct menu_item *item);
void menu_watch_set_type(struct menu_item *item, enum watch_type type);
struct menu_item *menu_add_userwatch(struct menu *menu, s32 x, s32 y, u32 address, enum watch_type type);
struct menu_item *menu_userwatch_address(struct menu_item *item);
struct menu_item *menu_userwatch_type(struct menu_item *item);
struct menu_item *menu_userwatch_watch(struct menu_item *item);
struct menu_item *menu_add_submenu(struct menu *menu, s32 x, s32 y, struct menu *submenu, const char *name);
struct menu_item *menu_add_switch(struct menu *menu, s32 x, s32 y, struct gfx_texture *texture_on, s32 texture_tile_on,
                                  u32 color_on, struct gfx_texture *texture_off, s32 texture_tile_off, u32 color_off,
                                  f32 scale, _Bool disable_shadow, menu_generic_callback callback_proc,
                                  void *callback_data);
void menu_switch_set(struct menu_item *item, _Bool state);
_Bool menu_switch_get(struct menu_item *item);
void menu_switch_toggle(struct menu_item *item);
struct menu_item *menu_add_button(struct menu *menu, s32 x, s32 y, const char *name, menu_action_callback callback_proc,
                                  void *callback_data);
struct menu_item *menu_add_button_icon(struct menu *menu, s32 x, s32 y, struct gfx_texture *texture, s32 texture_tile,
                                       u32 color, menu_action_callback callback_proc, void *callback_data);
struct menu_item *menu_add_positioning(struct menu *menu, s32 x, s32 y, menu_generic_callback callback_proc,
                                       void *callback_data);
struct menu_item *menu_add_checkbox(struct menu *menu, s32 x, s32 y, menu_generic_callback callback_proc,
                                    void *callback_data);
_Bool menu_checkbox_get(struct menu_item *item);
void menu_checkbox_set(struct menu_item *item, _Bool state);

#endif
