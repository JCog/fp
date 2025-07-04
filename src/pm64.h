#ifndef PM64_H
#define PM64_H
#include "types.h"
#include <n64/gbi.h>
#include <n64/message.h>
#include <stdarg.h>

#define JP                0x00
#define US                0x01

#define SCREEN_WIDTH      320
#define SCREEN_HEIGHT     240

#define ICON_PALETTE_SIZE 32

#if PM64_VERSION == US
#define SCRIPTS_GLOBAL_START         0x801049B0
#define ICONS_ITEMS_ROM_START        0x1CC310
#define ICONS_PARTNERS_ROM_START     0x97890
#define ICONS_STAR_SPIRITS_ROM_START 0x963B0
#define ICONS_BP_ROM_START           0x134520
#else
#define SCRIPTS_GLOBAL_START         0x80104b40
#define ICONS_ITEMS_ROM_START        0x1D4720
#define ICONS_PARTNERS_ROM_START     0x97A20
#define ICONS_STAR_SPIRITS_ROM_START 0x96540
#define ICONS_BP_ROM_START           0x13C820
#endif

#define SCRIPT_BOWSER_HALLWAY_TAKE_TURN 0x80222B48
#define SCRIPT_BOWSER_HALLWAY_FIRE      0x802267C8
#define SCRIPT_BOWSER_HALLWAY_STOMP     0x802248A4
#define SCRIPT_BOWSER_HALLWAY_CLAW      0x80225898
#define SCRIPT_BOWSER_HALLWAY_WAVE      0x80225DD0
#define SCRIPT_BOWSER_FINAL_1_TAKE_TURN 0x80228490
#define SCRIPT_BOWSER_FINAL_1_FIRE      0x8022BFF8
#define SCRIPT_BOWSER_FINAL_1_STOMP     0x8022A0F4
#define SCRIPT_BOWSER_FINAL_1_CLAW      0x8022B0E8
#define SCRIPT_BOWSER_FINAL_1_WAVE      0x8022B600
#define SCRIPT_BOWSER_FINAL_1_LIGHTNING 0x8022CD88
#define SCRIPT_BOWSER_FINAL_2_TAKE_TURN 0x8022F2BC
#define SCRIPT_BOWSER_FINAL_2_FIRE      0x80232C64
#define SCRIPT_BOWSER_FINAL_2_STOMP     0x80230D60
#define SCRIPT_BOWSER_FINAL_2_CLAW      0x80231D54
#define SCRIPT_BOWSER_FINAL_2_WAVE      0x8023226C
#define SCRIPT_BOWSER_FINAL_2_LIGHTNING 0x802339F4

// NOLINTBEGIN
typedef struct OSContPad {
    u16 button;
    s8 stick_x;
    s8 stick_y;
    u8 errno;
} OSContPad;

typedef union pm_Controller {
    struct {
        u32            : 12;
        u32 stickRight : 1;
        u32 stickLeft  : 1;
        u32 stickDown  : 1;
        u32 stickUp    : 1;
        u32 a          : 1;
        u32 b          : 1;
        u32 z          : 1;
        u32 start      : 1;
        u32 dUp        : 1;
        u32 dDown      : 1;
        u32 dLeft      : 1;
        u32 dRight     : 1;
        u32            : 2;
        u32 l          : 1;
        u32 r          : 1;
        u32 cUp        : 1;
        u32 cDown      : 1;
        u32 cLeft      : 1;
        u32 cRight     : 1;
    };
    struct {
        u8 : 8;
        u8 analog;
        u16 buttons;
    };
    u32 pad;
} pm_Controller; // size = 0x04

typedef struct pm_SaveGlobals {
    /* 0x00 */ char magicString[16]; /* "Mario Story 006" string */
    /* 0x10 */ s8 pad[32];           /* always zero */
    /* 0x30 */ s32 crc1;
    /* 0x34 */ s32 crc2;
    /* 0x38 */ s32 useMonoSound;
    /* 0x3C */ u32 lastFileSelected;
    /* 0x40 */ s8 reserved[64]; // unused
} pm_SaveGlobals;               // size = 0x80

typedef struct pm_GameStatus {
    /* 0x000 */ pm_Controller currentButtons[4]; /* raw input */
    /* 0x010 */ pm_Controller pressedButtons[4]; /* one frame when pressedButtons*/
    /* 0x020 */ pm_Controller heldButtons[4];    /* once every 4 frames */
    /* 0x030 */ pm_Controller prevButtons[4];
    /* 0x040 */ s8 stickX[4];
    /* 0x044 */ s8 stickY[4];
    /* 0x048 */ s16 holdTimerStartValue[4]; /* writes 0F to holdTimer when input is pressedButtons*/
    /* 0x050 */ char unk_0x50[8];
    /* 0x058 */ s16 holdTimer; /*counts from 0F to 00 to start held variable*/
    /* 0x05A */ char unk_0x5A[6];
    /* 0x060 */ s16 heldTimer; /*FFFF when nothing held, when holdTimer hits 0 it will cycle between 0 and 3*/
    /* 0x062 */ char unk_0x62[6];
    /* 0x068 */ s16 demoButtonInput;
    /* 0x06A */ s8 demoStickX;
    /* 0x06B */ s8 demoStickY;
    /* 0x06C */ s32 mainScriptID;
    /* 0x070 */ s8 isBattle;
    /* 0x071 */ s8 demoState;         /* (0 = not demo, 1 = map demo, 2 = demo map changing) */
    /* 0x072 */ s8 nextDemoScene;     /* which part of the demo to play next */
    /* 0x073 */ u8 controllerPlugged; /*needs to be 1 otherwise "no controller" */
    /* 0x074 */ s8 debugEnemyContact; /* 0=normal, 1=enemies can't interact, 2=defeat enemies on contact, 3=auto-defeat
                                          enemies in battle, 4=auto run away */
    /* 0x075 */ s8 debugQuizmo;       /* 1 to force a quizmo spawn every time */
    /* 0x076 */ s8 unk_0x76;
    /* 0x077 */ char unk_0x77;
    /* 0x078 */ s8 disableScripts;
    /* 0x079 */ char unk_0x79;
    /* 0x07A */ s8 musicEnabled;
    /* 0x07B */ char unk_0x7B;
    /* 0x07C */ s8 unk_0x7C;
    /* 0x07D */ s8 keepUsingPartnerOnMapChange;
    /* 0x07E */ u8 peachFlags;    /*bitfield, 1 = isPeach, 2 = isTransformed, 3 = hasParasol*/
    /* 0x07F */ s8 peachDisguise; /*1 = koopatrol, 2 = hammer bro, 3 = clubba */
    /* 0x080 */ u8 peachCookingIngredient;
    /* 0x081 */ s8 multiplayerEnabled;
    /* 0x082 */ s8 unk_82;
    /* 0x083 */ s8 unk_83;
    /* 0x084 */ s8 playerSpriteSet;
    /* 0x085 */ char unk_85;
    /* 0x086 */ u16 areaID;
    /* 0x088 */ u16 prevArea; /* used for calculating didAreaChange */
    /* 0x08A */ s16 didAreaChange;
    /* 0x08C */ u16 mapID;
    /* 0x08E */ u16 entryID;
    /* 0x090 */ u16 unk_0x90;
    /* 0x092 */ u16 unk_0x92;
    /* 0x094 */ f32 loadingZoneTangent;
    /* 0x098 */ Vec3f playerPos;
    /* 0x0A4 */ f32 playerYaw;
    /* 0x0A8 */ s8 creditsViewportMode;
    /* 0x0A9 */ s8 unk_A9;
    /* 0x0AA */ s8 demoFlags;
    /* 0x0AB */ u8 soundOutputMode;
    /* 0x0AC */ s8 introState;
    /* 0x0AD */ s8 introCounter;
    /* 0x0AE */ s8 bSkipIntro;
    /* 0x0AF */ s8 unk_AF;
    /* 0x0B0 */ s8 unk_B0;
    /* 0x0B1 */ char unk_B1[5];
    /* 0x0B6 */ s16 bootAlpha;
    /* 0x0B8 */ s16 bootBlue;
    /* 0x0BA */ s16 bootGreen;
    /* 0x0BC */ s16 bootRed;
    /* 0x0BE */ char unk_BE[94];
    /* 0x11C */ Vec3f playerGroundTraceAngles;
    /* 0x128 */ Vec3f playerGroundTraceNormal;
    /* 0x134 */ u16 frameCounter;
    /* 0x136 */ char unk_136[2];
    /* 0x138 */ s32 nextRNG;
    /* 0x13C */ s16 unk_13C;
    /* 0x13E */ char unk_13E[10];
    /* 0x148 */ s16 backgroundFlags; /* (bit 1 = enable, bit 2 is used for something else) */
    /* 0x14A */ s16 backgroundMinX;
    /* 0x14C */ s16 backgroundMinY;
    /* 0x14E */ s16 backgroundMaxX;
    /* 0x150 */ s16 backgroundMaxY;
    /* 0x152 */ s16 backgroundXOffset; /* used for parallax scroll */
    /* 0x154 */ void *backgroundRaster;
    /* 0x158 */ void *backgroundPalette;
    /* 0x15C */ s16 unk_15C;
    /* 0x15E */ u16 unk_15E;
    /* 0x160 */ Vec3s savedPos;
    /* 0x166 */ u8 saveSlot;
    /* 0x167 */ u8 loadType; /* 0 = from map, 1 = from main menu */
    /* 0x168 */ s32 saveCount;
    /* 0x16C */ char unk_16C[12];
} pm_GameStatus; // size = 0x178

