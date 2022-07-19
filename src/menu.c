#include "menu.h"
#include "common.h"
#include "gfx.h"
#include <list/list.h>
#include <stdlib.h>
#include <string.h>

void menu_init(struct menu *menu, s32 cell_width, s32 cell_height, struct gfx_font *font) {
    menu->cxoffset = MENU_NOVALUE;
    menu->cyoffset = MENU_NOVALUE;
    menu->pxoffset = MENU_NOVALUE;
    menu->pyoffset = MENU_NOVALUE;
    menu->cell_width = cell_width;
    menu->cell_height = cell_height;
    menu->font = font;
    menu->alpha = 1.f;
    list_init(&menu->items, sizeof(struct menu_item));
    menu->selector = NULL;
    menu->parent = NULL;
    menu->child = NULL;
    menu->highlight_color_animated = 0x000000;
    menu->highlight_color_static = 0x2020FF;
    menu->highlight_state[0] = 17;
    menu->highlight_state[1] = 19;
    menu->highlight_state[2] = 23;
}

void menu_imitate(struct menu *dest, struct menu *src) {
    dest->cell_width = src->cell_width;
    dest->cell_height = src->cell_height;
    dest->font = src->font;
    dest->alpha = src->alpha;
    dest->highlight_color_static = src->highlight_color_static;
}

void menu_destroy(struct menu *menu) {
    while (menu->items.first) {
        menu_item_remove(menu->items.first);
    }
}

s32 menu_get_cxoffset(struct menu *menu, _Bool inherit) {
    s32 cxoffset = menu->cxoffset;
    if (inherit && menu->parent) {
        cxoffset += menu_get_cxoffset(menu->parent, 1);
    }
    return cxoffset;
}

void menu_set_cxoffset(struct menu *menu, s32 cxoffset) {
    menu->cxoffset = cxoffset;
}

s32 menu_get_cyoffset(struct menu *menu, _Bool inherit) {
    s32 cyoffset = menu->cyoffset;
    if (inherit && menu->parent) {
        cyoffset += menu_get_cyoffset(menu->parent, 1);
    }
    return cyoffset;
}

void menu_set_cyoffset(struct menu *menu, s32 cyoffset) {
    menu->cyoffset = cyoffset;
}

s32 menu_get_pxoffset(struct menu *menu, _Bool inherit) {
    s32 pxoffset = menu->pxoffset;
    if (inherit && menu->parent) {
        pxoffset += menu_get_pxoffset(menu->parent, 1);
    }
    return pxoffset;
}

void menu_set_pxoffset(struct menu *menu, s32 pxoffset) {
    menu->pxoffset = pxoffset;
}

s32 menu_get_pyoffset(struct menu *menu, _Bool inherit) {
    s32 pyoffset = menu->pyoffset;
    if (inherit && menu->parent) {
        pyoffset += menu_get_pyoffset(menu->parent, 1);
    }
    return pyoffset;
}

void menu_set_pyoffset(struct menu *menu, s32 pyoffset) {
    menu->pyoffset = pyoffset;
}

s32 menu_get_cell_width(struct menu *menu, _Bool inherit) {
    if (inherit && menu->parent && menu->cell_width == MENU_NOVALUE) {
        return menu_get_cell_width(menu->parent, 1);
    }
    return menu->cell_width;
}

void menu_set_cell_width(struct menu *menu, s32 cell_width) {
    menu->cell_width = cell_width;
}

s32 menu_get_cell_height(struct menu *menu, _Bool inherit) {
    if (inherit && menu->parent && menu->cell_height == MENU_NOVALUE) {
        return menu_get_cell_height(menu->parent, 1);
    }
    return menu->cell_height;
}

void menu_set_cell_height(struct menu *menu, s32 cell_height) {
    menu->cell_height = cell_height;
}

struct gfx_font *menu_get_font(struct menu *menu, _Bool inherit) {
    if (inherit && menu->parent && menu->font == MENU_NOVALUE) {
        return menu_get_font(menu->parent, 1);
    }
    return menu->font;
}

void menu_set_font(struct menu *menu, struct gfx_font *font) {
    menu->font = font;
}

f32 menu_get_alpha(struct menu *menu, _Bool inherit) {
    f32 alpha = menu->alpha;
    if (inherit && menu->parent) {
        alpha *= menu_get_alpha(menu->parent, 1);
    }
    return alpha;
}

