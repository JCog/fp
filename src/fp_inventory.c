#include "menu.h"
#include "settings.h"

static char* PARTNERS = "none\0""goombario\0""kooper\0""bombette\0"
"parakarry\0""goompa\0""watt\0""sushie\0""lakilester\0""bow\0""goombaria\0"
"parakarry\0""goompa\0""watt\0""sushie\0""lakilester\0""bow\0""goombaria\0"
"twink\0";
static char* RANK = "none\0""super\0""ultra\0";

static int active_partner_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.stats.current_partner)
            menu_option_set(item, pm_player.stats.current_partner);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.stats.current_partner = menu_option_get(item);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int goombario_rank_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.party.goombario.upgrade)
            menu_option_set(item, pm_player.party.goombario.upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.party.goombario.upgrade = menu_option_get(item);
    return 0;
}

static int kooper_rank_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.party.kooper.upgrade)
            menu_option_set(item, pm_player.party.kooper.upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.party.kooper.upgrade = menu_option_get(item);
    return 0;
}

static int bombette_rank_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.party.bombette.upgrade)
            menu_option_set(item, pm_player.party.bombette.upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.party.bombette.upgrade = menu_option_get(item);
    return 0;
}

static int parakarry_rank_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.party.parakarry.upgrade)
            menu_option_set(item, pm_player.party.parakarry.upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.party.parakarry.upgrade = menu_option_get(item);
    return 0;
}

static int bow_rank_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.party.bow.upgrade)
            menu_option_set(item, pm_player.party.bow.upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.party.bow.upgrade = menu_option_get(item);
    return 0;
}

static int watt_rank_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.party.watt.upgrade)
            menu_option_set(item, pm_player.party.watt.upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.party.watt.upgrade = menu_option_get(item);
    return 0;
}

static int sushie_rank_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.party.sushie.upgrade)
            menu_option_set(item, pm_player.party.sushie.upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.party.sushie.upgrade = menu_option_get(item);
    return 0;
}

static int lakilester_rank_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.party.lakilester.upgrade)
            menu_option_set(item, pm_player.party.lakilester.upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.party.lakilester.upgrade = menu_option_get(item);
    return 0;
}

static int goompa_rank_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.party.goompa.upgrade)
            menu_option_set(item, pm_player.party.goompa.upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.party.goompa.upgrade = menu_option_get(item);
    return 0;
}

static int goombaria_rank_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.party.goombaria.upgrade)
            menu_option_set(item, pm_player.party.goombaria.upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.party.goombaria.upgrade = menu_option_get(item);
    return 0;
}

static int twink_rank_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.party.twink.upgrade)
            menu_option_set(item, pm_player.party.twink.upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.party.twink.upgrade = menu_option_get(item);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int goombario_in_party_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_player.party.goombario.in_party = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_player.party.goombario.in_party = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_player.party.goombario.in_party);
    }
    return 0;
}

static int kooper_in_party_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_player.party.kooper.in_party = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_player.party.kooper.in_party = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_player.party.kooper.in_party);
    }
    return 0;
}

static int bombette_in_party_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_player.party.bombette.in_party = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_player.party.bombette.in_party = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_player.party.bombette.in_party);
    }
    return 0;
}

static int parakarry_in_party_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_player.party.parakarry.in_party = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_player.party.parakarry.in_party = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_player.party.parakarry.in_party);
    }
    return 0;
}

static int bow_in_party_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_player.party.bow.in_party = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_player.party.bow.in_party = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_player.party.bow.in_party);
    }
    return 0;
}

static int watt_in_party_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_player.party.watt.in_party = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_player.party.watt.in_party = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_player.party.watt.in_party);
    }
    return 0;
}

static int sushie_in_party_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_player.party.sushie.in_party = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_player.party.sushie.in_party = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_player.party.sushie.in_party);
    }
    return 0;
}

static int lakilester_in_party_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_player.party.lakilester.in_party = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_player.party.lakilester.in_party = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_player.party.lakilester.in_party);
    }
    return 0;
}

static int goompa_in_party_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_player.party.goompa.in_party = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_player.party.goompa.in_party = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_player.party.goompa.in_party);
    }
    return 0;
}

static int goombaria_in_party_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_player.party.goombaria.in_party = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_player.party.goombaria.in_party = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_player.party.goombaria.in_party);
    }
    return 0;
}

static int twink_in_party_proc(struct menu_item* item,
    enum menu_callback_reason reason,
    void* data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_player.party.twink.in_party = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_player.party.twink.in_party = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_player.party.twink.in_party);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

struct menu* create_inventory_menu(void)
{
    static struct menu menu;
    static struct menu partners;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&partners, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");


    /*build main menu*/
    menu_add_submenu(&menu, 0, 1, &partners, "partners");

    /*build partners menu*/
    static int COLUMN_1_X = 11;
    static int COLUMN_2_X = 13;
    int y_value = 0;

    partners.selector = menu_add_submenu(&partners, 0, y_value++, NULL, "return");

    menu_add_static(&partners, 0, y_value, "active", 0xC0C0C0);
    menu_add_option(&partners, COLUMN_2_X, y_value++, PARTNERS,
        active_partner_proc, NULL);

    y_value++;
    menu_add_static(&partners, COLUMN_2_X, y_value++, "rank", 0xC0C0C0);

    menu_add_static(&partners, 0, y_value, "goombario", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, goombario_in_party_proc, NULL);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK,
        goombario_rank_proc, NULL);

    menu_add_static(&partners, 0, y_value, "kooper", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, kooper_in_party_proc, NULL);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK,
        kooper_rank_proc, NULL);

    menu_add_static(&partners, 0, y_value, "bombette", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, bombette_in_party_proc, NULL);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK,
        bombette_rank_proc, NULL);

    menu_add_static(&partners, 0, y_value, "parakarry", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, parakarry_in_party_proc, NULL);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK,
        parakarry_rank_proc, NULL);

    menu_add_static(&partners, 0, y_value, "bow", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, bow_in_party_proc, NULL);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK,
        bow_rank_proc, NULL);

    menu_add_static(&partners, 0, y_value, "watt", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, watt_in_party_proc, NULL);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK,
        watt_rank_proc, NULL);

    menu_add_static(&partners, 0, y_value, "sushie", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, sushie_in_party_proc, NULL);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK,
        sushie_rank_proc, NULL);

    menu_add_static(&partners, 0, y_value, "lakilester", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, lakilester_in_party_proc, NULL);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK,
        lakilester_rank_proc, NULL);

    y_value++;

    menu_add_static(&partners, 0, y_value, "goompa", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, goompa_in_party_proc, NULL);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK,
        goompa_rank_proc, NULL);

    menu_add_static(&partners, 0, y_value, "goombaria", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, goombaria_in_party_proc, NULL);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK,
        goombaria_rank_proc, NULL);

    menu_add_static(&partners, 0, y_value, "twink", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, twink_in_party_proc, NULL);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK,
        twink_rank_proc, NULL);

    return &menu;
}