typedef struct pm_PartnerStatus {
    /* 0x000 */ s8 partnerActionState;
    /* 0x001 */ s8 shouldResumeAbility;
    /* 0x002 */ s8 partnerAction_unk_2;
    /* 0x003 */ u8 actingPartner;
    /* 0x04 */ s16 stickX;
    /* 0x06 */ s16 stickY;
    /* 0x08 */ s32 currentButtons;
    /* 0x0C */ s32 pressedButtons;
    /* 0x10 */ s32 heldButtons;
    /* 0x14 */ s8 inputDisabled;
    /* 0x15 */ char unk_15[3];
} pm_PartnerStatus; // size = 0x18

typedef struct pm_UiStatus {
    /* 0x00 */ u32 hpText;
    /* 0x04 */ u32 hpIcon;
    /* 0x08 */ u32 fpText;
    /* 0x0C */ u32 fpIcon;
    /* 0x10 */ s32 coinIconIndex;
    /* 0x14 */ s32 coinSparkleIconIndex;
    /* 0x18 */ s32 starpointsIconIndex;
    /* 0x1C */ s32 starpointsShineIconIndex;
    /* 0x20 */ s32 iconIndex8;
    /* 0x24 */ s32 iconIndex9;
    /* 0x28 */ s32 iconIndexA;
    /* 0x2C */ s32 iconIndexB;
    /* 0x30 */ s32 starIconIndex;
    /* 0x34 */ s16 drawPosX; /* overall x-offset for whole UI */
    /* 0x36 */ s16 drawPosY; /* modulated as it appears, goes away */
    /* 0x38 */ s16 timer;    /*hud will show up after 0xF0 frames of idle*/
    /* 0x3A */ u8 hidden;    /*0 for shown, 1 for hidden*/
    /* 0x3B */ char unk_0x3B;
    /* 0x3C */ u8 hudShown; /*1 for shown, 0 for hidden. this flag does not affect menu behavior*/
    /* 0x3D */ s8 displayHP;
    /* 0x3E */ s8 displayFP;
    /* 0x3F */ char unk_0x3F;
    /* 0x40 */ u16 displayCoins;
    /* 0x42 */ s16 displayStarpoints;
    /* 0x44 */ s8 ignoreChanges; /* set != 0 to prevent automatic opening from HP/FP changes */
    /* 0x45 */ s8 unk_45[2];
    /* 0x47 */ s8 disabled; /* set != 0 for menu to be disabled completely */
    /* 0x48 */ u16 displayStarPower;
    /* 0x4A */ bool hpBlinking;
    /* 0x4B */ u8 hpBlinkCounter;
    /* 0x4C */ u8 hpBlinkTimer; /* until stop */
    /* 0x4D */ bool fpBlinking;
    /* 0x4E */ u8 fpBlinkCounter;
    /* 0x4F */ u8 fpBlinkTimer; /* until stop */
    /* 0x50 */ bool starPowerBlinking;
    /* 0x51 */ u8 starPowerBlinkCounter;
    /* 0x52 */ bool starPointsBlinking;
    /* 0x53 */ u8 starPointsBlinkCounter;
    /* 0x54 */ bool coinsBlinking; /*non-zero value will flash*/
    /* 0x55 */ u8 coinsBlinkCounter;
    /* 0x56 */ u8 coinsBlinkTimer; /* until stop */
    /* 0x57 */ char unk_57[3];
    /* 0x5A */ u8 spBarsToBlink; /* how many sp bars to blink */
    /* 0x5B */ char unk_5B;
    /* 0x5C */ s32 iconIndex10;
    /* 0x60 */ s32 iconIndex11;
    /* 0x64 */ s32 iconIndex12;
    /* 0x68 */ s32 iconIndex13;
    /* 0x6C */ s8 unk_6C[4];
} pm_UiStatus; // size = 0x70

typedef struct pm_PartnerData {
    /* 0x00 */ u8 enabled;
    /* 0x01 */ u8 level;
    /* 0x02 */ s16 unk_0x02[3];
} pm_PartnerData; // size = 0x08

typedef struct pm_Party {
    /* 0x00 */ pm_PartnerData unk_partner;
    /* 0x08 */ pm_PartnerData goombario;
    /* 0x10 */ pm_PartnerData kooper;
    /* 0x18 */ pm_PartnerData bombette;
    /* 0x20 */ pm_PartnerData parakarry;
    /* 0x28 */ pm_PartnerData goompa;
    /* 0x30 */ pm_PartnerData watt;
    /* 0x38 */ pm_PartnerData sushie;
    /* 0x40 */ pm_PartnerData lakilester;
    /* 0x48 */ pm_PartnerData bow;
    /* 0x50 */ pm_PartnerData goombaria;
    /* 0x58 */ pm_PartnerData twink;
} pm_Party; // size = 0x60

typedef struct pm_EffectInstance {
    /* 0x00 */ s32 flags;
    /* 0x04 */ s32 effectIndex;
    /* 0x08 */ s32 numParts;
    /* 0x0C */ void *data;
    /* 0x10 */ struct EffectGraphics *graphics;
} pm_EffectInstance; // size = 0x14

typedef struct pm_PlayerData {
    /* 0x000 */ s8 bootsLevel; /*start: 8010F450*/
    /* 0x001 */ s8 hammerLevel;
    /* 0x002 */ s8 curHP;
    /* 0x003 */ s8 curMaxHP; // hardMaxHP + HP Plus
    /* 0x004 */ s8 hardMaxHP;
    /* 0x005 */ s8 curFP;
    /* 0x006 */ s8 curMaxFP; // hardMaxFP + FP Plus
    /* 0x007 */ s8 hardMaxFP;
    /* 0x008 */ s8 maxBP;
    /* 0x009 */ s8 level;
    /* 0x00A */ u8 hasActionCommands;
    /* 0x00B */ char unk_0x0B;
    /* 0x00C */ s16 coins;
    /* 0x00E */ s8 fortressKeys;
    /* 0x00F */ u8 starPieces;
    /* 0x010 */ s8 starPoints;
    /* 0x011 */ char unk_0x11;
    /* 0x012 */ s8 currentPartner;
    /* 0x013 */ char unk_0x13;
    union {
        /* 0x014 */ pm_Party party;
        /* 0x014 */ pm_PartnerData partners[12];
    };
    /* 0x074 */ s16 keyItems[32];
    /* 0x0B4 */ s16 badges[128];
    /* 0x1B4 */ s16 invItems[10];
    /* 0x1C8 */ s16 storedItems[32];
    /* 0x208 */ s16 equippedBadges[64];
    /* 0x288 */ char unk_0x288;
    /* 0x289 */ s8 merleeSpellType;
    /* 0x28A */ s8 merleeCastsRemaining;
    /* 0x28B */ char pad_28B;
    /* 0x28C */ s16 merleeTurnCount;
    /* 0x28E */ s8 maxStarPower;
    /* 0x28F */ char pad_0x28F;
    union {
        struct {
            /* 0x290 */ s8 starPowerFullBars;
            /* 0x291 */ s8 starPowerPartialBars;
        };
        /* 0x290 */ s16 starPower;
    };
    /* 0x292 */ s8 starBeamLevel; // 1 for star beam, 2 for peach beam
    /* 0x293 */ char pad_293;
    /* 0x294 */ u16 actionCommandAttempts;
    /* 0x296 */ u16 actionCommandSuccesses;
    /* 0x298 */ u16 hitsTaken;
    /* 0x29A */ u16 hitsBlocked;
    /* 0x29C */ u16 playerFirstStrikes;
    /* 0x29E */ u16 enemyFirstStrikes;
    /* 0x2A0 */ u16 powerBounces;
    /* 0x2A2 */ u16 battlesCount;
    /* 0x2A4 */ u16 battlesWon;
    /* 0x2A6 */ u16 unk_2A6;
    /* 0x2A8 */ u16 battlesFled;
    /* 0x2AA */ u16 trainingsDone;
    /* 0x2AC */ s32 walkingStepsTaken;
    /* 0x2B0 */ s32 runningStepsTaken;
    /* 0x2B4 */ u32 totalCoinsEarned;
    /* 0x2B8 */ s16 idleFrameCounter; /* frames with no inputs, overflows every ~36 minutes of idling */
    /* 0x2BA */ char pad_2BA[2];
    /* 0x2BC */ u32 frameCounter; /* increases by 2 per frame */
    /* 0x2C0 */ u16 quizzesAnswered;
    /* 0x2C2 */ u16 quizzesCorrect;
    /* 0x2C4 */ s32 partnerUnlockedTime[12];
    /* 0x2F4 */ s32 partnerUsedTime[12];
    /* 0x324 */ s32 tradeEventStartTime;
    /* 0x328 */ s32 droTreeHintTime;
    /* 0x32C */ u16 starPiecesCollected;
    /* 0x32E */ u16 jumpGamePlays;
    /* 0x330 */ u32 jumpGameTotal; /* all-time winnings, max = 99999 */
    /* 0x334 */ u16 jumpGameRecord;
    /* 0x336 */ u16 smashGamePlays;
    /* 0x338 */ u32 smashGameTotal; /* all-time winnings, max = 99999 */
    /* 0x33C */ u16 smashGameRecord;
    /* 0x33E */ char pad_33E[2];
    /* 0x340 */ char reserved[0xE8];
} pm_PlayerData; // size = 0x428

