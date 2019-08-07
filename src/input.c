#include <stdint.h>
#include <stdarg.h>
#include "input.h"
#include "pm64.h"

#define BIND_END 6


uint16_t current_frame;
uint16_t previous_frame;
uint16_t press_release;


static uint16_t pad_pressed;
static uint16_t pad_held;
static uint16_t pad_released;
static uint16_t reserved;

void input_update(){

    uint16_t current_frame  = pm_status.raw.buttons;
    uint16_t previous_frame = pm_status.previous.buttons;
    uint16_t press_release  = pm_status.press_release.buttons;

}

uint16_t get_current_frame(){
	return current_frame;
}

uint16_t get_previous_frame(){
	return previous_frame;
}

uint16_t get_press_release(){
	return press_release;
}

/*

uint16_t get_pad_pressed(){
    return (pad_pressed ^ reserved) & pad_pressed;
}

uint16_t get_pad_pressed_unrestricted(){
    return pad_pressed;
}

uint16_t get_pad_released(){
    return pad_released;
}

uint16_t get_pad_held(){
    return (pad_held ^ reserved) & pad_held;
}

void reserve_buttons(uint16_t button_bitmask){
    reserved |= button_bitmask;
}

void free_buttons(uint16_t button_bitmask){
    reserved ^= button_bitmask;
}
*/