#ifndef WARP_INFO_H
#define WARP_INFO_H
#include "common.h"

#define AREA_COUNT 0x1C

struct MapInfo {
    const char *mapName;
    s32 entranceCount;
};

struct AreaInfo {
    const char *areaName;
    s32 mapCount;
    struct MapInfo maps[];
};

// clang-format off
struct AreaInfo areaGoombaVillage = {
    "goomba village",
    0x11,
    {
        {"mario's landing spot", 0x1},
        {"village", 0x6},
        {"outside playground", 0x3},
        {"playground", 0x2},
        {"outside village", 0x2},
        {"trail 2", 0x2},
        {"goomba bros", 0x2},
        {"trail 1", 0x2},
        {"outside toad town", 0x2},
        {"goomba king's castle", 0x2},
        {"outside castle", 0x2},
        {"mario's house", 0x5},
        {"title screen", 0x1},
        {"chapter screen", 0x9},
        {"card obtained", 0x7},
        {"save screen", 0x1},
        {"the end", 0x1},
    }
};

struct AreaInfo areaToadTown = {
    "toad town",
    0x8,
    {
        {"debug hub", 0xB},
        {"main gate", 0x8},
        {"central plaza", 0x6},
        {"below plaza", 0x6},
        {"train station", 0x4},
        {"residential area", 0x6},
        {"port", 0x4},
        {"riding whale", 0x2},
    }
};

struct AreaInfo areaSewers = {
    "sewers",
    0x16,
    {
        {"pipe hub", 0x5},
        {"shrink stomp", 0x1},
        {"west of pipe hub", 0x3},
        {"alternating floating platforms", 0x4},
        {"power smash", 0x1},
        {"toad town entrance", 0x4},
        {"rising platforms to super block", 0x3},
        {"lava lava island pipe", 0x5},
        {"boo's mansion pipe", 0x3},
        {"push block puzzle super block", 0x1},
        {"ultra blocks super block", 0x1},
        {"west of rip cheato", 0x2},
        {"rip cheato", 0x2},
        {"pipe to shiver city", 0x2},
        {"west of shrink stomp", 0x2},
        {"toad town island super block", 0x1},
        {"spike room", 0x3},
        {"outside blue door", 0x3},
        {"inside blue door", 0x2},
        {"narrow spiny path", 0x2},
        {"east of ultra boots", 0x2},
        {"ultra boots", 0x1},
    }
};

struct AreaInfo areaWhale = {
    "whale",
    0x2,
    {
        {"mouth", 0x2},
        {"belly", 0x1},
    }
};

struct AreaInfo areaPeachsCastle = {
    "peach's castle",
    0x19,
    {
        {"main 1 (light)", 0x7},
        {"main 2 (light)", 0x7},
        {"hallway 1 (light)", 0x2},
        {"hallway bowser (light)", 0x3},
        {"main 1 (dark)", 0x5},
        {"main 2 (dark)", 0x8},
        {"hallway 1 (dark)", 0x2},
        {"hallway bowser (dark)", 0x4},
        {"peach's room", 0xc},
        {"bowser's diary", 0x3},
        {"library", 0x1},
        {"mysterious chest", 0x1},
        {"gourmet guy", 0x1},
        {"kitchen", 0x1},
        {"toad house", 0x1},
        {"quiz show (packed up)", 0x1},
        {"far-right stairs", 0x2},
        {"outside hallway", 0x2},
        {"spiral staircase", 0x2},
        {"final bowser", 0x3},
        {"peach's balcony 1", 0x3},
        {"peach's balcony 2", 0x3},
        {"secret passage", 0x2},
        {"quiz show (unused)", 0x1},
        {"quiz show", 0x1},
    }
};

struct AreaInfo areaStarRegion = {
    "star region",
    0x9,
    {
        {"outside shooting star summit", 0x4},
        {"shooting star summit", 0x2},
        {"star way", 0x2},
        {"star haven", 0x2},
        {"outside palace", 0x8},
        {"palace", 0x5},
        {"merluvlee's house", 0x1},
        {"space", 0x6},
        {"flying", 0x3},
    }
};

