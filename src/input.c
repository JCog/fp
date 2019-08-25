#include <stdint.h>
#include <stdarg.h>
#include "input.h"
#include "pm64.h"
#include "commands.h"
#include "settings.h"


#define BIND_END 6

static int button_time[16];
static uint16_t pad_pressed_raw;
static uint16_t pad_pressed;
static uint16_t pad_released;
static uint16_t pad;
static uint16_t reserved;
static int bind_component_state[COMMAND_MAX];
static int bind_time[COMMAND_MAX];
static _Bool bind_pressed_raw[COMMAND_MAX];
static _Bool bind_pressed[COMMAND_MAX];

static int bind_get_component(uint16_t bind, int index){
    return (bind >> (4 * index)) & 0xF;
}

static uint16_t bind_get_bitmask(uint16_t bind){
    uint16_t p = 0;
    for(int i=0;i<4;i++){
        int c = bind_get_component(bind,i);
        if(c==BIND_END) break;
        p|= 1 << c;
    }
    return p;
}

uint16_t make_bind(int len, ...){
    va_list list;
    va_start(list,len);
    uint16_t ret = 0;
    uint16_t val = 0;
    for(int i=0;i<len;i++){
        val = va_arg(list,int);
        for(int j=0;j<16;j++){
            if(val & (1 << j)){
                ret |= j << (i * 4);
            }
        }
    }
    va_end(list);
    if(len<4){
        ret |= 6 << (len*4);
    }
    return ret;
}

const uint32_t input_button_color[] =
{
  0xFFA000,
  0xFFA000,
  0xFFA000,
  0xFFA000,
  0xC0C0C0,
  0xC0C0C0,
  0x000000,
  0x000000,
  0xC8C8C8,
  0xC8C8C8,
  0xC8C8C8,
  0xC8C8C8,
  0xC80000,
  0xC0C0C0,
  0x009600,
  0x5A5AFF,
};

void input_update(){
    uint16_t pm_pad = pm_status.raw.buttons;
    pad_pressed_raw = (pad ^ pm_pad) & pm_pad;
    pad_released = (pad ^ pm_pad) & ~pm_pad;
    pad = pm_pad;
    pad_pressed = 0;
    for(int i=0;i<16;i++){
        uint16_t p = 1 << i;
        if(pad & p){
            button_time[i]++;
        }else{
            button_time[i] = 0;
        }
        if((pad_pressed_raw & p) || button_time[i] >= INPUT_REPEAT_DELAY){
            pad_pressed |= p;
        }
    }
    uint16_t bind_pad[SETTINGS_BIND_MAX];
    _Bool bind_state[SETTINGS_BIND_MAX];
    for(int i=0; i < SETTINGS_BIND_MAX; i++){
        uint16_t *b = &settings->binds[i];
        bind_pad[i] = bind_get_bitmask(*b);
        int *cs = &bind_component_state[i];
        int j;
        uint16_t c;
        if((reserved & bind_pad[i]) && i!=0){
            *cs = 0;
        }else{
            int css = *cs;
            for(j=0;j<4;j++){
                c = bind_get_component(*b,j);
                if(c==BIND_END) break;
                uint8_t csm = 1 << j;
                if(*cs & csm){
                    if(pad & (1 << c))
                        continue;
                    else{
                        if(*cs & ~((1 << (j+1))-1)){
                            *cs = 0;
                        }else{
                            *cs = ~csm;
                        }
                        break;
                    }
                }
                if((pad_released & (1 << c)) || (css!=0 && (pad_pressed_raw & ~bind_pad[i]))){
                    *cs = 0;
                    break;
                }else if(pad_pressed_raw & (1 << c)){
                    *cs |= csm;
                }else
                {
                    break;
                }
            }
        }
        bind_state[i] = (*cs && (j==4 || c == BIND_END));
    }
    for(int i=0; i < SETTINGS_BIND_MAX; i++){
        uint16_t pi = bind_pad[i];
        for(int j = 0; bind_state[i] && j < SETTINGS_BIND_MAX; j++){
            if(!bind_state[j]) continue;
            uint16_t pj = bind_pad[j];
            if(pi!=pj && (pi & pj) == pi){
                bind_component_state[i] = 0;
                bind_state[i] = 0;
            }
        }
        bind_pressed_raw[i] = (bind_time[i] == 0 && bind_state[i]);
        if(!bind_state[i])
            bind_time[i] = 0;
        else{
            bind_time[i]++;
        }
        bind_pressed[i] = (bind_pressed_raw[i] || bind_time[i] >= INPUT_REPEAT_DELAY);
    }
}

_Bool input_bind_held(int index){
    return bind_time[index]>0;
}

_Bool input_bind_pressed(int index){
    return bind_pressed[index];
}

_Bool input_bind_pressed_raw(int index){
    return bind_pressed_raw[index];
}

void reserve_buttons(uint16_t button_bitmask){
    reserved |= button_bitmask;
}

void free_buttons(uint16_t button_bitmask){
    reserved ^= button_bitmask;
}