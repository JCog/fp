#include <stdlib.h>
#include <stdio.h>
#include "menu.h"
#include "settings.h"
#include "gfx.h"
#include "resource.h"

#define VISIABLE_X 9
#define ENTRY_X 8

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

#define MAX_NUM_ENTRIES 18

static struct menu_item *visible_checkbox_item;
static struct menu_item *address_input[MAX_NUM_ENTRIES];
static struct menu_item *data_type_option[MAX_NUM_ENTRIES];
static struct menu_item *data_value_static[MAX_NUM_ENTRIES];

static int checkbox_visible_proc(struct menu_item *item, enum menu_callback_reason reason, void* data)
{
	for(unsigned int i = 0; i < MAX_NUM_ENTRIES; i++)
	{
        	if (reason == MENU_CALLBACK_SWITCH_ON) 
		{
			data_value_static[i]->enabled = 1;
		}
        	
		if (reason == MENU_CALLBACK_SWITCH_OFF) 
		{
			data_value_static[i]->enabled = 0;
		}
	}
	return 0;
}

static int address_input_proc(struct menu_item *item, enum menu_callback_reason reason, void* data)
{
	if(reason == MENU_CALLBACK_CHANGED)
	{
		for(unsigned int entry = 0; entry < MAX_NUM_ENTRIES; entry++)
		{
			if(item == address_input[entry])
			{
				menu_watch_set_address(data_value_static[entry], menu_intinput_get(address_input[entry]));
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
		for(unsigned int entry = 0; entry < MAX_NUM_ENTRIES; entry++)
		{
			if(item == data_type_option[entry])
			{
				menu_watch_set_type(data_value_static[entry], menu_option_get(data_type_option[entry]));
				break;
			}
		}
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
    menu_checkbox_set(visible_checkbox_item, 1);

    for(unsigned int i = 0; i < MAX_NUM_ENTRIES; i++)
    {
	x_value = ENTRY_X;
    	address_input[i] = menu_add_intinput(&menu, x_value, y_value, 16, 8, address_input_proc, NULL);
	menu_intinput_set(address_input[i], 0x80000000);

	x_value += 9;    	
	data_type_option[i] = menu_add_option(&menu, x_value, y_value, data_type_options, data_type_opt_proc, NULL);

	x_value += 4;
	data_value_static[i] = menu_add_watch(&menu, x_value, y_value++, 0x80000000, WATCH_TYPE_U8); 
    }
    
    return &menu;
}