struct AreaInfo areaKoopaVillagePleasantPath = {
    "koopa village/pleasant path",
    0x9,
    {
        {"west village", 0x2},
        {"east village", 0x4},
        {"hp plus", 0x2},
        {"fuzzy mini game", 0x1},
        {"outside toad town", 0x2},
        {"west switch bridge", 0x2},
        {"outside koopa village", 0x3},
        {"east switch bridge", 0x2},
        {"outside fortress", 0x5},
    }
};

struct AreaInfo areaKoopaBrosFortress = {
    "koopa bros. fortress",
    0xB,
    {
        {"outside fortress", 0x6},
        {"inside fortress", 0x4},
        {"west of kooper puzzle", 0x5},
        {"kooper switch puzzle", 0x5},
        {"east of kooper puzzle", 0x6},
        {"block trap", 0x4},
        {"jail", 0x4},
        {"pit", 0x2},
        {"fire bars", 0x1},
        {"cannons", 0x2},
        {"koopa bros. fight", 0x1},
    }
};

struct AreaInfo areaMtRugged = {
    "mt. rugged",
    0x7,
    {
        {"whacka", 0x2},
        {"quake hammer", 0x4},
        {"magical seed", 0x3},
        {"damage dodge", 0x1},
        {"buzzar", 0x2},
        {"train", 0x2},
        {"riding train", 0x2},
    }
};

struct AreaInfo areaDryDryOutpost = {
    "dry dry outpost",
    0x2,
    {
        {"west", 0x5},
        {"east", 0x1},
    }
};

struct AreaInfo areaDryDryDesert = {
    "dry dry desert",
    0x32,
    {
        {"(0,0) fright jar", 0x4},
        {"(1,0)", 0x4},
        {"(2,0) outside ruins", 0x8},
        {"(3,0)", 0x4},
        {"(4,0)", 0x4},
        {"(5,0) lots of pokeys", 0x5},
        {"(6,0)", 0x4},
        {"(0,1) hidden thunder rage", 0x4},
        {"(1,1)", 0x4},
        {"(2,1)", 0x4},
        {"(3,1)", 0x4},
        {"(4,1) tweester to (5,0)", 0x5},
        {"(5,1)", 0x4},
        {"(6,1)", 0x4},
        {"(0,2) hidden mushrooms", 0x4},
        {"(1,2)", 0x4},
        {"(2,2)", 0x4},
        {"(3,2) tweester to (4,1)", 0x4},
        {"(4,2) runaway pay", 0x5},
        {"(5,2) coin blocks", 0x4},
        {"(6,2)", 0x4},
        {"(0,3) kolorado's camp", 0x6},
        {"(1,3)", 0x4},
        {"(2,3)", 0x5},
        {"(3,3) tweester to (4,2)", 0x4},
        {"(4,3) nomadimouse", 0x4},
        {"(5,3)", 0x4},
        {"(6,3) outside outpost", 0x4},
        {"(0,4) coin block", 0x4},
        {"(1,4) tweester to (2,3)", 0x4},
        {"(2,4)", 0x4},
        {"(3,4)", 0x4},
        {"(4,4)", 0x4},
        {"(5,4) spin attack", 0x5},
        {"(6,4) hidden life shroom", 0x4},
        {"(0,5)", 0x4},
        {"(1,5)", 0x4},
        {"(2,5) coin block", 0x4},
        {"(3,5)", 0x4},
        {"(4,5) tweester to (5,4)", 0x4},
        {"(5,5) coin block", 0x4},
        {"(6,5) oasis", 0x4},
        {"(0,6)", 0x4},
        {"(1,6) attack fx c", 0x4},
        {"(2,6)", 0x4},
        {"(3,6)", 0x4},
        {"(4,6)", 0x4},
        {"(5,6)", 0x4},
        {"(6,6) coin blocks", 0x4},
        {"desert entrance", 0x2},
    }
};

struct AreaInfo areaDryDryRuins = {
    "dry dry ruins",
    0x11,
    {
        {"entrance", 0x2},
        {"spike shield", 0x3},
        {"sand switch 1", 0x3},
        {"top-right hub", 0x5},
        {"pyramid stone", 0x1},
        {"sand switch 2", 0x2},
        {"pokey block", 0x2},
        {"left hub", 0x4},
        {"super hammer", 0x1},
        {"super block", 0x3},
        {"stone pedestals", 0x4},
        {"sand swtich 3", 0x4},
        {"lunar stone", 0x1},
        {"diamond stone", 0x1},
        {"tutankoopa", 0x1},
        {"bottom hallway", 0x2},
        {"outside tutankoopa", 0x2},
    }
};

