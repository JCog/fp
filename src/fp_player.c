#include <stdlib.h>
#include <stdio.h>
#include "menu.h"
#include "settings.h"
#include "gfx.h"
#include "resource.h"

// clang-format off
const char *PARTNERS = "none\0""goombario\0""kooper\0""bombette\0"
"parakarry\0""goompa\0""watt\0""sushie\0""lakilester\0""bow\0""goombaria\0"
"twink\0";
const char *RANK = "none\0""super\0""ultra\0";
const char *SPELL_TYPE = "none\0""+3 ATK\0""+3 DEF\0""EXP x2\0""Coins x2\0";
/* TODO: I hate this string array so much lmfao, if anyone knows a better way to do this, please lmk or submit a PR
 * only thing I can think of is to move it to a separate file */
const char *ITEM_LIST = "-\0""Jump\0""Spin Jump\0""Tornado Jump\0""Hammer\0""Super Hammer\0""Ultra Hammer\0"
"Lucky Star\0""Map\0""Big Map\0""First-Degree Card\0""Second-Degree Card\0""Third-Degree Card\0""Fourth-Degree Card\0"
"Diploma\0""Ultra Stone\0""Fortress Key\0""Ruins Key\0""Pulse Stone\0""Castle Key 1\0""Palace Key\0""Lunar Stone\0"
"Pyramid Stone\0""Diamond Stone\0""Golden Vase\0""Kooper's Shell\0""Castle Key 2\0""Forest Pass\0""Weight\0"
"Boo's Portrait\0""Crystal Berry\0""Mystical Key\0""Storeroom Key\0""Toy Train\0""Record\0""Frying Pan\0""Dictionary\0"
"Mystery Note\0""Suspicious Note\0""Crystal Ball\0""Screwdriver\0""Cookbook\0""Jade Raven\0""Magical Seed 1\0"
"Magical Seed 2\0""Magical Seed 3\0""Magical Seed 4\0""Toad Doll\0""Calculator\0""Bucket\0""Scarf\0""Red Key\0"
"Blue Key\0""a\0""Letter 01\0""Letter 02\0""Letter 03\0""Letter 04\0""Letter 05\0""Letter 06\0""Letter 07\0"
"Letter 08\0""Letter 09\0""Letter A\0""Letter 10\0""Letter 11\0""Letter B\0""Letter C\0""Letter D\0""Letter 12\0"
"Letter 13\0""Letter 14\0""Letter 15\0""Letter 16\0""Letter 17\0""Letter 18\0""Letter 19\0""Letter E\0""Letter 20\0"
"Letter 21\0""Letter 22\0""Letter 23\0""Letter 24\0""Artifact\0""Letter 25\0""Letter F\0""Dolly\0""Water Stone\0"
"Magical Bean\0""Fertile Soil\0""Miracle Water\0""Volcano Vase\0""Tape\0""Sugar\0""Salt\0""Egg 1\0""Cream\0"
"Strawberry\0""Butter\0""Cleanser\0""Water\0""Flour\0""Milk\0""Lyrics\0""Melody\0""Mailbag\0""Castle Key 3\0""Odd Key\0"
"Star Stone\0""Sneaky Parasol\0""Koopa Legends\0""Autograph 1\0""Empty Wallet\0""Autograph 2\0""Koopa Shell\0"
"Old Photo\0""Glasses\0""Photograph\0""Package\0""Red Jar\0""Castle Key 4\0""Warehouse Key\0""Prison Key 1\0"
"Silver Credit\0""Gold Credit\0""Prison Key 2\0""Prison Key 3\0""Prison Key 4\0""Fire Flower\0""Snowman Doll\0"
"Thunder Rage\0""Shooting Star\0""Thunder Bolt\0""Pebble\0""Dusty Hammer\0""Insecticide Herb\0""Stone Cap\0"
"Tasty Tonic\0""Mushroom\0""Volt Shroom\0""Super Shroom\0""Dried Shroom\0""Ultra Shroom\0""Sleepy Sheep\0""POW Block\0"
"Hustle Drink\0""Stop Watch\0""Whacka's Bump\0""Apple\0""Life Shroom\0""Mystery\0""Repel Gel\0""Fright Jar\0"
"Please Come Back\0""Dizzy Dial\0""Super Soda\0""Lemon\0""Lime\0""Blue Berry\0""Red Berry\0""Yellow Berry\0"
"Bubble Berry\0""Jammin' Jelly\0""Maple Syrup\0""Honey Syrup\0""Goomnut\0""Koopa Leaf\0""Dried Pasta\0""Dried Fruit\0"
"Strange Leaf\0""Cake Mix\0""Egg\0""Coconut\0""Melon\0""Stinky Herb\0""Iced Potato\0""Spicy Soup\0""Apple Pie\0"
"Honey Ultra\0""Maple Ultra\0""Jelly Ultra\0""Koopasta\0""Fried Shroom\0""Shroom Cake\0""Shroom Steak\0""Hot Shroom\0"
"Sweet Shroom\0""Yummy Meal\0""Healthy Juice\0""Bland Meal\0""Deluxe Feast\0""Special Shake\0""Big Cookie\0""Cake\0"
"Mistake\0""Koopa Tea\0""Honey Super\0""Maple Super\0""Jelly Super\0""Spaghetti\0""Egg Missile\0""Fried Egg\0"
"Honey Shroom\0""Honey Candy\0""Electro Pop\0""Fire Pop\0""Lime Candy\0""Coco Pop\0""Lemon Candy\0""Jelly Pop\0"
"Strange Cake\0""Kooky Cookie\0""Frozen Fries\0""Potato Salad\0""Nutty Cake\0""Maple Shroom\0""Boiled Egg\0"
"Yoshi Cookie\0""Jelly Shroom 1\0""Jelly Shroom 2\0""Jelly Shroom 3\0""Jelly Shroom 4\0""Jelly Shroom 5\0"
"Jelly Shroom 6\0""Spin Smash\0""Multibounce\0""Power Plus A\0""Dodge Master\0""Power Bounce\0""Spike Shield\0"
"First Attack\0""HP Plus A\0""Quake Hammer\0""Double Dip\0""Mystery Scroll\0""Sleep Stomp\0""Fire Shield\0"
"Quick Change\0""D-Down Pound\0""Dizzy Stomp\0""Smash Charge 0\0""Pretty Lucky\0""Feeling Fine\0""Attack FXA\0"
"All or Nothing\0""HP Drain\0""Jump Charge X\0""Slow Go\0""FP Plus A\0""Mega Rush\0""Ice Power\0""Defend Plus\0"
"Pay-Off\0""Money Money\0""Chill Out\0""Happy Heart A\0""Zap Tap\0""Berserker\0""Right On\0""Runaway Pay\0""Refund\0"
"Flower Saver A\0""Triple Dip\0""Hammer Throw\0""Mega Quake\0""Smash Charge\0""Jump Charge\0""S. Smash Chg.\0"
"S. Jump Chg.\0""Power Rush\0""Auto Jump\0""Auto Smash\0""Crazy Heart\0""Last Stand\0""Close Call\0""P-Up D-Down\0"
"Lucky Day\0""Mega HP Drain\0""P-Down D-Up\0""Power Quake\0""Auto Multibounce\0""Flower Fanatic\0""Heart Finder\0"
"Flower Finder\0""Spin Attack\0""Dizzy Attack\0""I Spy\0""Speedy Spin\0""Bump Attack\0""Power Jump\0""Super Jump\0"
"Mega Jump\0""Power Smash\0""Super Smash\0""Mega Smash\0""Power Smash X\0""Power Smash Y\0""Deep Focus A\0"
"Super Focus\0""Shrink Smash\0""Shell Crack\0""Kaiden\0""D-Down Jump\0""Shrink Stomp\0""Damage Dodge A\0"
"Earthquake Jump\0""Deep Focus B\0""Deep Focus C\0""HP Plus B\0""FP Plus B\0""Happy Heart B\0""Happy Heart X\0"
"Flower Saver B\0""Flower Saver X\0""Damage Dodge B\0""Damage Dodge X\0""Power Plus B\0""Power Plus X\0"
"Defend Plus X\0""Defend Plus Y\0""Happy Flower A\0""Happy Flower B\0""Happy Flower X\0""Area Focus\0""Peekaboo\0"
"Attack FXD\0""Attack FXB\0""Attack FXE\0""Attack FXC\0""Attack FXF 1\0""HP Plus C\0""HP Plus X\0""HP Plus Y\0"
"FP Plus C\0""FP Plus X\0""FP Plus Y\0""Healthy Healthy\0""Attack FXF 2\0""Attack FXF 3\0""Attack FXF 4\0"
"Attack FXF 5\0""Partner Attack\0""Heart\0""Coin\0""Heart Piece\0""Star Point\0""Full Heal\0""Flower\0""Star Piece\0"
"Present\0""Complete Cake\0""Bare Cake\0""Empty Cake Pan\0""Full Cake Pan\0""Empty Mixing Bowl\0""Full Mixing Bowl\0"
"Cake With Icing\0""Cake With Berries\0""Hammer 1 Icon\0""Hammer 2 Icon\0""Hammer 3 Icon\0""Boots 1 Icon\0"
"Boots 2 Icon\0""Boots 3 Icon\0""Items Icon\0";
// clang-format on

