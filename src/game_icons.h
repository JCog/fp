#ifndef ITEM_ICONS_H
#define ITEM_ICONS_H
#include "enums.h"
#include "fp.h"

typedef enum {
    Icon_ArrowLeft = 0x0,
    Icon_ArrowRight = 0x28,
    Icon_HandPointer = 0x50,
    Icon_AnimatedHandPointer = 0x78,
    Icon_FilledCircle = 0xC0,
    Icon_EmptyCircle = 0xE8,
    Icon_StatusSPShine = 0x23C,
    Icon_StatusStarPiece = 0x260,
    Icon_AsleepLoop = 0x2B8,
    Icon_AsleepBegin = 0x314,
    Icon_AsleepEnd = 0x4B4,
    Icon_ElectrifiedLoop = 0x60C,
    Icon_ElectrifiedBegin = 0x6F8,
    Icon_ElectrifiedEnd = 0x928,
    Icon_ParalyzedLoop = 0xA80,
    Icon_ParalyzedBegin = 0xB6C,
    Icon_ParalyzedEnd = 0xD9C,
    Icon_DizzyLoop = 0xEF4,
    Icon_DizzyBegin = 0xF50,
    Icon_DizzyEnd = 0x10F0,
    Icon_PoisonedLoop = 0x1248,
    Icon_PoisonedBegin = 0x12A4,
    Icon_PoisonedEnd = 0x1444,
    Icon_FrozenLoop = 0x159C,
    Icon_FrozenBegin = 0x15F8,
    Icon_FrozenEnd = 0x1798,
    Icon_WeakenedLoop = 0x18F0,
    Icon_WeakenedBegin = 0x192C,
    Icon_WeakenedEnd = 0x1AAC,
    Icon_StoppedBegin = 0x1C60,
    Icon_StoppedEnd = 0x1E00,
    Icon_ShrunkLoop = 0x1F58,
    Icon_ShrunkBegin = 0x1FB4,
    Icon_ShrunkEnd = 0x2154,
    Icon_TransparentLoop = 0x22AC,
    Icon_TransparentBegin = 0x2328,
    Icon_TransparentEnd = 0x24E8,
    Icon_BoostJumpLoop = 0x2640,
    Icon_BoostJumpBegin = 0x266C,
    Icon_BoostJumpEnd = 0x27CC,
    Icon_BoostHammerLoop = 0x2938,
    Icon_BoostHammerBegin = 0x2964,
    Icon_BoostHammerEnd = 0x2AC4,
    Icon_BoostPartner = 0x2C30,
    Icon_Surprise = 0x2C5C,
    Icon_FPCost = 0x2C88,
    Icon_FPCostReduced = 0x2CB4,
    Icon_FPCostReducedTwice = 0x2CE0,
    Icon_NotEnoughFP = 0x2D0C,
    Icon_POWCost = 0x2D38,
    Icon_POWCostReduced = 0x2D64,
    Icon_POWCostReducedTwice = 0x2D90,
    Icon_NotEnoughPOW = 0x2DBC,
    Icon_GreenArrowDown = 0x2DE8,
    Icon_GreenArrowUp = 0x2E34,
    Icon_UnusedPinkFrame = 0x2E80,
    Icon_UnusedDigit1 = 0x2EAC,
    Icon_UnusedDigit2 = 0x2ED8,
    Icon_UnusedDigit3 = 0x2F04,
    Icon_RedBar1 = 0x2F30,
    Icon_EmptyBar = 0x2F5C,
    Icon_RedBar2 = 0x2F88,
    Icon_MarioHead = 0x2FB4,
    Icon_Eldstar = 0x2FE0,
    Icon_EldstarDisabled = 0x300C,
    Icon_Mamar = 0x3038,
    Icon_MamarDisabled = 0x3064,
    Icon_Skolar = 0x3090,
    Icon_SkolarDisabled = 0x30BC,
    Icon_Muskular = 0x30E8,
    Icon_MuskularDisabled = 0x3114,
    Icon_Misstar = 0x3140,
    Icon_MisstarDisabled = 0x316C,
    Icon_Klevar = 0x3198,
    Icon_KlevarDisabled = 0x31C4,
    Icon_Kalmar = 0x31F0,
    Icon_KalmarDisabled = 0x321C,
    Icon_StarBeam = 0x3248,
    Icon_StarBeamDisabled = 0x3274,
    Icon_PeachBeam = 0x32A0,
    Icon_PeachBeamDisabled = 0x32CC,
    Icon_Partner0 = 0x32F8,
    Icon_Partner0Disabled = 0x3320,
    Icon_Goombario = 0x3348,
    Icon_GoombarioDisabled = 0x3370,
    Icon_Kooper = 0x3398,
    Icon_KooperDisabled = 0x33C0,
    Icon_Bombette = 0x33E8,
    Icon_BombetteDisabled = 0x3410,
    Icon_Parakarry = 0x3438,
    Icon_ParakarryDisabled = 0x3460,
    Icon_Bow = 0x3488,
    Icon_BowDisabled = 0x34B0,
    Icon_Watt = 0x34D8,
    Icon_WattDisabled = 0x3500,
    Icon_Sushie = 0x3528,
    Icon_SushieDisabled = 0x3550,
    Icon_Lakilester = 0x3578,
    Icon_LakilesterDisabled = 0x35A0,
    Icon_Partner9 = 0x35C8,
    Icon_Partner9Disabled = 0x35F0,
    Icon_PartnerA = 0x3618,
    Icon_PartnerADisabled = 0x3640,
    Icon_PartnerB = 0x3668,
    Icon_PartnerBDisabled = 0x3690,
    Icon_StatusTimes = 0x36B8,
    Icon_StatusSlash = 0x36E0,
    Icon_StatusDigit0 = 0x3708,
    Icon_StatusDigit1 = 0x3730,
    Icon_StatusDigit2 = 0x3758,
    Icon_StatusDigit3 = 0x3780,
    Icon_StatusDigit4 = 0x37A8,
    Icon_StatusDigit5 = 0x37D0,
    Icon_StatusDigit6 = 0x37F8,
    Icon_StatusDigit7 = 0x3820,
    Icon_StatusDigit8 = 0x3848,
    Icon_StatusDigit9 = 0x3870,
    Icon_StatusHP = 0x3898,
    Icon_StatusFP = 0x38C0,
    Icon_StatusSPIncrement1 = 0x38E8,
    Icon_StatusSPIncrement2 = 0x3910,
    Icon_StatusSPIncrement3 = 0x3938,
    Icon_StatusSPIncrement4 = 0x3960,
    Icon_StatusSPIncrement5 = 0x3988,
    Icon_StatusSPIncrement6 = 0x39B0,
    Icon_StatusSPIncrement7 = 0x39D8,
    Icon_StatusSPEmptyIncrement = 0x3A00,
    Icon_StatusStar1 = 0x3A28,
    Icon_StatusStar2 = 0x3A50,
    Icon_StatusStar3 = 0x3A78,
    Icon_StatusStar4 = 0x3AA0,
    Icon_StatusStar5 = 0x3AC8,
    Icon_StatusStar6 = 0x3AF0,
    Icon_StatusStar7 = 0x3B18,
    Icon_StatusStarEmpty = 0x3B40,
    Icon_StatusHeart = 0x3B68,
    Icon_StatusFlower = 0x3B88,
    Icon_StatusCoin = 0x3BA8,
    Icon_StatusStarPoint = 0x3CFC,
    Icon_MenuBoots1 = 0x3D94,
    Icon_MenuBoots1Disabled = 0x3DBC,
    Icon_MenuBoots2 = 0x3DE4,
    Icon_MenuBoots2Disabled = 0x3E0C,
    Icon_MenuBoots3 = 0x3E34,
    Icon_MenuBoots3Disabled = 0x3E5C,
    Icon_MenuHammer1 = 0x3E84,
    Icon_MenuHammer1Disabled = 0x3EAC,
    Icon_MenuHammer2 = 0x3ED4,
    Icon_MenuHammer2Disabled = 0x3EFC,
    Icon_MenuHammer3 = 0x3F24,
    Icon_MenuHammer3Disabled = 0x3F4C,
    Icon_MenuItem = 0x3F74,
    Icon_MenuItemDisabled = 0x3F9C,
    Icon_MenuStarPower = 0x3FC4,
    Icon_MenuStarPowerDisabled = 0x3FEC,
    Icon_Peril = 0x4014,
    Icon_Danger = 0x4064,
    Icon_Refund = 0x40B4,
    Icon_Happy = 0x40FC,
    Icon_HPDrain = 0x4124,
    Icon_BlueMeter = 0x414C,
    Icon_AButton = 0x4178,
    Icon_AButtonDown = 0x41A4,
    Icon_MashAButton = 0x41D0,
    Icon_PressAButton = 0x420C,
    Icon_SlowlyPressAButton = 0x4248,
    Icon_SlowlyMashAButton = 0x4284,
    Icon_StartButton = 0x42C0,
    Icon_StartButtonDown = 0x42EC,
    Icon_MashStartButton = 0x4318,
    Icon_PressStartButton = 0x4354,
    Icon_StartButtonText = 0x4390,
    Icon_RotateStickCW = 0x43BC,
    Icon_StickNeutral = 0x446C,
    Icon_StickHoldLeft = 0x4498,
    Icon_StickBackAndForth = 0x44D0,
    Icon_StickMashLeft = 0x4580,
    Icon_StickTapLeft = 0x45E0,
    Icon_StickTapNeutral = 0x4640,
    Icon_StickHoldDown = 0x46DC,
    Icon_StickMashDown = 0x4714,
    Icon_StickTapDown = 0x4774,
    Icon_StickSlowlyTapDown = 0x47D4,
    Icon_StickTapRight = 0x4834,
    Icon_RunAwayOK = 0x4894,
    Icon_MenuTimes = 0x48C0,
    Icon_PartnerRank1A = 0x48E8,
    Icon_PartnerRank1B = 0x4910,
    Icon_PartnerRank2A = 0x4938,
    Icon_PartnerRank2B = 0x4960,
    Icon_MoveDiamond = 0x4988,
    Icon_MoveBlueOrb = 0x49B0,
    Icon_MoveGreenOrb = 0x49D8,
    Icon_MoveRedOrb = 0x4A00,
    Icon_MoveDiamondDisabled = 0x4A28,
    Icon_MoveBlueOrbDisabled = 0x4A50,
    Icon_MoveGreenOrbDisabled = 0x4A78,
    Icon_MoveRedOrbDisabled = 0x4AA0
} icon_global;