u8 menu_get_alpha_i(struct menu *menu, _Bool inherit) {
    f32 alpha = menu_get_alpha(menu, inherit);
    if (alpha < 0.f) {
        alpha = 0.f;
    } else if (alpha > 1.f) {
        alpha = 1.f;
    }
    return alpha * 0xFF;
}

void menu_set_alpha(struct menu *menu, f32 alpha) {
    menu->alpha = alpha;
}

s32 menu_cell_screen_x(struct menu *menu, s32 x) {
    return (x + menu_get_cxoffset(menu, 1)) * menu_get_cell_width(menu, 1) + menu_get_pxoffset(menu, 1);
}

s32 menu_cell_screen_y(struct menu *menu, s32 y) {
    return (y + menu_get_cyoffset(menu, 1)) * menu_get_cell_height(menu, 1) + menu_get_pyoffset(menu, 1);
}

struct menu_item *menu_get_selector(struct menu *menu) {
    if (menu->child) {
        return menu_get_selector(menu->child);
    }
    return menu->selector;
}

struct menu *menu_get_top(struct menu *menu) {
    if (menu->parent) {
        return menu_get_top(menu->parent);
    }
    return menu;
}

struct menu *menu_get_front(struct menu *menu) {
    if (menu->child) {
        return menu_get_front(menu->child);
    }
    return menu;
}

s32 menu_think(struct menu *menu) {
    if (menu->child) {
        return menu_think(menu->child);
    }
    for (struct menu_item *item = menu->items.first; item; item = list_next(item)) {
        if (!item->enabled) {
            continue;
        }
        if (item->think_proc && item->think_proc(item)) {
            return 1;
        }
        if (item->imenu && menu_think(item->imenu)) {
            return 1;
        }
    }
    return 0;
}

void menu_draw(struct menu *menu) {
    if (menu->child) {
        return menu_draw(menu->child);
    }
    for (s32 i = 0; i < 3; ++i) {
        s32 shift = i * 8;
        u32 mask = 0xFF << shift;
        s32 v = (menu->highlight_color_animated & mask) >> shift;
        v += menu->highlight_state[i];
        if (v < 0x00 || v > 0xFF) {
            v = -v + (v > 0xFF ? 2 * 0xFF : 0);
            menu->highlight_state[i] = -menu->highlight_state[i];
        }
        menu->highlight_color_animated &= ~mask;
        menu->highlight_color_animated |= (u32)v << shift;
    }
    struct gfx_font *font = menu_get_font(menu, 1);
    u8 alpha = menu_get_alpha_i(menu, 1);
    for (struct menu_item *item = menu->items.first; item; item = list_next(item)) {
        if (!item->enabled) {
            continue;
        }
        struct menu_draw_params draw_params = {
            menu_item_screen_x(item),
            menu_item_screen_y(item),
            item->text,
            font,
            (item == menu->selector
                 ? (item->animate_highlight ? menu->highlight_color_animated : menu->highlight_color_static)
                 : item->color),
            alpha,
        };
        if (item->draw_proc && item->draw_proc(item, &draw_params)) {
            continue;
        }
        if (item->imenu) {
            menu_draw(item->imenu);
        }
        if (!draw_params.text || !draw_params.font) {
            continue;
        }
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params.color, draw_params.alpha));
        gfx_printf(draw_params.font, draw_params.x, draw_params.y, "%s", draw_params.text);
    }
}