static int halfword_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint16_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p) {
            menu_intinput_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        *p = menu_intinput_get(item);
    }
    return 0;
}

static int byte_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p) {
            menu_intinput_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        *p = menu_intinput_get(item);
    }
    return 0;
}

static int byte_optionmod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != *p) {
            menu_option_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        *p = menu_option_get(item);
    }
    return 0;
}

static int halfword_optionmod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint16_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != *p) {
            menu_option_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        *p = menu_option_get(item);
    }
    return 0;
}

static int checkbox_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *p = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        *p = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, *p);
    }
    return 0;
}

static int max_hp_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != pm_player.player_data.max_hp) {
            menu_intinput_set(item, pm_player.player_data.max_hp);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        pm_player.player_data.max_hp = menu_intinput_get(item);
        pm_player.player_data.menu_max_hp = menu_intinput_get(item);
    }
    return 0;
}

static int max_fp_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != pm_player.player_data.max_fp) {
            menu_intinput_set(item, pm_player.player_data.max_fp);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        pm_player.player_data.max_fp = menu_intinput_get(item);
        pm_player.player_data.menu_max_fp = menu_intinput_get(item);
    }
    return 0;
}

static int hammer_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t menu_hammer = menu_option_get(item) - 1;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_hammer != pm_player.player_data.hammer_upgrade) {
            menu_option_set(item, (uint8_t)pm_player.player_data.hammer_upgrade + 1);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        pm_player.player_data.hammer_upgrade = (uint8_t)menu_option_get(item) - 1;
    }
    return 0;
}

