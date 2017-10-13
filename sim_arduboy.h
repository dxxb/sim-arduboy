
#include <stdbool.h>


#define LCD_WIDTH_PX (128)
#define LCD_HEIGHT_PX (64)


struct sim_arduboy_opts {
	char *hex_file_path;
	int gdb_port;
	bool debug;
	int verbose;
	int pixel_size;
};
