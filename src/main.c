#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "lvgl.h"
#include "lv_port_disp_sdl2.h"
#include "lv_port_indev_sdl2.h"

static void hal_init();
static int tick_thread(void *data);

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    lv_init();
    hal_init();

    //lv_demo_widgets();
    //lv_demo_benchmark();
    //lv_demo_keypad_encoder();
    //lv_demo_printer();
    //lv_demo_stress();
    lv_ex_get_started_1();
    //lv_ex_get_started_2();
    //lv_ex_get_started_3();

    //lv_ex_style_1();
    //lv_ex_style_2();
    //lv_ex_style_3();
    //lv_ex_style_4();
    //lv_ex_style_5();
    //lv_ex_style_6();
    //lv_ex_style_7();
    //lv_ex_style_8();
    //lv_ex_style_9();
    //lv_ex_style_10();
    //lv_ex_style_11();

	Uint32 t0 = SDL_GetTicks();
	int quit = 0;
    while (!quit) {
        /* Periodically call the lv_task handler.
        * It could be done in a timer interrupt or an OS task too.*/
        lv_task_handler();

		Uint32 t1 = SDL_GetTicks();
		Uint32 td = t1 - t0; /* expect for wrap around */
		int timeout = td<5 ? 5-td: 0;
		SDL_Event event;
		if (SDL_WaitEventTimeout(&event, timeout)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = 1;
				break;
			default:
				break;
			}
		}

		static Uint32 last_log_tick = 0;
		Uint32 delta = t1 - last_log_tick;
		if (delta >= 100) {
			last_log_tick = t1;
		}

		t0 = t1;
    }

	SDL_Quit();

    return 0;
}

/**********************
*   STATIC FUNCTIONS
**********************/
static void hal_init(void)
{
	lv_port_disp_init();
	lv_port_indev_init();

#if 0
    /* Add the mouse (or touchpad) as input device
    * Use the 'mouse' driver which reads the PC's mouse*/
    mouse_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = mouse_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/
    lv_indev_drv_register(&indev_drv);

    /* If the PC keyboard driver is enabled in`lv_drv_conf.h`
    * add this as an input device. It might be used in some examples. */
#if USE_KEYBOARD
    lv_indev_drv_t kb_drv;
    lv_indev_drv_init(&kb_drv);
    kb_drv.type = LV_INDEV_TYPE_KEYPAD;
    kb_drv.read_cb = keyboard_read;
    kb_indev = lv_indev_drv_register(&kb_drv);
#endif
#endif

    /* Tick init.
    * You have to call 'lv_tick_inc()' in every milliseconds
    * Create an SDL thread to do this*/
    SDL_CreateThread(tick_thread, "tick", NULL);
}

static int tick_thread(void *data)
{
	while (1) {
		SDL_Delay(5);   /*Sleep for 1 millisecond*/
		lv_tick_inc(5);
    }

    return 0;
}
