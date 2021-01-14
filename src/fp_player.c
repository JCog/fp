#include "menu.h"
#include <stdio.h>
#include "settings.h"
#include "gfx.h"

const char *PARTNERS = "none\0""goombario\0""kooper\0""bombette\0"
"parakarry\0""goompa\0""watt\0""sushie\0""lakilester\0""bow\0""goombaria\0"
"twink\0";
const char *RANK = "none\0""super\0""ultra\0";
const char *SPELL_TYPE = "none\0""+3 ATK\0""+3 DEF\0""EXP x2\0""Coins x2\0";
const char *ITEM_LIST[] = {
        "Jump",
        "Spin Jump",
        "Tornado Jump",
        "Hammer",
        "Super Hammer",
        "Ultra Hammer",
        "Lucky Star",
        "Map",
        "Big Map",
        "1st Degree Card",
        "2nd Degree Card",
        "3rd Degree Card",
        "4th Degree Card",
        "Diploma",
        "Ultra Stone",
        "Fortress Key",
        "Ruins Key",
        "Pulse Stone",
        "Castle Key 1",
        "Palace Key",
        "Lunar Stone",
        "Pyramid Stone",
        "Diamond Stone",
        "Golden Vase",
        "Koopers Shell",
        "Castle Key 2",
        "Forest Pass",
        "Weight",
        "Boos Portrait",
        "Crystal Berry",
        "Mystical Key",
        "Storeroom Key",
        "Toy Train",
        "Record",
        "Frying Pan",
        "Dictionary",
        "Mystery Note",
        "Suspicious Note",
        "Crystal Ball",
        "Screwdriver",
        "Cookbook",
        "Jade Raven",
        "Magical Seed 1",
        "Magical Seed 2",
        "Magical Seed 3",
        "Magical Seed 4",
        "Toad Doll",
        "Calculator",
        "Bucket",
        "Scarf",
        "Red Key",
        "Blue Key",
        "a",
        "Letter 01",
        "Letter 02",
        "Letter 03",
        "Letter 04",
        "Letter 05",
        "Letter 06",
        "Letter 07",
        "Letter 08",
        "Letter 09",
        "a",
        "Letter 10",
        "Letter 11",
        "a",
        "a",
        "a",
        "Letter 12",
        "Letter 13",
        "Letter 14",
        "Letter 15",
        "Letter 16",
        "Letter 17",
        "Letter 18",
        "Letter 19",
        "a",
        "Letter 20",
        "Letter 21",
        "Letter 22",
        "Letter 23",
        "Letter 24",
        "Artifact",
        "Letter 25",
        "a",
        "Dolly",
        "Water Stone",
        "Magical Bean",
        "Fertile Soil",
        "Miracle Water",
        "Volcano Vase",
        "Tape",
        "Sugar",
        "Salt",
        "Egg 1",
        "Cream",
        "Strawberry",
        "Butter",
        "Cleanser",
        "Water",
        "Flour",
        "Milk",
        "Lyrics",
        "Melody",
        "Mailbag",
        "Castle Key 3",
        "Odd Key",
        "Star Stone",
        "Sneaky Parasol",
        "Koopa Legends",
        "Autograph 1",
        "Empty Wallet",
        "Autograph 2",
        "Koopa Shell",
        "Old Photo",
        "Glasses",
        "Photograph",
        "Package",
        "Red Jar",
        "Castle Key 4",
        "Warehouse Key",
        "Prison Key 1",
        "Silver Credit",
        "Gold Credit",
        "Prison Key 2",
        "Prison Key 3",
        "Prison Key 4",
        "Fire Flower",
        "Snowman Doll",
        "Thunder Rage",
        "Shooting Star",
        "Thunder Bolt",
        "Pebble",
        "Dusty Hammer",
        "Insecticide Herb",
        "Stone Cap",
        "Tasty Tonic",
        "Mushroom",
        "Volt Shroom",
        "Super Shroom",
        "Dried Shroom",
        "Ultra Shroom",
        "Sleepy Sheep",
        "POW Block",
        "Hustle Drink",
        "Stop Watch",
        "Whackas Bump",
        "Apple",
        "Life Shroom",
        "Mystery",
        "Repel Gel",
        "Fright Jar",
        "Please Come Back",
        "Dizzy Dial",
        "Super Soda",
        "Lemon",
        "Lime",
        "Blue Berry",
        "Red Berry",
        "Yellow Berry",
        "Bubble Berry",
        "Jammin Jelly",
        "Maple Syrup",
        "Honey Syrup",
        "Goomnut",
        "Koopa Leaf",
        "Dried Pasta",
        "Dried Fruit",
        "Strange Leaf",
        "Cake Mix",
        "Egg 2",
        "Coconut",
        "Melon",
        "Stinky Herb",
        "Iced Potato",
        "Spicy Soup",
        "Apple Pie",
        "Honey Ultra",
        "Maple Ultra",
        "Jelly Ultra",
        "Koopasta",
        "Fried Shroom",
        "Shroom Cake",
        "Shroom Steak",
        "Hot Shroom",
        "Sweet Shroom",
        "Yummy Meal",
        "Healthy Juice",
        "Bland Meal",
        "Deluxe Feast",
        "Special Shake",
        "Big Cookie",
        "Cake",
        "Mistake",
        "Koopa Tea",
        "Honey Super",
        "Maple Super",
        "Jelly Super",
        "Spaghetti",
        "Egg Missile",
        "Fried Egg",
        "Honey Shroom",
        "Honey Candy",
        "Electro Pop",
        "Fire Pop",
        "Lime Candy",
        "Coco Pop",
        "Lemon Candy",
        "Jelly Pop",
        "Strange Cake",
        "Kooky Cookie",
        "Frozen Fries",
        "Potato Salad",
        "Nutty Cake",
        "Maple Shroom",
        "Boiled Egg",
        "Yoshi Cookie",
        "Jelly Shroom 1",
        "Jelly Shroom 2",
        "Jelly Shroom 3",
        "Jelly Shroom 4",
        "Jelly Shroom 5",
        "Jelly Shroom 6",
        "Spin Smash",
        "Multibounce",
        "Power Plus A",
        "Dodge Master",
        "Power Bounce",
        "Spike Shield",
        "First Attack",
        "HP Plus A",
        "Quake Hammer",
        "Double Dip",
        "Mystery Scroll",
        "Sleep Stomp",
        "Fire Shield",
        "Quick Change",
        "D Down Pound",
        "Dizzy Stomp",
        "Smash Charge 0",
        "Pretty Lucky",
        "Feeling Fine",
        "Attack FXA",
        "All or Nothing",
        "HP Drain",
        "Jump Charge 0",
        "Slow Go",
        "FP Plus A",
        "Mega Rush",
        "Ice Power",
        "Defend Plus A",
        "Pay Off",
        "Money Money",
        "Chill Out",
        "Happy Heart A",
        "Zap Tap",
        "Berserker",
        "Right On",
        "Runaway Pay",
        "Refund",
        "Flower Saver A",
        "Triple Dip",
        "Hammer Throw",
        "Mega Quake",
        "Smash Charge",
        "Jump Charge",
        "S Smash Chg",
        "S Jump Chg",
        "Power Rush",
        "Auto Jump",
        "Auto Smash",
        "Crazy Heart",
        "Last Stand",
        "Close Call",
        "P Up D Down",
        "Lucky Day",
        "Mega HP Drain",
        "P Down D Up",
        "Power Quake",
        "Auto Multibounce",
        "Flower Fanatic",
        "Heart Finder",
        "Flower Finder",
        "Spin Attack",
        "Dizzy Attack",
        "I Spy",
        "Speedy Spin",
        "Bump Attack",
        "Power Jump",
        "Super Jump",
        "Mega Jump",
        "Power Smash 1",
        "Super Smash",
        "Mega Smash",
        "Power Smash 2",
        "Power Smash 3",
        "Deep Focus 1",
        "Super Focus",
        "Shrink Smash",
        "Shell Crack",
        "Kaiden",
        "D Down Jump",
        "Shrink Stomp",
        "Damage Dodge A",
        "Earthquake Jump",
        "Deep Focus 2",
        "Deep Focus 3",
        "HP Plus B",
        "FP Plus B",
        "Happy Heart B",
        "Happy Heart X",
        "Flower Saver B",
        "Flower Saver X",
        "Damage Dodge B",
        "Damage Dodge X",
        "Power Plus B",
        "Power Plus X",
        "Defend Plus X",
        "Defend Plus Y",
        "Happy Flower A",
        "Happy Flower B",
        "Happy Flower X",
        "Group Focus",
        "Peekaboo",
        "Attack FXD",
        "Attack FXB",
        "Attack FXE",
        "Attack FXC",
        "Attack FXF",
        "HP Plus C",
        "HP Plus X",
        "HP Plus Y",
        "FP Plus C",
        "FP Plus X",
        "FP Plus Y",
        "Healthy Healthy",
        "Attack FXF 2",
        "Attack FXF 3",
        "Attack FXF 4",
        "Attack FXF 5",
        "Partner Attack",
        "Heart",
        "Coin",
        "Heart Piece",
        "Star Point",
        "Full Heal",
        "Flower",
        "Star Piece",
        "Present",
        "Complete Cake",
        "Bare Cake",
        "Empty Cake Pan",
        "Full Cake Pan",
        "Empty Mixing Bowl",
        "Full Mixing Bowl",
        "Cake With Icing",
        "Cake With Berries",
        "Hammer 1 Icon",
        "Hammer 2 Icon",
        "Hammer 3 Icon",
        "Boots 1 Icon",
        "Boots 2 Icon",
        "Boots 3 Icon",
        "Items Icon"
};