typedef struct pm_PlayerStatus {
    /* 0x000 */ s32 flags; /* third byte related to cutscenes - write 0 to break free */
    /* 0x004 */ s32 animFlags;
    /* 0x008 */ u16 currentStateTime;
    /* 0x00A */ s8 unk_0x0A;
    /* 0x00B */ char unk_0x0B;
    /* 0x00C */ s8 peachDisguise;
    /* 0x00D */ s8 unk_0D;
    /* 0x00E */ u8 alpha1; /* 0xFF when spinning */
    /* 0x00D */ u8 alpha2;
    /* 0x010 */ s16 blinkTimer; /* used when running away */
    /* 0x012 */ s16 moveFrames;
    /* 0x014 */ s8 enableCollisionOverlapsCheck; /*changed when talking/opening doors/loading zones*/
    /* 0x015 */ s8 inputEnabledCounter;          /* whether the C-up menu can appear */
    /* 0x016 */ Vec3s lastGoodPosition;
    /* 0x01C */ Vec3f extraVelocity;
    /* 0x028 */ Vec3f position;
    /* 0x034 */ Vec2XZf groundAngles; /* angles along X/Z axes of ground beneath player */
    /* 0x03C */ Vec2XZf jumpFromPos;
    /* 0x044 */ Vec2XZf landPos;
    /* 0x04C */ f32 jumpFromHeight;
    /* 0x050 */ f32 jumpApexHeight;
    /* 0x054 */ f32 currentSpeed;
    /* 0x058 */ f32 walkSpeed; /*constant: 0x40000000 = 2.0*/
    /* 0x05C */ f32 runSpeed;  /*constant: 0x40800000 = 4.0*/
    /* 0x060 */ s32 unk_0x60;
    /* 0x064 */ f32 overlapPushAmount;
    /* 0x068 */ f32 groundNormalPitch;
    /* 0x06C */ f32 maxJumpSpeed; /*used by jumping func to compare if jump_var_1 less than const*/
    /* 0x070 */ f32 ySpeed;       /*related to rise/fall currentSpeed*/
    /* 0x074 */ f32 yAcceleration;
    /* 0x078 */ f32 yJerk;
    /* 0x07C */ f32 ySnap;
    /* 0x080 */ f32 targetYaw; /*locking this makes you move in only that direction regardless of control stick angle*/
    /* 0x084 */ f32 currentYaw;
    /* 0x088 */ f32 overlapPushYaw;
    /* 0x08C */ f32 pitch;
    /* 0x090 */ f32 bodyRotation; /*used for turning effect*/
    /* 0x094 */ f32 unk_0x94[3];
    /* 0x0A0 */ f32 heading;
    /* 0x0A4 */ s32 trueAnimation;     /* 1st byte: back turned=01 | 4th byte: animations 00-32*/
    /* 0x0A8 */ f32 spriteFacingAngle; /*0.0=left, 180.0=right*/
    /* 0x0AC */ char unk_0xAC[4];
    /* 0x0B0 */ s16 colliderHeight;
    /* 0x0B2 */ s16 colliderDiameter;
    /* 0x0B4 */ u8 actionState;
    /* 0x0B5 */ u8 prevActionState;
    /* 0x0B6 */ s8 fallState; /*also used as sleep state in Peach idle action*/
    /* 0x0B7 */ char unk_B7;
    /* 0x0B8 */ u32 anim;
    /* 0x0BC */ u16 unk_BC;
    /* 0x0BE */ s8 renderMode;
    /* 0x0BF */ s8 unk_BF;
    /* 0x0C0 */ u16 framesInAir;
    /* 0x0C2 */ s16 unk_0xC2;
    /* 0x0C4 */ char unk_0xC4;
    /* 0x0C5 */ s8 unk_C5;
    /* 0x0C6 */ u16 interactingWithID; /*only for doors?*/
    /* 0x0C8 */ void *encounteredNPC;
    /* 0x0CC */ s32 shadowID;
    /* 0x0D0 */ f32 *unk_D0;
    /* 0x0D4 */ f32 spinRate;
    /* 0x0D8 */ pm_EffectInstance *unk_D8; // effect 46
    /* 0x0DC */ pm_Controller currentButtons;
    /* 0x0E0 */ pm_Controller previousButtons;
    /* 0x0E4 */ pm_Controller heldButtons;
    /* 0x0E8 */ s32 stickAxisX;
    /* 0x0EC */ s32 stickAxisY;
    /* 0x0F0 */ pm_Controller currentButtonsBuffer[10];
    /* 0x118 */ pm_Controller previousButtonsBuffer[10];
    /* 0x140 */ pm_Controller heldButtonsBufferr[10];
    /* 0x168 */ s32 stickAxisXBuffer[10];
    /* 0x190 */ s32 stickAxisYBuffer[10];
    /* 0x1B8 */ s32 inputBufPos;
    /* 0x1BC */ char unk_0x1BC[0xCC];
    /* 0x288 */ u8 spinCooldownTimer; /*4 frames at the end of spin*/
    /* 0x289 */ char unk_0x289[0x02];
    /* 0x28B */ u8 spinTimer;
    /* 0x28C */ char unk_0x28C[0x20];
    /* 0x2AC */ f32 spinSpeed;
    /* 0x2B0 */ char unk_0x2B0[0x04];
    /* 0x2B4 */ char unk_0x2B4[0x01];
    /* 0x2B5 */ u8 spinDuration;
    /* 0x2B6 */ char unk_0x228[0x02];
    /* 0x2B8 */ char unk_0x2B8[0x10];
    /* 0x2C8 */ pm_PlayerData playerData;
} pm_PlayerStatus; // size = 0x6F0

typedef struct pm_SaveMetadata {
    /* 0x00 */ s32 timePlayed;
    /* 0x04 */ u8 spiritsRescued;
    /* 0x05 */ char unk_05[0x1];
    /* 0x06 */ s8 level;
    /* 0x07 */ unsigned char filename[8];
    /* 0x0F */ char unk_0F[0x1];
    /* 0x10 */ s32 unk_10;
    /* 0x14 */ s32 unk_14;
} pm_SaveMetadata; // size = 0x18

typedef struct {
    /* 0x0000 */ char magicString[16]; /* "Mario Story 006" */
    /* 0x0010 */ s8 padding[32];       /* always zero */
    /* 0x0030 */ s32 crc1;
    /* 0x0034 */ s32 crc2;
    /* 0x0038 */ s32 saveSlot;
    /* 0x003C */ s32 saveCount;
    /* 0x0040 */ pm_PlayerData playerData;
    /* 0x0468 */ s16 areaID;
    /* 0x046A */ s16 mapID;
    /* 0x046C */ s16 entryID;
    /* 0x046E */ char unk_46E[2];
    /* 0x0470 */ u32 enemyDefeatFlags[60][12];
    /* 0x0FB0 */ u32 globalFlags[64];
    /* 0x10B0 */ s8 globalBytes[512];
    /* 0x12B0 */ u32 areaFlags[8];
    /* 0x12D0 */ s8 areaBytes[16];
    /* 0x12E0 */ s8 debugEnemyContact;
    /* 0x12E1 */ s8 unk_12E1;
    /* 0x12E2 */ s8 unk_12E2;
    /* 0x12E3 */ s8 musicEnabled;
    /* 0x12E4 */ s8 unk_12E4[0x2];
    /* 0x12E6 */ Vec3s savePos;
    /* 0x12EC */ pm_SaveMetadata saveMetadata;
    /* 0x1304 */ char unk_1304[0x7C];
} pm_SaveData; // size = 0x1380

typedef struct pm_SelectableTarget {
    /* 0x00 */ s16 actorID;
    /* 0x02 */ s16 partID; /* sometimes loaded as byte from 0x3 */
    /* 0x04 */ Vec3s pos;
    /* 0x0A */ s16 unk_0A;
    /* 0x0C */ s16 unk_0C;
    /* 0x0E */ s16 unk_0E;
    /* 0x10 */ s8 unk_10;
    /* 0x11 */ s8 homeCol; /* from xpos --> 0-3 */
    /* 0x12 */ s8 homeRow; /* from ypos --> 0-3 */
    /* 0x13 */ s8 layer;   /* from zpos? --> 0-1 */
} pm_SelectableTarget;     // size = 0x14

struct pm_Evt;
typedef s32 pm_Bytecode;
typedef s32 pm_ApiStatus;
typedef pm_ApiStatus (*pm_ApiFunc)(struct pm_Evt *, s32);

typedef struct pm_ActorState {
    /* 0x00 */ Vec3f currentPos;
    /* 0x0C */ Vec3f goalPos;
    /* 0x18 */ Vec3f unk_18;
    /* 0x24 */ f32 unk_24;
    /* 0x28 */ f32 unk_28;
    /* 0x2C */ char unk_2C[0x4];
    /* 0x30 */ Vec3f unk_30;
    /* 0x3C */ f32 acceleration;
    /* 0x40 */ f32 speed;
    /* 0x44 */ f32 velocity;
    /* 0x48 */ f32 angle;
    /* 0x4C */ f32 distance;
    /* 0x50 */ f32 bounceDivisor;
    /* 0x54 */ char unk_54[0x4];
    /* 0x58 */ s32 animJumpRise;
    /* 0x5C */ s32 animJumpFall;
    /* 0x60 */ s32 animJumpLand;
    /* 0x64 */ s16 moveTime;
    /* 0x66 */ s16 moveArcAmplitude;
    /* 0x68 */ char unk_68[3];
    /* 0x6B */ u8 jumpPartIndex;
    /* 0x6C */ void *unk_6C;
    /* 0x70 */ char unk_70[12];
    union {
        /* 0x7C */ s32 varTable[16];
        /* 0x7C */ f32 varTableF[16];
        /* 0x7C */ void *varTablePtr[16];
    };
} pm_ActorState; // size = 0xBC;

