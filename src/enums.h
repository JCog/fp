#ifndef ENUMS_H
#define ENUMS_H

typedef int8_t ActionState;
enum action_state {
    ACTION_STATE_IDLE,
    ACTION_STATE_WALK,
    ACTION_STATE_RUN,
    ACTION_STATE_JUMP,
    ACTION_STATE_BOUNCE,         /* Used with Kooper */
    ACTION_STATE_HOP,            /* Released A before apex of jump */
    ACTION_STATE_LAUNCH,         /* Shy Guy Toybox jack-in-the-boxes */
    ACTION_STATE_LAND_ON_SWITCH, /* Small red/blue ! switches */
    ACTION_STATE_FALLING,
    ACTION_STATE_STEP_DOWN,
    ACTION_STATE_LAND,
    ACTION_STATE_STEP_DOWN_LAND,

    // Following action states prohibit movement
    ACTION_STATE_TALK, /* Reading signs doesn't count */
    ACTION_STATE_SPIN_JUMP,
    ACTION_STATE_GROUND_POUND,
    ACTION_STATE_ULTRA_JUMP,
    ACTION_STATE_ULTRA_POUND,
    ACTION_STATE_SLIDE,
    ACTION_STATE_HAMMER,
    ACTION_STATE_13,
    ACTION_STATE_14,
    ACTION_STATE_HIT_FIRE,   /* Causes Mario to fly up and take damage. Used for fire bars. */
    ACTION_STATE_UNKNOWN_16, /* some kind of knockback, does no damage */
    ACTION_STATE_HIT_LAVA,
    ACTION_STATE_18,
    ACTION_STATE_SNEAKY_PARASOL,
    ACTION_STATE_SPIN,
    ACTION_STATE_ENEMY_FIRST_STRIKE,
    ACTION_STATE_GET_STAR_SPIRIT,
    ACTION_STATE_USE_SPINNING_FLOWER,
    ACTION_STATE_USE_MUNCHLESIA, /* Set by the jan_09 squishy flower entity; throws the player in the air. */
    ACTION_STATE_USE_TWEESTER,
    ACTION_STATE_BOUNCE_OFF_SWITCH, /* Small red/blue ! switches */
    ACTION_STATE_RIDE,
    ACTION_STATE_STEP_UP,
    ACTION_STATE_23,
    ACTION_STATE_24,
    ACTION_STATE_25,
    ACTION_STATE_USE_SPRING,
};

typedef uint16_t Area;
enum areas {
    AREA_GOOMBA_VILLAGE,
    AREA_TOAD_TOWN,
    AREA_SEWERS,
    AREA_WHALE,
    AREA_PEACHS_CASTLE,
    AREA_STAR_REGION,
    AREA_KOOPA_VILLAGE_PLEASANT_PATH,
    AREA_KOOPA_BROS_FORTRESS,
    AREA_MT_RUGGED,
    AREA_DRY_DRY_OUTPOST,
    AREA_DRY_DRY_DESERT,
    AREA_DRY_DRY_RUINS,
    AREA_FOREVER_FOREST,
    AREA_BOOS_MANSION,
    AREA_GUSTY_GULCH,
    AREA_TUBBAS_MANSION,
    AREA_SHY_GUYS_TOY_BOX,
    AREA_LAVA_LAVA_ISLAND,
    AREA_VOLCANO,
    AREA_FLOWER_FIELDS,
    AREA_SHIVER_REGION,
    AREA_CRYSTAL_PALACE,
    AREA_BOWSERS_CASTLE,
    AREA_OUTSIDE_PEACHS_CASTLE,
    AREA_CREDITS,
    AREA_MINI_GAMES,
    AREA_GAME_OVER,
    AREA_DEBUG,
};

typedef uint8_t Partner;
enum partners {
    PARTNER_NONE,
    PARTNER_GOOMBARIO,
    PARTNER_KOOPER,
    PARTNER_BOMBETTE,
    PARTNER_PARAKARRY,
    PARTNER_GOOMPA,
    PARTNER_WATT,
    PARTNER_SUSHIE,
    PARTNER_LAKILESTER,
    PARTNER_BOW,
    PARTNER_GOOMBARIA,
    PARTNER_TWINK,
};