static int in_party_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    partner_t *partner = (partner_t *)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        partner->in_party = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        partner->in_party = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, partner->in_party);
    }
    return 0;
}

static int rank_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    partner_t *partner = (partner_t *)data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != partner->upgrade) {
            menu_option_set(item, partner->upgrade);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        partner->upgrade = menu_option_get(item);
    }
    return 0;
}

static int item_int_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint16_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p) {
            menu_intinput_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        if (menu_intinput_get(item) > 0x16c) {
            *p = 0x16c;
        } else {
            *p = menu_intinput_get(item);
        }
    }
    return 0;
}

static int star_power_full_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p) {
            menu_intinput_set(item, *p);
        }
        if (pm_player.player_data.star_power.full_bars_filled >= pm_player.player_data.star_power.star_spirits_saved) {
            menu_intinput_set(item, pm_player.player_data.star_power.star_spirits_saved);
            *p = pm_player.player_data.star_power.star_spirits_saved;
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        if (menu_intinput_get(item) > pm_player.player_data.star_power.star_spirits_saved) {
            *p = pm_player.player_data.star_power.star_spirits_saved;
        } else {
            *p = menu_intinput_get(item);
        }
    }
    return 0;
}

static int star_power_partial_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p) {
            menu_intinput_set(item, *p);
        }
        if (pm_player.player_data.star_power.full_bars_filled >= pm_player.player_data.star_power.star_spirits_saved) {
            menu_intinput_set(item, 0);
            *p = 0;
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        if (pm_player.player_data.star_power.full_bars_filled > 6) {
            *p = 0;
        } else {
            *p = menu_intinput_get(item);
        }
    }
    return 0;
}

