/**
 * @file lv_port_disp_templ.c
 *
 */
#include <assert.h>
#include <SDL.h>
#include "lvglj.h"
#include "lv_conf.h"
#include "lv_port_disp_sdl2.h"

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp_sdl2.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
#if LV_USE_GPU
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);
static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
        const lv_area_t * fill_area, lv_color_t color);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
static SDL_Window *win = NULL;
static SDL_Surface *wsuf = NULL;
static SDL_Renderer *rend = NULL;
static SDL_Texture *tex = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /* LVGL requires a buffer where it draws the objects. The buffer's has to be greater than 1 display row
     *
     * There are three buffering configurations:
     * 1. Create ONE buffer with some rows: 
     *      LVGL will draw the display's content here and writes it to your display
     * 
     * 2. Create TWO buffer with some rows: 
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     * 
     * 3. Create TWO screen-sized buffer: 
     *      Similar to 2) but the buffer have to be screen sized. When LVGL is ready it will give the
     *      whole frame to display. This way you only need to change the frame buffer's address instead of
     *      copying the pixels.
     * */

    /* Example for 1) */
#if 0
    static lv_disp_buf_t disp_buf_1;
    static lv_color_t buf1_1[LV_HOR_RES_MAX * 10];                      /*A buffer for 10 rows*/
    lv_disp_buf_init(&disp_buf_1, buf1_1, NULL, LV_HOR_RES_MAX * 10);   /*Initialize the display buffer*/
#endif

    /* Example for 2) */
#if 0
    static lv_disp_buf_t disp_buf_2;
    static lv_color_t buf2_1[LV_HOR_RES_MAX * 10];                        /*A buffer for 10 rows*/
    static lv_color_t buf2_2[LV_HOR_RES_MAX * 10];                        /*An other buffer for 10 rows*/
    lv_disp_buf_init(&disp_buf_2, buf2_1, buf2_2, LV_HOR_RES_MAX * 10);   /*Initialize the display buffer*/
#endif

    /* Example for 3) */
    static lv_disp_buf_t disp_buf_3;
    static lv_color_t buf3_1[LV_HOR_RES_MAX * LV_VER_RES_MAX];            /*A screen sized buffer*/
    static lv_color_t buf3_2[LV_HOR_RES_MAX * LV_VER_RES_MAX];            /*An other screen sized buffer*/
    lv_disp_buf_init(&disp_buf_3, buf3_1, buf3_2, LV_HOR_RES_MAX * LV_VER_RES_MAX);   /*Initialize the display buffer*/


    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = LV_HOR_RES_MAX;//480;
    disp_drv.ver_res = LV_VER_RES_MAX;//320;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.buffer = &disp_buf_3;

#if LV_USE_GPU
    /*Optionally add functions to access the GPU. (Only in buffered mode, LV_VDB_SIZE != 0)*/

    /*Blend two color array using opacity*/
    disp_drv.gpu_blend_cb = gpu_blend;

    /*Fill a memory array with a color*/
    disp_drv.gpu_fill_cb = gpu_fill;
#endif

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

void lv_port_disp_term(void)
{
	SDL_DestroyWindow(win);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* Initialize your display and the required peripherals. */
static void disp_init(void)
{
    /*You code here*/
	win = SDL_CreateWindow(
		"DBConsole",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		LV_HOR_RES_MAX,
		LV_VER_RES_MAX,
		0);
	if (win == NULL)
		abort();

	rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	assert(rend != NULL);

	tex = SDL_CreateTexture(rend,
							SDL_PIXELFORMAT_ARGB8888,
							SDL_TEXTUREACCESS_STREAMING,
							LV_HOR_RES_MAX, LV_VER_RES_MAX);
	assert(tex);

	wsuf = SDL_GetWindowSurface(win);
	if (wsuf == NULL)
		abort();

	SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(rend);
	SDL_RenderCopy(rend, tex, NULL, NULL);
	SDL_RenderPresent(rend);

	SDL_FillRect(wsuf, NULL, SDL_MapRGB(wsuf->format, 0,0,0x20));
	SDL_UpdateWindowSurface(win);
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
#if 0
    int32_t x;
    int32_t y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            /* Put a pixel to the display. For example: */
            /* put_px(x, y, *color_p)*/
            color_p++;
        }
    }
#endif
	const int dx = area->x1;
	const int dy = area->y1;
	const int width  = area->x2+1 - area->x1;
	const int height = area->y2+1 - area->y1;
	const int pitch = width*LV_COLOR_DEPTH/8;
#if   LV_COLOR_DEPTH == 1
	const int src_format = SDL_PIXELFORMAT_INDEX1LSB;
#elif LV_COLOR_DEPTH == 8
	const int src_format = SDL_PIXELFORMAT_RGB332;
#elif LV_COLOR_DEPTH == 16
	const int src_format = SDL_PIXELFORMAT_RGB565;
#elif LV_COLOR_DEPTH == 32
	const int src_format = SDL_PIXELFORMAT_ARGB8888;
#else
	const int src_format = SDL_PIXELFORMAT_ARGB8888;
#endif
	/* SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "x,y,w,h=%d,%d,%d,%d d=%d\n",dx,dy,width,height,LV_COLOR_DEPTH); */
	const void *dst = (void *)((uintptr_t)wsuf->pixels + wsuf->pitch*dy + wsuf->format->BytesPerPixel*dx);

#if 1 /* Surface version */
	SDL_LockSurface(wsuf);
	SDL_ConvertPixels(
		width, height,
		src_format, color_p, width*LV_COLOR_DEPTH/8,
		wsuf->format->format, dst, wsuf->pitch);
	SDL_UnlockSurface(wsuf);
	SDL_UpdateWindowSurface(win);

    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
#else /* Texture version */
	SDL_Rect rc  = {
		.x=area->x1,
		.y=area->y1,
		.w=area->x2+1 - area->x1,
		.h=area->y2+1 - area->y1,
	};
	void *pixels = NULL;
	int tex_pitch = 0;
	//Verbose(">Lock");
	SDL_LockTexture(tex, NULL, &pixels, &tex_pitch);
	SDL_UpdateTexture(tex, NULL, color_p, tex_pitch);
	SDL_UnlockTexture(tex);
	//Verbose("<Unlock");

	SDL_RenderCopy(rend, tex, NULL, NULL);
	SDL_RenderPresent(rend);

	SDL_UpdateWindowSurface(win);

    lv_disp_flush_ready(disp_drv);
#endif


}


/*OPTIONAL: GPU INTERFACE*/
#if LV_USE_GPU

/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
    /*It's an example code which should be done by your GPU*/
    uint32_t i;
    for(i = 0; i < length; i++) {
        dest[i] = lv_color_mix(dest[i], src[i], opa);
    }
}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color)
{
    /*It's an example code which should be done by your GPU*/
    int32_t x, y;
    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/

    for(y = fill_area->y1; y <= fill_area->y2; y++) {
        for(x = fill_area->x1; x <= fill_area->x2; x++) {
            dest_buf[x] = color;
        }
        dest_buf+=dest_width;    /*Go to the next line*/
    }
}

#endif  /*LV_USE_GPU*/

#else /* Enable this file at the top */

/* This dummy typedef exists purely to silence -Wpedantic. */
typedef int keep_pedantic_happy;
#endif
