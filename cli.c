
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "sim_arduboy.h"
#include "arduboy_avr.h"
#include "arduboy_sdl.h"


void print_usage(char *argv[])
{
	fprintf(stderr, "%s [-d] [-v] filename.hex\n", argv[0]);
}


int parse_cmdline(int argc, char *argv[], struct sim_arduboy_opts *opts)
{
	int ch, ret = -1;

	/* set defaults */
	opts->pixel_size = 2;
	/* parse command line */
	while ((ch = getopt(argc, argv, "hdv")) != -1) {
		switch (ch) {
			case 'd':
				opts->debug = true;
				break;
			case 'v':
				opts->verbose++;
				break;
			case 'h':
				ret = 0;
				goto usage;
			default:
				goto usage;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc) {
		opts->hex_file_path = argv[0];
	} else {
		goto usage;
	}
	return 0;

usage:
	print_usage(argv);
	return ret;
}


int main (int argc, char *argv[])
{
	int ret;
	struct sim_arduboy_opts opts;

	memset(&opts, 0, sizeof(opts));
	ret = parse_cmdline(argc, argv, &opts);
	if (ret) {
		goto done;
	}

	opts.win_width = OLED_WIDTH_PX * opts.pixel_size;
	opts.win_height = OLED_HEIGHT_PX * opts.pixel_size;

	ret = arduboy_avr_setup(&opts);
	if (!ret) {
		ret = arduboy_sdl_setup(&opts);
		if (!ret) {
			while (!ret) {
				ret = arduboy_sdl_loop();
				arduboy_avr_loop();
			}
			arduboy_sdl_teardown();
		}
		arduboy_avr_teardown();
	}

done:
	if (ret) {
		perror("Exiting with error");
	}
	return ret;
}