static int peach_or_mario_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_status.peach_flags |= (1 << 0);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_status.peach_flags &= ~(1 << 0);
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_status.peach_flags & (1 << 0));
    }
    return 0;
}

static int peach_transformed_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_status.peach_flags |= (1 << 1);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_status.peach_flags &= ~(1 << 1);
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_status.peach_flags & (1 << 1));
    }
    return 0;
}

static int peach_parasol_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_status.peach_flags |= (1 << 2);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_status.peach_flags &= ~(1 << 2);
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_status.peach_flags & (1 << 2));
    }
    return 0;
}

static void item_up_proc(struct menu_item *item, void *data) {
    uint16_t *start_item = (uint16_t *)data;
    uint16_t *up_item = start_item - 1;

    uint16_t temp_item = *up_item;
    *up_item = *start_item;
    *start_item = temp_item;
}

static void item_down_proc(struct menu_item *item, void *data) {
    uint16_t *start_item = (uint16_t *)data;
    uint16_t *down_item = start_item + 1;

    uint16_t temp_item = *down_item;
    *down_item = *start_item;
    *start_item = temp_item;
}

static void tab_prev_proc(struct menu_item *item, void *data) {
    menu_tab_previous(data);
}

static void tab_next_proc(struct menu_item *item, void *data) {
    menu_tab_next(data);
}

struct menu *create_player_menu(void) {
    static struct menu menu;
    static struct menu partners;
    static struct menu status;
    static struct menu items;
    static struct menu stored_items;
    static struct menu key_items;
    static struct menu badges;
    static struct menu star_power;
    static struct menu peach;
    static struct menu merlee;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&status, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&partners, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&stored_items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&key_items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&badges, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&star_power, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&peach, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&merlee, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    {
        int y_value = 0;
        menu.selector = menu_add_submenu(&menu, 0, y_value++, NULL, "return");

        /*build player menu*/
        menu_add_submenu(&menu, 0, y_value++, &status, "status");
        menu_add_submenu(&menu, 0, y_value++, &partners, "partners");
        menu_add_submenu(&menu, 0, y_value++, &items, "items");
        menu_add_submenu(&menu, 0, y_value++, &stored_items, "stored items");
        menu_add_submenu(&menu, 0, y_value++, &key_items, "key items");
        menu_add_submenu(&menu, 0, y_value++, &badges, "badges");
        menu_add_submenu(&menu, 0, y_value++, &star_power, "star power");
        menu_add_submenu(&menu, 0, y_value++, &peach, "princess peach");
        menu_add_submenu(&menu, 0, y_value++, &merlee, "merlee");
    }