enum ItemIDs {
    /*0x000*/ ITEM_NONE,
    /*0x001*/ ITEM_JUMP,
    /*0x002*/ ITEM_SPIN_JUMP,
    /*0x003*/ ITEM_TORNADO_JUMP,
    /*0x004*/ ITEM_HAMMER,
    /*0x005*/ ITEM_SUPER_HAMMER,
    /*0x006*/ ITEM_ULTRA_HAMMER,
    /*0x007*/ ITEM_LUCKY_STAR,
    /*0x008*/ ITEM_MAP,
    /*0x009*/ ITEM_BIG_MAP,
    /*0x00A*/ ITEM_FIRST_DEGREE_CARD,
    /*0x00B*/ ITEM_SECOND_DEGREE_CARD,
    /*0x00C*/ ITEM_THIRD_DEGREE_CARD,
    /*0x00D*/ ITEM_FOURTH_DEGREE_CARD,
    /*0x00E*/ ITEM_DIPLOMA,
    /*0x00F*/ ITEM_ULTRA_STONE,
    /*0x010*/ ITEM_KOOPA_FORTRESS_KEY,
    /*0x011*/ ITEM_RUINS_KEY,
    /*0x012*/ ITEM_PULSE_STONE,
    /*0x013*/ ITEM_TUBBA_CASTLE_KEY,
    /*0x014*/ ITEM_CRYSTAL_PALACE_KEY,
    /*0x015*/ ITEM_LUNAR_STONE,
    /*0x016*/ ITEM_PYRAMID_STONE,
    /*0x017*/ ITEM_DIAMOND_STONE,
    /*0x018*/ ITEM_GOLDEN_VASE,
    /*0x019*/ ITEM_KOOPER_SHELL,
    /*0x01A*/ ITEM_BOWSER_CASTLE_KEY,
    /*0x01B*/ ITEM_FOREST_PASS,
    /*0x01C*/ ITEM_BOO_WEIGHT,
    /*0x01D*/ ITEM_BOO_PORTRAIT,
    /*0x01E*/ ITEM_CRYSTAL_BERRY,
    /*0x01F*/ ITEM_MYSTICAL_KEY,
    /*0x020*/ ITEM_STOREROOM_KEY,
    /*0x021*/ ITEM_TOY_TRAIN,
    /*0x022*/ ITEM_BOO_RECORD,
    /*0x023*/ ITEM_FRYING_PAN,
    /*0x024*/ ITEM_DICTIONARY,
    /*0x025*/ ITEM_MYSTERY_NOTE,
    /*0x026*/ ITEM_SUSPICIOUS_NOTE,
    /*0x027*/ ITEM_CRYSTAL_BALL,
    /*0x028*/ ITEM_SCREWDRIVER,
    /*0x029*/ ITEM_COOKBOOK,
    /*0x02A*/ ITEM_JADE_RAVEN,
    /*0x02B*/ ITEM_MAGICAL_SEED1,
    /*0x02C*/ ITEM_MAGICAL_SEED2,
    /*0x02D*/ ITEM_MAGICAL_SEED3,
    /*0x02E*/ ITEM_MAGICAL_SEED4,
    /*0x02F*/ ITEM_TOAD_DOLL,
    /*0x030*/ ITEM_CALCULATOR,
    /*0x031*/ ITEM_SNOWMAN_BUCKET,
    /*0x032*/ ITEM_SNOWMAN_SCARF,
    /*0x033*/ ITEM_RED_KEY,
    /*0x034*/ ITEM_BLUE_KEY,
    /*0x035*/ ITEM_UNUSED_LETTER01,
    /*0x036*/ ITEM_LETTER01,
    /*0x037*/ ITEM_LETTER02,
    /*0x038*/ ITEM_LETTER03,
    /*0x039*/ ITEM_LETTER04,
    /*0x03A*/ ITEM_LETTER05,
    /*0x03B*/ ITEM_LETTER06,
    /*0x03C*/ ITEM_LETTER07,
    /*0x03D*/ ITEM_LETTER08,
    /*0x03E*/ ITEM_LETTER09,
    /*0x03F*/ ITEM_UNUSED_LETTER02,
    /*0x040*/ ITEM_LETTER10,
    /*0x041*/ ITEM_LETTER11,
    /*0x042*/ ITEM_UNUSED_LETTER03,
    /*0x043*/ ITEM_UNUSED_LETTER04,
    /*0x044*/ ITEM_UNUSED_LETTER05,
    /*0x045*/ ITEM_LETTER12,
    /*0x046*/ ITEM_LETTER13,
    /*0x047*/ ITEM_LETTER14,
    /*0x048*/ ITEM_LETTER15,
    /*0x049*/ ITEM_LETTER16,
    /*0x04A*/ ITEM_LETTER17,
    /*0x04B*/ ITEM_LETTER18,
    /*0x04C*/ ITEM_LETTER19,
    /*0x04D*/ ITEM_UNUSED_LETTER06,
    /*0x04E*/ ITEM_LETTER20,
    /*0x04F*/ ITEM_LETTER21,
    /*0x050*/ ITEM_LETTER22,
    /*0x051*/ ITEM_LETTER23,
    /*0x052*/ ITEM_LETTER24,
    /*0x053*/ ITEM_ARTIFACT,
    /*0x054*/ ITEM_LETTER25,
    /*0x055*/ ITEM_UNUSED_LETTER07,
    /*0x056*/ ITEM_DOLLY,
    /*0x057*/ ITEM_WATER_STONE,
    /*0x058*/ ITEM_MAGICAL_BEAN,
    /*0x059*/ ITEM_FERTILE_SOIL,
    /*0x05A*/ ITEM_MIRACLE_WATER,
    /*0x05B*/ ITEM_VOLCANO_VASE,
    /*0x05C*/ ITEM_KOOT_THE_TAPE,
    /*0x05D*/ ITEM_BAKING_SUGAR,
    /*0x05E*/ ITEM_BAKING_SALT,
    /*0x05F*/ ITEM_BAKING_EGG,
    /*0x060*/ ITEM_BAKING_CREAM,
    /*0x061*/ ITEM_BAKING_STRAWBERRY,
    /*0x062*/ ITEM_BAKING_BUTTER,
    /*0x063*/ ITEM_BAKING_CLEANSER,
    /*0x064*/ ITEM_BAKING_WATER,
    /*0x065*/ ITEM_BAKING_FLOUR,
    /*0x066*/ ITEM_BAKING_MILK,
    /*0x067*/ ITEM_LYRICS,
    /*0x068*/ ITEM_MELODY,
    /*0x069*/ ITEM_MAILBAG,
    /*0x06A*/ ITEM_PEACH_KEY1,
    /*0x06B*/ ITEM_ODD_KEY,
    /*0x06C*/ ITEM_STAR_STONE,
    /*0x06D*/ ITEM_SNEAKY_PARASOL,
    /*0x06E*/ ITEM_KOOT_KOOPA_LEGENDS,
    /*0x06F*/ ITEM_KOOT_MERLUVLEE_AUTOGRAPH,
    /*0x070*/ ITEM_KOOT_EMPTY_WALLET,
    /*0x071*/ ITEM_KOOT_LUIGI_AUTOGRAPH,
    /*0x072*/ ITEM_KOOT_SHELL,
    /*0x073*/ ITEM_KOOT_OLD_PHOTO,
    /*0x074*/ ITEM_KOOT_GLASSES,
    /*0x075*/ ITEM_KOOT_ALT_PHOTO,
    /*0x076*/ ITEM_KOOT_PACKAGE,
    /*0x077*/ ITEM_KOOT_RED_JAR,
    /*0x078*/ ITEM_PEACH_KEY2,
    /*0x079*/ ITEM_WAREHOUSE_KEY,
    /*0x07A*/ ITEM_PRISON_KEY1,
    /*0x07B*/ ITEM_SILVER_CREDIT,
    /*0x07C*/ ITEM_GOLD_CREDIT,
    /*0x07D*/ ITEM_PRISON_KEY2,
    /*0x07E*/ ITEM_PRISON_KEY3,
    /*0x07F*/ ITEM_PRISON_KEY4,
    /*0x080*/ ITEM_FIRE_FLOWER,
    /*0x081*/ ITEM_SNOWMAN_DOLL,
    /*0x082*/ ITEM_THUNDER_RAGE,
    /*0x083*/ ITEM_SHOOTING_STAR,
    /*0x084*/ ITEM_THUNDER_BOLT,
    /*0x085*/ ITEM_PEBBLE,
    /*0x086*/ ITEM_DUSTY_HAMMER,
    /*0x087*/ ITEM_INSECTICIDE_HERB,
    /*0x088*/ ITEM_STONE_CAP,
    /*0x089*/ ITEM_TASTY_TONIC,
    /*0x08A*/ ITEM_MUSHROOM,
    /*0x08B*/ ITEM_VOLT_SHROOM,
    /*0x08C*/ ITEM_SUPER_SHROOM,
    /*0x08D*/ ITEM_DRIED_SHROOM,
    /*0x08E*/ ITEM_ULTRA_SHROOM,
    /*0x08F*/ ITEM_SLEEPY_SHEEP,
    /*0x090*/ ITEM_P_O_W_BLOCK,
    /*0x091*/ ITEM_HUSTLE_DRINK,
    /*0x092*/ ITEM_STOP_WATCH,
    /*0x093*/ ITEM_WHACKAS_BUMP,
    /*0x094*/ ITEM_APPLE,
    /*0x095*/ ITEM_LIFE_SHROOM,
    /*0x096*/ ITEM_MYSTERY,
    /*0x097*/ ITEM_REPEL_GEL,
    /*0x098*/ ITEM_FRIGHT_JAR,
    /*0x099*/ ITEM_PLEASE_COME_BACK,
    /*0x09A*/ ITEM_DIZZY_DIAL,
    /*0x09B*/ ITEM_SUPER_SODA,
    /*0x09C*/ ITEM_LEMON,
    /*0x09D*/ ITEM_LIME,
    /*0x09E*/ ITEM_BLUE_BERRY,
    /*0x09F*/ ITEM_RED_BERRY,
    /*0x0A0*/ ITEM_YELLOW_BERRY,
    /*0x0A1*/ ITEM_BUBBLE_BERRY,
    /*0x0A2*/ ITEM_JAMMIN_JELLY,
    /*0x0A3*/ ITEM_MAPLE_SYRUP,
    /*0x0A4*/ ITEM_HONEY_SYRUP,
    /*0x0A5*/ ITEM_GOOMNUT,
    /*0x0A6*/ ITEM_KOOPA_LEAF,
    /*0x0A7*/ ITEM_DRIED_PASTA,
    /*0x0A8*/ ITEM_DRIED_FRUIT,
    /*0x0A9*/ ITEM_STRANGE_LEAF,
    /*0x0AA*/ ITEM_CAKE_MIX,
    /*0x0AB*/ ITEM_EGG,
    /*0x0AC*/ ITEM_COCONUT,
    /*0x0AD*/ ITEM_MELON,
    /*0x0AE*/ ITEM_STINKY_HERB,
    /*0x0AF*/ ITEM_ICED_POTATO,
    /*0x0B0*/ ITEM_SPICY_SOUP,
    /*0x0B1*/ ITEM_APPLE_PIE,
    /*0x0B2*/ ITEM_HONEY_ULTRA,
    /*0x0B3*/ ITEM_MAPLE_ULTRA,
    /*0x0B4*/ ITEM_JELLY_ULTRA,
    /*0x0B5*/ ITEM_KOOPASTA,
    /*0x0B6*/ ITEM_FRIED_SHROOM,
    /*0x0B7*/ ITEM_SHROOM_CAKE,
    /*0x0B8*/ ITEM_SHROOM_STEAK,
    /*0x0B9*/ ITEM_HOT_SHROOM,
    /*0x0BA*/ ITEM_SWEET_SHROOM,
    /*0x0BB*/ ITEM_YUMMY_MEAL,
    /*0x0BC*/ ITEM_HEALTHY_JUICE,
    /*0x0BD*/ ITEM_BLAND_MEAL,
    /*0x0BE*/ ITEM_DELUXE_FEAST,
    /*0x0BF*/ ITEM_SPECIAL_SHAKE,
    /*0x0C0*/ ITEM_BIG_COOKIE,
    /*0x0C1*/ ITEM_CAKE,
    /*0x0C2*/ ITEM_MISTAKE,
    /*0x0C3*/ ITEM_KOOPA_TEA,
    /*0x0C4*/ ITEM_HONEY_SUPER,
    /*0x0C5*/ ITEM_MAPLE_SUPER,
    /*0x0C6*/ ITEM_JELLY_SUPER,
    /*0x0C7*/ ITEM_SPAGHETTI,
    /*0x0C8*/ ITEM_EGG_MISSILE,
    /*0x0C9*/ ITEM_FRIED_EGG,
    /*0x0CA*/ ITEM_HONEY_SHROOM,
    /*0x0CB*/ ITEM_HONEY_CANDY,
    /*0x0CC*/ ITEM_ELECTRO_POP,
    /*0x0CD*/ ITEM_FIRE_POP,
    /*0x0CE*/ ITEM_LIME_CANDY,
    /*0x0CF*/ ITEM_COCO_POP,
    /*0x0D0*/ ITEM_LEMON_CANDY,
    /*0x0D1*/ ITEM_JELLY_POP,
    /*0x0D2*/ ITEM_STRANGE_CAKE,
    /*0x0D3*/ ITEM_KOOKY_COOKIE,
    /*0x0D4*/ ITEM_FROZEN_FRIES,
    /*0x0D5*/ ITEM_POTATO_SALAD,
    /*0x0D6*/ ITEM_NUTTY_CAKE,
    /*0x0D7*/ ITEM_MAPLE_SHROOM,
    /*0x0D8*/ ITEM_BOILED_EGG,
    /*0x0D9*/ ITEM_YOSHI_COOKIE,
    /*0x0DA*/ ITEM_JELLY_SHROOM1,
    /*0x0DB*/ ITEM_JELLY_SHROOM2,
    /*0x0DC*/ ITEM_JELLY_SHROOM3,
    /*0x0DD*/ ITEM_JELLY_SHROOM4,
    /*0x0DE*/ ITEM_JELLY_SHROOM5,
    /*0x0DF*/ ITEM_JELLY_SHROOM6,
    /*0x0E0*/ ITEM_SPIN_SMASH,
    /*0x0E1*/ ITEM_MULTIBOUNCE,
    /*0x0E2*/ ITEM_POWER_PLUS_A,
    /*0x0E3*/ ITEM_DODGE_MASTER,
    /*0x0E4*/ ITEM_POWER_BOUNCE,
    /*0x0E5*/ ITEM_SPIKE_SHIELD,
    /*0x0E6*/ ITEM_FIRST_ATTACK,
    /*0x0E7*/ ITEM_HP_PLUS_A,
    /*0x0E8*/ ITEM_QUAKE_HAMMER,
    /*0x0E9*/ ITEM_DOUBLE_DIP,
    /*0x0EA*/ ITEM_MYSTERY_SCROLL,
    /*0x0EB*/ ITEM_SLEEP_STOMP,
    /*0x0EC*/ ITEM_FIRE_SHIELD,
    /*0x0ED*/ ITEM_QUICK_CHANGE,
    /*0x0EE*/ ITEM_D_DOWN_POUND,
    /*0x0EF*/ ITEM_DIZZY_STOMP,
    /*0x0F0*/ ITEM_SMASH_CHARGE0,
    /*0x0F1*/ ITEM_PRETTY_LUCKY,
    /*0x0F2*/ ITEM_FEELING_FINE,
    /*0x0F3*/ ITEM_ATTACK_FX_A,
    /*0x0F4*/ ITEM_ALLOR_NOTHING,
    /*0x0F5*/ ITEM_HP_DRAIN,
    /*0x0F6*/ ITEM_JUMP_CHARGE0,
    /*0x0F7*/ ITEM_SLOW_GO,
    /*0x0F8*/ ITEM_FP_PLUS_A,
    /*0x0F9*/ ITEM_MEGA_RUSH,
    /*0x0FA*/ ITEM_ICE_POWER,
    /*0x0FB*/ ITEM_DEFEND_PLUS_A,
    /*0x0FC*/ ITEM_PAY_OFF,
    /*0x0FD*/ ITEM_MONEY_MONEY,
    /*0x0FE*/ ITEM_CHILL_OUT,
    /*0x0FF*/ ITEM_HAPPY_HEART_A,
    /*0x100*/ ITEM_ZAP_TAP,
    /*0x101*/ ITEM_BERSERKER,
    /*0x102*/ ITEM_RIGHT_ON,
    /*0x103*/ ITEM_RUNAWAY_PAY,
    /*0x104*/ ITEM_REFUND,
    /*0x105*/ ITEM_FLOWER_SAVER_A,
    /*0x106*/ ITEM_TRIPLE_DIP,
    /*0x107*/ ITEM_HAMMER_THROW,
    /*0x108*/ ITEM_MEGA_QUAKE,
    /*0x109*/ ITEM_SMASH_CHARGE,
    /*0x10A*/ ITEM_JUMP_CHARGE,
    /*0x10B*/ ITEM_S_SMASH_CHG,
    /*0x10C*/ ITEM_S_JUMP_CHG,
    /*0x10D*/ ITEM_POWER_RUSH,
    /*0x10E*/ ITEM_AUTO_JUMP,
    /*0x10F*/ ITEM_AUTO_SMASH,
    /*0x110*/ ITEM_CRAZY_HEART,
    /*0x111*/ ITEM_LAST_STAND,
    /*0x112*/ ITEM_CLOSE_CALL,
    /*0x113*/ ITEM_P_UP_D_DOWN,
    /*0x114*/ ITEM_LUCKY_DAY,
    /*0x115*/ ITEM_MEGA_HP_DRAIN,
    /*0x116*/ ITEM_P_DOWN_D_UP,
    /*0x117*/ ITEM_POWER_QUAKE,
    /*0x118*/ ITEM_AUTO_MULTIBOUNCE,
    /*0x119*/ ITEM_FLOWER_FANATIC,
    /*0x11A*/ ITEM_HEART_FINDER,
    /*0x11B*/ ITEM_FLOWER_FINDER,
    /*0x11C*/ ITEM_SPIN_ATTACK,
    /*0x11D*/ ITEM_DIZZY_ATTACK,
    /*0x11E*/ ITEM_I_SPY,
    /*0x11F*/ ITEM_SPEEDY_SPIN,
    /*0x120*/ ITEM_BUMP_ATTACK,
    /*0x121*/ ITEM_POWER_JUMP,
    /*0x122*/ ITEM_SUPER_JUMP,
    /*0x123*/ ITEM_MEGA_JUMP,
    /*0x124*/ ITEM_POWER_SMASH1,
    /*0x125*/ ITEM_SUPER_SMASH,
    /*0x126*/ ITEM_MEGA_SMASH,
    /*0x127*/ ITEM_POWER_SMASH2,
    /*0x128*/ ITEM_POWER_SMASH3,
    /*0x129*/ ITEM_DEEP_FOCUS1,
    /*0x12A*/ ITEM_SUPER_FOCUS,
    /*0x12B*/ ITEM_SHRINK_SMASH,
    /*0x12C*/ ITEM_SHELL_CRACK,
    /*0x12D*/ ITEM_KAIDEN,
    /*0x12E*/ ITEM_D_DOWN_JUMP,
    /*0x12F*/ ITEM_SHRINK_STOMP,
    /*0x130*/ ITEM_DAMAGE_DODGE_A,
    /*0x131*/ ITEM_EARTHQUAKE_JUMP,
    /*0x132*/ ITEM_DEEP_FOCUS2,
    /*0x133*/ ITEM_DEEP_FOCUS3,
    /*0x134*/ ITEM_HP_PLUS_B,
    /*0x135*/ ITEM_FP_PLUS_B,
    /*0x136*/ ITEM_HAPPY_HEART_B,
    /*0x137*/ ITEM_HAPPY_HEART_X,
    /*0x138*/ ITEM_FLOWER_SAVER_B,
    /*0x139*/ ITEM_FLOWER_SAVER_X,
    /*0x13A*/ ITEM_DAMAGE_DODGE_B,
    /*0x13B*/ ITEM_DAMAGE_DODGE_X,
    /*0x13C*/ ITEM_POWER_PLUS_B,
    /*0x13D*/ ITEM_POWER_PLUS_X,
    /*0x13E*/ ITEM_DEFEND_PLUS_X,
    /*0x13F*/ ITEM_DEFEND_PLUS_Y,
    /*0x140*/ ITEM_HAPPY_FLOWER_A,
    /*0x141*/ ITEM_HAPPY_FLOWER_B,
    /*0x142*/ ITEM_HAPPY_FLOWER_X,
    /*0x143*/ ITEM_GROUP_FOCUS,
    /*0x144*/ ITEM_PEEKABOO,
    /*0x145*/ ITEM_ATTACK_FX_D,
    /*0x146*/ ITEM_ATTACK_FX_B,
    /*0x147*/ ITEM_ATTACK_FX_E,
    /*0x148*/ ITEM_ATTACK_FX_C,
    /*0x149*/ ITEM_ATTACK_FX_F,
    /*0x14A*/ ITEM_HP_PLUS_C,
    /*0x14B*/ ITEM_HP_PLUS_X,
    /*0x14C*/ ITEM_HP_PLUS_Y,
    /*0x14D*/ ITEM_FP_PLUS_C,
    /*0x14E*/ ITEM_FP_PLUS_X,
    /*0x14F*/ ITEM_FP_PLUS_Y,
    /*0x150*/ ITEM_HEALTHY_HEALTHY,
    /*0x151*/ ITEM_ATTACK_FX_F2,
    /*0x152*/ ITEM_ATTACK_FX_F3,
    /*0x153*/ ITEM_ATTACK_FX_F4,
    /*0x154*/ ITEM_ATTACK_FX_F5,
    /*0x155*/ ITEM_PARTNER_ATTACK,
    /*0x156*/ ITEM_HEART,
    /*0x157*/ ITEM_COIN,
    /*0x158*/ ITEM_HEART_PIECE,
    /*0x159*/ ITEM_STAR_POINT,
    /*0x15A*/ ITEM_HEART_POINT,
    /*0x15B*/ ITEM_FLOWER_POINT,
    /*0x15C*/ ITEM_STAR_PIECE,
    /*0x15D*/ ITEM_PRESENT,
    /*0x15E*/ ITEM_CAKE_DONE,
    /*0x15F*/ ITEM_CAKE_BARE,
    /*0x160*/ ITEM_CAKE_PAN,
    /*0x161*/ ITEM_CAKE_BATTER,
    /*0x162*/ ITEM_CAKE_BOWL,
    /*0x163*/ ITEM_CAKE_MIXED,
    /*0x164*/ ITEM_CAKE_WITH_ICING,
    /*0x165*/ ITEM_CAKE_WITH_BERRIES,
    /*0x166*/ ITEM_HAMMER1_ICON,
    /*0x167*/ ITEM_HAMMER2_ICON,
    /*0x168*/ ITEM_HAMMER3_ICON,
    /*0x169*/ ITEM_BOOTS1_ICON,
    /*0x16A*/ ITEM_BOOTS2_ICON,
    /*0x16B*/ ITEM_BOOTS3_ICON,
    /*0x16C*/ ITEM_ITEMS_ICON,
};