static int item_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color,
                                               draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    int chHeight = menu_get_cell_height(item->owner, 1);
    int x = draw_params->x;
    int y = draw_params->y;

    int menuY = 0;
    int i;
    for (i = 0; i < 10; i++) {
        const char *item_string;
        if (pm_player.items[i] == 0) {
            item_string = "-";
        }
        else {
            item_string = ITEM_LIST[pm_player.items[i] - 1];
        }
        gfx_printf(font, x, y + chHeight * menuY++, "%s", item_string);
    }
    return 1;
}

static int key_item_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color,
                                               draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    int chHeight = menu_get_cell_height(item->owner, 1);
    int chWidth = menu_get_cell_width(item->owner, 1);
    int x = draw_params->x;
    int y = draw_params->y;

    int menuY = 0;
    int i;
    for (i = 0; i < 16; i++) {
        const char *item_string;
        if (pm_player.key_items[i] == 0) {
            item_string = "-";
        }
        else {
            item_string = ITEM_LIST[pm_player.key_items[i] - 1];
        }
        gfx_printf(font, x, y + chHeight * menuY++, "%.10s", item_string);
    }
    menuY = 0;
    for (; i < 32; i++) {
        const char *item_string;
        if (pm_player.key_items[i] == 0) {
            item_string = "-";
        }
        else {
            item_string = ITEM_LIST[pm_player.key_items[i] - 1];
        }
        gfx_printf(font, x + chWidth * 18, y + chHeight * menuY++, "%.10s", item_string);
    }
    return 1;
}

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

