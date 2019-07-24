    
#include <stdio.h>
#include <stdlib.h>
#include <startup.h>
#include <inttypes.h>
#include "fp.h"
#include "pm64.h"
#include "gfx.h"

_Bool ready = 0;

void fp_main(void){

	gfx_begin();
	char test_string[100]  = "hello, printing is working";
	gfx_printf(10, 10, test_string);
	gfx_finish();

	pm_GameUpdate(); /* displaced function call - advances 1 game frame*/
}

void init(){

    gfx_init();
    ready = 1;

}

// Initilizes and uses new stack instead of using graph threads stack. 
static void init_stack(void (*func)(void)) {
    static _Alignas(8) __attribute__((section(".stack"))) 
    char stack[0x2000];
    __asm__ volatile(   "la     $t0, %1;"
                        "sw     $sp, -0x04($t0);"
                        "sw     $ra, -0x08($t0);"
                        "addiu  $sp, $t0, -0x08;"
                        "jalr   %0;"
                        "nop;"
                        "lw     $ra, 0($sp);"
                        "lw     $sp, 4($sp);"
                        ::
                        "r"(func),
                        "i"(&stack[sizeof(stack)]));
}


/* fp entry point - init stack and call main function */
ENTRY void _start(void){

	init_gp();
	if(!ready){
		init_stack(init);
	}
	init_stack(fp_main);

}
