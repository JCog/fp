#include <stdlib.h>
#include <stdio.h>
#include "menu.h"
#include "settings.h"
#include "gfx.h"
#include "resource.h"

#define VISIABLE_X 9


const char data_type_options[] = {
    "u8\0"
    "s8\0"
    "x8\0"
    "u16\0"
    "s16\0"
    "x16\0"
    "u32\0"
    "s32\0"
    "x32\0"
    "f32\0"
};

static struct menu_item *visible_checkbox_item;
static struct menu_item *address_input[SETTINGS_WATCHES_MAX];
static struct menu_item *data_type_option[SETTINGS_WATCHES_MAX];
static struct menu_item *data_value_static[SETTINGS_WATCHES_MAX];
static struct menu_item *anchor_item[SETTINGS_WATCHES_MAX];
static struct menu_item *pos_item[SETTINGS_WATCHES_MAX];

static int checkbox_visible_proc(struct menu_item *item, enum menu_callback_reason reason, void* data)
{
    for(unsigned int i = 0; i < SETTINGS_WATCHES_MAX; i++)
    {
        if (reason == MENU_CALLBACK_SWITCH_ON) 
        {
            data_value_static[i]->enabled = !settings->watch_info[i].anchored;
            settings->bits.watches_enabled = 1;
        }
            
        if (reason == MENU_CALLBACK_SWITCH_OFF) 
        {
            data_value_static[i]->enabled = 0;
            settings->bits.watches_enabled = 0;
        }
        
        if (reason == MENU_CALLBACK_THINK) 
        {
            menu_checkbox_set(visible_checkbox_item, settings->bits.watches_enabled);
            data_value_static[i]->enabled = (!settings->watch_info[i].anchored && settings->bits.watches_enabled);
        }
    }
    return 0;
}

static int address_input_proc(struct menu_item *item, enum menu_callback_reason reason, void* data)
{
    unsigned int entry = (unsigned int)data;
    if(reason == MENU_CALLBACK_CHANGED)
    {
        menu_watch_set_address(data_value_static[entry], menu_intinput_get(address_input[entry]));
        settings->watch_address[entry] = menu_intinput_get(address_input[entry]);
    }
    
    else if (reason == MENU_CALLBACK_THINK_INACTIVE) 
    {
        menu_intinput_set(address_input[entry], settings->watch_address[entry]);
        menu_watch_set_address(data_value_static[entry], settings->watch_address[entry]);
    }
    
    return 0;
}   

static int data_type_opt_proc(struct menu_item *item, enum menu_callback_reason reason, void* data)
{
    unsigned int entry = (unsigned int)data;
    if(reason == MENU_CALLBACK_CHANGED)
    {
        menu_watch_set_type(data_value_static[entry], menu_option_get(data_type_option[entry]));
        settings->watch_info[entry].type = menu_option_get(data_type_option[entry]);
    }
    
    if (reason == MENU_CALLBACK_THINK) 
    {
        menu_option_set(data_type_option[entry], settings->watch_info[entry].type);
        menu_watch_set_type(data_value_static[entry], settings->watch_info[entry].type);
    }
    return 0;
}

static int anchor_proc(struct menu_item* item, enum menu_callback_reason reason, void* data)
{
    unsigned int idx = (unsigned int)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) 
    {
        settings->watch_info[idx].anchored = 1;
        data_value_static[idx]->enabled = 0;
        pos_item[idx]->enabled = 1;
    }
    if(reason == MENU_CALLBACK_SWITCH_OFF)
    {
        pos_item[idx]->enabled = 0;
        settings->watch_info[idx].anchored = 0;
        data_value_static[idx]->enabled = menu_checkbox_get(visible_checkbox_item);
    }
    if (reason == MENU_CALLBACK_THINK) 
    {
        menu_switch_set(anchor_item[idx], settings->watch_info[idx].anchored);
        pos_item[idx]->enabled = settings->watch_info[idx].anchored;
    }
    return 0;
}

static int pos_mod_proc(struct menu_item* item, enum menu_callback_reason reason, void* data)
{
    unsigned int idx = (unsigned int)data;
    int16_t *x = &settings->watch_x[idx];
    int16_t *y = &settings->watch_y[idx];
    int dist = 2;
    if (input_pad() & BUTTON_Z) {
        dist *= 2;
    }
    switch (reason) {
        case MENU_CALLBACK_ACTIVATE:    input_reserve(BUTTON_Z);  break;
        case MENU_CALLBACK_DEACTIVATE:  input_free(BUTTON_Z);     break;
        case MENU_CALLBACK_NAV_UP:      *y -= dist;               break;
        case MENU_CALLBACK_NAV_DOWN:    *y += dist;               break;
        case MENU_CALLBACK_NAV_LEFT:    *x -= dist;               break;
        case MENU_CALLBACK_NAV_RIGHT:   *x += dist;               break;
        case MENU_CALLBACK_THINK: 
            pos_item[idx]->enabled = (settings->watch_info[idx].anchored && settings->bits.watches_enabled);
            break;
        default:
            break;
    }
    return 0;
}

struct menu *create_watches_menu(void)
{
    static struct menu menu;
    
    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    int y_value = 0;
    int x_value = 0;
    menu.selector = menu_add_submenu(&menu, 0, y_value++, NULL, "return");

    menu_add_static(&menu, 0, y_value, "visible", 0xC0C0C0);
    visible_checkbox_item = menu_add_checkbox(&menu, VISIABLE_X, y_value++, checkbox_visible_proc, NULL);
    
    struct gfx_texture *t_anchor = resource_load_grc_texture("anchor");

    for(unsigned int i = 0; i < SETTINGS_WATCHES_MAX; i++)
    {
        x_value = 0;
        //Put Remove here

        x_value+=2;
        anchor_item[i] = menu_add_switch(&menu, x_value++, y_value, t_anchor, 1, 0xFFFFFF, t_anchor, 0, 0xFFFFFF, 1, 0, anchor_proc, (void*)i);

        x_value+=2;
        pos_item[i] = menu_add_positioning(&menu, x_value++, y_value, pos_mod_proc, (void*)i);

        x_value+=4;
        address_input[i] = menu_add_intinput(&menu, x_value, y_value, 16, 8, address_input_proc, (void*)i);

        x_value += 9;       
        data_type_option[i] = menu_add_option(&menu, x_value, y_value, data_type_options, data_type_opt_proc, (void*)i);

        x_value += 4;
        data_value_static[i] = menu_add_watch(&menu, x_value, y_value++, 0x80000000, WATCH_TYPE_U8); 
        
    }
    
    return &menu;
}
