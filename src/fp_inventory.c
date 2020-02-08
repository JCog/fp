#include "menu.h"
#include "settings.h"

const char *PARTNERS = "none\0""goombario\0""kooper\0""bombette\0"
"parakarry\0""goompa\0""watt\0""sushie\0""lakilester\0""bow\0""goombaria\0"
"twink\0";
const char *RANK = "none\0""super\0""ultra\0";

static int halfword_mod_proc(struct menu_item *item,
    enum menu_callback_reason reason,
    void *data)
{
    uint16_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p)
            menu_intinput_set(item, *p);
    }
    else if (reason == MENU_CALLBACK_CHANGED)
        *p = menu_intinput_get(item);
    return 0;
}

static int byte_mod_proc(struct menu_item *item,
    enum menu_callback_reason reason,
    void *data)
{
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p)
            menu_intinput_set(item, *p);
    }
    else if (reason == MENU_CALLBACK_CHANGED)
        *p = menu_intinput_get(item);
    return 0;
}

static int action_commands_proc(struct menu_item *item,
    enum menu_callback_reason reason,
    void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_player.stats.has_action_command = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_player.stats.has_action_command = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_player.stats.has_action_command);
    }
    return 0;
}

static int max_hp_proc(struct menu_item *item,
    enum menu_callback_reason reason,
    void *data)
{
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != pm_player.stats.max_hp)
            menu_intinput_set(item, pm_player.stats.max_hp);
    }
    else if (reason == MENU_CALLBACK_CHANGED) {
        pm_player.stats.max_hp = menu_intinput_get(item);
        pm_player.stats.menu_max_hp = menu_intinput_get(item);
    }
    return 0;
}

static int max_fp_proc(struct menu_item *item,
    enum menu_callback_reason reason,
    void *data)
{
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != pm_player.stats.max_fp)
            menu_intinput_set(item, pm_player.stats.max_fp);
    }
    else if (reason == MENU_CALLBACK_CHANGED) {
        pm_player.stats.max_fp = menu_intinput_get(item);
        pm_player.stats.menu_max_fp = menu_intinput_get(item);
    }
    return 0;
}

static int boots_proc(struct menu_item *item,
    enum menu_callback_reason reason,
    void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.stats.boots_upgrade)
            menu_option_set(item, pm_player.stats.boots_upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.stats.boots_upgrade = menu_option_get(item);
    return 0;
}

