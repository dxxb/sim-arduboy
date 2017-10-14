/*
	Copyright 2017 Delio Brignoli <brignoli.delio@gmail.com>

	Arduboy board implementation using simavr.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "sim_arduboy.h"
#include "arduboy_sdl.h"
#include "arduboy_avr.h"


static struct mod_state {
	SDL_Window *sdl_window;
	SDL_GLContext sdl_gl_context;
} mod_s;


static inline enum button_e key_to_button_e(int key)
{
	switch (key)
	{
		case SDLK_UP:
			return BTN_UP;
		case SDLK_DOWN:
			return BTN_DOWN;
		case SDLK_LEFT:
			return BTN_LEFT;
		case SDLK_RIGHT:
			return BTN_RIGHT;
		case SDLK_z:
			return BTN_A;
		case SDLK_x:
			return BTN_B;
		case SDLK_q:
			exit(0);
	}
	return -1;
}


static inline enum button_e dpad_to_button_e(uint8_t dpad_btn)
{
	switch (dpad_btn)
	{
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			return BTN_UP;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			return BTN_DOWN;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			return BTN_LEFT;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			return BTN_RIGHT;
		case SDL_CONTROLLER_BUTTON_A:
		case SDL_CONTROLLER_BUTTON_X:
			return BTN_A;
		case SDL_CONTROLLER_BUTTON_B:
		case SDL_CONTROLLER_BUTTON_Y:
			return BTN_B;
	}
	return -1;
}

static void key_event(int key, bool pressed)
{
	arduboy_avr_button_event(key_to_button_e(key), pressed);
}

static void dpad_event(uint8_t dpad_btn, bool pressed)
{
	arduboy_avr_button_event(dpad_to_button_e(dpad_btn), pressed);
}

/* Function called whenever redisplay needed */
void arduboy_sdl_render_frame(void)
{
	SDL_GL_SwapWindow(mod_s.sdl_window);
}

int arduboy_sdl_setup(struct sim_arduboy_opts *opts)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return -1;
	}

	mod_s.sdl_window = SDL_CreateWindow("Sim-Arduboy",
										SDL_WINDOWPOS_UNDEFINED,
										SDL_WINDOWPOS_UNDEFINED,
										opts->win_width,
										opts->win_height,
										SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	assert(mod_s.sdl_window != NULL);
	mod_s.sdl_gl_context = SDL_GL_CreateContext(mod_s.sdl_window);
	assert(mod_s.sdl_gl_context != NULL);
	return 0;
}

int arduboy_sdl_loop(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_QUIT:
				return -1;
			case SDL_KEYDOWN:
				if (event.key.repeat)
					continue;
				key_event(event.key.keysym.sym, true);
				break;
			case SDL_KEYUP:
				if (event.key.repeat)
					continue;
				key_event(event.key.keysym.sym, false);
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				dpad_event(event.cbutton.button, true);
				break;
			case SDL_CONTROLLERBUTTONUP:
				dpad_event(event.cbutton.button, false);
				break;
		}
	}
	return 0;
}

void arduboy_sdl_teardown(void)
{
	SDL_DestroyWindow(mod_s.sdl_window);
	SDL_Quit();
}
