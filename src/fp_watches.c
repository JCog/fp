#include <stdlib.h>
#include <stdio.h>
#include <vector/vector.h>
#include "menu.h"
#include "settings.h"
#include "gfx.h"
#include "resource.h"
#include "fp.h"

#define VISIABLE_X 9
#define DEFAULT_ADDRESS 0x80000000
#define DEFAULT_TYPE  WATCH_TYPE_U8



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

struct watch_item
{
    struct menu_item *remove_item;
    struct menu_item *address_input;
    struct menu_item *data_type_option;
    struct menu_item *data_value_static;
    struct menu_item *anchor_item;
    struct menu_item *pos_item;
};

static struct vector watches;

static struct menu menu;

static struct menu_item *visible_checkbox_item;
static struct menu_item *add_item;

static int checkbox_visible_proc(struct menu_item *item, enum menu_callback_reason reason, void* data)
{
    for(unsigned int idx = 0; idx < watches.size; idx++)
    {
        struct watch_item *entry = vector_at(&watches, idx);
        if (reason == MENU_CALLBACK_SWITCH_ON) 
        {
            if(!settings->watch_info[idx].anchored)
                menu_item_enable(entry->data_value_static); 
            else
                menu_item_disable(entry->data_value_static); 
            settings->bits.watches_enabled = 1;
        }
            
        if (reason == MENU_CALLBACK_SWITCH_OFF) 
        {
            menu_item_disable(entry->data_value_static); 
            settings->bits.watches_enabled = 0;
        }
    }
    return 0;
}

static int address_input_proc(struct menu_item *item, enum menu_callback_reason reason, void* data)
{
    if(reason == MENU_CALLBACK_CHANGED)
    {
        for(unsigned int idx = 0; idx < watches.size; idx++)
        {
            struct watch_item *entry = vector_at(&watches, idx);
            if(entry->address_input == item)
            {
                menu_watch_set_address(entry->data_value_static, menu_intinput_get(item));
                settings->watch_address[idx] = menu_intinput_get(item);
                break;
            }
        }
    }
    return 0;
}   

static int data_type_opt_proc(struct menu_item *item, enum menu_callback_reason reason, void* data)
{
    if(reason == MENU_CALLBACK_CHANGED)
    {
        for(unsigned int idx = 0; idx < watches.size; idx++)
        {
            struct watch_item *entry = vector_at(&watches, idx);
            if(entry->data_type_option == item)
            {
                menu_watch_set_type(entry->data_value_static, menu_option_get(item));
                settings->watch_info[idx].type = menu_option_get(item);
                break;
            }
        }
    }
    return 0;
}

static int anchor_proc(struct menu_item* item, enum menu_callback_reason reason, void* data)
{
    for(unsigned int idx = 0; idx < watches.size; idx++)
    {
        struct watch_item *entry = vector_at(&watches, idx);
        if(entry->anchor_item == item)
        {
            if(reason == MENU_CALLBACK_SWITCH_ON)
            {
                settings->watch_info[idx].anchored = 1;
                menu_item_disable(entry->data_value_static);
                menu_item_enable(entry->pos_item);
            }
            
            if(reason == MENU_CALLBACK_SWITCH_OFF)
            {
                settings->watch_info[idx].anchored = 0;
                if(menu_checkbox_get(visible_checkbox_item))
                    menu_item_enable(entry->data_value_static);
                menu_item_disable(entry->pos_item);
            }

            break;
        }
    }
    return 0;
}

static int pos_mod_proc(struct menu_item* item, enum menu_callback_reason reason, void* data)
{
    unsigned int idx = 0;
    struct watch_item *entry = NULL;
    
    for(idx = 0; idx < watches.size; idx++)
    {
        entry = vector_at(&watches, idx);
        if(item == entry->pos_item)
            break;
    }
    
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
        default:
            break;
    }
    return 0;
}

static void remove_watch_proc(struct menu_item* item, void* data)
{
    for(unsigned int idx = 0; idx < watches.size; idx++)
    {
        struct watch_item *entry = vector_at(&watches, idx);
        if(entry->remove_item == item)
        {
            menu_navigate(item->owner, MENU_NAVIGATE_UP);
            menu_item_remove(entry->remove_item);
            menu_item_remove(entry->anchor_item);
            menu_item_remove(entry->pos_item);
            menu_item_remove(entry->address_input);
            menu_item_remove(entry->data_type_option);
            menu_item_remove(entry->data_value_static);
            
            for(unsigned int jdx = idx + 1; jdx < watches.size; jdx++)
            {
                struct watch_item *update_entry = vector_at(&watches, jdx);
                update_entry->remove_item->y--;
                update_entry->anchor_item->y--;
                update_entry->pos_item->y--;
                update_entry->address_input->y--;
                update_entry->data_type_option->y--;
                update_entry->data_value_static->y--;
                
                //if(jdx < watches.size - 1)
                {
                    settings->watch_info[jdx - 1].type = settings->watch_info[jdx].type;
                    settings->watch_info[jdx - 1].anchored = settings->watch_info[jdx].anchored;
                    settings->watch_address[jdx - 1] = settings->watch_address[jdx];
                    settings->watch_x[jdx - 1] = settings->watch_x[jdx];
                    settings->watch_y[jdx - 1] = settings->watch_y[jdx];
                }
                
            }
            
            vector_erase(&watches, idx, 1);
            settings->n_watches--;
            add_item->y = settings->n_watches + 2;
            break;
        }
    }
}