typedef struct pm_ActorMovement {
    /* 0x00 */ Vec3f currentPos;
    /* 0x0C */ Vec3f goalPos;
    /* 0x18 */ Vec3f unk_18;
    /* 0x24 */ char unk_24[24];
    /* 0x3C */ f32 acceleration;
    /* 0x40 */ f32 speed;
    /* 0x44 */ f32 velocity;
    /* 0x48 */ f32 angle;
    /* 0x4C */ f32 distance;
} pm_ActorMovement; // size = 0x50;

typedef struct pm_Actor {
    /* 0x000 */ s32 flags;
    /* 0x004 */ s32 flags2;
    /* 0x008 */ void *actorBlueprint; /* ActorBlueprint* */
    /* 0x00C */ pm_ActorState state;
    /* 0x0C8 */ pm_ActorMovement fly;
    /* 0x118 */ f32 flyElapsed;
    /* 0x11C */ char unk_11C[4];
    /* 0x120 */ s16 flyTime;
    /* 0x122 */ s16 flyArcAmplitude;
    /* 0x124 */ char unk_124[16];
    /* 0x134 */ u8 unk_134;
    /* 0x135 */ u8 footStepCounter;
    /* 0x136 */ u8 actorType;
    /* 0x137 */ char unk_137;
    /* 0x138 */ Vec3f homePos;
    /* 0x144 */ Vec3f currentPos;
    /* 0x150 */ Vec3s headOffset;
    /* 0x156 */ Vec3s healthBarPosition;
    /* 0x15C */ Vec3f rotation;
    /* 0x168 */ Vec3s rotationPivotOffset;
    /* 0x16E */ char unk_16E[2];
    /* 0x170 */ Vec3f scale;
    /* 0x17C */ Vec3f scaleModifier; /* multiplies normal scale factors componentwise */
    /* 0x188 */ f32 scalingFactor;
    /* 0x18C */ f32 yaw;
    /* 0x190 */ Vec2bu size;
    /* 0x192 */ s16 actorID;
    /* 0x194 */ s8 unk_194;
    /* 0x195 */ s8 unk_195;
    /* 0x196 */ s8 unk_196;
    /* 0x197 */ s8 unk_197;
    /* 0x198 */ Vec2b unk_198;
    /* 0x19A */ s8 unk_19A;
    /* 0x19B */ char unk_19B[1];
    /* 0x19C */ s32 actorTypeData1[6]; /* 4 = jump sound, 5 = attack sound */ // TODO: struct
    /* 0x1B4 */ s16 actorTypeData1b[2];
    /* 0x1B8 */ s8 currentHP;
    /* 0x1B9 */ s8 maxHP;
    /* 0x1BA */ char unk_1BA[2];
    /* 0x1BC */ s8 hpFraction; /* used to render HP bar */
    /* 0x1BD */ char unk_1BD[3];
    /* 0x1C0 */ s32 *idleScriptSource;
    /* 0x1C4 */ s32 *takeTurnScriptSource;
    /* 0x1C8 */ s32 *onHitScriptSource;
    /* 0x1CC */ s32 *onTurnChanceScriptSource;
    /* 0x1D0 */ struct pm_Evt *idleScript;
    /* 0x1D4 */ struct pm_Evt *takeTurnScript;
    /* 0x1D8 */ struct pm_Evt *onHitScript;
    /* 0x1DC */ struct pm_Evt *onTurnChangeScript;
    /* 0x1E0 */ s32 idleScriptID;
    /* 0x1E4 */ s32 takeTurnID;
    /* 0x1E8 */ s32 onHitID;
    /* 0x1EC */ s32 onTurnChangeID;
    /* 0x1F0 */ s8 lastEventType;
    /* 0x1F1 */ s8 turnPriority;
    /* 0x1F2 */ s8 enemyIndex; /* actorID = this | 200 */
    /* 0x1F3 */ s8 numParts;
    /* 0x1F4 */ void *partsTable; // ActorPart
    /* 0x1F8 */ s16 lastDamageTaken;
    /* 0x1FA */ s16 hpChangeCounter;
    /* 0x1FC */ u16 damageCounter;
    /* 0x1FE */ char unk_1FE[2];
    /* 0x200 */ pm_EffectInstance *unk_200; // fxAttackResultRext
    /* 0x204 */ s8 unk_204;
    /* 0x205 */ s8 unk_205;
    /* 0x206 */ s8 unk_206;
    /* 0x207 */ s8 extraCoinBonus;
    /* 0x208 */ s8 unk_208;
    /* 0x209 */ char unk_209[3];
    /* 0x20C */ s32 *statusTable;
    /* 0x210 */ s8 debuff;
    /* 0x211 */ s8 debuffDuration;
    /* 0x212 */ s8 staticStatus; /* 0B = yes */
    /* 0x213 */ s8 staticDuration;
    /* 0x214 */ s8 stoneStatus; /* 0C = yes */
    /* 0x215 */ s8 stoneDuration;
    /* 0x216 */ s8 koStatus; /* 0D = yes */
    /* 0x217 */ s8 koDuration;
    /* 0x218 */ s8 transStatus; /* 0E = yes */
    /* 0x219 */ s8 transDuration;
    /* 0x21A */ char unk_21A[2];
    /* 0x21C */ u8 status;
    /* 0x21D */ s8 unk_21D;
    /* 0x21E */ s16 unk_21E;
    /* 0x220 */ s8 isGlowing; // not the case for goombario
    /* 0x221 */ s8 attackBoost;
    /* 0x222 */ s8 defenseBoost;
    /* 0x223 */ s8 chillOutAmount; /* attack reduction */
    /* 0x224 */ s8 chillOutTurns;
    /* 0x225 */ char unk_225[3];
    /* 0x228 */ pm_EffectInstance *icePillarEffect;
    /* 0x22C */ pm_SelectableTarget targetData[24];
    /* 0x40C */ s8 targetListLength;
    /* 0x40D */ s8 targetIndexList[24]; /* into targetData */
    /* 0x425 */ s8 selectedTargetIndex; /* into target index list */
    /* 0x426 */ s8 targetPartIndex;
    /* 0x427 */ char unk_427;
    /* 0x428 */ s16 targetActorID;
    /* 0x42A */ char unk_42A[2];
    union {
        /* 0x42C */ void *ptr; /* Shadow* */
        /* 0x42C */ s32 id;
    };
    /* 0x430 */ f32 shadowScale; /* = actor size / 24.0 */
    /* 0x434 */ s16 renderMode;  /* initially 0xD, set to 0x22 if any part is transparent */
    /* 0x436 */ s16 hudElementDataIndex;
    /* 0x438 */ s32 unk_438[2]; /* ??? see FUN_80253974 */
    /* 0x440 */ pm_EffectInstance *debuffEffect;
} pm_Actor; // size = 0x444

typedef struct pm_Evt {
    /* 0x000 */ u8 state;
    /* 0x001 */ u8 currentArgc;
    /* 0x002 */ u8 currentOpcode;
    /* 0x003 */ u8 priority;
    /* 0x004 */ u8 groupFlags;
    /* 0x005 */ s8 blocked;     /* 1 = blocking */
    /* 0x006 */ s8 loopDepth;   /* how many nested loops we are in, >= 8 hangs forever */
    /* 0x007 */ s8 switchDepth; /* how many nested switches we are in, max = 8 */
    /* 0x008 */ pm_Bytecode *ptrNextLine;
    /* 0x00C */ pm_Bytecode *ptrReadPos;
    /* 0x010 */ s8 labelIndices[16];
    /* 0x020 */ void *labelPositions[16];
    /* 0x060 */ void *userData;                /* unknown pointer; allocated on the heap, free'd in kill_script() */
    /* 0x064 */ struct pm_Evt *blockingParent; /* parent? */
    /* 0x068 */ struct pm_Evt *childScript;
    /* 0x06C */ struct pm_Evt *parentScript; /* brother? */
    union {
        /* 0x070 */ s32 functionTemp[4];
        /* 0x070 */ f32 functionTempF[4];
        /* 0x070 */ void *functionTempPtr[4];
    };
    /* 0x080 */ pm_ApiFunc callFunction;
    union {
        /* 0x084 */ s32 varTable[16];
        /* 0x084 */ f32 varTableF[16];
        /* 0x084 */ void *varTablePtr[16];
    };
    /* 0x0C4 */ s32 varFlags[3];
    /* 0x0D0 */ s32 loopStartTable[8];
    /* 0x0F0 */ s32 loopCounterTable[8];
    /* 0x110 */ s8 switchBlockState[8];
    /* 0x118 */ s32 switchBlockValue[8];
    /* 0x138 */ s32 *buffer;
    /* 0x13C */ s32 *array;
    /* 0x140 */ s32 *flagArray;
    /* 0x144 */ s32 id;
    union {
        /* 0x148 */ s32 enemyID;
        /* 0x148 */ s32 actorID;
        /* 0x148 */ void *enemy;              /// (Enemy*) < For overworld scripts owned by an Npc
        /* 0x148 */ struct pm_Actor_t *actor; ///< For battle scripts
    } owner1;                                 ///< Initially -1
    union {
        /* 0x14C */ s32 npcID;
        /* 0x14C */ s32 triggerID;
        /* 0x14C */ void *npc;     /// (Npc*) < For overworld scripts owned by an Npc
        /* 0x14C */ void *trigger; /// (Trigger*)
    } owner2;                      ///< Initially -1
    /* 0x150 */ f32 timeScale;
    /* 0x154 */ f32 frameCounter;
    /* 0x158 */ s32 unk_158;
    /* 0x15C */ pm_Bytecode *ptrFirstLine;
    /* 0x160 */ pm_Bytecode *ptrSavedPosition;
    /* 0x164 */ pm_Bytecode *ptrCurrentLine;
} pm_Evt; // size = 0x168