static int hammer_proc(struct menu_item *item,
    enum menu_callback_reason reason,
    void *data) {
    uint8_t menu_hammer = menu_option_get(item) - 1;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_hammer != pm_player.stats.hammer_upgrade)
            menu_option_set(item, (uint8_t)pm_player.stats.hammer_upgrade + 1);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.stats.hammer_upgrade = (uint8_t)menu_option_get(item) - 1;
    return 0;
}

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
    static struct menu stats;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&stats, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&partners, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");


    /*build inventory menu*/
    menu_add_submenu(&menu, 0, 1, &stats, "stats");
    menu_add_submenu(&menu, 0, 2, &partners, "partners");

    /*build stats menu*/
    const int STATS_X_0 = 0;
    const int STATS_X_1 = 16;
    int y_value = 0;
    stats_t *pm_stats = &pm_player.stats;

    stats.selector = menu_add_submenu(&stats, STATS_X_0, y_value++, NULL, "return");

    menu_add_static(&stats, STATS_X_0, y_value, "boots", 0xC0C0C0);
    menu_add_option(&stats, STATS_X_1, y_value++, "normal\0""super\0""ultra\0", boots_proc, NULL);

    menu_add_static(&stats, STATS_X_0, y_value, "hammer", 0xC0C0C0);
    menu_add_option(&stats, STATS_X_1, y_value++, "none\0""normal\0""super\0""ultra\0", hammer_proc, NULL);

    menu_add_static(&stats, STATS_X_0, y_value, "action commands", 0xC0C0C0);
    menu_add_checkbox(&stats, STATS_X_1, y_value++, action_commands_proc, NULL);

    menu_add_static(&stats, STATS_X_0, y_value, "hp", 0xC0C0C0);
    menu_add_intinput(&stats, STATS_X_1, y_value, 10, 2, byte_mod_proc, &pm_stats->hp);
    menu_add_static(&stats, STATS_X_1 + 2, y_value, "/", 0xC0C0C0);
    menu_add_intinput(&stats, STATS_X_1 + 3, y_value++, 10, 2, max_hp_proc, NULL);

    menu_add_static(&stats, STATS_X_0, y_value, "fp", 0xC0C0C0);
    menu_add_intinput(&stats, STATS_X_1, y_value, 10, 2, byte_mod_proc, &pm_stats->fp);
    menu_add_static(&stats, STATS_X_1 + 2, y_value, "/", 0xC0C0C0);
    menu_add_intinput(&stats, STATS_X_1 + 3, y_value++, 10, 2, max_fp_proc, NULL);

    menu_add_static(&stats, STATS_X_0, y_value, "bp", 0xC0C0C0);
    menu_add_intinput(&stats, STATS_X_1, y_value++, 10, 2, byte_mod_proc, &pm_stats->bp);

    menu_add_static(&stats, STATS_X_0, y_value, "level", 0xC0C0C0);
    menu_add_intinput(&stats, STATS_X_1, y_value++, 10, 2, byte_mod_proc, &pm_stats->level);

    menu_add_static(&stats, STATS_X_0, y_value, "star points", 0xC0C0C0);
    menu_add_intinput(&stats, STATS_X_1, y_value++, 10, 2, byte_mod_proc, &pm_stats->star_points);

    menu_add_static(&stats, STATS_X_0, y_value, "star pieces", 0xC0C0C0);
    menu_add_intinput(&stats, STATS_X_1, y_value++, 10, 2, byte_mod_proc, &pm_stats->star_pieces);

    menu_add_static(&stats, STATS_X_0, y_value, "coins", 0xC0C0C0);
    menu_add_intinput(&stats, STATS_X_1, y_value++, 10, 3, halfword_mod_proc, &pm_stats->coins);

    /*build partners menu*/
    const int PARTNERS_X_0 = 0;
    const int PARTNERS_X_1 = 11;
    const int PARTNERS_X_2 = 13;
    y_value = 0;

    partners.selector = menu_add_submenu(&partners, 0, y_value++, NULL, "return");

    menu_add_static(&partners, PARTNERS_X_0, y_value, "active", 0xC0C0C0);
    menu_add_option(&partners, PARTNERS_X_2, y_value++, PARTNERS,
        active_partner_proc, NULL);

    y_value++;
    menu_add_static(&partners, PARTNERS_X_2, y_value++, "rank", 0xC0C0C0);

    menu_add_static(&partners, PARTNERS_X_0, y_value, "goombario", 0xC0C0C0);
    menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, &pm_player.party.goombario);
    menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, &pm_player.party.goombario);

    menu_add_static(&partners, PARTNERS_X_0, y_value, "kooper", 0xC0C0C0);
    menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, &pm_player.party.kooper);
    menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, &pm_player.party.kooper);

    menu_add_static(&partners, PARTNERS_X_0, y_value, "bombette", 0xC0C0C0);
    menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, &pm_player.party.bombette);
    menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, &pm_player.party.bombette);

    menu_add_static(&partners, PARTNERS_X_0, y_value, "parakarry", 0xC0C0C0);
    menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, &pm_player.party.parakarry);
    menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, &pm_player.party.parakarry);

    menu_add_static(&partners, PARTNERS_X_0, y_value, "bow", 0xC0C0C0);
    menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, &pm_player.party.bow);
    menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, &pm_player.party.bow);

    menu_add_static(&partners, PARTNERS_X_0, y_value, "watt", 0xC0C0C0);
    menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, &pm_player.party.watt);
    menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, &pm_player.party.watt);

    menu_add_static(&partners, PARTNERS_X_0, y_value, "sushie", 0xC0C0C0);
    menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, &pm_player.party.sushie);
    menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, &pm_player.party.sushie);

    menu_add_static(&partners, PARTNERS_X_0, y_value, "lakilester", 0xC0C0C0);
    menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, &pm_player.party.lakilester);
    menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, &pm_player.party.lakilester);

    y_value++;

    menu_add_static(&partners, PARTNERS_X_0, y_value, "goompa", 0xC0C0C0);
    menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, &pm_player.party.goompa);
    menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, &pm_player.party.goompa);

    menu_add_static(&partners, PARTNERS_X_0, y_value, "goombaria", 0xC0C0C0);
    menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, &pm_player.party.goombaria);
    menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, &pm_player.party.goombaria);

    menu_add_static(&partners, PARTNERS_X_0, y_value, "twink", 0xC0C0C0);
    menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, &pm_player.party.twink);
    menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, &pm_player.party.twink);

    return &menu;
}