    {
        /*build status menu*/
        const int STATS_X = 16;
        int y_value = 0;

        status.selector = menu_add_submenu(&status, 0, y_value++, NULL, "return");

        menu_add_static(&status, 0, y_value, "boots", 0xC0C0C0);
        menu_add_option(&status, STATS_X, y_value++,
                        "normal\0"
                        "super\0"
                        "ultra\0",
                        byte_optionmod_proc, &pm_player.player_data.boots_upgrade);

        menu_add_static(&status, 0, y_value, "hammer", 0xC0C0C0);
        menu_add_option(&status, STATS_X, y_value++,
                        "none\0"
                        "normal\0"
                        "super\0"
                        "ultra\0",
                        hammer_proc, NULL);

        menu_add_static(&status, 0, y_value, "action commands", 0xC0C0C0);
        menu_add_checkbox(&status, STATS_X, y_value++, checkbox_mod_proc, &pm_player.player_data.has_action_command);

        menu_add_static(&status, 0, y_value, "hp", 0xC0C0C0);
        menu_add_intinput(&status, STATS_X, y_value, 10, 2, byte_mod_proc, &pm_player.player_data.hp);
        menu_add_static(&status, STATS_X + 2, y_value, "/", 0xC0C0C0);
        menu_add_intinput(&status, STATS_X + 3, y_value++, 10, 2, max_hp_proc, NULL);

        menu_add_static(&status, 0, y_value, "fp", 0xC0C0C0);
        menu_add_intinput(&status, STATS_X, y_value, 10, 2, byte_mod_proc, &pm_player.player_data.fp);
        menu_add_static(&status, STATS_X + 2, y_value, "/", 0xC0C0C0);
        menu_add_intinput(&status, STATS_X + 3, y_value++, 10, 2, max_fp_proc, NULL);

        menu_add_static(&status, 0, y_value, "bp", 0xC0C0C0);
        menu_add_intinput(&status, STATS_X, y_value++, 10, 2, byte_mod_proc, &pm_player.player_data.bp);

        menu_add_static(&status, 0, y_value, "level", 0xC0C0C0);
        menu_add_intinput(&status, STATS_X, y_value++, 10, 2, byte_mod_proc, &pm_player.player_data.level);

        menu_add_static(&status, 0, y_value, "star points", 0xC0C0C0);
        menu_add_intinput(&status, STATS_X, y_value++, 10, 2, byte_mod_proc, &pm_player.player_data.star_points);

        menu_add_static(&status, 0, y_value, "star pieces", 0xC0C0C0);
        menu_add_intinput(&status, STATS_X, y_value++, 10, 2, byte_mod_proc, &pm_player.player_data.star_pieces);

        menu_add_static(&status, 0, y_value, "coins", 0xC0C0C0);
        menu_add_intinput(&status, STATS_X, y_value++, 10, 3, halfword_mod_proc, &pm_player.player_data.coins);
    }

