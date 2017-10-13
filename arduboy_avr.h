
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