typedef struct pm_BattleStatus {
    /* 0x000 */ s32 flags1;
    /* 0x004 */ s32 flags2;
    union {
        /* 0x008 */ s32 varTable[16];
        /* 0x008 */ f32 varTableF[16];
        /* 0x008 */ void *varTablePtr[16];
    };
    /* 0x048 */ s8 currentSubmenu;
    /* 0x049 */ char unk_49[3];
    /* 0x04C */ s8 unk_4C[16];
    /* 0x05C */ s8 unk_5C[16];
    /* 0x06C */ s16 unk_6C;
    /* 0x06E */ s16 unk_6E;
    /* 0x070 */ s16 menuDisableFlags; /* 1 = jump, 2 = hammer, 4 = items */
    /* 0x072 */ char unk_72[2];
    /* 0x074 */ s32 unk_74;
    /* 0x078 */ s8 totalStarPoints;
    /* 0x079 */ s8 pendingStarPoints;       /* how many to add */
    /* 0x07A */ s8 incrementStarPointDelay; /* related to star points, set to 0x28 when they are dropped */
    /* 0x07B */ u8 damageTaken;
    /* 0x07C */ s8 changePartnerAllowed;
    /* 0x07D */ s8 menuStatus[4]; /* < -1 = automatically pick the first move, 0 = disabled, 1 = enabled */
    /* 0x081 */ s8 actionSuccess;
    /* 0x082 */ char unk_82;
    /* 0x083 */ s8 unk_83;
    /* 0x084 */ s8 unk_84;
    /* 0x085 */ s8 unk_85;
    /* 0x086 */ s8 unk_86;
    /* 0x087 */ s8 blockResult;  /* 0 = fail, 1 = success, -1 = mashed */
    /* 0x088 */ s8 itemUsesLeft; /* set to 2 for double dip, 3 for triple dip */
    /* 0x089 */ s8 hpDrainCount;
    /* 0x08A */ s8 nextMerleeSpellType;
    /* 0x08B */ s8 hustleTurns; /* numTurns from hustle drink, normally 0 */
    /* 0x08C */ s8 unk_8C;
    /* 0x08D */ s8 unk_8D;
    /* 0x08E */ s8 initialEnemyCount; /* used for SP award bonus */
    /* 0x08F */ char unk_8F[1];
    /* 0x090 */ s16 unk_90;
    /* 0x092 */ s8 unk_92;
    /* 0x093 */ s8 unk_93;
    /* 0x094 */ s8 unk_94;
    /* 0x095 */ s8 unk_95;
    /* 0x096 */ s8 hammerCharge;
    /* 0x097 */ s8 jumpCharge;
    /* 0x098 */ char unk_98;
    /* 0x099 */ u8 rushFlags; /* 1 = mega rush, 2 = power rush */
    /* 0x09A */ s8 outtaSightActive;
    /* 0x09B */ s8 turboChargeTurnsLeft;
    /* 0x09C */ u8 turboChargeAmount; /* unused? */
    /* 0x09D */ s8 waterBlockTurnsLeft;
    /* 0x09E */ u8 waterBlockAmount; /* unused? */
    /* 0x09F */ char unk_9F;
    /* 0x0A0 */ struct pm_EffectInstance_t *waterBlockEffect;
    /* 0x0A4 */ s8 cloudNineTurnsLeft;
    /* 0x0A5 */ s8 cloudNineDodgeChance; /* = 50% */
    /* 0x0A6 */ char unk_A6[2];
    /* 0x0A8 */ struct pm_EffectInstance_t *cloudNineEffect;
    /* 0x0AC */ s8 merleeAttackBoost;
    /* 0x0AD */ s8 merleeDefenseBoost;
    /* 0x0AE */ s8 hammerLossTurns;
    /* 0x0AF */ s8 jumpLossTurns;
    /* 0x0B0 */ s8 itemLossTurns;
    /* 0x0B1 */ char unk_B1[3];
    /* 0x0B4 */ void *preUpdateCallback;
    /* 0x0B8 */ char unk_B8[4];
    /* 0x0BC */ void *controlScript; /* control handed over to this when changing partners */
    /* 0x0C0 */ s32 controlScriptID;
    /* 0x0C4 */ void *camMovementScript;
    /* 0x0C8 */ s32 camMovementScriptID;
    /* 0x0CC */ char unk_CC[12];
    /* 0x0D8 */ pm_Actor *playerActor;
    /* 0x0DC */ pm_Actor *partnerActor;
    /* 0x0E0 */ pm_Actor *enemyActors[24];
    /* 0x140 */ s16 enemyIDs[24];
    /* 0x170 */ s8 nextEnemyIndex; /* (during enemy turn) who should go next */
    /* 0x171 */ s8 numEnemyActors;
    /* 0x172 */ s16 activeEnemyActorID; /* (during enemy turn) enemy currently using their move */
    /* 0x174 */ struct pm_Actor_t *currentTurnEnemy;
    /* 0x178 */ s8 moveCategory; ///< 0 = jump, 1 = hammer, 5 = partner, ...
    /* 0x179 */ char unk_179;
    /* 0x17A */ s16 selectedItemID;
    /* 0x17C */ s16 selectedMoveID;
    /* 0x17E */ s16 currentAttackDamage;
    /* 0x180 */ s16 lastAttackDamage;
    /* 0x182 */ char unk_182[2];
    /* 0x184 */ s32
        currentTargetListFlags; /* set when creating a target list, also obtain from the flags field of moves */
    /* 0x188 */ s32 currentAttackElement;
    /* 0x18C */ s32 currentAttackEventSuppression;
    /* 0x190 */ s32 currentAttackStatus;
    /* 0x194 */ u8 statusChance;
    /* 0x195 */ s8 statusDuration;
    /* 0x196 */ char unk_196;
    /* 0x197 */ u8 targetHomeIndex; /* some sort of home idnex used for target list construction */
    /* 0x198 */ s8 powerBounceCounter;
    /* 0x199 */ s8 wasStatusInflicted; /* during last attack */
    /* 0x19A */ u8 unk_19A;
    /* 0x19B */ char unk_19B[5];
    /* 0x1A0 */ s16 currentTargetID;  /* selected? */
    /* 0x1A2 */ s8 currentTargetPart; /* selected? */
    /* 0x1A3 */ char unk_1A3;
    /* 0x1A4 */ s16 currentTargetID2;
    /* 0x1A6 */ s8 currentTargetPart2;
    /* 0x1A7 */ s8 battlePhase;
    /* 0x1A8 */ s16 attackerActorID;
    /* 0x1AA */ char unk_1AA[4];
    /* 0x1AE */ s16 submenuIcons[24]; /* icon IDs */
    /* 0x1DE */ u8 submenuMoves[24];  /* move IDs */
    /* 0x1F6 */ s8 submenuStatus[24]; ///< @see enum BattleSubmenuStatus
    /* 0x20E */ u8 submenuMoveCount;
    /* 0x20F */ char unk_20F;
    /* 0x210 */ s32 currentButtonsDown;
    /* 0x214 */ s32 currentButtonsPressed;
    /* 0x218 */ s32 currentButtonsHeld;
    /* 0x21C */ s32 stickX;
    /* 0x220 */ s32 stickY;
    /* 0x224 */ s32 inputBitmask;
    /* 0x228 */ s32 dpadX; /* 0-360 */
    /* 0x22C */ s32 dpadY; /* 0-60 */
    /* 0x230 */ s32 holdInputBuffer[64];
    /* 0x330 */ s32 pushInputBuffer[64];
    /* 0x430 */ s8 holdInputBufferPos;
    /* 0x431 */ s8 inputBufferPos;
    /* 0x432 */ s8 unk_432;
    /* 0x433 */ char unk_433;
    /* 0x434 */ s32 *unk_434;
    /* 0x438 */ void *foregroundModelData;
    /* 0x43C */ void *unk_43C;
    /* 0x440 */ u8 tattleFlags[28];
    /* 0x45C */ char unk_45C[4];
} pm_BattleStatus; // size = 0x460