typedef struct {
    /* 0x00 */ s32 foldIdx;
    /* 0x04 */ Vec3f position;
    /* 0x10 */ Vec3f rotation;
    /* 0x1C */ Vec3f scale;
    /* 0x28 */ Vec2s pivot;
    /* 0x30 */ VtxRect unk_30[3];
} game_icon_transform; // size = 0xF0

typedef struct {
    /* 0x00 */ u32 flags;
    /* 0x04 */ HudScript *readPos;
    /* 0x08 */ HudScript *anim;
    /* 0x0C */ HudScript *loopStartPos;
    /* 0x10 */ u8 *rasterAddr;
    /* 0x14 */ u8 *paletteAddr;
    /* 0x18 */ s32 memOffset;
    /* 0x1C */ game_icon_transform *hudTransform;
    /* 0x20 */ f32 deltaSizeX;
    /* 0x24 */ f32 deltaSizeY;
    /* 0x28 */ f32 unkImgScale[2];
    /* 0x30 */ f32 uniformScale;
    /* 0x34 */ s32 widthScale;  ///< X10
    /* 0x38 */ s32 heightScale; ///< X10
    /* 0x3C */ s16 renderPosX;
    /* 0x3E */ s16 renderPosY;
    /* 0x40 */ Vec2b screenPosOffset;
    /* 0x42 */ Vec3b worldPosOffset;
    /* 0x45 */ s8 drawSizePreset;
    /* 0x46 */ s8 tileSizePreset;
    /* 0x47 */ s8 updateTimer;
    /* 0x48 */ u8 sizeX; /* screen size? */
    /* 0x49 */ u8 sizeY; /* screen size? */
    /* 0x4A */ u8 opacity;
    /* 0x4B */ Color_RGB8 tint;
    /* 0x4E */ Vec2bu customImageSize;
    /* 0x50 */ Vec2bu customDrawSize;
} game_icon; // size = 0x54

game_icon *game_icons_create_global(icon_global icon, s32 x, s32 y, u8 alpha,
                                    f32 scale); /// note that not every icon works yet
game_icon *game_icons_create_item(Item item, s32 x, s32 y, u8 alpha, f32 scale, _Bool grayscale);
game_icon *game_icons_create_partner(Partner partner, s32 x, s32 y, u8 alpha, f32 scale, _Bool grayscale);

void game_icons_set_render_pos(game_icon *icon, s32 x, s32 y);
void game_icons_set_scale(game_icon *icon, f32 scale);
void game_icons_set_alpha(game_icon *icon, s32 opacity);
void game_icons_set_tint(game_icon *icon, u8 r, u8 g, u8 b);

void game_icons_draw(game_icon *icon);
void game_icons_delete(game_icon *icon);

game_icon *game_icons_update_next();

#endif
