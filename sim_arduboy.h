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


#define OLED_WIDTH_PX (128)
#define OLED_HEIGHT_PX (64)

#define LUMA_INC (256*2/3)
#define LUMA_DECAY (256/3)

#define SSD1306_FRAME_PERIOD_US (7572)
#define GL_FRAME_PERIOD_US (SSD1306_FRAME_PERIOD_US*12)


struct sim_arduboy_opts {
	char *hex_file_path;
	int gdb_port;
	bool debug;
	int verbose;
	int pixel_size;
	int win_width;
	int win_height;
};
