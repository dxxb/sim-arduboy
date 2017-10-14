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

#include <stdbool.h>

enum button_e {
	BTN_UP = 0,
	BTN_DOWN,
	BTN_LEFT,
	BTN_RIGHT,
	BTN_A,
	BTN_B,
	BTN_COUNT,
};

struct sim_arduboy_opts;

int arduboy_avr_setup(struct sim_arduboy_opts *opts);
void arduboy_avr_loop(void);
void arduboy_avr_teardown(void);

void arduboy_avr_button_event(enum button_e btn_e, bool pressed);
