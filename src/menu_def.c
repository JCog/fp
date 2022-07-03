#include <stdlib.h>
#include <stdint.h>
#include "gfx.h"
#include "menu.h"

struct static_icon_data {
    game_icon *icon;
    struct gfx_texture *texture;
    s32 texture_tile;
    f32 scale;
};

struct menu_item *menu_add_static(struct menu *menu, s32 x, s32 y, const char *text, u32 color) {
    struct menu_item *item = menu_item_add(menu, x, y, text, color);
    item->selectable = 0;
    return item;
}

static s32 static_icon_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    struct static_icon_data *data = item->data;
    s32 cw = menu_get_cell_width(item->owner, 1);
    if (data->icon) {
        draw_params->x += cw / 2;
        draw_params->y -= (gfx_font_xheight(draw_params->font) + 1) / 2;
        game_icons_set_render_pos(data->icon, draw_params->x, draw_params->y);
        game_icons_set_alpha(data->icon, draw_params->alpha);
        game_icons_draw(data->icon);
    } else {
        s32 w = data->texture->tile_width * data->scale;
        s32 h = data->texture->tile_height * data->scale;
        struct gfx_sprite sprite = {
            data->texture,
            data->texture_tile,
            draw_params->x + (cw - w) / 2,
            draw_params->y - (gfx_font_xheight(draw_params->font) + h + 1) / 2,
            data->scale,
            data->scale,
        };
        gfx_mode_replace(GFX_MODE_FILTER, G_TF_BILERP);
        gfx_mode_replace(GFX_MODE_DROPSHADOW, 0);
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
        gfx_sprite_draw(&sprite);
        gfx_mode_pop(GFX_MODE_FILTER);
        gfx_mode_pop(GFX_MODE_DROPSHADOW);
    }
    return 1;
}

struct menu_item *menu_add_static_icon(struct menu *menu, s32 x, s32 y, struct gfx_texture *texture, s32 texture_tile,
                                       u32 color, f32 scale) {
    struct static_icon_data *data = malloc(sizeof(*data));
    data->icon = NULL;
    data->texture = texture;
    data->texture_tile = texture_tile;
    data->scale = scale;
    struct menu_item *item = menu_item_add(menu, x, y, NULL, color);
    item->data = data;
    item->selectable = 0;
    item->draw_proc = static_icon_draw_proc;
    return item;
}

struct menu_item *menu_add_static_game_icon(struct menu *menu, s32 x, s32 y, game_icon *icon) {
    struct static_icon_data *data = malloc(sizeof(*data));
    data->icon = icon;
    data->texture = NULL;
    data->texture_tile = 0;
    data->scale = 1.f;
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0xFFFFFF);
    item->data = data;
    item->selectable = 0;
    item->draw_proc = static_icon_draw_proc;
    return item;
}

struct menu_item *menu_add_static_custom(struct menu *menu, s32 x, s32 y,
                                         s32 (*draw_proc)(struct menu_item *item, struct menu_draw_params *draw_params),
                                         const char *text, u32 color) {
    struct menu_item *item = menu_item_add(menu, x, y, text, color);
    item->selectable = 0;
    item->draw_proc = draw_proc;
    return item;
}

static s32 tooltip_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    struct menu *tool_menu = item->data;
    while (tool_menu->child) {
        tool_menu = tool_menu->child;
    }
    if (tool_menu->selector && tool_menu->selector->tooltip) {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
        gfx_printf(draw_params->font, draw_params->x, draw_params->y, "%s", tool_menu->selector->tooltip);
    }
    return 1;
}

static s32 tooltip_destroy_proc(struct menu_item *item) {
    item->data = NULL;
    return 0;
}

struct menu_item *menu_add_tooltip(struct menu *menu, s32 x, s32 y, struct menu *tool_menu, u32 color) {
    struct menu_item *item = menu_item_add(menu, x, y, NULL, color);
    item->data = tool_menu;
    item->selectable = 0;
    item->draw_proc = tooltip_draw_proc;
    item->destroy_proc = tooltip_destroy_proc;
    return item;
}

static s32 imenu_think_proc(struct menu_item *item) {
    if (item->imenu) {
        item->imenu->cxoffset = item->x;
        item->imenu->cyoffset = item->y;
        item->imenu->pxoffset = item->pxoffset;
        item->imenu->pyoffset = item->pyoffset;
    }
    return 0;
}

static s32 imenu_navigate_proc(struct menu_item *item, enum menu_navigation nav) {
    if (item->imenu) {
        menu_navigate(item->imenu, nav);
        return 1;
    }
    return 0;
}

static s32 imenu_activate_proc(struct menu_item *item) {
    if (item->imenu) {
        menu_activate(item->imenu);
        return 1;
    }
    return 0;
}

struct menu_item *menu_add_imenu(struct menu *menu, s32 x, s32 y, struct menu **p_imenu) {
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0);
    item->selectable = 0;
    item->think_proc = imenu_think_proc;
    item->navigate_proc = imenu_navigate_proc;
    item->activate_proc = imenu_activate_proc;
    struct menu *imenu = malloc(sizeof(*imenu));
    menu_init(imenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    imenu->parent = menu;
    item->imenu = imenu;
    if (p_imenu) {
        *p_imenu = imenu;
    }
    return item;
}

struct tab_data {
    struct menu *tabs;
    s32 n_tabs;
    s32 current_tab;
};

static s32 tab_destroy_proc(struct menu_item *item) {
    item->imenu = NULL;
    return 0;
}

struct menu_item *menu_add_tab(struct menu *menu, s32 x, s32 y, struct menu *tabs, s32 n_tabs) {
    struct tab_data *data = malloc(sizeof(*data));
    data->tabs = tabs;
    data->n_tabs = n_tabs;
    data->current_tab = -1;
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0);
    item->data = data;
    item->selectable = 0;
    item->think_proc = imenu_think_proc;
    item->navigate_proc = imenu_navigate_proc;
    item->activate_proc = imenu_activate_proc;
    item->destroy_proc = tab_destroy_proc;
    return item;
}

void menu_tab_goto(struct menu_item *item, s32 tab_index) {
    struct tab_data *data = item->data;
    if (data->tabs) {
        if (data->current_tab >= 0) {
            struct menu *tab = &data->tabs[data->current_tab];
            struct menu_item *selector = menu_get_selector(tab);
            if (selector) {
                menu_select_top(item->owner, NULL);
            }
            tab->parent = NULL;
            item->imenu = NULL;
        }
        data->current_tab = tab_index;
        if (data->current_tab >= 0) {
            struct menu *tab = &data->tabs[data->current_tab];
            tab->parent = item->owner;
            item->imenu = tab;
        }
    }
}

void menu_tab_previous(struct menu_item *item) {
    struct tab_data *data = item->data;
    if (data->n_tabs >= 0) {
        s32 tab_index = (data->current_tab + data->n_tabs - 1) % data->n_tabs;
        menu_tab_goto(item, tab_index);
    }
}

void menu_tab_next(struct menu_item *item) {
    struct tab_data *data = item->data;
    if (data->n_tabs >= 0) {
        s32 tab_index = (data->current_tab + 1) % data->n_tabs;
        menu_tab_goto(item, tab_index);
    }
}
