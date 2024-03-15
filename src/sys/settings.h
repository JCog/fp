#ifndef SETTINGS_H
#define SETTINGS_H
#include "commands.h"
#include "common.h"

#define SETTINGS_SAVE_FILE_SIZE 0x1380
#define SETTINGS_PROFILE_MAX    4
#define SETTINGS_VERSION        6
#define SETTINGS_FIO_PAGE       7

#define SETTINGS_WATCHES_MAX    18
#define SETTINGS_BIND_MAX       COMMAND_MAX
#define SETTINGS_LOG_MAX        4

enum Cheats {
    CHEAT_HP,
    CHEAT_FP,
    CHEAT_COINS,
    CHEAT_STAR_POWER,
    CHEAT_STAR_PIECES,
    CHEAT_PERIL,
    CHEAT_AUTO_MASH,
    CHEAT_AUTO_ACTION_CMD,
    CHEAT_PEEKABOO,
    CHEAT_BRIGHTEN_ROOM,
    CHEAT_HIDE_HUD,
    CHEAT_MUTE_MUSIC,
    CHEAT_QUIZMO,
    CHEAT_MAX
};

struct WatchInfo {
    u8 type        : 4;
    u8 anchored    : 1;
    u8 positionSet : 1;
};

struct SettingsData {
    /* order elements by size for space-efficient packing */
    u32 watchAddress[SETTINGS_WATCHES_MAX];
    u32 cheats;
    s16 menuX;
    s16 menuY;
    s16 inputDisplayX;
    s16 inputDisplayY;
    s16 logX;
    s16 logY;
    s16 timerX;
    s16 timerY;
    s16 watchX[SETTINGS_WATCHES_MAX];
    s16 watchY[SETTINGS_WATCHES_MAX];
    u16 binds[SETTINGS_BIND_MAX];
    s8 cheatEnemyContact;
    s8 controlStickRange;
    u8 controlStick;
    u8 menuFontResource;
    u8 menuDropShadow;
    u8 inputDisplay;
    u8 log;
    u8 timerShow;
    u8 timerLogging;
    u8 trainerBowserEnabled;
    u8 trainerLzsEnabled;
    u8 trainerAcEnabled;
    u8 trainerClippyEnabled;
    u8 battleDebug;
    u8 watchesVisible;
    struct WatchInfo watchInfo[SETTINGS_WATCHES_MAX];
    u8 nWatches;
};

struct SettingsHeader {
    u16 version;
    u16 dataSize;
    u16 dataChecksum;
};

struct Settings {
    struct SettingsHeader header;
    struct SettingsData data;
};

void settingsLoadDefault(void);
void applyMenuSettings(void);
void settingsSave(s32 profile);
bool settingsLoad(s32 profile);

extern struct SettingsData *settings;

#endif