static int rank_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    partner_t *partner = (partner_t *)data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != partner->upgrade)
            menu_option_set(item, partner->upgrade);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        partner->upgrade = menu_option_get(item);
    return 0;
}

static int spell_type_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_player.merlee.spell_type)
            menu_option_set(item, pm_player.merlee.spell_type);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_player.merlee.spell_type = menu_option_get(item);
    return 0;
}

struct menu *create_player_menu(void)
{
    static struct menu menu;
    static struct menu partners;
    static struct menu stats;
    static struct menu items;
    static struct menu key_items;
    static struct menu merlee;
    static struct menu star_power;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&stats, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&partners, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&key_items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&merlee, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&star_power, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");


    /*build player menu*/
    menu_add_submenu(&menu, 0, 1, &stats, "stats");
    menu_add_submenu(&menu, 0, 2, &partners, "partners");
    menu_add_submenu(&menu, 0, 3, &items, "items");
    menu_add_submenu(&menu, 0, 4, &key_items, "key items");
    menu_add_submenu(&menu, 0, 5, &merlee, "merlee");
    menu_add_submenu(&menu, 0, 6, &star_power, "star power");

    /*build stats menu*/
    const int STATS_X_0 = 0;
    const int STATS_X_1 = 16;
    int y_value = 0;
    stats_t *pm_stats = &pm_player.stats;

    stats.selector = menu_add_submenu(&stats, STATS_X_0, y_value++, NULL, "return");

    menu_add_static(&stats, STATS_X_0, y_value, "story progress", 0xC0C0C0);
    menu_add_intinput(&stats, STATS_X_1, y_value++, 16, 2, byte_mod_proc, &pm_unk3.story_progress);

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
    menu_add_option(&partners, PARTNERS_X_2, y_value++, PARTNERS, active_partner_proc, NULL);

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