    {
        /*build partners menu*/
        const int PARTNERS_X_1 = 11;
        const int PARTNERS_X_2 = 13;
        int y_value = 0;

        partner_t *partner_list[] = {
            &pm_player.player_data.party.goombario, &pm_player.player_data.party.kooper,
            &pm_player.player_data.party.bombette,  &pm_player.player_data.party.parakarry,
            &pm_player.player_data.party.bow,       &pm_player.player_data.party.watt,
            &pm_player.player_data.party.sushie,    &pm_player.player_data.party.lakilester,
            &pm_player.player_data.party.goompa,    &pm_player.player_data.party.goombaria,
            &pm_player.player_data.party.twink,
        };

        static const char *partner_names[] = {
            "goombario", "kooper",     "bombette", "parakarry", "bow",   "watt",
            "sushie",    "lakilester", "goompa",   "goombaria", "twink",
        };

        partners.selector = menu_add_submenu(&partners, 0, y_value++, NULL, "return");

        menu_add_static(&partners, 0, y_value, "active", 0xC0C0C0);
        menu_add_option(&partners, PARTNERS_X_2, y_value++, PARTNERS, byte_optionmod_proc,
                        &pm_player.player_data.current_partner);

        y_value++;
        menu_add_static(&partners, PARTNERS_X_2, y_value++, "rank", 0xC0C0C0);

        for (int i = 0; i < 8; i++) {
            menu_add_static(&partners, 0, y_value, partner_names[i], 0xC0C0C0);
            menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, partner_list[i]);
            menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, partner_list[i]);
        }
        y_value++;
        for (int i = 8; i < 11; i++) {
            menu_add_static(&partners, 0, y_value, partner_names[i], 0xC0C0C0);
            menu_add_checkbox(&partners, PARTNERS_X_1, y_value, in_party_proc, partner_list[i]);
            menu_add_option(&partners, PARTNERS_X_2, y_value++, RANK, rank_proc, partner_list[i]);
        }
    }

    {
        /*build items menu*/
        int y_value = 0;

        items.selector = menu_add_submenu(&items, 0, y_value++, NULL, "return");

        int i;
        struct gfx_texture *t_arrow = resource_get(RES_ICON_ARROW);
        for (i = 0; i < 10; i++) {
            char buffer[4];
            sprintf(buffer, "%d:", i);
            menu_add_static(&items, 0, y_value, buffer, 0xC0C0C0);
            if (i != 0) {
                menu_add_button_icon(&items, 3, y_value, t_arrow, 0, 0xFFFFFF, item_up_proc,
                                     &pm_player.player_data.items[i]);
            }
            if (i != 9) {
                menu_add_button_icon(&items, 4, y_value, t_arrow, 1, 0xFFFFFF, item_down_proc,
                                     &pm_player.player_data.items[i]);
            }
            menu_add_intinput(&items, 6, y_value, 16, 3, item_int_proc, &pm_player.player_data.items[i]);
            menu_add_option(&items, 10, y_value++, ITEM_LIST, halfword_optionmod_proc, &pm_player.player_data.items[i]);
        }

        menu_add_static(&items, 0, 18, "normal item range: 0x80 - 0xdf", 0xC0C0C0);
    }

    {
        /*build stored items menu*/
        int y_value = 0;

        stored_items.selector = menu_add_submenu(&stored_items, 0, y_value++, NULL, "return");

        int stored_items_page_count = 2;
        struct menu *stored_items_pages = malloc(sizeof(*stored_items_pages) * stored_items_page_count);
        struct menu_item *stored_items_tab =
            menu_add_tab(&stored_items, 0, y_value++, stored_items_pages, stored_items_page_count);
        int page_size = 16;
        struct gfx_texture *t_arrow = resource_get(RES_ICON_ARROW);
        for (int i = 0; i < stored_items_page_count; ++i) {
            struct menu *page = &stored_items_pages[i];
            menu_init(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
            y_value = 0;
            for (int j = 0; j < page_size; ++j) {
                char buffer[4];
                int item_index = j + i * page_size;
                sprintf(buffer, "%02d:", item_index);

                menu_add_static(page, 0, y_value, buffer, 0xC0C0C0);
                if (item_index != 0) {
                    menu_add_button_icon(page, 4, y_value, t_arrow, 0, 0xFFFFFF, item_up_proc,
                                         &pm_player.player_data.stored_items[item_index]);
                }
                if (item_index != 31) {
                    menu_add_button_icon(page, 5, y_value, t_arrow, 1, 0xFFFFFF, item_down_proc,
                                         &pm_player.player_data.stored_items[item_index]);
                }
                menu_add_intinput(page, 7, y_value, 16, 3, item_int_proc,
                                  &pm_player.player_data.stored_items[item_index]);
                menu_add_option(page, 11, y_value++, ITEM_LIST, halfword_optionmod_proc,
                                &pm_player.player_data.stored_items[item_index]);
            }
        }
        menu_tab_goto(stored_items_tab, 0);
        menu_add_button(&stored_items, 8, 0, "<", tab_prev_proc, stored_items_tab);
        menu_add_button(&stored_items, 10, 0, ">", tab_next_proc, stored_items_tab);

        menu_add_static(&stored_items, 0, 18, "normal item range: 0x80 - 0xdf", 0xC0C0C0);
    }

    {
        /*build key items menu*/
        int y_value = 0;

        key_items.selector = menu_add_submenu(&key_items, 0, y_value++, NULL, "return");

        int key_items_page_count = 2;
        int page_size = 16;
        struct gfx_texture *t_arrow = resource_get(RES_ICON_ARROW);
        struct menu *key_items_pages = malloc(sizeof(*key_items_pages) * key_items_page_count);
        struct menu_item *key_items_tab = menu_add_tab(&key_items, 0, y_value++, key_items_pages, key_items_page_count);
        for (int i = 0; i < key_items_page_count; ++i) {
            struct menu *page = &key_items_pages[i];
            menu_init(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
            y_value = 0;
            for (int j = 0; j < page_size; ++j) {
                char buffer[4];
                int item_index = j + i * page_size;
                sprintf(buffer, "%02d:", item_index);

                menu_add_static(page, 0, y_value, buffer, 0xC0C0C0);
                if (item_index != 0) {
                    menu_add_button_icon(page, 4, y_value, t_arrow, 0, 0xFFFFFF, item_up_proc,
                                         &pm_player.player_data.key_items[item_index]);
                }
                if (item_index != 31) {
                    menu_add_button_icon(page, 5, y_value, t_arrow, 1, 0xFFFFFF, item_down_proc,
                                         &pm_player.player_data.key_items[item_index]);
                }
                menu_add_intinput(page, 7, y_value, 16, 3, item_int_proc, &pm_player.player_data.key_items[item_index]);
                menu_add_option(page, 11, y_value++, ITEM_LIST, halfword_optionmod_proc,
                                &pm_player.player_data.key_items[item_index]);
            }
        }
        menu_tab_goto(key_items_tab, 0);
        menu_add_button(&key_items, 8, 0, "<", tab_prev_proc, key_items_tab);
        menu_add_button(&key_items, 10, 0, ">", tab_next_proc, key_items_tab);

        menu_add_static(&key_items, 0, 18, "key item range: 0x7 - 0x7f", 0xC0C0C0);
    }

    {
        /*build badges menu*/
        int y_value = 0;

        badges.selector = menu_add_submenu(&badges, 0, y_value++, NULL, "return");

        int badge_page_count = 8;
        int page_size = 16;
        struct gfx_texture *t_arrow = resource_get(RES_ICON_ARROW);
        struct menu *badge_pages = malloc(sizeof(*badge_pages) * badge_page_count);
        struct menu_item *badge_tab = menu_add_tab(&badges, 0, y_value++, badge_pages, badge_page_count);
        for (int i = 0; i < badge_page_count; ++i) {
            struct menu *page = &badge_pages[i];
            menu_init(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
            y_value = 0;
            for (int j = 0; j < page_size; ++j) {
                char buffer[5];
                int item_index = j + i * page_size;
                sprintf(buffer, "%02d:", item_index);

                menu_add_static(page, 0, y_value, buffer, 0xC0C0C0);
                if (item_index != 0) {
                    menu_add_button_icon(page, 4, y_value, t_arrow, 0, 0xFFFFFF, item_up_proc,
                                         &pm_player.player_data.badges[item_index]);
                }
                if (item_index != 127) {
                    menu_add_button_icon(page, 5, y_value, t_arrow, 1, 0xFFFFFF, item_down_proc,
                                         &pm_player.player_data.badges[item_index]);
                }
                menu_add_intinput(page, 7, y_value, 16, 3, item_int_proc, &pm_player.player_data.badges[item_index]);
                menu_add_option(page, 11, y_value++, ITEM_LIST, halfword_optionmod_proc,
                                &pm_player.player_data.badges[item_index]);
            }
        }
        menu_tab_goto(badge_tab, 0);
        menu_add_button(&badges, 8, 0, "<", tab_prev_proc, badge_tab);
        menu_add_button(&badges, 10, 0, ">", tab_next_proc, badge_tab);

        menu_add_static(&badges, 0, 18, "badge range: 0xe0 - 0x154", 0xC0C0C0);
    }

    {
        /*build star power menu*/
        const int STAR_POWER_X = 19;
        int y_value = 0;

        star_power.selector = menu_add_submenu(&star_power, 0, y_value++, NULL, "return");

        menu_add_static(&star_power, 0, y_value, "star spirits saved", 0xC0C0C0);
        menu_add_intinput(&star_power, STAR_POWER_X, y_value++, 10, 1, byte_mod_proc,
                          &pm_player.player_data.star_power.star_spirits_saved);

        menu_add_static(&star_power, 0, y_value, "bars full", 0xC0C0C0);
        menu_add_intinput(&star_power, STAR_POWER_X, y_value++, 10, 1, star_power_full_proc,
                          &pm_player.player_data.star_power.full_bars_filled);

        menu_add_static(&star_power, 0, y_value, "partial bar", 0xC0C0C0);
        menu_add_intinput(&star_power, STAR_POWER_X, y_value++, 16, 2, star_power_partial_proc,
                          &pm_player.player_data.star_power.partial_bars_filled);

        menu_add_static(&star_power, 0, y_value, "beam", 0xC0C0C0);
        menu_add_option(&star_power, STAR_POWER_X, y_value++,
                        "none\0"
                        "star beam\0"
                        "peach beam\0",
                        byte_optionmod_proc, &pm_player.player_data.star_power.beam_rank);
    }

    {
        /*build peach menu*/
        const int PEACH_X = 12;
        int y_value = 0;

        peach.selector = menu_add_submenu(&peach, 0, y_value++, NULL, "return");

        menu_add_static(&peach, 0, y_value, "peach", 0xC0C0C0);
        menu_add_checkbox(&peach, PEACH_X, y_value++, peach_or_mario_proc, NULL);

        menu_add_static(&peach, 0, y_value, "transformed", 0xC0C0C0);
        menu_add_checkbox(&peach, PEACH_X, y_value++, peach_transformed_proc, NULL);

        menu_add_static(&peach, 0, y_value, "parasol", 0xC0C0C0);
        menu_add_checkbox(&peach, PEACH_X, y_value++, peach_parasol_proc, NULL);

        menu_add_static(&peach, 0, y_value, "disguise", 0xC0C0C0);
        menu_add_option(&peach, PEACH_X, y_value++,
                        "none\0"
                        "koopatrol\0"
                        "hammer bro\0"
                        "clubba\0",
                        byte_optionmod_proc, &pm_status.peach_disguise);
    }

    {
        /*build merlee menu*/
        const int MERLEE_X = 16;
        int y_value = 0;

        merlee.selector = menu_add_submenu(&merlee, 0, y_value++, NULL, "return");

        menu_add_static(&merlee, 0, y_value, "spell type", 0xC0C0C0);
        menu_add_option(&merlee, MERLEE_X, y_value++, SPELL_TYPE, byte_optionmod_proc,
                        &pm_player.player_data.merlee.spell_type);

        menu_add_static(&merlee, 0, y_value, "casts remaining", 0xC0C0C0);
        menu_add_intinput(&merlee, MERLEE_X, y_value++, 10, 2, byte_mod_proc,
                          &pm_player.player_data.merlee.casts_remaining);

        menu_add_static(&merlee, 0, y_value, "turns remaining", 0xC0C0C0);
        menu_add_intinput(&merlee, MERLEE_X, y_value++, 10, 3, byte_mod_proc,
                          &pm_player.player_data.merlee.turns_until_spell);

        return &menu;
    }
}
