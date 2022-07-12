#include <stdlib.h>
#include <list/list.h>
#include "item_button.h"
#include "gfx.h"

const char *item_names[] = {
    "empty",
    "Jump",
    "Spin Jump",
    "Tornado Jump",
    "Hammer",
    "Super Hammer",
    "Ultra Hammer",
    "Lucky Star",
    "Map",
    "Big Map",
    "First-Degree Card",
    "Second-Degree Card",
    "Third-Degree Card",
    "Fourth-Degree Card",
    "Diploma",
    "Ultra Stone",
    "Fortress Key",
    "Ruins Key",
    "Pulse Stone",
    "Tubba's Castle Key",
    "Palace Key",
    "Lunar Stone",
    "Pyramid Stone",
    "Diamond Stone",
    "Golden Jar",
    "Kooper's Shell",
    "Bowser's Castle Key",
    "Forest Pass",
    "Weight",
    "Boo's Portrait",
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
    "Unused Letter 1",
    "Letter to Merlon",
    "Letter to Goompa",
    "Letter to Mort T.",
    "Letter to Russ T.",
    "Letter to Mayor Penguin",
    "Letter to Merlow",
    "Letter to Fice T.",
    "Letter to Nomadimouse",
    "Letter to Minh T.",
    "Unused Letter 2",
    "Letter to Goompapa 1",
    "Letter to Igor",
    "Unused Letter 3",
    "Unused Letter 4",
    "Unused Letter 5",
    "Letter to Franky",
    "Letter to Muss T.",
    "Letter to Koover 1",
    "Letter to Fishmael",
    "Letter to Koover 2",
    "Letter to Mr. E.",
    "Letter to Miss T.",
    "Letter to Little Mouser",
    "Unused Letter 6",
    "Letter to Dane T. 1",
    "Letter to Red Yoshi Kid",
    "Letter to Dane T. 2",
    "Letter to Frost T.",
    "Letter to Goompapa 2",
    "Artifact",
    "Letter to Kolorado",
    "Unused Letter 7",
    "Dolly",
    "Water Stone",
    "Magical Bean",
    "Fertile Soil",
    "Miracle Water",
    "Volcano Vase",
    "Tape",
    "Baking Sugar",
    "Baking Salt",
    "Baking Egg 1",
    "Baking Cream",
    "Baking Strawberry",
    "Baking Butter",
    "Baking Cleanser",
    "Baking Water",
    "Baking Flour",
    "Baking Milk",
    "Lyrics",
    "Melody",
    "Mailbag",
    "Peach's Castle Key",
    "Odd Key",
    "Star Stone",
    "Sneaky Parasol",
    "Koopa Legends",
    "Luigi's Autograph",
    "Empty Wallet",
    "Merluvlee's Autograph",
    "Koopa Shell",
    "Old Photo",
    "Glasses",
    "Unused Old Photo",
    "Package",
    "Red Jar",
    "Peach's Castle Key 2",
    "Warehouse Key",
    "Prison Key",
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
    "Whacka's Bump",
    "Apple",
    "Life Shroom",
    "Mystery?",
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
    "Jammin' Jelly",
    "Maple Syrup",
    "Honey Syrup",
    "Goomnut",
    "Koopa Leaf",
    "Dried Pasta",
    "Dried Fruit",
    "Strange Leaf",
    "Cake Mix",
    "Egg",
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
    "Jelly Shroom",
    "Unused Jelly Shroom 1",
    "Unused Jelly Shroom 2",
    "Unused Jelly Shroom 3",
    "Unused Jelly Shroom 4",
    "Unused Jelly Shroom 5",
    "Spin Smash",
    "Multibounce",
    "Power Plus 1",
    "Dodge Master",
    "Power Bounce",
    "Spike Shield",
    "First Attack",
    "HP Plus 1",
    "Quake Hammer",
    "Double Dip",
    "Mystery Scroll",
    "Sleep Stomp",
    "Fire Shield",
    "Quick Change",
    "D-Down Pound",
    "Dizzy Stomp",
    "Smash Charge 0",
    "Pretty Lucky",
    "Feeling Fine",
    "Attack FX A",
    "All or Nothing",
    "HP Drain",
    "Jump Charge 0",
    "Slow Go",
    "FP Plus 1",
    "Mega Rush",
    "Ice Power",
    "Defend Plus",
    "Pay-Off",
    "Money Money",
    "Chill Out",
    "Happy Heart 1",
    "Zap Tap",
    "Berserker",
    "Right On",
    "Runaway Pay",
    "Refund",
    "Flower Saver 1",
    "Triple Dip",
    "Hammer Throw",
    "Mega Quake",
    "Smash Charge",
    "Jump Charge",
    "S. Smash Chg.",
    "S. Jump Chg.",
    "Power Rush",
    "Auto Jump",
    "Auto Smash",
    "Crazy Heart",
    "Last Stand",
    "Close Call",
    "P-Up D-Down",
    "Lucky Day",
    "Mega HP Drain",
    "P-Down D-Up",
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
    "Power Smash",
    "Super Smash",
    "Mega Smash",
    "Power Smash 2",
    "Power Smash 3",
    "Deep Focus 1",
    "Super Focus",
    "Shrink Smash",
    "Shell Crack",
    "Kaiden",
    "D-Down Jump",
    "Shrink Stomp",
    "Damage Dodge 1",
    "Earthquake Jump",
    "Deep Focus 2",
    "Deep Focus 3",
    "HP Plus 2",
    "FP Plus 2",
    "Happy Heart 2",
    "Happy Heart 3",
    "Flower Saver 2",
    "Flower Saver 3",
    "Damage Dodge 2",
    "Damage Dodge 3",
    "Power Plus 2",
    "Power Plus 3",
    "Defend Plus 2",
    "Defend Plus 3",
    "Happy Flower 1",
    "Happy Flower 2",
    "Happy Flower 3",
    "Group Focus",
    "Peekaboo",
    "Attack FX D",
    "Attack FX B",
    "Attack FX E",
    "Attack FX C",
    "Attack FX F 1",
    "HP Plus 3",
    "HP Plus 4",
    "HP Plus 5",
    "FP Plus 3",
    "FP Plus 4",
    "FP Plus 5",
    "Healthy Healthy",
    "Attack FX F 2",
    "Attack FX F 3",
    "Attack FX F 4",
    "Attack FX F 5",
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
    "Items Icon",
};

