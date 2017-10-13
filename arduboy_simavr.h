
#include "ssd1306_virt.h"

struct sim_arduboy_opts;
struct avr_t;

struct arduboy_simavr_mod_state {
	struct avr_t *avr;
	ssd1306_t ssd1306;
	uint64_t start_time_ns;
	bool yield;
};

extern struct arduboy_simavr_mod_state *arduboy_simavr_mod_s;

void avr_run_loop(void);
int arduboy_simavr_setup(struct sim_arduboy_opts *opts, struct avr_t **out);
void arduboy_simavr_teardown(void);