enum StoryProgress {
    /* 80 */ STORY_INTRO = -128,

    /* 81 */ STORY_CH0_WAKE_UP,
    /* 82 */ STORY_CH0_MET_INNKEEPER,
    /* 83 */ STORY_UNUSED_FFFFFF83,
    /* 84 */ STORY_CH0_MET_GOOMPA,
    /* 85 */ STORY_CH0_GATE_CRUSHED,
    /* 86 */ STORY_CH0_FELL_OFF_CLIFF,
    /* 87 */ STORY_CH0_GOOMPA_JOINED_PARTY,
    /* 88 */ STORY_CH0_LOOKING_FOR_HAMMER,
    /* 89 */ STORY_CH0_FOUND_HAMMER,
    /* 8a */ STORY_CH0_DEFEATED_JR_TROOPA,
    /* 8b */ STORY_CH0_LEFT_THE_PLAYGROUND,
    /* 8c */ STORY_UNUSED_FFFFFF8C,
    /* 8d */ STORY_CH0_GOOMBARIO_JOINED_PARTY,
    /* 8e */ STORY_CH0_SMASHED_GATE_BLOCK,
    /* 8f */ STORY_CH0_DEFEATED_GOOMBA_BROS,
    /* 90 */ STORY_CH0_DEFEATED_GOOMBA_KING,
    /* 91 */ STORY_CH0_FOUND_GATEHOUSE_SWITCH,
    /* 92 */ STORY_CH0_HIT_GATEHOUSE_SWITCH,
    /* 93 */ STORY_CH0_OPENED_BRIDGE_TO_TOAD_TOWN,
    /* 94 */ STORY_CH0_KAMMY_RETURNED_TO_BOWSER,
    /* 95 */ STORY_CH0_ARRIVED_AT_TOAD_TOWN,
    /* 96 */ STORY_CH0_MET_STAR_SPIRITS,
    /* 97 */ STORY_CH0_BEGAN_PEACH_MISSION,
    /* 98 */ STORY_CH0_TWINK_GAVE_LUCKY_STAR,
    /* 99 */ STORY_CH0_RETURNED_TO_TOAD_TOWN,
    /* 9a */ STORY_CH0_SPOKE_WITH_MERLIN,
    /* 9b */ STORY_CH0_TOLD_MERLIN_ABOUT_DARK_TOADS,
    /* 9c */ STORY_CH0_MERLIN_REVEALED_KOOPA_BROS,