typedef struct pm_Enemy {
    /* 0x00 */ s32 flags;
    /* 0x04 */ s8 encounterIndex;
    /* 0x05 */ s8 encountered;
    /* 0x06 */ u8 scriptGroup;    /* scripts launched for this npc controller will be assigned this group */
    /* 0x07 */ s8 hitboxIsActive; // when set, contact will trigger a first strike
    /* 0x08 */ s16 npcID;
    /* 0x0A */ s16 spawnPos[3];
    /* 0x10 */ Vec3s unk_10; // TODO hitbox pos?
    /* 0x16 */ char unk_16[2];
    /* 0x18 */ void *npcSettings;
    /* 0x1C */ void *initBytecode;
    /* 0x20 */ void *interactBytecode;
    /* 0x24 */ void *aiBytecode;
    /* 0x28 */ void *hitBytecode;
    /* 0x2C */ void *auxBytecode;
    /* 0x30 */ void *defeatBytecode;
    /* 0x34 */ struct pm_Evt *initScript;
    /* 0x38 */ struct pm_Evt *interactScript;
    /* 0x3C */ struct pm_Evt *aiScript;
    /* 0x40 */ struct pm_Evt *hitScript;
    /* 0x44 */ struct pm_Evt *auxScript;
    /* 0x48 */ struct pm_Evt *defeatScript;
    /* 0x4C */ s32 initScriptID;
    /* 0x50 */ s32 interactScriptID;
    /* 0x54 */ s32 aiScriptID;
    /* 0x58 */ s32 hitScriptID;
    /* 0x5C */ s32 auxScriptID;
    /* 0x60 */ s32 defeatScriptID;
    /* 0x64 */ void *unk_64;
    /* 0x68 */ char unk_68[4];
    /* 0x6C */ union {
        /*      */ s32 varTable[16];
        /*      */ f32 varTableF[16];
        /*      */ void *varTablePtr[16];
    /*      */ };
    /* 0xAC */ u8 aiDetectFlags; // detect player flags: 1 = require line of sight | 2 = adjust hitbox for moving player
    /* 0xAD */ char unk_AD[3];
    /* 0xB0 */ u32 aiFlags;
    /* 0xB4 */ s8 aiSuspendTime;
    /* 0xB5 */ s8 instigatorValue; // value passed to first actor in formation if battle triggered with this enemy
    /* 0xB6 */ char unk_B6[2];
    /* 0xB8 */ void *unk_B8;          // some bytecode
    /* 0xBC */ struct pm_Evt *unk_BC; // some script
    /* 0xC0 */ s32 unk_C0;            // some script ID
    /* 0xC4 */ s32 unk_C4;
    /* 0xC8 */ s32 unk_C8;
    /* 0xCC */ s32 *animList;
    /* 0xD0 */ void *territory;
    /* 0xD4 */ void *drops;
    /* 0xD8 */ u32 tattleMsg;
    /* 0xDC */ s32 unk_DC;
    /* 0xE0 */ s16 savedNpcYaw;
    /* 0xE2 */ char unk_E2[6];
} pm_Enemy; // size = 0xE8

typedef struct pm_Encounter {
    /* 0x00 */ s32 count;
    /* 0x04 */ pm_Enemy *enemy[16];
    /* 0x44 */ s16 battle;
    /* 0x46 */ s16 stage;
    /* 0x48 */ s16 encounterID;
    /* 0x4A */ char unk_4C[2];
} pm_Encounter; // size = 0x4C

typedef struct pm_FieldStatus {
    /* 0x00 */ s8 status;
    /* 0x01 */ char pad_01;
    /* 0x02 */ s16 duration;
} pm_FieldStatus;

typedef struct pm_EncounterStatus {
    /* 0x000 */ s32 flags;
    /* 0x004 */ s8 firstStrikeType; /* 0 = none, 1 = player, 2 = enemy */
    /* 0x005 */ s8 hitType;         /* 1 = none/enemy, 2 = jump */
    /* 0x006 */ s8 hitTier;         /* 0 = normal, 1 = super, 2 = ultra */
    /* 0x007 */ char unk_07;
    /* 0x008 */ s8 unk_08;
    /* 0x009 */ s8 battleOutcome;         /* 0 = won, 1 = lost */
    /* 0x00A */ s8 battleTriggerCooldown; ///< set to 15 after victory, 45 after fleeing
    /* 0x00B */ bool hasMerleeCoinBonus;  /* triple coins when TRUE */
    /* 0x00C */ u8 damageTaken;           /* valid after battle */
    /* 0x00D */ char unk_0D;
    /* 0x00E */ s16 coinsEarned; /* valid after battle */
    /* 0x010 */ s8 instigatorValue;
    /* 0x011 */ s8 forbidFleeing;
    /* 0x012 */ s8 scriptedBattle; ///< battle started by StartBattle but not by encounter
    /* 0x013 */ s8 dropWhackaBump;
    /* 0x014 */ s32 songID;
    /* 0x018 */ s32 unk_18;
    /* 0x01C */ s8 numEncounters; /* number of encounters for current map (in list) */
    /* 0x01D */ s8 curAreaIndex;
    /* 0x01E */ u8 curMapIndex;
    /* 0x01F */ u8 curEntryIndex;
    /* 0x020 */ s8 mapID;
    /* 0x021 */ s8 resetMapEncounterFlags;
    /* 0x022 */ char unk_22[2];
    /* 0x024 */ s32 *npcGroupList;
    /* 0x028 */ pm_Encounter *encounterList[24];
    /* 0x088 */ pm_Encounter *curEncounter;
    /* 0x08C */ pm_Enemy *curEnemy;
    /* 0x090 */ s32 fadeOutAmount;
    /* 0x094 */ s32 unk_94;
    /* 0x098 */ s32 fadeOutAccel;
    /* 0x09C */ s32 battleStartCountdown;
    /* 0x0A0 */ pm_FieldStatus dizzyAttack;
    /* 0x0A4 */ pm_FieldStatus unusedAttack1;
    /* 0x0A8 */ pm_FieldStatus unusedAttack2;
    /* 0x0AC */ pm_FieldStatus unusedAttack3;
    /* 0x0B0 */ s32 defeatFlags[60][12];
    /* 0xBF0 */ s16 recentMaps[2];
} pm_EncounterStatus; // size = 0xFB8

typedef struct pm_CameraInitData {
    /* 0x00 */ s16 flags;
    /* 0x02 */ s8 updateMode;
    /* 0x03 */ char unk_03;
    /* 0x04 */ s16 viewWidth;
    /* 0x06 */ s16 viewHeight;
    /* 0x08 */ s16 viewStartX;
    /* 0x0A */ s16 viewStartY;
    /* 0x0C */ s16 nearClip;
    /* 0x0E */ s16 farClip;
    /* 0x10 */ s16 vfov;
} pm_CameraInitData; // size = 0x12;

typedef struct pm_CameraUnk {
    /* 0x00 */ s16 unk_00;
    /* 0x02 */ s16 unk_02;
    /* 0x04 */ char unk_04[0x8];
    /* 0x0C */ s32 unk_0C;
    /* 0x10 */ char unk_10[0x54];
    /* 0x64 */ s32 unk_64;
    /* 0x68 */ char unk_68[0x24];
} pm_CameraUnk; // size = 0x8C

typedef struct pm_CameraControlSettings {
    /* 0x00 */ s32 type;
    /* 0x04 */ f32 boomLength;
    /* 0x08 */ f32 boomPitch;
    union {
        struct {
            f32 Ax;
            f32 Ay;
            f32 Az;
            f32 Bx;
            f32 By;
            f32 Bz;
        } two;
        struct {
            f32 Ax;
            f32 Cx;
            f32 Az;
            f32 Bx;
            f32 Cz;
            f32 Bz;
        } three;
    } points;
    /* 0x24 */ f32 viewPitch;
    /* 0x28 */ s32 flag;
} pm_CameraControlSettings; // size = 0x2C

typedef struct pm_CamConfiguration {
    /* 0x00 */ f32 boomYaw;
    /* 0x04 */ f32 boomLength;
    /* 0x08 */ f32 boomPitch;
    /* 0x0C */ f32 viewPitch;
    /* 0x10 */ Vec3f targetPos;
} pm_CamConfiguration; // size = 0x1C

