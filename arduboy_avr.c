
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sim_avr.h>
#include <avr_ioport.h>
#include <avr_extint.h>
#include <sim_hex.h>
#include <sim_gdb.h>
#include <sim_time.h>
#include <ssd1306_virt.h>

#include "sim_arduboy.h"
#include "arduboy_avr.h"
#include "arduboy_sdl.h"
#include "ssd1306_gl.h"


#define MHZ_16 (16000000)


static struct button_info {
	enum button_e btn_id;
	avr_irq_t *irq;
	const char *name;
	char port_name;
	int port_idx;
	bool pressed;
} buttons[BTN_COUNT] = {
	{0, NULL, "btn.up", 'F', 7}, /* BTN_UP */
	{1, NULL, "btn.down", 'F', 4},
	{2, NULL, "btn.left", 'F', 5},
	{3, NULL, "btn.right", 'F', 6},
	{4, NULL, "btn.a", 'E', 6},
	{5, NULL, "btn.b", 'B', 4},
};

/* SSD1306 wired to the SPI bus, with the following additional pins: */
static ssd1306_wiring_t ssd1306_wiring =
{
	.chip_select.port = 'D',
	.chip_select.pin = 6,
	.data_instruction.port = 'D',
	.data_instruction.pin = 4,
	.reset.port = 'D',
	.reset.pin = 7,
};

static struct arduboy_avr_mod_state {
	struct avr_t *avr;
	ssd1306_t ssd1306;
	uint64_t start_time_ns;
	bool yield;
} mod_s;

/*
Simavr's default sleep callback results in simulated time and
wall clock time to diverge over time. This replacement tries to
keep them in sync by sleeping for the time required to match the
expected sleep deadline in wall clock time.
*/
static void avr_callback_sleep_sync(
		avr_t *avr,
		avr_cycle_count_t how_long)
{
	struct timespec tp;

	/* figure out how long we should wait to match the sleep deadline */
	uint64_t deadline_ns = avr_cycles_to_nsec(avr, avr->cycle + how_long);
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
	uint64_t runtime_ns = (tp.tv_sec*1000000000+tp.tv_nsec) - mod_s.start_time_ns;
	if (runtime_ns >= deadline_ns) {
		return;
	}

	uint64_t sleep_us = (deadline_ns - runtime_ns)/1000;
	usleep(sleep_us);
	return;
}

static avr_cycle_count_t update_luma(
		avr_t *avr,
		avr_cycle_count_t when,
		void *param)
{
	ssd1306_gl_update_lumamap(param, LUMA_DECAY, LUMA_INC);
	return avr->cycle + avr_usec_to_cycles(avr, SSD1306_FRAME_PERIOD_US);
}

static avr_cycle_count_t render_timer_callback(
			avr_t *avr,
			avr_cycle_count_t when,
			void *param)
{
	ssd1306_gl_render(param);
	arduboy_sdl_render_frame();
	mod_s.yield = true;
	return avr->cycle + avr_usec_to_cycles(avr, GL_FRAME_PERIOD_US);
}

struct ssd1306_t *arduboy_avr_ssd1306(void)
{
	return &mod_s.ssd1306;
}

void arduboy_avr_button_event(enum button_e btn_e, bool pressed)
{
	struct button_info *btn = (btn_e < BTN_COUNT) ? &buttons[btn_e] : NULL;
	if (btn && btn->pressed != pressed) {
		avr_raise_irq(btn->irq, !pressed);
		btn->pressed = pressed;
	}
}

void arduboy_avr_loop(void)
{
	avr_t *avr = mod_s.avr;
	mod_s.yield = false;
	while (!mod_s.yield) {
		avr->run(avr);
		int state = avr->state;
		if (state == cpu_Done || state == cpu_Crashed)
			break;
	}
}

int arduboy_avr_setup(struct sim_arduboy_opts *opts)
{
	avr_t *avr = avr_make_mcu_by_name("atmega32u4");
	if (!avr) {
		return -1;
	}

	avr_init(avr);

	/*
	BTN_A is wired to INT6 which defaults to level triggered.
	This means that while button A is pressed the interrupt triggers
	continuously. This is very expensive to simulate so we set non-strict
	level trigger mode for INT6.

	Why doesn't this affect real h/w?
	*/
	avr_extint_set_strict_lvl_trig(avr, EXTINT_IRQ_OUT_INT6, 0);

	{
		/* Load .hex and setup program counter */
		uint32_t boot_base, boot_size;
		uint8_t * boot = read_ihex_file(opts->hex_file_path, &boot_size, &boot_base);
		if (!boot) {
			fprintf(stderr, "Unable to load %s\n", opts->hex_file_path);
			return -1;
		}
		memcpy(avr->flash + boot_base, boot, boot_size);
		free(boot);
		avr->pc = boot_base;
		/* end of flash, remember we are writing /code/ */
		avr->codeend = avr->flashend;
	}

	/* more simulation parameters */
	avr->log = 1 + opts->verbose;
	avr->frequency = MHZ_16;
	avr->sleep = avr_callback_sleep_sync;
	avr->run_cycle_limit = avr_usec_to_cycles(avr, 2*GL_FRAME_PERIOD_US);

	/* setup and connect display controller */
	ssd1306_init(avr, &mod_s.ssd1306, OLED_WIDTH_PX, OLED_HEIGHT_PX);
	ssd1306_connect(&mod_s.ssd1306, &ssd1306_wiring);
	ssd1306_gl_init(opts->pixel_size, opts->win_width, opts->win_height);

	/* setup and connect buttons */
	for (int btn_idx=0; btn_idx<BTN_COUNT; btn_idx++) {
		struct button_info *binfo = &buttons[btn_idx];
		binfo->irq = avr_alloc_irq(&avr->irq_pool, 0, 1, &binfo->name);
		uint32_t iop_ctl = AVR_IOCTL_IOPORT_GETIRQ(binfo->port_name);
		avr_irq_t *iop_irq = avr_io_getirq(avr, iop_ctl, binfo->port_idx);
		avr_connect_irq(binfo->irq, iop_irq);
		/* pull up pin */
		avr_raise_irq(binfo->irq, 1);
	}

	/* Take simulation start time */
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
	mod_s.start_time_ns = tp.tv_sec*1000000000+tp.tv_nsec;

	/* Setup display render timers */
	avr_cycle_timer_register_usec(avr, SSD1306_FRAME_PERIOD_US, update_luma, &mod_s.ssd1306);
	avr_cycle_timer_register_usec(avr, GL_FRAME_PERIOD_US, render_timer_callback, &mod_s.ssd1306);

	/* setup for GDB debugging */
	avr->gdb_port = opts->gdb_port;
	if (opts->debug) {
		avr->state = cpu_Stopped;
		avr_gdb_init(avr);
	}

	mod_s.avr = avr;
	return 0;
}

void arduboy_avr_teardown(void)
{

}