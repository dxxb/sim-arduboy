
#include <assert.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <OpenGL/gl.h>

#include "sim_arduboy.h"
#include "arduboy_sdl.h"
#include "arduboy_simavr.h"
#include "sim_avr.h"
#include "sim_time.h"
#include "avr_ioport.h"
#include "ssd1306_virt.h"
#include "ssd1306_gl.h"


#define LUMA_INC (256*2/3)
#define LUMA_DECAY (256/3)

#define SSD1306_FRAME_PERIOD_US (7572)
#define GL_FRAME_PERIOD_US (SSD1306_FRAME_PERIOD_US*12)


static struct mod_state {
	avr_t *avr;
	int win_width;
	int win_height;
	float pixel_size;
	SDL_Window *sdl_window;
	SDL_GLContext sdl_gl_context;
} mod_s;


enum button_e {
	BTN_UP = 0,
	BTN_DOWN,
	BTN_LEFT,
	BTN_RIGHT,
	BTN_A,
	BTN_B,
	BTN_COUNT,
};

struct button_info {
	enum button_e btn_id;
	avr_irq_t *irq;
	const char *name;
	char port_name;
	int port_idx;
	bool pressed;
};

static struct button_info buttons[BTN_COUNT] = {
	{0, NULL, "btn.up", 'F', 7}, /* BTN_UP */
	{1, NULL, "btn.down", 'F', 4},
	{2, NULL, "btn.left", 'F', 5},
	{3, NULL, "btn.right", 'F', 6},
	{4, NULL, "btn.a", 'E', 6},
	{5, NULL, "btn.b", 'B', 4},
};

struct btn_event {
	enum button_e btn_id;
	avr_irq_t *irq;
	bool pressed;
};


static avr_cycle_count_t update_luma(
		avr_t *avr,
		avr_cycle_count_t when,
		void *param)
{
	update_lumamap(&arduboy_simavr_mod_s->ssd1306, LUMA_DECAY, LUMA_INC);
	return avr->cycle + avr_usec_to_cycles(avr, SSD1306_FRAME_PERIOD_US);
}


static inline struct button_info *key_to_button_info(int key)
{
	switch (key)
	{
		case SDLK_UP:
			return &buttons[BTN_UP];
		case SDLK_DOWN:
			return &buttons[BTN_DOWN];
		case SDLK_LEFT:
			return &buttons[BTN_LEFT];
		case SDLK_RIGHT:
			return &buttons[BTN_RIGHT];
		case SDLK_z:
			return &buttons[BTN_A];
		case SDLK_x:
			return &buttons[BTN_B];
		case SDLK_q:
			exit(0);
			return NULL;
	}
	return NULL;
}


static inline struct button_info *dpad_to_button_info(uint8_t dpad_btn)
{
	switch (dpad_btn)
	{
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			return &buttons[BTN_UP];
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			return &buttons[BTN_DOWN];
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			return &buttons[BTN_LEFT];
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			return &buttons[BTN_RIGHT];
		case SDL_CONTROLLER_BUTTON_A:
		case SDL_CONTROLLER_BUTTON_X:
			return &buttons[BTN_A];
		case SDL_CONTROLLER_BUTTON_B:
		case SDL_CONTROLLER_BUTTON_Y:
			return &buttons[BTN_B];
	}
	return NULL;
}


void notify_button_event(struct button_info *btn, bool pressed)
{
	if (btn && btn->pressed != pressed) {
		struct btn_event e = {btn->btn_id, btn->irq, pressed};
		avr_raise_irq(e.irq, !e.pressed);
		btn->pressed = pressed;
	}
}

void key_press(int key)
{
	struct button_info *btn = key_to_button_info(key);
	notify_button_event(btn, true);
}

void key_release(int key)
{
	struct button_info *btn = key_to_button_info(key);
	notify_button_event(btn, false);
}

void dpad_press(uint8_t dpad_btn)
{
	struct button_info *btn = dpad_to_button_info(dpad_btn);
	notify_button_event(btn, true);
}

void dpad_release(uint8_t dpad_btn)
{
	struct button_info *btn = dpad_to_button_info(dpad_btn);
	notify_button_event(btn, false);
}