typedef struct pm_Camera {
    /* 0x000 */ u16 flags;
    /* 0x002 */ s16 moveFlags;
    /* 0x004 */ s16 updateMode;
    /* 0x006 */ s16 unk_06;
    /* 0x008 */ s16 isChangingMap;
    /* 0x00A */ s16 viewportW;
    /* 0x00C */ s16 viewportH;
    /* 0x00E */ s16 viewportStartX;
    /* 0x010 */ s16 viewportStartY;
    /* 0x012 */ s16 nearClip;
    /* 0x014 */ s16 farClip;
    /* 0x016 */ char unk_16[2];
    /* 0x018 */ f32 vfov;
    /* 0x01C */ s16 auxPitch;
    /* 0x01E */ s16 auxBoomLength;
    /* 0x020 */ s16 lookAt_dist;
    /* 0x022 */ s16 auxBoomPitch;
    /* 0x024 */ s16 auxBoomYaw;
    /* 0x026 */ s16 auxBoomZOffset;
    /* 0x028 */ s16 unk_28;
    /* 0x02A */ s16 zoomPercent;
    /* 0x02C */ s16 bgColor[3];
    /* 0x032 */ Vec3s targetScreenCoords;
    /* 0x038 */ u16 perspNorm;
    /* 0x03A */ char unk_3A[2];
    /* 0x03C */ Vec3f lookAt_eye;
    /* 0x048 */ Vec3f lookAt_obj;
    /* 0x054 */ Vec3f lookAt_obj_target;
    /* 0x060 */ Vec3f targetPos;
    /* 0x06C */ f32 currentYaw;
    /* 0x070 */ f32 unk_70;
    /* 0x074 */ f32 currentBoomYaw;
    /* 0x078 */ f32 currentBoomLength;
    /* 0x07C */ f32 currentYOffset;
    /* 0x080 */ char unk_80[4];
    /* 0x084 */ Vec3f trueRotation;
    /* 0x090 */ f32 currentBlendedYawNegated;
    /* 0x094 */ f32 currentPitch;
    /* 0x098 */ f32 unk_98;
    /* 0x09C */ f32 unk_9C;
    /* 0x0A0 */ Vp vp;
    /* 0x0B0 */ Vp vpAlt;
    /* 0x0C0 */ s32 unk_C0;
    /* 0x0C4 */ f32 unk_C4;
    /* 0x0C8 */ char unk_C8[0xC];
    /* 0x0D4 */ Matrix4f perspectiveMatrix;
    /* 0x114 */ Matrix4f viewMtxPlayer;  /* centers on player */
    /* 0x154 */ Matrix4f viewMtxLeading; /* leads player slightly */
    /* 0x194 */ Matrix4f viewMtxShaking; /* used while ShakeCam is active */
    /* 0x1D4 */ char unk_1D4[0x28];
    /* 0x1FC */ void (*fpDoPreRender)(struct pm_Camera *);
    /* 0x200 */ void (*fpDoPostRender)(struct pm_Camera *);
    /* 0x204 */ Mtx *unkMatrix;
    /* 0x208 */ s32 unk_208;
    /* 0x20C */ Matrix4s *unkEffectMatrix;
    /* 0x210 */ char unk_210[0x2];
    /* 0x212 */ s16 unk_212;
    /* 0x214 */ pm_CameraUnk unk_214[4];
    /* 0x444 */ pm_CameraControlSettings *prevController;
    /* 0x448 */ pm_CameraControlSettings *currentController;
    /* 0x44C */ pm_CamConfiguration prevConfiguration;
    /* 0x468 */ pm_CamConfiguration goalConfiguration;
    /* 0x484 */ f32 interpAlpha;
    /* 0x488 */ f32 linearInterp;
    /* 0x48C */ f32 linearInterpScale; /* 3.0? */
    /* 0x490 */ f32 moveSpeed;
    /* 0x494 */ f32 unk_494;
    /* 0x498 */ f32 unk_498;
    /* 0x49C */ f32 unk_49C;
    /* 0x4A0 */ f32 savedTargetY;
    /* 0x4A4 */ Vec3f prevTargetPos;
    /* 0x4B0 */ Vec3f movePos;
    /* 0x4BC */ Vec3f prevPrevMovePos;
    /* 0x4C8 */ Vec3f prevMovePos;
    /* 0x4D4 */ u16 prevPrevFollowFlags;
    /* 0x4D6 */ u16 prevFollowFlags;
    /* 0x4D8 */ pm_CameraControlSettings controlSettings;
    /* 0x504 */ u16 followPlayer;
    /* 0x506 */ u16 panActive;
    /* 0x508 */ f32 panPhase;
    /* 0x50C */ f32 leadAmount;
    /* 0x510 */ f32 unk_510;
    /* 0x514 */ f32 unk_514;
    /* 0x518 */ f32 unk_518;
    /* 0x51C */ s32 unk_51C;
    /* 0x520 */ f32 unk_520;
    /* 0x524 */ f32 unk_524;
    /* 0x528 */ f32 unk_528;
    /* 0x52C */ s32 unk_52C;
    /* 0x530 */ s32 unk_530;
    /* 0x534 */ pm_CameraControlSettings *aabbForZoneBelow;
    /* 0x538 */ char unk_538[0x18];
    /* 0x550 */ f32 unk_550;
    /* 0x554 */ s16 unk_554;
    /* 0x556 */ s16 unk_556;
} pm_Camera; // size = 0x558

typedef struct pm_ActionCommandStatus {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 hudElements[15];
    /* 0x40 */ char unk_40[0x4];
    /* 0x44 */ s16 barFillLevel;
    /* 0x46 */ s16 unk_46;
    /* 0x48 */ s16 unk_48;
    /* 0x4A */ s16 actionCommandID; // current action command id?
    /* 0x4C */ s16 state;
    /* 0x4E */ s16 unk_4E;
    /* 0x50 */ s16 unk_50;
    /* 0x52 */ s16 unk_52;
    /* 0x54 */ s16 unk_54;
    /* 0x56 */ s16 hudElementX;
    /* 0x58 */ s16 hudElementY;
    /* 0x5A */ s16 unk_5A;
    /* 0x5C */ s8 unk_5C;
    /* 0x5D */ s8 unk_5D;
    /* 0x5E */ s8 autoSucceed;
    /* 0x5F */ s8 unk_5F;
    /* 0x60 */ s8 unk_60;
    /* 0x61 */ s8 unk_61;
    /* 0x62 */ s8 unk_62;
    /* 0x63 */ s8 unk_63;
    /* 0x64 */ s16 unk_64;
    /* 0x66 */ s16 unk_66;
    /* 0x68 */ s16 unk_68;
    /* 0x6A */ s16 unk_6A;
    /* 0x6C */ s16 unk_6C;
    /* 0x6E */ s16 hitsTakenIsMax;
    /* 0x70 */ s16 unk_70;
    /* 0x72 */ s16 unk_72;
    /* 0x74 */ s16 mashMeterCutoffs[6]; // upper bounds for each interval
    /* 0x80 */ s8 mashMeterIntervals;
} pm_ActionCommandStatus; // size = 0x84

typedef struct pm_ItemData {
    /* 0x00 */ s32 nameMsg;
    /* 0x04 */ s16 hudElemID;
    /* 0x06 */ s16 sortValue;
    /* 0x08 */ s32 targetFlags;
    /* 0x0C */ s16 sellValue;
    /* 0x0E */ char unk_0E[2];
    /* 0x10 */ s32 fullDescMsg;
    /* 0x14 */ s32 shortDescMsg;
    /* 0x18 */ s16 typeFlags;
    /* 0x1A */ u8 moveID;
    /* 0x1B */ s8 potencyA;
    /* 0x1C */ s8 potencyB;
    /* 0x1D */ char unk_1D[3];
} pm_ItemData; // size = 0x20

typedef struct pm_HudElementSize {
    /* 0x00 */ s16 width;
    /* 0x02 */ s16 height;
    /* 0x04 */ s16 size;
} pm_HudElementSize; // size = 0x06

typedef struct pm_IconHudScriptPair {
    /* 0x00 */ s32 *enabled;
    /* 0x04 */ s32 *disabled;
} pm_IconHudScriptPair; // size = 0x08

typedef struct pm_Map {
    /* 0x00 */ char *id;     ///< "xxx_yyy" excluding null terminator.
    /* 0x04 */ void *config; // MapConfig*
    /* 0x08 */ void *dmaStart;
    /* 0x0C */ void *dmaEnd;
    /* 0x10 */ void *dmaDest;
    /* 0x14 */ char *bgName;
    /* 0x18 */ void *init; /// s32(*MapInit)(void) < Return TRUE to skip normal asset (shape/hit/bg/tex) loading.
    union {
        /* 0x1C */ u32 word;
        struct {
            /* 0x1C */ char unk_1C[0x2];
            /* 0x1E */ s8 songVariation; ///< 0 or 1. @see bgm_get_map_default_variation
            /* 0x1F */ s8 flags;
        };
    };
} pm_Map; // size = 0x20

typedef struct pm_Area {
    /* 0x00 */ s32 mapCount;
    /* 0x04 */ pm_Map *maps;
    /* 0x08 */ char *id;   ///< "area_xxx"
    /* 0x0C */ char *name; ///< JP debug name.
} pm_Area;                 // size = 0x10