    /*build items menu*/
    const int ITEMS_X_0 = 0;
    const int ITEMS_X_1 = 3;
    const int ITEMS_X_2 = 7;
    y_value = 0;

    items.selector = menu_add_submenu(&items, ITEMS_X_0, y_value++, NULL, "return");

    menu_add_static_custom(&items, ITEMS_X_2, y_value, item_draw_proc, NULL, 0xFFFFFF);

    int i;
    for (i = 0; i < 10; i++) {
        char buffer[3];
        sprintf(buffer, "%d:", i);
        menu_add_static(&items, ITEMS_X_0, y_value, buffer, 0xC0C0C0);
        menu_add_intinput(&items, ITEMS_X_1, y_value++, 16, 3, halfword_mod_proc, &pm_player.items[i]);
    }

    /*build key items menu*/
    const int KEY_ITEMS_X_0 = 0;
    const int KEY_ITEMS_X_1 = 3;
    const int KEY_ITEMS_X_2 = 18;
    const int KEY_ITEMS_X_3 = 21;
    y_value = 0;

    key_items.selector = menu_add_submenu(&key_items, KEY_ITEMS_X_0, y_value++, NULL, "return");

    menu_add_static_custom(&key_items, KEY_ITEMS_X_1 + 4, y_value, key_item_draw_proc, NULL, 0xFFFFFF);

    for (i = 0; i < 16; i++) {
        char buffer[4];
        sprintf(buffer, "%02d:", i);
        menu_add_static(&key_items, KEY_ITEMS_X_0, y_value, buffer, 0xC0C0C0);
        menu_add_intinput(&key_items, KEY_ITEMS_X_1, y_value++, 16, 3, halfword_mod_proc, &pm_player.key_items[i]);
    }
    y_value = 1;
    for (; i < 32; i++) {
        char buffer[4];
        sprintf(buffer, "%02d:", i);
        menu_add_static(&key_items, KEY_ITEMS_X_2, y_value, buffer, 0xC0C0C0);
        menu_add_intinput(&key_items, KEY_ITEMS_X_3, y_value++, 16, 3, halfword_mod_proc, &pm_player.key_items[i]);
    }

    /*build merlee menu*/
    const int MERLEE_X_0 = 0;
    const int MERLEE_X_1 = 16;
    y_value = 0;

    merlee.selector = menu_add_submenu(&merlee, MERLEE_X_0, y_value++, NULL, "return");

    menu_add_static(&merlee, MERLEE_X_0, y_value, "spell type", 0xC0C0C0);
    menu_add_option(&merlee, MERLEE_X_1, y_value++, SPELL_TYPE, spell_type_proc, NULL);

    menu_add_static(&merlee, MERLEE_X_0, y_value, "casts remaining", 0xC0C0C0);
    menu_add_intinput(&merlee, MERLEE_X_1, y_value++, 10, 2, byte_mod_proc, &pm_player.merlee.casts_remaining);

    menu_add_static(&merlee, MERLEE_X_0, y_value, "turns remaining", 0xC0C0C0);
    menu_add_intinput(&merlee, MERLEE_X_1, y_value++, 10, 2, byte_mod_proc, &pm_player.merlee.turns_until_spell);

    /*build star power menu*/
    const int STAR_POWER_X_0 = 0;
    const int STAR_POWER_X_1 = 19;
    y_value = 0;

    star_power.selector = menu_add_submenu(&star_power, STAR_POWER_X_0, y_value++, NULL, "return");

    menu_add_static(&star_power, STAR_POWER_X_0, y_value, "star spirits saved", 0xC0C0C0);
    menu_add_intinput(&star_power, STAR_POWER_X_1 + 1, y_value++, 10, 1, byte_mod_proc, &pm_player.star_power.star_spirits_saved);

    menu_add_static(&star_power, STAR_POWER_X_0, y_value, "bars full", 0xC0C0C0);
    menu_add_intinput(&star_power, STAR_POWER_X_1 + 1, y_value++, 10, 1, byte_mod_proc, &pm_player.star_power.full_bars_filled);

    menu_add_static(&star_power, STAR_POWER_X_0, y_value, "partial bar", 0xC0C0C0);
    menu_add_intinput(&star_power, STAR_POWER_X_1, y_value++, 16, 2, byte_mod_proc, &pm_player.star_power.partial_bars_filled);

    return &menu;
}