#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <vector/vector.h>
#include "flags.h"
#include "gfx.h"
#include "menu.h"
#include "resource.h"
#include "pm64.h"

#define FLAG_LOG_LENGTH 16
#define FLAG_VIEW_ROWS  16

static s32 view_record_index;
static struct menu_item *view_record_name;
static struct menu_item *view_pageup;
static struct menu_item *view_pagedown;
static struct menu_item *view_rows[FLAG_VIEW_ROWS];
static struct menu_item *view_cells[FLAG_VIEW_ROWS * 0x10];

struct flag_record {
    s32 word_size;
    s32 length;
    void *data;
    void *comp;
    s16 index_length;
    const char *name;
    s32 view_offset;
};

struct flag_event {
    s32 record_index;
    s32 flag_index;
    _Bool value;
    char description[32];
};

static struct vector records;
static struct vector events;

static void add_record(size_t word_size, size_t length, void *data, const char *name) {
    struct flag_record *record = vector_push_back(&records, 1, NULL);
    record->word_size = word_size;
    record->length = length;
    record->data = data;
    record->name = name;
    record->comp = calloc(length, word_size);
    s32 n_flags = word_size * 8 * length;
    record->index_length = ((s32)(ceilf(log2f(n_flags))) + 3) / 4;
    record->view_offset = 0;
}

static void add_event(s32 record_index, s32 flag_index, _Bool value) {
    if (events.size >= FLAG_LOG_LENGTH) {
        vector_erase(&events, 0, 1);
    }
    struct flag_record *r = vector_at(&records, record_index);
    struct flag_event *e = vector_push_back(&events, 1, NULL);
    e->record_index = record_index;
    e->flag_index = flag_index;
    e->value = value;
    sprintf(e->description, "%s[0x%0*lx] := %i", r->name, r->index_length, flag_index, value);
}

static u32 get_flag_word(void *data, size_t word_size, s32 index) {
    if (word_size == 1) {
        return ((u8 *)data)[index];
    } else if (word_size == 2) {
        return ((u16 *)data)[index];
    } else if (word_size == 4) {
        return ((u32 *)data)[index];
    }
    return 0;
}

static void modify_flag(void *data, size_t word_size, s32 flag_index, _Bool value) {
    s32 word = flag_index / (word_size * 8);
    s32 bit = flag_index % (word_size * 8);
    if (word_size == 1) {
        u8 *p = data;
        if (value) {
            p[word] |= (1 << bit);
        } else {
            p[word] &= ~(1 << bit);
        }
    } else if (word_size == 2) {
        u16 *p = data;
        if (value) {
            p[word] |= (1 << bit);
        } else {
            p[word] &= ~(1 << bit);
        }
    } else if (word_size == 4) {
        u32 *p = data;
        if (value) {
            p[word] |= (1 << bit);
        } else {
            p[word] &= ~(1 << bit);
        }
    }
}

static s32 log_think_proc(struct menu_item *item) {
    for (s32 i = 0; i < records.size; ++i) {
        struct flag_record *r = vector_at(&records, i);
        for (s32 j = 0; j < r->length; ++j) {
            u32 wd = get_flag_word(r->data, r->word_size, j);
            u32 wc = get_flag_word(r->comp, r->word_size, j);
            u32 d = wd ^ wc;
            if (d != 0) {
                for (s32 k = 0; k < r->word_size * 8; ++k) {
                    if ((d >> k) & 1) {
                        add_event(i, r->word_size * 8 * j + k, (wd >> k) & 1);
                    }
                }
            }
        }
    }
    update_flag_records();
    return 0;
}

static s32 log_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    s32 x = draw_params->x;
    s32 y = draw_params->y;
    struct gfx_font *font = draw_params->font;
    u32 color = draw_params->color;
    u8 alpha = draw_params->alpha;
    s32 ch = menu_get_cell_height(item->owner, 1);
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(color, alpha));
    for (s32 i = 0; i < events.size && i < FLAG_LOG_LENGTH; ++i) {
        struct flag_event *e = vector_at(&events, events.size - i - 1);
        gfx_printf(font, x, y + ch * i, "%s", e->description);
    }
    return 1;
}

static void log_undo_proc(struct menu_item *item, void *data) {
    if (events.size == 0) {
        return;
    }
    struct flag_event *e = vector_at(&events, events.size - 1);
    struct flag_record *r = vector_at(&records, e->record_index);
    modify_flag(r->data, r->word_size, e->flag_index, !e->value);
    modify_flag(r->comp, r->word_size, e->flag_index, !e->value);
    vector_erase(&events, events.size - 1, 1);
}

static void log_clear_proc(struct menu_item *item, void *data) {
    vector_erase(&events, 0, events.size);
}