struct AreaInfo areaForeverForest = {
    "forever forest",
    0xC,
    {
        {"dancing flowers", 0x4},
        {"tree with eyes", 0x4},
        {"blue flowers", 0x4},
        {"magical seed", 0x4},
        {"flashing mushrooms", 0x4},
        {"fading/rotating flowers", 0x5},
        {"exit", 0x4},
        {"hp plus", 0x4},
        {"fp plus", 0x4},
        {"outside toad town", 0x2},
        {"outside boo's mansion", 0x4},
        {"outside gusty gulch", 0x2},
    }
};

struct AreaInfo areaBoosMansion = {
    "boo's mansion",
    0x9,
    {
        {"entrance", 0x7},
        {"falling chandelier", 0x3},
        {"shop", 0x3},
        {"super boots", 0x1},
        {"throwback mario", 0x1},
        {"boo's portrait", 0x2},
        {"weight mini game", 0x1},
        {"record mini game", 0x1},
        {"bow's room", 0x3},
    }
};

struct AreaInfo areaGustyGulch = {
    "gusty gulch",
    0xB,
    {
        {"repel gel room", 0x2},
        {"package boo", 0x2},
        {"parakarry gap", 0x2},
        {"tubba ambush", 0x2},
        {"outside windmill", 0x4},
        {"inside windmill", 0x2},
        {"windmill spring", 0x2},
        {"windmill hallway 1", 0x2},
        {"tubba's heart", 0x1},
        {"windmill hallway 2", 0x2},
        {"windmill hallway 3", 0x2},
    }
};

struct AreaInfo areaTubbasMansion = {
    "tubba's mansion",
    0x13,
    {
        {"outside mansion", 0x3},
        {"center room", 0x7},
        {"bottom-left hallway", 0x4},
        {"grandfather clock", 0x6},
        {"super block", 0x2},
        {"staircase with wooden panel", 0x1},
        {"bottom-left key chest", 0x2},
        {"star piece on small table", 0x1},
        {"bottom-right hallway", 0x2},
        {"middle-left hallway", 0x4},
        {"above d-down jump", 0x2},
        {"d-down jump", 0x4},
        {"spikes", 0x1},
        {"mega rush", 0x1},
        {"top-left staircase", 0x2},
        {"top-left hallway", 0x3},
        {"clubbas guarding key", 0x1},
        {"outside tubba's bedroom", 0x2},
        {"tubba's bedroom", 0x1},
    }
};

struct AreaInfo areaShyGuysToyBox = {
    "shy guy's toy box",
    0x11,
    {
        {"calculator", 0x1},
        {"bombable barricade", 0x2},
        {"blue station", 0x7},
        {"storeroom key", 0x1},
        {"gourmet guy", 0x4},
        {"pink station", 0x5},
        {"frying pan", 0x1},
        {"green station", 0x4},
        {"dictionary", 0x1},
        {"red station", 0x4},
        {"outside big lantern ghost", 0x2},
        {"big lantern ghost", 0x1},
        {"anti guy", 0x2},
        {"outside general guy", 0x2},
        {"general guy", 0x1},
        {"riding train", 0x2},
        {"east of gourmet guy", 0x6},
    }
};

struct AreaInfo areaLavaLavaIsland = {
    "lava lava island",
    0x16,
    {
        {"whale", 0x3},
        {"beach", 0x2},
        {"yoshi chief", 0x2},
        {"shop", 0x4},
        {"vase chest", 0x4},
        {"power quake", 0x4},
        {"jade raven statue", 0x4},
        {"yellow yoshi", 0x1},
        {"super block", 0x3},
        {"hidden pipe", 0x4},
        {"blue yoshi", 0x1},
        {"dark cave", 0x1},
        {"jungle entrance", 0x2},
        {"geyser puzzle", 0x2},
        {"vine puzzle", 0x2},
        {"jungle putrid piranha fight", 0x2},
        {"outside tree", 0x5},
        {"inside tree (bottom)", 0x2},
        {"tree branch", 0x2},
        {"inside tree (top)", 0x2},
        {"volcano entrance", 0x4},
        {"raphael", 0x1},
    }
};