struct item_data {
    menu_action_callback callback_proc;
    void *callback_data;
    enum item_type item_type;
    game_icon *icon;
    f32 scale;
    s32 anim_state;
    u16 *item_slot;
    u16 item_id;
};

static game_icon *get_empty_icon(enum item_type type) {
    game_icon *icon;
    switch (type) {
        case ITEM_TYPE_NORMAL:
        case ITEM_TYPE_STORED: icon = game_icons_create_item(ITEM_FIRE_FLOWER, 0); break;
        case ITEM_TYPE_KEY: icon = game_icons_create_item(ITEM_KOOPA_FORTRESS_KEY, 0); break;
        case ITEM_TYPE_BADGE:
        default: icon = game_icons_create_item(ITEM_SPEEDY_SPIN, 0); break;
    }
    game_icons_set_tint(icon, 0, 0, 0);
    game_icons_set_alpha(icon, 127);

    return icon;
}

static u16 *get_item_slot(enum item_type type, u16 item_index) {
    switch (type) {
        case ITEM_TYPE_NORMAL: return &pm_player.player_data.items[item_index];
        case ITEM_TYPE_KEY: return &pm_player.player_data.key_items[item_index];
        case ITEM_TYPE_STORED: return &pm_player.player_data.stored_items[item_index];
        case ITEM_TYPE_BADGE:
        default: return &pm_player.player_data.badges[item_index];
    }
}

static s32 enter_proc(struct menu_item *item, enum menu_switch_reason reason) {
    struct item_data *data = item->data;
    data->anim_state = 0;
    return 0;
}

static s32 think_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    if (data->item_id != *data->item_slot) {
        game_icons_delete(data->icon);
        if (*data->item_slot == 0) {
            data->icon = get_empty_icon(data->item_type);
        } else {
            data->icon = game_icons_create_item(*data->item_slot, 0);
            game_icons_set_alpha(data->icon, 255);
        }
        data->item_id = *data->item_slot;
        item->tooltip = item_names[*data->item_slot];
    }
    return 0;
}

static s32 draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    struct item_data *data = item->data;
    if (data->anim_state > 0) {
        ++draw_params->x;
        ++draw_params->y;
        data->anim_state = (data->anim_state + 1) % 3;
    }
    s32 cw = menu_get_cell_width(item->owner, 1);
    game_icon *icon = data->icon;
    game_icons_set_scale(icon, data->scale);
    s32 w = game_icons_get_width(icon);
    s32 h = game_icons_get_height(icon);
    s32 x = draw_params->x + (cw - w) / 2;
    s32 y = draw_params->y - (gfx_font_xheight(draw_params->font) + h) / 2;
    if (item->owner->selector == item) {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha * 0x80 / 0xFF));
        gfx_mode_replace(GFX_MODE_COMBINE, G_CC_MODE(G_CC_PRIMITIVE, G_CC_PRIMITIVE));
        gfx_disp(gsSPScisTextureRectangle(qs102(x), qs102(y), qs102(x + w), qs102(y + h), 0, 0, 0, 0, 0));
        gfx_mode_pop(GFX_MODE_COMBINE);
        // game_icons_set_drop_shadow(data->icon, 1);
        // game_icons_set_pos(icon, x + icon->render_pos_offset.x + 1, y + icon->render_pos_offset.y + 1);
    } else {
        // game_icons_set_drop_shadow(data->icon, 0);
    }
    game_icons_set_pos(icon, x + icon->render_pos_offset.x, y + icon->render_pos_offset.y);
    game_icons_draw(data->icon);
    return 1;
}

static s32 activate_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    data->callback_proc(item, data->callback_data);
    data->anim_state = 1;
    return 1;
}

struct menu_item *menu_add_item_button(struct menu *menu, s32 x, s32 y, enum item_type type, u16 item_slot_index,
                                       f32 scale, menu_action_callback callback_proc, void *callback_data) {
    struct item_data *data = malloc(sizeof(*data));
    data->callback_proc = callback_proc;
    data->callback_data = callback_data;
    data->item_type = type;
    data->icon = get_empty_icon(type);
    data->scale = scale;
    data->anim_state = 0;
    data->item_slot = get_item_slot(type, item_slot_index);
    data->item_id = 0;
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0xFFFFFF);
    item->data = data;
    item->enter_proc = enter_proc;
    item->think_proc = think_proc;
    item->draw_proc = draw_proc;
    item->activate_proc = activate_proc;
    return item;
}

u16 *menu_item_button_get_slot(struct menu_item *item) {
    struct item_data *data = item->data;
    return data->item_slot;
}

enum item_type menu_item_button_get_type(struct menu_item *item) {
    struct item_data *data = item->data;
    return data->item_type;
}
