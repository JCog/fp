#include "menu.h"
#include "fp.h"
static const char *labels[] =
    {
        "0:",
        "1:",
        "2:",
        "3:",
        "4:",
        "5:",
        "6:",
        "7:",
        "8:",
        "9:",
    };

static int byte_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p)
            menu_intinput_set(item, *p);
    }
    else if (reason == MENU_CALLBACK_CHANGED)
        *p = menu_intinput_get(item);
    return 0;
}

static void warp_proc(struct menu_item *item, void *data) {
    struct fav_room_info *p = data;
    if (fp_warp(p->group, p->room, p->entrance)) {
        fp.saved_group = p->group;
        fp.saved_room = p->room;
        fp.saved_entrance = p->entrance;
    }
}

static void current_proc(struct menu_item *item, void *data) {
    struct fav_room_info *p = data;
    p->group = pm_status.group_id;
    p->room = pm_status.room_id;
    p->entrance = pm_status.entrance_id;
}

static void save_settings_proc(struct menu_item *item, void *data)
{
    settings_save(fp.profile);
    fp_log("saved profile %i", fp.profile);
}

void create_favorite_rooms_menu(struct menu *menu)
{
    /* initialize menu */
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");

    menu_add_static(menu, 8, 1, "group", 0xC0C0C0);
    menu_add_static(menu, 14, 1, "room", 0xC0C0C0);
    menu_add_static(menu, 19, 1, "entrance", 0xC0C0C0);

    for (int i = 0; i < SETTINGS_ROOMS_MAX; i++) {
        menu_add_button(menu, 0, i + 2, "warp", warp_proc, &settings->rooms[i]);
        menu_add_static(menu, 5, i + 2, labels[i], 0xC0C0C0);
        menu_add_intinput(menu, 8, i + 2, 16, 2, byte_mod_proc, &settings->rooms[i].group);
        menu_add_intinput(menu, 14, i + 2, 16, 2, byte_mod_proc, &settings->rooms[i].room);
        menu_add_intinput(menu, 19, i + 2, 16, 2, byte_mod_proc, &settings->rooms[i].entrance);
        menu_add_button(menu, 28, i + 2, "current", current_proc, &settings->rooms[i]);
    }

    menu_add_button(menu, 0, 13, "save settings", save_settings_proc, NULL);
}