struct AreaInfo areaVolcano = {
    "volcano",
    0x11,
    {
        {"entrance", 0x2},
        {"sinking platforms", 0x3},
        {"central hub", 0x5},
        {"fire bar super block", 0x1},
        {"hallway to block puzzle", 0x2},
        {"block puzzle", 0x3},
        {"ultra hammer", 0x1},
        {"dizzy stomp", 0x1},
        {"pulley super block", 0x3},
        {"spike ball hallway", 0x2},
        {"fire bar island", 0x2},
        {"spike ball puzzle", 0x2},
        {"outside lava piranha", 0x3},
        {"lava piranha", 0x4},
        {"escape 1", 0x2},
        {"escape 2", 0x1},
        {"escape 3", 0x1},
    }
};

struct AreaInfo areaFlowerFields = {
    "flower fields",
    0x14,
    {
        {"central hub", 0xc},
        {"petunia", 0x3},
        {"posie", 0x2},
        {"yellow flower", 0x2},
        {"west of petunia", 0x2},
        {"lily", 0x3},
        {"maze", 0x8},
        {"rosie", 0x2},
        {"lakilester fight", 0x2},
        {"bubble plant", 0x2},
        {"sun tower", 0x2},
        {"spike puzzle", 0x2},
        {"west of machine", 0x2},
        {"puff-puff machine", 0x2},
        {"clouds entrance", 0x4},
        {"huff n. puff fight", 0x1},
        {"well", 0x1},
        {"blue flower", 0x2},
        {"bubble berry tree", 0x3},
        {"red flower", 0x2},
    }
};

struct AreaInfo areaShiverRegion = {
    "shiver region",
    0xC,
    {
        {"west city", 0x2},
        {"center city", 0x4},
        {"west snowfield", 0x2},
        {"center snowfield", 0x3},
        {"east snowfield", 0x2},
        {"starborn valley", 0x1},
        {"mountain start", 0x2},
        {"super block", 0x2},
        {"item pedestals", 0x2},
        {"ice staircase", 0x3},
        {"east city", 0x3},
        {"star stone", 0x1},
    }
};

struct AreaInfo areaCrystalPalace = {
    "crystal palace",
    0x20,
    {
        {"outside palace", 0x5},
        {"main hub", 0x6},
        {"south of main hub", 0x3},
        {"north of main hub", 0x3},
        {"blue key chest", 0x1},
        {"shooting star", 0x1},
        {"bombette duplighost puzzle", 0x2},
        {"north-west basement hallway", 0x2},
        {"red key chest", 0x1},
        {"p-down d-up chest", 0x1},
        {"blue doors", 0x4},
        {"east of blue doors", 0x2},
        {"cave outside palace", 0x1},
        {"red doors", 0x4},
        {"clubba statues", 0x3},
        {"kooper duplighost puzzle", 0x2},
        {"mini hub", 0x5},
        {"south of mini hub", 0x2},
        {"north of mini hub", 0x2},
        {"palace key chest", 0x1},
        {"p-up d-down chest", 0x1},
        {"kooper switch", 0x4},
        {"statue puzzle", 0x3},
        {"crystal king fight", 0x1},
        {"red switch puzzle", 0x3},
        {"west of statue puzzle", 0x4},
        {"triple dip chest", 0x3},
        {"south-east bottom hallway", 0x2},
        {"north-east bottom hallway", 0x2},
        {"south-east top hallway", 0x2},
        {"north-east top hallway", 0x2},
        {"west of crystal king", 0x2},
    }
};

