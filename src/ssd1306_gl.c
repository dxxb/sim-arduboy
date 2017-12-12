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
#include <string.h>

#if __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "ssd1306_gl.h"
#include <ssd1306_virt.h>


static struct mod_state {
	int win_width;
	int win_height;
	float pixel_size;
	uint8_t luma_pixmap[SSD1306_VIRT_COLUMNS*SSD1306_VIRT_PAGES*8];
} mod_s;


static inline void gl_set_bg_colour_(uint8_t invert, float opacity)
{
	if (invert) {
		glColor4f(1.0f, 1.0f, 1.0f, opacity);
	} else {
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	}
}

static inline void gl_set_fg_colour_(uint8_t invert, float opacity)
{
	if (invert) {
		glColor4f(0.0f, 0.0f, 0.0f, opacity);
	} else {
		glColor4f(1.0f, 1.0f, 1.0f, opacity);
	}
}

static float contrast_to_opacity_(uint8_t contrast)
{
	// Typically the screen will be clearly visible even at 0 contrast
	return contrast / 512.0 + 0.5;
}

void ssd1306_gl_render(struct ssd1306_t *ssd1306)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!ssd1306_get_flag(ssd1306, SSD1306_FLAG_DISPLAY_ON)) {
		return;
	}

	const uint8_t seg_remap_default = ssd1306_get_flag(ssd1306, SSD1306_FLAG_SEGMENT_REMAP_0);
	const uint8_t seg_comscan_default = ssd1306_get_flag(ssd1306, SSD1306_FLAG_COM_SCAN_NORMAL);
	const float pixel_size = mod_s.pixel_size;

	// Set up projection matrix
	glMatrixMode(GL_PROJECTION);
	// Start with an identity matrix
	glLoadIdentity();
	glOrtho(0, mod_s.win_width, 0, mod_s.win_height, 0, 10);
	// Apply vertical and horizontal display mirroring
	glScalef(seg_remap_default ? -1 : 1, seg_comscan_default ? 1 : -1, 1);
	glTranslatef(seg_remap_default ? -mod_s.win_width : 0, seg_comscan_default ? 0: -mod_s.win_height, 0);

	/* No need to setup GL modelview matrix because it defaults to identity */

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	float opacity = contrast_to_opacity_(ssd1306->contrast_register);
	int invert = ssd1306_get_flag (ssd1306, SSD1306_FLAG_DISPLAY_INVERTED);
	gl_set_bg_colour_(invert, opacity);

	glTranslatef (0, 0, 0);

	glBegin (GL_QUADS);
	glVertex2f (0, ssd1306->rows*pixel_size);
	glVertex2f (0, 0);
	glVertex2f (ssd1306->columns*pixel_size, 0);
	glVertex2f (ssd1306->columns*pixel_size, ssd1306->rows*pixel_size);
	
	uint8_t *px_ptr = mod_s.luma_pixmap;
	float v_ofs = 0;
	while (v_ofs < ssd1306->rows*pixel_size) {
		float h_ofs = 0;
		while (h_ofs < ssd1306->columns*pixel_size) {
			gl_set_fg_colour_(invert, ((float)(*px_ptr))/255.0 * opacity);
			glVertex2f(h_ofs + pixel_size, v_ofs + pixel_size);
			glVertex2f(h_ofs, v_ofs + pixel_size);
			glVertex2f(h_ofs, v_ofs);
			glVertex2f(h_ofs + pixel_size, v_ofs);

			h_ofs += pixel_size;
			px_ptr++;
		}
		v_ofs += pixel_size;
	}
	glEnd ();
}

void ssd1306_gl_update_lumamap(struct ssd1306_t *ssd1306, const uint8_t luma_decay, const uint8_t luma_inc)
{
	uint8_t *column_ptr = mod_s.luma_pixmap;
	for (int p = 0; p < SSD1306_VIRT_PAGES; p++) {
		for (int c = 0; c < SSD1306_VIRT_COLUMNS; c++) {
			uint8_t px_col = ssd1306->vram[p][c];
			for (int px_idx = 0; px_idx < 8*SSD1306_VIRT_COLUMNS; px_idx += SSD1306_VIRT_COLUMNS) {
				int16_t luma = column_ptr[px_idx];
				luma -= luma_decay;
				if (px_col & 0x1) {
					luma += luma_inc;
				}
				/* clamp value to [0, 255] */
				if (luma < 0) {
					luma = 0;
				} else if (luma > 255) {
					luma = 255;
				}
				column_ptr[px_idx] = luma;
				px_col >>= 1;
			}
			column_ptr++;
		}
		column_ptr += SSD1306_VIRT_COLUMNS*7;
	}
}

void ssd1306_gl_init(float pixel_size, int win_width, int win_height)
{
	mod_s.win_width = win_width;
	mod_s.win_height = win_height;
	mod_s.pixel_size = pixel_size;
	memset(&mod_s.luma_pixmap, 0, sizeof(mod_s.luma_pixmap));
}
