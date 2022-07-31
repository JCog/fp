#ifndef FP_H
#define FP_H
#include "common.h"
#include "settings.h"
#include <vector/vector.h>

enum CamBhv {
    CAMBHV_MANUAL,
    CAMBHV_BIRDSEYE,
    CAMBHV_RADIAL,
};

struct LogEntry {
    char *msg;
    s32 age;
};

typedef struct {
    bool ready;
    struct Menu *mainMenu;
    struct Menu *global;
    struct Menu *menuMem;
    struct Menu *menuWatches;
    struct MenuItem *menuWatchlist;
    u8 profile;
    bool settingsLoaded;
    bool versionShown;
    s64 cpuCounter;
    s32 cpuCounterFreq;
    bool timerMoving;
    bool menuActive;
    struct LogEntry log[SETTINGS_LOG_MAX];
    Vec3f savedPos;
    f32 savedFacingAngle;
    f32 savedMovementAngle;
    u16 savedArea;
    u16 savedMap;
    u16 savedEntrance;
    s8 aceFrameWindow;
    u16 aceLastTimer;
    bool aceLastFlagStatus;
    bool aceLastJumpStatus;
    bool turbo;
    bool warp;
    u8 warpDelay;
    char *lastImportedSavePath;
    bool freeCam;
    bool lockCam;
    enum CamBhv camBhv;
    s16 camDistMin;
    s16 camDistMax;
    f32 camPitch;
    f32 camYaw;
    Vec3f camPos;
    pm_Controller inputMask;
    bool camEnabledBefore;
    s32 pendingFrames;
    Gfx *bgMasterGfxPos;
    Gfx *frameMasterGfxPos;
} FpCtxt;

extern FpCtxt fp;

void fpLog(const char *fmt, ...);
bool fpWarp(enum Areas area, u16 map, u16 entrance);
void fpSetGlobalFlag(s32 flagIndex, bool value);
void fpSetAreaFlag(s32 flagIndex, bool value);
void fpSetGlobalByte(s32 byteIndex, s8 value);
s32 fpImportFile(const char *path, void *data);
void fpSetInputMask(u16 pad, u8 x, u8 y);
void fpUpdateCam(void);

struct Menu *createWarpsMenu(void);
struct Menu *createCheatsMenu(void);
struct Menu *createPlayerMenu(void);
struct Menu *createFileMenu(void);
struct Menu *createPracticeMenu(void);
struct Menu *createDebugMenu(void);
struct Menu *createSettingsMenu(void);
struct Menu *createCameraMenu(void);

#define CHEAT_ACTIVE(cheat) (settings->cheats & (1 << cheat))

#endif
