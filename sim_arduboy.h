
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
