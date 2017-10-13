
struct sim_arduboy_opts;
struct avr_t;

int arduboy_sdl_setup(struct sim_arduboy_opts *opts, struct avr_t *avr);
int arduboy_sdl_loop(void);
void arduboy_sdl_teardown(void);
