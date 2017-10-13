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

struct ssd1306_t;

void ssd1306_gl_update_lumamap(struct ssd1306_t *ssd1306, const uint8_t luma_decay, const uint8_t luma_inc);
void ssd1306_gl_render(struct ssd1306_t *ssd1306);
void ssd1306_gl_init(float pixel_size, int win_width, int win_height);