    /* 9d */ STORY_CH1_KNOCKED_SWITCH_FROM_TREE,
    /* 9e */ STORY_CH1_MADE_FIRST_BRIDGE,
    /* 9f */ STORY_CH1_ARRIVED_AT_KOOPA_VILLAGE,
    /* a0 */ STORY_CH1_PROMISED_TO_HELP_KOOPER,
    /* a1 */ STORY_CH1_FUZZY_THIEF_LEFT_TOWN,
    /* a2 */ STORY_CH1_FUZZY_THIEF_RAN_AWAY,
    /* a3 */ STORY_CH1_FUZZY_THIEF_HID_IN_TREE,
    /* a4 */ STORY_CH1_KOOPER_JOINED_PARTY,
    /* a5 */ STORY_CH1_MADE_SECOND_BRIDGE,
    /* a6 */ STORY_CH1_ARRIVED_AT_KOOPA_FORTRESS,
    /* a7 */ STORY_CH1_SPOTTED_BY_KOOPA_BROS,
    /* a8 */ STORY_CH1_KOOPA_BROS_HID_KEY,
    /* a9 */ STORY_CH1_DEFEATED_BASEMENT_GUARD,
    /* aa */ STORY_CH1_LOWERED_FIRST_STAIRS,
    /* ab */ STORY_CH1_KOOPA_BROS_SET_TRAP,
    /* ac */ STORY_CH1_MARIO_ACTIVATED_TRAP,
    /* ad */ STORY_CH1_BOMBETTE_JOINED_PARTY,
    /* ae */ STORY_CH1_DEFEATED_DUNGEON_GUARDS,
    /* af */ STORY_CH1_LOWERED_SECOND_STAIRS,
    /* b0 */ STORY_CH1_RAISED_SUBMERGED_STAIRS,
    /* b1 */ STORY_CH1_KOOPA_BROS_FIRING_BLASTERS,
    /* b2 */ STORY_CH1_DEFEATED_KOOPA_BROS,
    /* b3 */ STORY_CH1_STAR_SPIRIT_RESCUED,
    /* b4 */ STORY_CH1_BEGAN_PEACH_MISSION,
    /* b5 */ STORY_CH1_STAR_SPRIT_DEPARTED,

