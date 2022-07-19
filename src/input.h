#ifndef INPUT_H
#define INPUT_H
#include "common.h"
#include "menu.h"

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

u16 bind_make(s32 length, ...);
s32 bind_get_component(u16 bind, s32 index);
u16 bind_get_bitmask(u16 bind);

void input_update(void);
u16 input_z_pad(void);
s8 input_x(void);
s8 input_y(void);
u16 input_pad(void);
u16 input_pressed_raw(void);
u16 input_pressed(void);
u16 input_released(void);
void input_reserve(u16 bitmask);
void input_free(u16 bitmask);
void input_bind_set_disable(s32 index, _Bool value);
void input_bind_set_override(s32 index, _Bool value);
_Bool input_bind_held(s32 index);
_Bool input_bind_pressed_raw(s32 index);
_Bool input_bind_pressed(s32 index);

struct menu_item *binder_create(struct menu *menu, s32 x, s32 y, s32 bind_index);

extern const u32 input_button_color[];

#endif