static void update_view(void) {
    struct flag_record *r = vector_at(&records, view_record_index);
    strcpy(view_record_name->text, r->name);
    s32 n_flags = r->word_size * 8 * r->length;
    view_pageup->enabled = view_pagedown->enabled = (n_flags > FLAG_VIEW_ROWS * 0x10);
    for (s32 y = 0; y < FLAG_VIEW_ROWS; ++y) {
        struct menu_item *row = view_rows[y];
        row->enabled = (r->view_offset + y * 0x10 < n_flags);
        if (row->enabled) {
            sprintf(view_rows[y]->text, "%04lx", r->view_offset + y * 0x10);
        }
        for (s32 x = 0; x < 0x10; ++x) {
            s32 n = y * 0x10 + x;
            struct menu_item *cell = view_cells[n];
            cell->enabled = (r->view_offset + n < n_flags);
            if (cell->enabled) {
                cell->think_proc(cell);
            }
        }
    }
}

static void goto_record(s32 record_index) {
    view_record_index = record_index;
    update_view();
}

static void prev_record_proc(struct menu_item *item, void *data) {
    goto_record((view_record_index + records.size - 1) % records.size);
}

static void next_record_proc(struct menu_item *item, void *data) {
    goto_record((view_record_index + 1) % records.size);
}

static void page_up_proc(struct menu_item *item, void *data) {
    struct flag_record *r = vector_at(&records, view_record_index);
    if (r->view_offset > 0) {
        r->view_offset -= 0x10;
        update_view();
    }
}

static void page_down_proc(struct menu_item *item, void *data) {
    struct flag_record *r = vector_at(&records, view_record_index);
    s32 n_flags = r->word_size * 8 * r->length;
    if (r->view_offset + FLAG_VIEW_ROWS * 0x10 < n_flags) {
        r->view_offset += 0x10;
        update_view();
    }
}

static s32 flag_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    s32 flag_index = (s32)data;
    struct flag_record *r = vector_at(&records, view_record_index);
    flag_index += r->view_offset;
    if (reason == MENU_CALLBACK_THINK) {
        s32 word = flag_index / (r->word_size * 8);
        s32 bit = flag_index % (r->word_size * 8);
        _Bool v = (get_flag_word(r->data, r->word_size, word) >> bit) & 1;
        menu_switch_set(item, v);
    } else if (reason == MENU_CALLBACK_SWITCH_ON) {
        modify_flag(r->data, r->word_size, flag_index, 1);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        modify_flag(r->data, r->word_size, flag_index, 0);
    }
    return 0;
}

void flag_menu_create(struct menu *menu) {
    /* initialize data */
    vector_init(&records, sizeof(struct flag_record));
    vector_init(&events, sizeof(struct flag_event));
    add_record(4, 64, &pm_save_data.global_flags, "global flags");
    add_record(4, 8, &pm_save_data.area_flags, "area flags");
    add_record(4, 600, pm_enemy_defeat_flags, "enemy defeat flags");
    add_record(1, 26, &pm_save_data.global_bytes[0x16D], "tattle flags");
    /* initialize menus */
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");
    {
        static struct menu log;
        menu_add_submenu(menu, 0, 1, &log, "log");
        menu_init(&log, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
        log.selector = menu_add_submenu(&log, 0, 0, NULL, "return");
        menu_add_button(&log, 0, 1, "undo", log_undo_proc, NULL);
        menu_add_button(&log, 5, 1, "clear", log_clear_proc, NULL);
        struct menu_item *log_item = menu_item_add(&log, 0, 2, NULL, 0xC0C0C0);
        log_item->selectable = 0;
        log_item->think_proc = log_think_proc;
        log_item->draw_proc = log_draw_proc;
    }
    {
        menu_add_button(menu, 4, 1, "<", prev_record_proc, NULL);
        menu_add_button(menu, 6, 1, ">", next_record_proc, NULL);
        view_record_name = menu_add_static(menu, 8, 1, NULL, 0xC0C0C0);
        view_record_name->text = malloc(32);
        struct gfx_texture *t_arrow = resource_get(RES_ICON_ARROW);
        view_pageup = menu_add_button_icon(menu, 0, 2, t_arrow, 0, 0xFFFFFF, page_up_proc, NULL);
        view_pagedown = menu_add_button_icon(menu, 2, 2, t_arrow, 1, 0xFFFFFF, page_down_proc, NULL);
        menu_add_static(menu, 4, 2, "0123456789abcdef", 0xC0C0C0);
        static struct gfx_texture *t_flag;
        if (!t_flag) {
            t_flag = resource_load_grc_texture("flag_icons");
        }
        for (s32 y = 0; y < FLAG_VIEW_ROWS; ++y) {
            view_rows[y] = menu_add_static(menu, 0, 3 + y, NULL, 0xC0C0C0);
            view_rows[y]->text = malloc(5);
            for (s32 x = 0; x < 0x10; ++x) {
                s32 n = y * 0x10 + x;
                view_cells[n] = menu_add_switch(menu, 4 + x, 3 + y, t_flag, 1, 0xFFFFFF, t_flag, 0, 0xFFFFFF, 0.75f, 1,
                                                flag_proc, (void *)n);
            }
        }
        goto_record(0);
    }
}

void update_flag_records(void) {
    for (s32 i = 0; i < records.size; ++i) {
        struct flag_record *r = vector_at(&records, i);
        memcpy(r->comp, r->data, r->word_size * r->length);
    }
}
