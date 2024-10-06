#ifndef FP_H
#define FP_H
#include "common.h"
#include "sys/settings.h"
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
    Vec3f eye;
    Vec3f obj;
    f32 pitch;
    f32 yaw;
} FpCam;

typedef struct {
    u32 buttons;
    s8 stickX;
    s8 stickY;
} FpControllerMask;

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
    FpCam cam;
    bool resetCam;
    FpControllerMask inputMask;
    bool camEnabledBefore;
    pm_Camera savedCam;
    s8 freeCamMoveSpeed;
    s8 freeCamPanSpeed;
} FpCtxt;

extern FpCtxt fp;

void fpLog(const char *fmt, ...);
bool fpWarp(enum Areas area, u16 map, u16 entrance);
void fpSetGlobalFlag(s32 flagIndex, bool value);
void fpSetAreaFlag(s32 flagIndex, bool value);
void fpSetGlobalByte(s32 byteIndex, s8 value);
s32 fpImportFile(const char *path, void *data);
void fpSetInputMask(u32 pad, s8 x, s8 y);
void fpUpdateCam(void);
void fpSaveSettingsProc(struct MenuItem *item, void *data);
void setFreeCamMoveSpeed(s8 s);
void setFreeCamPanSpeed(s8 s);

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
