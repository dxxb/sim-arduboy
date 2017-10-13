
struct sim_arduboy_opts;
struct ssd1306_t;

int arduboy_sdl_setup(struct sim_arduboy_opts *opts);
void arduboy_sdl_render_frame(void);
int arduboy_sdl_loop(void);
void arduboy_sdl_teardown(void);