static void menu_nav_compare(struct menu *menu, struct menu_item *selector, enum menu_navigation nav, s32 nav_x,
                             s32 nav_y, struct menu_item **near_item, struct menu_item **far_item, s32 *npa, s32 *fpa,
                             s32 *npe, s32 *fpe) {
    if (menu->child) {
        return menu_nav_compare(menu->child, selector, nav, nav_x, nav_y, near_item, far_item, npa, fpa, npe, fpe);
    }
    s32 sel_x = selector ? menu_item_screen_x(selector) : 0;
    s32 sel_y = selector ? menu_item_screen_y(selector) : 0;
    for (struct menu_item *item = menu->items.first; item; item = list_next(item)) {
        if (!item->enabled) {
            continue;
        }
        if (item->imenu) {
            menu_nav_compare(item->imenu, selector, nav, nav_x, nav_y, near_item, far_item, npa, fpa, npe, fpe);
        }
        if (item == menu->selector || !item->selectable) {
            continue;
        }

        if (selector && selector->chain_links[nav] == item) {
            *near_item = item;
            *npe = 0;
            return;
        }
        s32 distance_x = menu_item_screen_x(item) - sel_x;
        s32 distance_y = menu_item_screen_y(item) - sel_y;
        s32 distance_pa = (nav_x ? distance_x * nav_x : distance_y * nav_y);
        s32 distance_pe = (nav_y ? distance_x : distance_y);
        if (distance_pe < 0) {
            distance_pe = -distance_pe;
        }
        if (distance_pa > 0 &&
            (*near_item == NULL || distance_pe < *npe || (distance_pe == *npe && distance_pa < *npa))) {
            *npa = distance_pa;
            *npe = distance_pe;
            *near_item = item;
        }
        if (distance_pa < 0 &&
            (*far_item == NULL || -distance_pa > *fpa || (-distance_pa == *fpa && distance_pe < *fpe))) {
            *fpa = -distance_pa;
            *fpe = distance_pe;
            *far_item = item;
        }
    }
}

void menu_navigate(struct menu *menu, enum menu_navigation nav) {
    if (menu->child) {
        return menu_navigate(menu->child, nav);
    }
    if (menu->selector && menu->selector->navigate_proc && menu->selector->navigate_proc(menu->selector, nav)) {
        return;
    }
    s32 nav_x = (nav == MENU_NAVIGATE_LEFT ? -1 : (nav == MENU_NAVIGATE_RIGHT ? 1 : 0));
    s32 nav_y = (nav == MENU_NAVIGATE_UP ? -1 : (nav == MENU_NAVIGATE_DOWN ? 1 : 0));
    if (nav_x == 0 && nav_y == 0) {
        return;
    }
    s32 npa = 0;
    s32 fpa = 0;
    s32 npe = 0;
    s32 fpe = 0;
    struct menu_item *near_item = NULL;
    struct menu_item *far_item = NULL;
    menu_nav_compare(menu, menu->selector, nav, nav_x, nav_y, &near_item, &far_item, &npa, &fpa, &npe, &fpe);
    if (near_item) {
        menu_select(menu, near_item);
    } else if (far_item) {
        menu_select(menu, far_item);
    }
}

void menu_activate(struct menu *menu) {
    if (menu->child) {
        return menu_activate(menu->child);
    }
    if (menu->selector && menu->selector->activate_proc) {
        menu->selector->activate_proc(menu->selector);
    }
}

void menu_enter(struct menu *menu, struct menu *submenu) {
    if (menu->child) {
        return menu_enter(menu->child, submenu);
    }
    menu_signal_leave(menu, MENU_SWITCH_ENTER);
    menu->child = submenu;
    submenu->parent = menu;
    menu_signal_enter(submenu, MENU_SWITCH_ENTER);
}

struct menu *menu_return(struct menu *menu) {
    if (menu->child) {
        return menu_return(menu->child);
    }
    struct menu *parent = menu->parent;
    if (!parent || parent->child != menu) {
        return NULL;
    }
    menu_signal_leave(menu, MENU_SWITCH_RETURN);
    menu->parent = NULL;
    parent->child = NULL;
    menu_signal_enter(parent, MENU_SWITCH_RETURN);
    return parent;
}

void menu_select(struct menu *menu, struct menu_item *item) {
    if (menu->child) {
        return menu_select(menu->child, item);
    }
    if (menu->selector) {
        menu->selector->owner->selector = NULL;
    }
    menu->selector = item;
    item->owner->selector = item;
}

void menu_signal_enter(struct menu *menu, enum menu_switch_reason reason) {
    if (menu->child) {
        return menu_signal_enter(menu->child, reason);
    }
    for (struct menu_item *item = menu->items.first; item; item = list_next(item)) {
        if (item->enter_proc && item->enter_proc(item, reason)) {
            continue;
        }
        if (item->imenu) {
            menu_signal_enter(item->imenu, reason);
        }
    }
}

void menu_signal_leave(struct menu *menu, enum menu_switch_reason reason) {
    if (menu->child) {
        return menu_signal_leave(menu->child, reason);
    }
    for (struct menu_item *item = menu->items.first; item; item = list_next(item)) {
        if (item->leave_proc && item->leave_proc(item, reason)) {
            continue;
        }
        if (item->imenu) {
            menu_signal_leave(item->imenu, reason);
        }
    }
}