    /* b6 */ STORY_CH2_DEFEATED_JR_TROOPA,
    /* b7 */ STORY_CH2_RETURNED_TO_TOAD_TOWN,
    /* b8 */ STORY_CH2_ARRIVED_AT_MT_RUGGED,
    /* b9 */ STORY_CH2_SPOKE_WITH_PARAKARRY,
    /* ba */ STORY_CH2_PARAKARRY_JOINED_PARTY,
    /* bb */ STORY_CH2_ARRIVED_AT_DRY_DRY_DESERT,
    /* bc */ STORY_CH2_ARRIVED_AT_DRY_DRY_OUTPOST,
    /* bd */ STORY_CH2_SHADY_MOUSE_LEFT_SHOP,
    /* be */ STORY_CH2_SPOKE_WITH_SHEEK,
    /* bf */ STORY_CH2_SHADY_MOUSE_ENTERED_SHOP,
    /* c0 */ STORY_CH2_BOUGHT_SECRET_ITEMS,
    /* c1 */ STORY_CH2_GOT_PULSE_STONE,
    /* c2 */ STORY_CH2_UNCOVERED_DRY_DRY_RUINS,
    /* c3 */ STORY_CH2_DRAINED_FIRST_SAND_ROOM,
    /* c4 */ STORY_CH2_DRAINED_SECOND_SAND_ROOM,
    /* c5 */ STORY_CH2_GOT_SUPER_HAMMER,
    /* c6 */ STORY_CH2_DRAINED_THIRD_SAND_ROOM,
    /* c7 */ STORY_CH2_SOLVED_ARTIFACT_PUZZLE,
    /* c8 */ STORY_CH2_DEFEATED_TUTANKOOPA,
    /* c9 */ STORY_CH2_STAR_SPIRIT_RESCUED,
    /* ca */ STORY_CH2_BEGAN_PEACH_MISSION,
    /* cb */ STORY_CH2_STAR_SPRIT_DEPARTED,
    /* cc */ STORY_UNUSED_FFFFFFCC,

