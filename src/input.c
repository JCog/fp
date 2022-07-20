#include "input.h"
#include "menu.h"
#include "resource.h"
#include "settings.h"
#include <stdarg.h>
#include <stdlib.h>

static s8 joy_x;
static s8 joy_y;
static u16 pad;
static s32 button_time[16];
static u16 pad_pressed_raw;
static u16 pad_pressed;
static u16 pad_released;
static u16 pad_reserved;
static s32 button_reserve_count[16];
static s32 bind_component_state[COMMAND_MAX];
static s32 bind_time[COMMAND_MAX];
static bool bind_pressed_raw[COMMAND_MAX];
static bool bind_pressed[COMMAND_MAX];
static bool bind_disable[COMMAND_MAX];
static bool bind_override[COMMAND_MAX];
static bool input_enabled = TRUE;

static s32 bitmask_button_index(u16 bitmask) {
    for (s32 i = 0; i < 16; ++i) {
        if (bitmask & (1 << i)) {
            return i;
        }
    }
    return -1;
}

u16 bind_make(s32 length, ...) {
    u16 bind = 0;
    va_list vl;
    va_start(vl, length);
    for (s32 i = 0; i < length; ++i) {
        bind |= bitmask_button_index(va_arg(vl, int)) << (i * 4);
    }
    va_end(vl);
    if (length < 4) {
        bind |= BIND_END << (length * 4);
    }
    return bind;
}

s32 bind_get_component(u16 bind, s32 index) {
    return (bind >> (4 * index)) & 0x000F;
}

u16 bind_get_bitmask(u16 bind) {
    u16 p = 0;
    for (s32 i = 0; i < 4; ++i) {
        s32 c = bind_get_component(bind, i);
        if (c == BIND_END) {
            break;
        }
        p |= 1 << c;
    }
    return p;
}

// clang-format off
const u32 input_button_color[] = {
    0xFFA000, 0xFFA000, 0xFFA000, 0xFFA000, 0xC0C0C0, 0xC0C0C0, 0x000000, 0x000000,
    0xC8C8C8, 0xC8C8C8, 0xC8C8C8, 0xC8C8C8, 0xC80000, 0xC0C0C0, 0x009600, 0x5A5AFF,
};
// clang-format on

void input_update(void) {
    joy_x = pm_gGameStatus.stickX[0];
    joy_y = pm_gGameStatus.stickY[0];
    u16 pm_pad = pm_gGameStatus.currentButtons[0].buttons;
    pad_pressed_raw = (pad ^ pm_pad) & pm_pad;
    pad_released = (pad ^ pm_pad) & ~pm_pad;
    pad = pm_pad;
    pad_pressed = 0;
    for (s32 i = 0; i < 16; ++i) {
        u16 p = 1 << i;
        if (pad & p) {
            ++button_time[i];
        } else {
            button_time[i] = 0;
        }
        if ((pad_pressed_raw & p) || button_time[i] >= INPUT_REPEAT_DELAY) {
            pad_pressed |= p;
        }
    }
    u16 bind_pad[SETTINGS_BIND_MAX];
    bool bind_state[SETTINGS_BIND_MAX];
    for (s32 i = 0; i < SETTINGS_BIND_MAX; ++i) {
        u16 *b = &settings->binds[i];
        bind_pad[i] = bind_get_bitmask(*b);
        s32 *cs = &bind_component_state[i];
        s32 j;
        u16 c;
        if (!input_enabled || bind_disable[i] || (!bind_override[i] && (pad_reserved & bind_pad[i]))) {
            *cs = 0;
        } else {
            s32 css = *cs;
            for (j = 0; j < 4; ++j) {
                c = bind_get_component(*b, j);
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
                if ((pad_released & (1 << c)) || (css != 0 && (pad_pressed_raw & ~bind_pad[i]))) {
                    *cs = 0;
                    break;
                } else if (pad_pressed_raw & (1 << c)) {
                    *cs |= csm;
                } else {
                    break;
                }
            }
        }
        bind_state[i] = (*cs && (j == 4 || c == BIND_END));
    }
    for (s32 i = 0; i < SETTINGS_BIND_MAX; ++i) {
        u16 pi = bind_pad[i];
        for (s32 j = 0; bind_state[i] && j < SETTINGS_BIND_MAX; ++j) {
            if (!bind_state[j]) {
                continue;
            }
            u16 pj = bind_pad[j];
            if (pi != pj && (pi & pj) == pi) {
                bind_component_state[i] = 0;
                bind_state[i] = FALSE;
            }
        }
        bind_pressed_raw[i] = (bind_time[i] == 0 && bind_state[i]);
        if (!bind_state[i]) {
            bind_time[i] = 0;
        } else {
            ++bind_time[i];
        }
        bind_pressed[i] = (bind_pressed_raw[i] || bind_time[i] >= INPUT_REPEAT_DELAY);
    }
}

