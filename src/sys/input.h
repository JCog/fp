#ifndef INPUT_H
#define INPUT_H
#include "common.h"
#include "menu/menu.h"

#define INPUT_REPEAT_DELAY 8
#define BIND_END           6

// stick values only valid for pm_Controller
#define BUTTON_C_RIGHT     0x00000001
#define BUTTON_C_LEFT      0x00000002
#define BUTTON_C_DOWN      0x00000004
#define BUTTON_C_UP        0x00000008
#define BUTTON_R           0x00000010
#define BUTTON_L           0x00000020
#define BUTTON_D_RIGHT     0x00000100
#define BUTTON_D_LEFT      0x00000200
#define BUTTON_D_DOWN      0x00000400
#define BUTTON_D_UP        0x00000800
#define BUTTON_START       0x00001000
#define BUTTON_Z           0x00002000
#define BUTTON_B           0x00004000
#define BUTTON_A           0x00008000
#define BUTTON_STICK_UP    0x00010000
#define BUTTON_STICK_DOWN  0x00020000
#define BUTTON_STICK_LEFT  0x00040000
#define BUTTON_STICK_RIGHT 0x00080000

u16 bindMake(s32 length, ...);
s32 bindGetComponent(u16 bind, s32 index);
u16 bindGetBitmask(u16 bind);

void inputUpdate(void);
u16 inputZPad(void);
s8 inputX(void);
s8 inputY(void);
pm_Controller inputPad(void);
pm_Controller inputPressedRaw(void);
pm_Controller inputPressed(void);
pm_Controller inputReleased(void);
void inputReserve(u16 bitmask);
void inputFree(u16 bitmask);
void inputBindSetDisable(s32 index, bool value);
void inputBindSetOverride(s32 index, bool value);
bool inputBindHeld(s32 index);
bool inputBindPressedRaw(s32 index);
bool inputBindPressed(s32 index);

struct MenuItem *binderCreate(struct Menu *menu, s32 x, s32 y, s32 bindIndex);

extern const u32 inputButtonColor[];

#endif