    /* cd */ STORY_CH3_SAW_BOO_ENTER_FOREST,
    /* ce */ STORY_CH3_INVITED_TO_BOOS_MANSION,
    /* cf */ STORY_CH3_ALLOWED_INTO_FOREVER_FOREST,
    /* d0 */ STORY_CH3_ARRIVED_AT_BOOS_MANSION,
    /* d1 */ STORY_CH3_OPENED_BOOS_MANSION_GATE,
    /* d2 */ STORY_CH3_ENTERED_BOOS_MANSION,
    /* d3 */ STORY_CH3_TRIGGERED_DOOR_JUMP_SCARE,
    /* d4 */ STORY_CH3_GOT_RECORD,
    /* d5 */ STORY_CH3_PLAYED_THE_RECORD,
    /* d6 */ STORY_CH3_GOT_WEIGHT,
    /* d7 */ STORY_CH3_WEIGHED_DOWN_CHANDELIER,
    /* d8 */ STORY_CH3_GOT_SUPER_BOOTS,
    /* d9 */ STORY_CH3_HIT_HUGE_BLUE_SWITCH,
    /* da */ STORY_CH3_GOT_BOO_PORTRAIT,
    /* db */ STORY_CH3_RESTORED_BOO_PORTRAIT,
    /* dc */ STORY_CH3_BOW_JOINED_PARTY,
    /* dd */ STORY_CH3_UNLOCKED_GUSTY_GULCH,
    /* de */ STORY_CH3_ARRIVED_AT_GHOST_TOWN,
    /* df */ STORY_CH3_SAW_TUBBA_EAT_BOO,
    /* e0 */ STORY_CH3_ARRIVED_AT_TUBBAS_MANOR,
    /* e1 */ STORY_UNUSED_FFFFFFE1,
    /* e2 */ STORY_CH3_TUBBA_BEGAN_NAPPING,
    /* e3 */ STORY_CH3_TUBBA_WOKE_UP,
    /* e4 */ STORY_CH3_TUBBA_SMASHED_THE_BRIDGES,
    /* e5 */ STORY_CH3_TUBBA_CHASED_MARIO_IN_HALL,
    /* e6 */ STORY_CH3_TUBBA_CHASED_MARIO_IN_FOYER,
    /* e7 */ STORY_CH3_ESCAPED_TUBBAS_MANOR,
    /* e8 */ STORY_CH3_UNLOCKED_WINDY_MILL,
    /* e9 */ STORY_CH3_WENT_DOWN_THE_WELL,
    /* ea */ STORY_CH3_HEART_FLED_FIRST_TUNNEL,
    /* eb */ STORY_UNUSED_FFFFFFEB,
    /* ec */ STORY_UNUSED_FFFFFFEC,
    /* ed */ STORY_CH3_HEART_FLED_SECOND_TUNNEL,
    /* ee */ STORY_CH3_HEART_ESCAPED_WELL,
    /* ef */ STORY_CH3_HEART_ESCAPED_WINDY_MILL,
    /* f0 */ STORY_CH3_DEFEATED_TUBBA_BLUBBA,
    /* f1 */ STORY_CH3_STAR_SPIRIT_RESCUED,
    /* f2 */ STORY_CH3_BEGAN_PEACH_MISSION,
    /* f3 */ STORY_CH3_STAR_SPRIT_DEPARTED,