/* Function called whenever redisplay needed */
void
displayCB (void)
{
	const uint8_t seg_remap_default = ssd1306_get_flag(&arduboy_simavr_mod_s->ssd1306, SSD1306_FLAG_SEGMENT_REMAP_0);
	const uint8_t seg_comscan_default = ssd1306_get_flag(&arduboy_simavr_mod_s->ssd1306, SSD1306_FLAG_COM_SCAN_NORMAL);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up projection matrix
	glMatrixMode(GL_PROJECTION);
	// Start with an identity matrix
	glLoadIdentity();
	glOrtho(0, mod_s.win_width, 0, mod_s.win_height, 0, 10);
	// Apply vertical and horizontal display mirroring
	glScalef(seg_remap_default ? -1 : 1, seg_comscan_default ? 1 : -1, 1);
	glTranslatef(seg_remap_default ? -mod_s.win_width : 0, seg_comscan_default ? 0: -mod_s.win_height, 0);

	// Select modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	// Start with an identity matrix
	glLoadIdentity();
	ssd1306_gl_render(&arduboy_simavr_mod_s->ssd1306);
	glPopMatrix();
	SDL_GL_SwapWindow(mod_s.sdl_window);
}


static avr_cycle_count_t schedule_render(
			avr_t *avr,
			avr_cycle_count_t when,
			void *param)
{
	displayCB();
	arduboy_simavr_mod_s->yield = true;
	return avr->cycle + avr_usec_to_cycles(avr, GL_FRAME_PERIOD_US);
}


int arduboy_sdl_setup(struct sim_arduboy_opts *opts, struct avr_t *avr)
{
	avr->run_cycle_limit = avr_usec_to_cycles(avr, 2*GL_FRAME_PERIOD_US);

	mod_s.win_width = arduboy_simavr_mod_s->ssd1306.columns * opts->pixel_size;
	mod_s.win_height = arduboy_simavr_mod_s->ssd1306.rows * opts->pixel_size;
	mod_s.pixel_size = opts->pixel_size;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return -1;
	}

	mod_s.sdl_window = SDL_CreateWindow("Sim-Arduboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mod_s.win_width, mod_s.win_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	assert(mod_s.sdl_window != NULL);
	mod_s.sdl_gl_context = SDL_GL_CreateContext(mod_s.sdl_window);
	assert(mod_s.sdl_gl_context != NULL);

	ssd1306_gl_init(mod_s.pixel_size);

	for (int btn_idx=0; btn_idx<BTN_COUNT; btn_idx++) {
		struct button_info *binfo = &buttons[btn_idx];
		binfo->irq = avr_alloc_irq(&avr->irq_pool, 0, 1, &binfo->name);
		uint32_t iop_ctl = AVR_IOCTL_IOPORT_GETIRQ(binfo->port_name);
		avr_irq_t *iop_irq = avr_io_getirq(avr, iop_ctl, binfo->port_idx);
		avr_connect_irq(binfo->irq, iop_irq);
		/* pull up pin */
		avr_raise_irq(binfo->irq, 1);
	}

	avr_cycle_timer_register_usec(avr, SSD1306_FRAME_PERIOD_US, update_luma, &arduboy_simavr_mod_s->ssd1306);
	avr_cycle_timer_register_usec(avr, GL_FRAME_PERIOD_US, schedule_render, &arduboy_simavr_mod_s->ssd1306);

	return 0;
}

int arduboy_sdl_loop(void)
{
	bool quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
					if (event.key.repeat)
						continue;
					key_press(event.key.keysym.sym);
					break;
				case SDL_KEYUP:
					if (event.key.repeat)
						continue;
					key_release(event.key.keysym.sym);
					break;
				case SDL_CONTROLLERBUTTONDOWN:
					dpad_press(event.cbutton.button);
					break;
				case SDL_CONTROLLERBUTTONUP:
					dpad_release(event.cbutton.button);
					break;
			}
		}
		avr_run_loop();
	}
	return 0;
}

void arduboy_sdl_teardown(void)
{
	SDL_DestroyWindow(mod_s.sdl_window);
	SDL_Quit();
}