s8 input_x(void) {
    return joy_x;
}

s8 input_y(void) {
    return joy_y;
}

u16 input_pad(void) {
    if (input_enabled) {
        return pad;
    } else {
        return 0;
    }
}

u16 input_pressed_raw(void) {
    if (input_enabled) {
        return pad_pressed_raw;
    } else {
        return 0;
    }
}

u16 input_pressed(void) {
    if (input_enabled) {
        return pad_pressed;
    } else {
        return 0;
    }
}

u16 input_released(void) {
    if (input_enabled) {
        return pad_released;
    } else {
        return 0;
    }
}

void input_reserve(u16 bitmask) {
    for (s32 i = 0; i < 16; ++i) {
        if (bitmask & (1 << i)) {
            ++button_reserve_count[i];
        }
    }
    pad_reserved |= bitmask;
}

void input_free(u16 bitmask) {
    for (s32 i = 0; i < 16; ++i) {
        u16 b = 1 << i;
        if (bitmask & b) {
            --button_reserve_count[i];
            if (button_reserve_count[i] == 0) {
                pad_reserved &= ~b;
            }
        }
    }
}

void input_bind_set_disable(s32 index, bool value) {
    bind_disable[index] = value;
}

void input_bind_set_override(s32 index, bool value) {
    bind_override[index] = value;
}

bool input_bind_held(s32 index) {
    return bind_time[index] > 0;
}

bool input_bind_pressed_raw(s32 index) {
    return bind_pressed_raw[index];
}

bool input_bind_pressed(s32 index) {
    return bind_pressed[index];
}

struct item_data {
    s32 bind_index;
    s32 state;
};

static s32 think_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    u16 *b = &settings->binds[data->bind_index];
    if (data->state == 1) {
        if (!pad) {
            data->state = 2;
        } else if (button_time[bitmask_button_index(BUTTON_L)] >= INPUT_REPEAT_DELAY) {
            *b = bind_make(0);
            item->animate_highlight = FALSE;
            data->state = 0;
            input_enabled = TRUE;
        }
    }
    if (data->state == 2) {
        if (pad) {
            *b = bind_make(0);
            data->state = 3;
        }
    }
    if (data->state == 3) {
        u16 p = bind_get_bitmask(*b);
        if (pad == 0) {
            item->animate_highlight = FALSE;
            data->state = 0;
            input_enabled = TRUE;
        } else {
            u16 pp = pad_pressed_raw & ~p;
            for (s32 i = 0; pp && i < 4; ++i) {
                s32 c = bind_get_component(*b, i);
                if (c != BIND_END) {
                    continue;
                }
                c = bitmask_button_index(pp);
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

static s32 draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    struct item_data *data = item->data;
    struct gfx_texture *texture = resource_get(RES_ICON_BUTTONS);
    s32 cw = menu_get_cell_width(item->owner, TRUE);
    s32 x = draw_params->x + (cw - texture->tile_width) / 2;
    s32 y = draw_params->y - (gfx_font_xheight(draw_params->font) + texture->tile_height + 1) / 2;
    u16 b = settings->binds[data->bind_index];
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
    for (s32 i = 0; i < 4; ++i) {
        u16 c = bind_get_component(b, i);
        if (c == BIND_END) {
            if (i == 0) {
                gfx_printf(draw_params->font, draw_params->x, draw_params->y, "unbound");
            }
            break;
        }
        struct gfx_sprite sprite = {
            texture, c, 0, x + i * 10, y, 1.f, 1.f,
        };
        if (item->owner->selector != item) {
            gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(input_button_color[c], draw_params->alpha));
        }
        gfx_sprite_draw(&sprite);
    }
    return 1;
}

static s32 activate_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    if (data->state == 0) {
        item->animate_highlight = TRUE;
        data->state = 1;
        input_enabled = FALSE;
    }
    return 1;
}

struct menu_item *binder_create(struct menu *menu, s32 x, s32 y, s32 bind_index) {
    struct item_data *data = malloc(sizeof(*data));
    data->bind_index = bind_index;
    data->state = 0;
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0xFFFFFF);
    item->data = data;
    item->text = malloc(12);
    item->think_proc = think_proc;
    item->draw_proc = draw_proc;
    item->activate_proc = activate_proc;
    return item;
}