typedef struct pm_Npc {
    /* 0x000 */ s32 flags;
    /* 0x004 */ void (*onUpdate)(struct pm_Npc *); ///< Run before anything else for this NPC in update_npcs()
    /* 0x008 */ void (*onRender)(struct pm_Npc *); ///< Run after the display list for this NPC is built
    /* 0x00C */ f32 yaw;
    /* 0x010 */ f32 planarFlyDist; /* also used for speech, temp0? */
    /* 0x014 */ f32 jumpScale;     /* also used for speech, temp1? */
    /* 0x018 */ f32 moveSpeed;
    /* 0x01C */ f32 jumpVelocity;
    /* 0x020 */ union {
        void *any;
        void *motion; ///< Null unless flag 0x100000 is set.
        void *chomp;
        void *quizmo;
        void *followData;
        struct pm_Npc *keepAwayNpc;
        s32 *keepAwayStarted;
    } blur;
    /* 0x024 */ s32 spriteInstanceID;
    /* 0x028 */ u32 currentAnim;
    /* 0x02C */ s32 animNotifyValue;
    /* 0x030 */ f32 animationSpeed;
    /* 0x034 */ f32 renderYaw;
    /* 0x038 */ Vec3f pos;
    /* 0x044 */ Vec3f rotation;
    /* 0x050 */ f32 rotationPivotOffsetY;
    /* 0x054 */ Vec3f scale;
    /* 0x060 */ Vec3f moveToPos;
    /* 0x06C */ Vec3f colliderPos; /* used during collision with player */
    /* 0x078 */ s32 shadowIndex;
    /* 0x07C */ f32 shadowScale;
    /* 0x080 */ s32 collisionChannel; /* flags used with collision tracing */
    /* 0x084 */ s16 currentFloor;     /* colliderID */
    /* 0x086 */ s16 currentWall;      /* colliderID */
    /* 0x088 */ s16 isFacingAway;
    /* 0x08A */ s16 yawCamOffset;
    /* 0x08C */ s16 turnAroundYawAdjustment;
    /* 0x08E */ s16 duration; // TODO: name less vaguely
    /* 0x090 */ Vec3s homePos;
    /* 0x096 */ s16 unk_96;
    /* 0x098 */ s16 foldType;
    /* 0x09A */ s16 foldArg1;
    /* 0x09C */ s16 foldArg2;
    /* 0x09E */ s16 foldArg3;
    /* 0x0A0 */ s16 foldArg4;
    /* 0x0A2 */ u16 foldFlags;
    /* 0x0A4 */ s8 npcID;
    /* 0x0A5 */ char unk_A5;
    /* 0x0A6 */ s16 collisionRadius;
    /* 0x0A8 */ s16 collisionHeight;
    /* 0x0AA */ s8 renderMode;
    /* 0x0AB */ s8 verticalRenderOffset;
    /* 0x0AC */ u8 alpha;
    /* 0x0AD */ u8 alpha2; ///< Multiplied with Npc::alpha
    /* 0x0AE */ char unk_AE[2];
    /* 0x0B0 */ u32 *extraAnimList;
    /* 0x0B4 */ s8 palSwapType; // 0..4 inclusive
    /* 0x0B5 */ s8 palSwapPrevType;
    /* 0x0B6 */ s8 dirtyPalettes;
    /* 0x0B7 */ s8 palSwapState;
    /* 0x0B8 */ char unk_B8[4];
    /* 0x0BC */ s16 palSwapTimer;
    /* 0x0BE */ s16 palSwapLerpAlpha;
    /* 0x0C0 */ s8 unk_C0;
    /* 0x0C1 */ s8 paletteCount;
    /* 0x0C2 */ char unk_C2[2];
    /* 0x0C4 */ u16 **spritePaletteList;
    /* 0x0C8 */ u16 localPaletteData[16][16];
    /* 0x2C8 */ u16 *localPalettes[16];
    /* 0x308 */ s16 unk_308;
    /* 0x30A */ s16 unk_30A;
    /* 0x30C */ u16 unk_30C;
    /* 0x30E */ s16 unk_30E;
    /* 0x310 */ s16 unk_310;
    /* 0x312 */ s16 unk_312;
    /* 0x314 */ s16 unk_314;
    /* 0x316 */ s16 unk_316;
    /* 0x318 */ f32 screenSpaceOffset2D[2];
    /* 0x320 */ f32 verticalStretch;
    /* 0x324 */ struct EffectInstance *decorations[2];
    /* 0x32C */ s8 decorationType[2];
    /* 0x32E */ s8 changedDecoration[2];
    /* 0x330 */ s8 decorationInitialised[2];
    /* 0x332 */ s16 decorationUnk[2];
    /* 0x336 */ char unk_336[10];
} pm_Npc; // size = 0x340

typedef struct pm_Action {
    /* 0x00 */ void (*update)(void);
    /* 0x04 */ void *dmaStart;
    /* 0x08 */ void *dmaEnd;
    /* 0x0C */ s8 flag;
} pm_Action; // size = 0x10

typedef void *(*PrintCallback)(void *, const char *, u32);
typedef pm_Evt *pm_ScriptList[128];

/* Data */
#define extern_data extern __attribute__((section(".data")))
extern_data u32 osMemSize;
extern_data pm_GameStatus pm_gGameStatus;
extern_data s32 pm_gCurrentCameraID;
extern_data pm_ItemData pm_gItemTable[0x16C];
extern_data pm_IconHudScriptPair pm_gItemHudScripts[337];
extern_data pm_Area pm_gAreas[29];
extern_data u32 nuScRetraceCounter;
extern_data s32 pm_gEncounterSubState;
extern_data s32 pm_timeFreezeMode;
extern_data s32 pm_gEncounterState;
extern_data void *pm_gSoundManager;
extern_data u16 *nuGfxCfb_ptr;
extern_data s32 pm_gOverrideFlags;
extern_data Gfx *pm_gMainGfxPos;
extern_data s32 pm_LogicalSaveInfo[4][2];
extern_data s16 pm_CurGameMode;
extern_data s16 pm_gMapTransitionAlpha;
extern_data s16 pm_gMapTransitionState;
extern_data pm_EncounterStatus pm_gCurrentEncounter;
extern_data pm_Camera pm_gCameras[4];
extern_data pm_EffectInstance *pm_gEffectInstances[96];
extern_data pm_SaveGlobals pm_gSaveGlobals;
extern_data pm_SaveData pm_gCurrentSaveFile;
extern_data s32 pm_gBattleState;
extern_data pm_BattleStatus pm_gBattleStatus;
extern_data s32 pm_gBattleSubState;
extern_data pm_Action pm_PlayerActionsTable[39];
extern_data s32 pm_gPopupState;
extern_data pm_PartnerStatus pm_gPartnerStatus;
extern_data pm_UiStatus pm_gUiStatus;
extern_data pm_PlayerStatus pm_gPlayerStatus;
extern_data pm_HudElementSize pm_gHudElementSizes[26];
extern_data s16 pm_MusicCurrentVolume;
extern_data pm_ActionCommandStatus pm_gActionCommandStatus;
extern_data s32 pm_battle_move_power_bounce_BaseHitChance;
extern_data s32 pm_gNumScripts;
extern_data pm_ScriptList *pm_gCurrentScriptListPtr;
extern_data s32 pm_gScriptIndexList[128];

/* Functions */
void osSyncPrintf(const char *fmt, ...);
void pm_step_game_loop(void);
void pm_fio_load_globals(void);
bool pm_fio_save_globals(void);
s32 pm_fio_validate_globals_checksums(void *buffer);
bool pm_fio_fetch_saved_file_info(void);
bool pm_fio_load_game(s32 saveSlot);
void pm_fio_deserialize_state(void);
void pm_fio_read_flash(s32 slot, void *buffer, u32 size); // writes to buffer in 128-byte blocks
void pm_fio_write_flash(s32 slot, void *buffer, u32 size);
void pm_fio_erase_flash(s32 slot);
void pm_set_curtain_scale_goal(f32 goal);
void pm_set_curtain_scale(f32 scale);
void pm_set_curtain_draw_callback(void *callback);
void pm_set_curtain_fade_goal(f32 goal);
void pm_set_curtain_fade(f32 fade);
void pm_update_cameras(void);
void pm_update_camera_no_interp(pm_Camera *camera);
void pm_update_camera_zone_interp(pm_Camera *camera);
void pm_set_game_mode(s32 mode);
pm_Npc *pm_get_npc_unsafe(s32 npcID);
pm_Npc *pm_get_npc_safe(s32 npcID);
void pm_au_sfx_reset_players(void *soundManager);
void pm_snd_ambient_stop_all(s32 time);
void pm_audio_set_stereo(void);
void pm_audio_set_mono(void);
void pm_remove_effect(pm_EffectInstance *effect);
void nuPiReadRom(u32 rom_addr, void *buf_ptr, u32 size);
void nuContDataGet(OSContPad *contpad, u32 cont_no);
void osWritebackDCacheAll(void);
s32 _Printf(PrintCallback pfn, void *arg, const char *fmt, va_list ap);
u64 osGetTime(void);
void osSetTime(u64 time);
OSThread *__osGetActiveQueue(void);
void osViSwapBuffer(void *vaddr);
void osViBlack(u8 active);
void osViRepeatLine(u8 active);
s32 pm_player_raycast_general(s32 mode, f32 startX, f32 startY, f32 startZ, f32 dirX, f32 dirY, f32 dirZ, f32 *hitX,
                              f32 *hitY, f32 *hitZ, f32 *hitDepth, f32 *hitNx, f32 *hitNy, f32 *hitNz);
void pm_disable_player_input(void);
void pm_update_player_input(void);
s32 pm_is_ability_active(s32 arg0);
void pm_hide_popup_menu(void);
void pm_destroy_popup_menu(void);
void pm_state_render_frontUI(void);
void pm_clear_printers(void);
void pm_set_screen_overlay_alpha(s32 idx, f32 alpha);
s32 pm_set_map_transition_effect(s32 transition);
s32 pm_update_exit_map_screen_overlay(s16 *progress);
void pm_clear_windows(void);
void pm_sfx_play_sound(s32 sound_id);
void pm_bgm_set_song(s32 player_index, s32 song_id, s32 variation, s32 fade_out_time, s16 volume);
void pm_bgm_pop_battle_song(void);
pm_ApiStatus pm_UseIdleAnimation(pm_Evt *script, s32 isInitialCall);
pm_ApiStatus pm_GotoMap(pm_Evt *script, s32 isInitialCall);
void pm_entity_SaveBlock_save_data(void);

/* Convenience Values */
#define STORY_PROGRESS pm_gCurrentSaveFile.globalBytes[0]
// NOLINTEND

#endif
