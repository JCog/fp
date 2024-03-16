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
    CHEAT_MAX
};

struct WatchInfo {
    u8 type        : 4;
    u8 anchored    : 1;
    u8 positionSet : 1;
};

struct SettingsBits {
    u32 fontResource   : 4;
    u32 dropShadow     : 1;
    u32 inputDisplay   : 1;
    u32 log            : 1;
    u32 timerShow      : 1;
    u32 timerLogging   : 1;
    u32 battleDebug    : 3;
    u32 quizmoDebug    : 1;
    u32 watchesVisible : 1;
};

struct TrainerBits {
    u32 bowserEnabled : 1;
    u32 lzsEnabled    : 1;
    u32 acEnabled     : 1;
    u32 clippyEnabled : 1;
};

struct SettingsData {
    /* order elements by size for space-efficient packing */
    u32 watchAddress[SETTINGS_WATCHES_MAX];
    u32 cheats;
    struct SettingsBits bits;
    struct TrainerBits trainerBits;
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
