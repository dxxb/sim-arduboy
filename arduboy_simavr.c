
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sim_arduboy.h"
#include "arduboy_simavr.h"
#include "sim_avr.h"
#include "avr_ioport.h"
#include "avr_extint.h"
#include "sim_hex.h"
#include "sim_gdb.h"
#include "sim_time.h"
#include "ssd1306_virt.h"


#define MHZ_16 (16000000)

static struct arduboy_simavr_mod_state mod_s;

struct arduboy_simavr_mod_state *arduboy_simavr_mod_s = &mod_s;

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

void avr_run_loop(void)
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

int arduboy_simavr_setup(struct sim_arduboy_opts *opts, struct avr_t **out)
{
	uint32_t boot_base, boot_size;

	assert(out != NULL);

	avr_t *avr = avr_make_mcu_by_name("atmega32u4");
	if (!avr) {
		return -1;
	}
	*out = avr;
	mod_s.avr = avr;

	uint8_t * boot = read_ihex_file(opts->hex_file_path, &boot_size, &boot_base);
	if (!boot) {
		fprintf(stderr, "Unable to load %s\n", opts->hex_file_path);
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

	memcpy(avr->flash + boot_base, boot, boot_size);
	free(boot);
	avr->pc = boot_base;
	/* end of flash, remember we are writing /code/ */
	avr->codeend = avr->flashend;
	avr->log = 1 + opts->verbose;
	avr->frequency = MHZ_16;
	avr->sleep = avr_callback_sleep_sync;

	ssd1306_init(avr, &mod_s.ssd1306, LCD_WIDTH_PX, LCD_HEIGHT_PX);
	ssd1306_connect(&mod_s.ssd1306, &ssd1306_wiring);

	avr->gdb_port = opts->gdb_port;
	if (opts->debug) {
		avr->state = cpu_Stopped;
		avr_gdb_init(avr);
	}

	{
		// Take start time
		struct timespec tp;
		clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
		mod_s.start_time_ns = tp.tv_sec*1000000000+tp.tv_nsec;
	}
	return 0;
}

void arduboy_simavr_teardown(void)
{

}