struct AreaInfo areaBowsersCastle = {
    "bowser's castle",
    0x32,
    {
        {"dark basement 1", 0x2},
        {"dark basement 2", 0x2},
        {"post-basement", 0x2},
        {"kooper switch puzzle", 0x2},
        {"bombette switch puzzle", 0x2},
        {"outside jail (hardened)", 0x5},
        {"outside jail (lava)", 0x5},
        {"sinking lava platforms", 0x2},
        {"lava platform switches", 0x3},
        {"lavafall", 0x3},
        {"lava key chest", 0x1},
        {"lava harden switch", 0x1},
        {"jail", 0x2},
        {"hub 1", 0x4},
        {"hub 2", 0x4},
        {"outside stair puzzle", 0x4},
        {"stair puzzle", 0x4},
        {"hallway to door 1", 0x2},
        {"hallway to flood room", 0x2},
        {"west of stair puzzle", 0x3},
        {"duplighost fight", 0x2},
        {"outside castle (starship)", 0x6},
        {"balcony", 0x2},
        {"outside castle", 0x4},
        {"starship dock", 0x2},
        {"inside castle", 0x2},
        {"jail door", 0x4},
        {"quiz door", 0x2},
        {"jr fight door", 0x2},
        {"stairs to toad house 1", 0x2},
        {"toad house 1", 0x1},
        {"stairs to toad house 2", 0x2},
        {"toad house 2", 0x1},
        {"shop", 0x1},
        {"key to stair puzzle", 0x1},
        {"ultra shroom reward", 0x1},
        {"west of duplighost fight", 0x2},
        {"dry bones bowser blocks", 0x2},
        {"bowser block hallway 1", 0x2},
        {"bony beetle bowser blocks", 0x3},
        {"bowser block hallway 2", 0x2},
        {"hammer bro bowser blocks", 0x3},
        {"bowser block hallway 3", 0x2},
        {"fake key room", 0x1},
        {"bowser block hallway 4", 0x2},
        {"quiz door key", 0x1},
        {"stairs to peach's castle", 0x2},
        {"cannon room", 0x2},
        {"west flood room", 0x4},
        {"east flood room", 0x4},
    }
};

struct AreaInfo areaOutsidePeachsCastle = {
    "outside peach's castle",
    0x5,
    {
        {"prologue castle", 0x5},
        {"castle ruins", 0x2},
        {"top of bowser's castle", 0x3},
        {"zoomed out", 0x7},
        {"castle lifting off", 0x1},
    }
};

struct AreaInfo areaCredits = {
    "credits",
    0x2,
    {
        {"day", 0x1},
        {"night", 0x2},
    }
};

struct AreaInfo areaMiniGames = {
    "mini games",
    0x4,
    {
        {"reception", 0x3},
        {"jump attack", 0x1},
        {"smash attack", 0x1},
        {"debug room", 0x1},
    }
};

struct AreaInfo areaGameOver = {
    "game over",
    0x1,
    {
        {"game over", 0x1},
    }
};

struct AreaInfo areaDebug = {
    "debug",
    0x9,
    {
        {"test 0", 0x2},
        {"test 1", 0x2},
        {"test 2", 0x2},
        {"test 3", 0x2},
        {"test 4", 0x4},
        {"test 5", 0x1},
        {"test 6", 0x1},
        {"test 7", 0x1},
        {"test 8", 0x1},
    }
};
// clang-format on

struct AreaInfo *areaInfoList[] = {
    /* 0x00 */ &areaGoombaVillage,
    /* 0x01 */ &areaToadTown,
    /* 0x02 */ &areaSewers,
    /* 0x03 */ &areaWhale,
    /* 0x04 */ &areaPeachsCastle,
    /* 0x05 */ &areaStarRegion,
    /* 0x06 */ &areaKoopaVillagePleasantPath,
    /* 0x07 */ &areaKoopaBrosFortress,
    /* 0x08 */ &areaMtRugged,
    /* 0x09 */ &areaDryDryOutpost,
    /* 0x0A */ &areaDryDryDesert,
    /* 0x0B */ &areaDryDryRuins,
    /* 0x0C */ &areaForeverForest,
    /* 0x0D */ &areaBoosMansion,
    /* 0x0E */ &areaGustyGulch,
    /* 0x0F */ &areaTubbasMansion,
    /* 0x10 */ &areaShyGuysToyBox,
    /* 0x11 */ &areaLavaLavaIsland,
    /* 0x12 */ &areaVolcano,
    /* 0x13 */ &areaFlowerFields,
    /* 0x14 */ &areaShiverRegion,
    /* 0x15 */ &areaCrystalPalace,
    /* 0x16 */ &areaBowsersCastle,
    /* 0x17 */ &areaOutsidePeachsCastle,
    /* 0x18 */ &areaCredits,
    /* 0x19 */ &areaMiniGames,
    /* 0x1A */ &areaGameOver,
    /* 0x1B */ &areaDebug,
};

#endif
