#ifndef INPUT_H
#define INPUT_H
#include "common.h"
#include "menu/menu.h"

#define INPUT_REPEAT_DELAY 8
#define BIND_END           6

#define BUTTON_C_RIGHT     0x0001
#define BUTTON_C_LEFT      0x0002
#define BUTTON_C_DOWN      0x0004
#define BUTTON_C_UP        0x0008
#define BUTTON_R           0x0010
#define BUTTON_L           0x0020
#define BUTTON_D_RIGHT     0x0100
#define BUTTON_D_LEFT      0x0200
#define BUTTON_D_DOWN      0x0400
#define BUTTON_D_UP        0x0800
#define BUTTON_START       0x1000
#define BUTTON_Z           0x2000
#define BUTTON_B           0x4000
#define BUTTON_A           0x8000

u16 bindMake(s32 length, ...);
s32 bindGetComponent(u16 bind, s32 index);
u16 bindGetBitmask(u16 bind);

void inputUpdate(void);
u16 inputZPad(void);
s8 inputX(void);
s8 inputY(void);
u16 inputPad(void);
u16 inputPressedRaw(void);
u16 inputPressed(void);
u16 inputReleased(void);
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