void menu_navigate_top(struct menu *menu, enum menu_navigation nav) {
    if (menu->parent) {
        return menu_navigate_top(menu->parent, nav);
    }
    return menu_navigate(menu, nav);
}

void menu_activate_top(struct menu *menu) {
    if (menu->parent) {
        return menu_activate_top(menu->parent);
    }
    return menu_activate(menu);
}

void menu_enter_top(struct menu *menu, struct menu *submenu) {
    if (menu->parent) {
        return menu_enter_top(menu->parent, submenu);
    }
    return menu_enter(menu, submenu);
}

struct menu *menu_return_top(struct menu *menu) {
    if (menu->parent) {
        return menu_return_top(menu->parent);
    }
    return menu_return(menu);
}

void menu_select_top(struct menu *menu, struct menu_item *item) {
    if (menu->parent) {
        return menu_select_top(menu->parent, item);
    }
    return menu_select(menu, item);
}

struct menu_item *menu_item_add(struct menu *menu, s32 x, s32 y, const char *text, u32 color) {
    struct menu_item *item = list_push_back(&menu->items, NULL);
    if (item) {
        item->owner = menu;
        item->enabled = 1;
        item->x = x;
        item->y = y;
        item->pxoffset = 0;
        item->pyoffset = 0;
        if (text) {
            item->text = malloc(strlen(text) + 1);
            strcpy(item->text, text);
        } else {
            item->text = NULL;
        }
        item->tooltip = NULL;
        item->color = color;
        item->animate_highlight = 0;
        item->data = NULL;
        item->selectable = 1;
        item->imenu = NULL;
        item->enter_proc = NULL;
        item->leave_proc = NULL;
        item->think_proc = NULL;
        item->draw_proc = NULL;
        item->navigate_proc = NULL;
        item->activate_proc = NULL;
        item->destroy_proc = NULL;
    }
    return item;
}

static void menu_deselect(struct menu *menu, struct menu_item *item) {
    if (menu->selector == item) {
        menu->selector = NULL;
    }
    if (menu->parent && menu->parent->child != menu) {
        menu_deselect(menu->parent, item);
    }
}

void menu_item_enable(struct menu_item *item) {
    item->enabled = 1;
}

void menu_item_disable(struct menu_item *item) {
    item->enabled = 0;
    menu_deselect(item->owner, item);
}

void menu_item_transfer(struct menu_item *item, struct menu *menu) {
    if (menu == item->owner) {
        return;
    }
    menu_deselect(item->owner, item);
    list_transfer(&menu->items, NULL, &item->owner->items, item);
    item->owner = menu;
}

void menu_item_remove(struct menu_item *item) {
    if (!item->destroy_proc || !item->destroy_proc(item)) {
        if (item->imenu) {
            menu_destroy(item->imenu);
            free(item->imenu);
        }
        if (item->text) {
            free(item->text);
        }
        if (item->data) {
            free(item->data);
        }
    }
    menu_deselect(item->owner, item);
    list_erase(&item->owner->items, item);
}

void menu_item_add_chain_link(struct menu_item *from_item, struct menu_item *to_item, enum menu_navigation direction) {
    from_item->chain_links[direction] = to_item;
}

void menu_item_create_chain(struct menu_item *items[], s32 items_size, enum menu_navigation nav_direction, _Bool loop,
                            _Bool reverse_chain) {
    if (!reverse_chain) {
        for (s32 i = 0; i < items_size - 1; i++) {
            menu_item_add_chain_link(items[i], items[i + 1], nav_direction);
        }
        if (loop) {
            menu_item_add_chain_link(items[items_size - 1], items[0], nav_direction);
        }
    } else {
        for (s32 i = items_size - 1; i > 0; i--) {
            menu_item_add_chain_link(items[i], items[i - 1], nav_direction);
        }
        if (loop) {
            menu_item_add_chain_link(items[0], items[items_size - 1], nav_direction);
        }
    }
}

s32 menu_item_screen_x(struct menu_item *item) {
    return menu_cell_screen_x(item->owner, item->x) + item->pxoffset;
}

s32 menu_item_screen_y(struct menu_item *item) {
    return menu_cell_screen_y(item->owner, item->y) + item->pyoffset;
}