    /* f4 */ STORY_CH4_FRYING_PAN_STOLEN,
    /* f5 */ STORY_CH4_MET_WITH_TWINK,
    /* f6 */ STORY_CH4_FOUND_HIDDEN_DOOR,
    /* f7 */ STORY_CH4_ENTERED_THE_TOY_BOX,
    /* f8 */ STORY_CH4_GOT_STOREROOM_KEY,
    /* f9 */ STORY_CH4_RETURNED_STOREROOM_KEY,
    /* fa */ STORY_CH4_GOT_TOY_TRAIN,
    /* fb */ STORY_CH4_RETURNED_TOY_TRAIN,
    /* fc */ STORY_CH4_GOT_FRYING_PAN,
    /* fd */ STORY_CH4_GOT_TAYCE_TS_CAKE,
    /* fe */ STORY_CH4_GAVE_CAKE_TO_GOURMET_GUY,
    /* ff */ STORY_CH4_PULLED_SWITCH_SWITCH,
    /* 00 */ STORY_CH4_SOLVED_COLOR_PUZZLE,
    /* 01 */ STORY_CH4_DEFEATED_LANTERN_GHOST,
    /* 02 */ STORY_CH4_WATT_JOINED_PARTY,
    /* 03 */ STORY_CH4_OPENED_GENERAL_GUY_ROOM,
    /* 04 */ STORY_CH4_DEFEATED_GENERAL_GUY,
    /* 05 */ STORY_CH4_STAR_SPIRIT_RESCUED,
    /* 06 */ STORY_CH4_BEGAN_PEACH_MISSION,
    /* 07 */ STORY_CH4_STAR_SPRIT_DEPARTED,

    /* 08 */ STORY_CH5_WHALE_MOUTH_OPEN,
    /* 09 */ STORY_CH5_ENTERED_WHALE,
    /* 0a */ STORY_CH5_DEFEATED_FUZZIPEDE,
    /* 0b */ STORY_CH5_REACHED_LAVA_LAVA_ISLAND,
    /* 0c */ STORY_CH5_KOLORADO_ESCAPED_FUZZIES,
    /* 0d */ STORY_CH5_KOLORADO_ESCAPED_SPEAR_GUYS,
    /* 0e */ STORY_CH5_YOSHI_CHILDREN_ARE_MISSING,
    /* 0f */ STORY_CH5_SUSHIE_JOINED_PARTY,
    /* 10 */ STORY_CH5_ALL_YOSHI_CHILDREN_RESCUED,
    /* 11 */ STORY_CH5_GOT_JADE_RAVEN,
    /* 12 */ STORY_CH5_MOVED_RAVEN_STATUE,
    /* 13 */ STORY_CH5_DEFEATED_PIRANHAS_MINIBOSS,
    /* 14 */ STORY_CH5_REACHED_RAFAELS_TREE,
    /* 15 */ STORY_CH5_RAFAEL_LEFT_NEST,
    /* 16 */ STORY_CH5_RAFAEL_MOVED_ROOT,
    /* 17 */ STORY_CH5_RAFAEL_WAITING_FOR_MARIO,
    /* 18 */ STORY_CH5_ZIP_LINE_READY,
    /* 19 */ STORY_CH5_ENTERED_MT_LAVA_LAVA,
    /* 1a */ STORY_CH5_KOLORADO_FELL_IN_LAVA,
    /* 1b */ STORY_CH5_LAVA_STREAM_BLOCKED,
    /* 1c */ STORY_CH5_GOT_ULTRA_HAMMER,
    /* 1d */ STORY_CH5_SMASHED_ULTRA_BLOCK,
    /* 1e */ STORY_CH5_KOLORADO_FELL_IN_PIT,
    /* 1f */ STORY_CH5_KOLORADO_AT_DEAD_END,
    /* 20 */ STORY_CH5_HIDDEN_PASSAGE_OPEN,
    /* 21 */ STORY_CH5_KOLORADO_RAN_AHEAD,
    /* 22 */ STORY_CH5_KOLORADO_IN_TREASURE_ROOM,
    /* 23 */ STORY_CH5_DEFEATED_LAVA_PIRANHA,
    /* 24 */ STORY_CH5_MT_LAVA_LAVA_ERUPTING,
    /* 25 */ STORY_CH5_OPENED_ESCAPE_ROUTE,
    /* 26 */ STORY_CH5_BEGAN_PEACH_MISSION,
    /* 27 */ STORY_CH5_STAR_SPIRIT_DEPARTED,

    /* 28 */ STORY_CH6_TRADED_VASE_FOR_SEED,
    /* 29 */ STORY_CH6_RETURNED_TO_TOAD_TOWN,
    /* 2a */ STORY_CH6_FLOWER_GATE_OPEN,
    /* 2b */ STORY_CH6_ARRIVED_AT_FLOWER_FIELDS,
    /* 2c */ STORY_CH6_ASKED_TO_DEFEAT_MONTY_MOLES,
    /* 2d */ STORY_CH6_GOT_MAGICAL_BEAN,
    /* 2e */ STORY_CH6_GOT_FERTILE_SOIL,
    /* 2f */ STORY_CH6_GOT_CRYSTAL_BERRY,
    /* 30 */ STORY_CH6_GOT_WATER_STONE,
    /* 31 */ STORY_CH6_FILLED_SPRING_WITH_WATER,
    /* 32 */ STORY_CH6_SPOKE_WITH_THE_SUN,
    /* 33 */ STORY_CH6_LAKILESTER_JOINED_PARTY,
    /* 34 */ STORY_CH6_DEFEATED_PUFF_PUFF_GUARDS,
    /* 35 */ STORY_CH6_DESTROYED_PUFF_PUFF_MACHINE,
    /* 36 */ STORY_CH6_WISTERWOOD_GAVE_HINT,
    /* 37 */ STORY_CH6_GREW_MAGIC_BEANSTALK,
    /* 38 */ STORY_CH6_DEFEATED_HUFF_N_PUFF,
    /* 39 */ STORY_CH6_STAR_SPIRIT_RESCUED,
    /* 3a */ STORY_CH6_BEGAN_PEACH_MISSION,
    /* 3b */ STORY_CH6_STAR_SPRIT_DEPARTED,

