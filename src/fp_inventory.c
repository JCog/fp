#include "menu.h"
#include "settings.h"

const char *PARTNERS = "none\0""goombario\0""kooper\0""bombette\0"
"parakarry\0""goompa\0""watt\0""sushie\0""lakilester\0""bow\0""goombaria\0"
"twink\0";
const char *RANK = "none\0""super\0""ultra\0";

static int active_partner_proc(struct menu_item *item,
    enum menu_callback_reason reason,
    void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.stats.current_partner)
            menu_option_set(item, pm_player.stats.current_partner);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.stats.current_partner = menu_option_get(item);
    return 0;
}

static int in_party_proc(struct menu_item *item,
    enum menu_callback_reason reason,
    void *data) {
    partner_t *partner = (partner_t *)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        partner->in_party = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        partner->in_party = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, partner->in_party);
    }
    return 0;
}

static int rank_proc(struct menu_item *item,
    enum menu_callback_reason reason,
    void *data) {
    partner_t *partner = (partner_t *)data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != partner->upgrade)
            menu_option_set(item, partner->upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        partner->upgrade = menu_option_get(item);
    return 0;
}

struct menu *create_inventory_menu(void)
{
    static struct menu menu;
    static struct menu partners;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&partners, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");


    /*build inventory menu*/
    menu_add_submenu(&menu, 0, 1, &partners, "partners");

    /*build partners menu*/
    const int COLUMN_0_X = 0;
    const int COLUMN_1_X = 11;
    const int COLUMN_2_X = 13;
    int y_value = 0;

    partners.selector = menu_add_submenu(&partners, 0, y_value++, NULL, "return");

    menu_add_static(&partners, COLUMN_0_X, y_value, "active", 0xC0C0C0);
    menu_add_option(&partners, COLUMN_2_X, y_value++, PARTNERS,
        active_partner_proc, NULL);

    y_value++;
    menu_add_static(&partners, COLUMN_2_X, y_value++, "rank", 0xC0C0C0);

    menu_add_static(&partners, COLUMN_0_X, y_value, "goombario", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, in_party_proc, &pm_player.party.goombario);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK, rank_proc, &pm_player.party.goombario);

    menu_add_static(&partners, COLUMN_0_X, y_value, "kooper", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, in_party_proc, &pm_player.party.kooper);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK, rank_proc, &pm_player.party.kooper);

    menu_add_static(&partners, COLUMN_0_X, y_value, "bombette", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, in_party_proc, &pm_player.party.bombette);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK, rank_proc, &pm_player.party.bombette);

    menu_add_static(&partners, COLUMN_0_X, y_value, "parakarry", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, in_party_proc, &pm_player.party.parakarry);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK, rank_proc, &pm_player.party.parakarry);

    menu_add_static(&partners, COLUMN_0_X, y_value, "bow", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, in_party_proc, &pm_player.party.bow);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK, rank_proc, &pm_player.party.bow);

    menu_add_static(&partners, COLUMN_0_X, y_value, "watt", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, in_party_proc, &pm_player.party.watt);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK, rank_proc, &pm_player.party.watt);

    menu_add_static(&partners, COLUMN_0_X, y_value, "sushie", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, in_party_proc, &pm_player.party.sushie);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK, rank_proc, &pm_player.party.sushie);

    menu_add_static(&partners, COLUMN_0_X, y_value, "lakilester", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, in_party_proc, &pm_player.party.lakilester);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK, rank_proc, &pm_player.party.lakilester);

    y_value++;

    menu_add_static(&partners, COLUMN_0_X, y_value, "goompa", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, in_party_proc, &pm_player.party.goompa);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK, rank_proc, &pm_player.party.goompa);

    menu_add_static(&partners, COLUMN_0_X, y_value, "goombaria", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, in_party_proc, &pm_player.party.goombaria);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK, rank_proc, &pm_player.party.goombaria);

    menu_add_static(&partners, COLUMN_0_X, y_value, "twink", 0xC0C0C0);
    menu_add_checkbox(&partners, COLUMN_1_X, y_value, in_party_proc, &pm_player.party.twink);
    menu_add_option(&partners, COLUMN_2_X, y_value++, RANK, rank_proc, &pm_player.party.twink);

    return &menu;
}