static void add_watch_proc(struct menu_item* item, void* data)
{
    int x_value = 0;
    int y_value = watches.size + 2;
    struct gfx_texture *t_anchor = resource_load_grc_texture("anchor");
    struct gfx_texture *t_list = resource_load_grc_texture("list_icons");
    
    if(settings->n_watches < SETTINGS_WATCHES_MAX)
    {
        unsigned int setting_idx = settings->n_watches;
        struct watch_item *watch_item = vector_push_back(&watches, 1, NULL);
        watch_item->remove_item = menu_add_button_icon(item->owner, 0, y_value, t_list, 1, 0xff0000, remove_watch_proc, watch_item);
        x_value+=2;
        watch_item->anchor_item = menu_add_switch(item->owner, x_value++, y_value, t_anchor, 1, 0xFFFFFF, t_anchor, 0, 0xFFFFFF, 1, 0, anchor_proc, watch_item);

        x_value+=2;
        watch_item->pos_item = menu_add_positioning(item->owner, x_value++, y_value, pos_mod_proc, watch_item);
        menu_item_disable(watch_item->pos_item);

        x_value+=4;
        watch_item->address_input = menu_add_intinput(item->owner, x_value, y_value, 16, 8, address_input_proc, watch_item);
        menu_intinput_set(watch_item->address_input, DEFAULT_ADDRESS);

        x_value += 9;       
        watch_item->data_type_option = menu_add_option(item->owner, x_value, y_value, data_type_options, data_type_opt_proc, watch_item);

        x_value += 4;
        watch_item->data_value_static = menu_add_watch(item->owner, x_value, y_value++, DEFAULT_ADDRESS, DEFAULT_TYPE); 
        if(!menu_checkbox_get(visible_checkbox_item))
            menu_item_disable(watch_item->data_value_static);
        
        settings->watch_address[setting_idx] = DEFAULT_ADDRESS;
        settings->watch_info[setting_idx].anchored = 0;
        settings->watch_info[setting_idx].type = DEFAULT_TYPE;
        settings->watch_x[setting_idx] = SETTINGS_WATCH_SCREEN_X;
        settings->watch_y[setting_idx] = SETTINGS_WATCH_SCREEN_Y + (setting_idx * 0x8);
        
        settings->n_watches++;
        
        add_item->y = y_value;
    }
}

struct menu *create_watches_menu(void)
{
    int y_value = 0;
    
    vector_init(&watches, sizeof(struct watch_item));
    
    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    menu.selector = menu_add_submenu(&menu, 0, y_value++, NULL, "return");

    menu_add_static(&menu, 0, y_value, "visible", 0xC0C0C0);
    visible_checkbox_item = menu_add_checkbox(&menu, VISIABLE_X, y_value++, checkbox_visible_proc, NULL);
    
    struct gfx_texture *t_list = resource_load_grc_texture("list_icons");
    
    add_item = menu_add_button_icon(&menu, 0, y_value++, t_list, 0, 0x00ff00, add_watch_proc, NULL);
    
    return &menu;
}

void apply_watch_settings()
{
    int x_value = 0;
    int y_value = 2;
    struct gfx_texture *t_anchor = resource_load_grc_texture("anchor");
    struct gfx_texture *t_list = resource_load_grc_texture("list_icons");
    struct watch_item *watch_item = NULL;
    
    for(unsigned int idx = 0; idx < watches.size; idx++)
    {
        watch_item = vector_at(&watches, idx);
        menu_item_remove(watch_item->remove_item);
        menu_item_remove(watch_item->anchor_item);
        menu_item_remove(watch_item->pos_item);
        menu_item_remove(watch_item->address_input);
        menu_item_remove(watch_item->data_type_option);
        menu_item_remove(watch_item->data_value_static);
    }
    vector_clear(&watches);
    
    menu_checkbox_set(visible_checkbox_item, settings->bits.watches_enabled);

    for(unsigned int idx = 0; idx < settings->n_watches; idx++)
    {
        x_value = 0;
        watch_item = vector_push_back(&watches, 1, NULL);
        
        watch_item->remove_item = menu_add_button_icon(&menu, x_value, y_value, t_list, 1, 0xff0000, remove_watch_proc, NULL);

        x_value+=2;
        watch_item->anchor_item = menu_add_switch(&menu, x_value++, y_value, t_anchor, 1, 0xFFFFFF, t_anchor, 0, 0xFFFFFF, 1, 0, anchor_proc, NULL);
        menu_switch_set(watch_item->anchor_item, settings->watch_info[idx].anchored);

        x_value+=2;
        watch_item->pos_item = menu_add_positioning(&menu, x_value++, y_value, pos_mod_proc, NULL);

        x_value+=4;
        watch_item->address_input = menu_add_intinput(&menu, x_value, y_value, 16, 8, address_input_proc, NULL);
        menu_intinput_set(watch_item->address_input, settings->watch_address[idx]);

        x_value += 9;       
        watch_item->data_type_option = menu_add_option(&menu, x_value, y_value, data_type_options, data_type_opt_proc, NULL);
        menu_option_set(watch_item->data_type_option, settings->watch_info[idx].type);

        x_value += 4;
        watch_item->data_value_static = menu_add_watch(&menu, x_value, y_value++, DEFAULT_ADDRESS, DEFAULT_TYPE); 
        menu_watch_set_address(watch_item->data_value_static, settings->watch_address[idx]);
        menu_watch_set_type(watch_item->data_value_static, settings->watch_info[idx].type);

        if(!settings->watch_info[idx].anchored && settings->bits.watches_enabled)
            menu_item_enable(watch_item->data_value_static);
        else
            menu_item_disable(watch_item->data_value_static); 
    }
    add_item->y = settings->n_watches + 2;
}