    /* 3c */ STORY_CH7_RETURNED_TO_TOAD_TOWN,
    /* 3d */ STORY_CH7_INVITED_TO_STARBORN_VALLEY,
    /* 3e */ STORY_CH7_ARRIVED_AT_SHIVER_CITY,
    /* 3f */ STORY_CH7_MAYOR_MURDER_MYSTERY,
    /* 40 */ STORY_CH7_SHATTERED_FROZEN_POND,
    /* 41 */ STORY_CH7_SPOKE_WITH_HERRINGWAY,
    /* 42 */ STORY_CH7_HERRINGWAY_AT_MAYORS_HOUSE,
    /* 43 */ STORY_CH7_MAYOR_MURDER_SOLVED,
    /* 44 */ STORY_CH7_DEFEATED_JR_TROOPA,
    /* 45 */ STORY_CH7_DEFEATED_MONSTAR,
    /* 46 */ STORY_CH7_ARRIVED_AT_STARBORN_VALLEY,
    /* 47 */ STORY_CH7_MERLE_APOLOGIZED,
    /* 48 */ STORY_CH7_GOT_SNOWMAN_SCARF,
    /* 49 */ STORY_CH7_GOT_SNOWMAN_BUCKET,
    /* 4a */ STORY_CH7_UNLOCKED_SHIVER_MOUNTAIN,
    /* 4b */ STORY_CH7_DEFEATED_FIRST_DUPLIGHOST,
    /* 4c */ STORY_CH7_GOT_STAR_STONE,
    /* 4d */ STORY_CH7_RAISED_FROZEN_STAIRS,
    /* 4e */ STORY_CH7_ARRIVED_AT_CRYSTAL_PALACE,
    /* 4f */ STORY_CH7_DEFEATED_MIRROR_DUPLIGHOSTS,
    /* 50 */ STORY_CH7_DEFEATED_BOMBETTE_DUPLIGHOSTS,
    /* 51 */ STORY_CH7_DEFEATED_CLUBBAS,
    /* 52 */ STORY_CH7_DEFEATED_KOOPER_DUPLIGHOSTS,
    /* 53 */ STORY_CH7_EXTENDED_PALACE_BRIDGE,
    /* 54 */ STORY_CH7_FOUND_HIDDEN_ROOM_UNDER_STATUE,
    /* 55 */ STORY_CH7_SOLVED_ALBINO_DINO_PUZZLE,
    /* 56 */ STORY_CH7_DEFEATED_CRYSTAL_KING,
    /* 57 */ STORY_CH7_STAR_SPIRIT_RESCUED,
    /* 58 */ STORY_CH7_BEGAN_PEACH_MISSION,
    /* 59 */ STORY_CH7_STAR_SPRIT_DEPARTED,

    /* 5a */ STORY_CH8_OPENED_PATH_TO_STAR_WAY,
    /* 5b */ STORY_CH8_REACHED_STAR_HAVEN,
    /* 5c */ STORY_CH8_STAR_SHIP_ACTIVATED,
    /* 5d */ STORY_UNUSED_5D,
    /* 5e */ STORY_CH8_REACHED_BOWSERS_CASTLE,
    /* 5f */ STORY_CH8_REACHED_PEACHS_CASTLE,

    /* 60 */ STORY_EPILOGUE,
};

enum {
    /* 0x00 */ HUD_ELEMENT_OP_End,
    /* 0x01 */ HUD_ELEMENT_OP_SetRGBA,
    /* 0x02 */ HUD_ELEMENT_OP_SetCI,
    /* 0x03 */ HUD_ELEMENT_OP_Restart,
    /* 0x04 */ HUD_ELEMENT_OP_Loop,
    /* 0x05 */ HUD_ELEMENT_OP_SetTileSize,
    /* 0x06 */ HUD_ELEMENT_OP_SetSizesAutoScale,
    /* 0x07 */ HUD_ELEMENT_OP_SetSizesFixedScale,
    /* 0x08 */ HUD_ELEMENT_OP_SetVisible,
    /* 0x09 */ HUD_ELEMENT_OP_SetHidden,
    /* 0x0A */ HUD_ELEMENT_OP_AddTexelOffsetX,
    /* 0x0B */ HUD_ELEMENT_OP_AddTexelOffsetY,
    /* 0x0C */ HUD_ELEMENT_OP_SetTexelOffset,
    /* 0x0D */ HUD_ELEMENT_OP_SetImage,
    /* 0x0E */ HUD_ELEMENT_OP_SetScale,
    /* 0x0F */ HUD_ELEMENT_OP_SetAlpha,
    /* 0x10 */ HUD_ELEMENT_OP_RandomDelay,
    /* 0x11 */ HUD_ELEMENT_OP_Delete,
    /* 0x12 */ HUD_ELEMENT_OP_UseIA8,
    /* 0x13 */ HUD_ELEMENT_OP_SetCustomSize,
    /* 0x14 */ HUD_ELEMENT_OP_RandomRestart,
    /* 0x15 */ HUD_ELEMENT_OP_op_15,
    /* 0x16 */ HUD_ELEMENT_OP_op_16,
    /* 0x17 */ HUD_ELEMENT_OP_RandomBranch,
    /* 0x18 */ HUD_ELEMENT_OP_SetFlags,
    /* 0x19 */ HUD_ELEMENT_OP_ClearFlags,
    /* 0x1A */ HUD_ELEMENT_OP_PlaySound,
    /* 0x1B */ HUD_ELEMENT_OP_SetPivot,
};

#endif
