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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sim_arduboy.h"
#include "arduboy_avr.h"
#include "arduboy_sdl.h"


void print_usage(char *argv[])
{
	fprintf(stderr, "%s [-d] [-v] [-p pixel_size] [-k keymap] filename.hex\n", argv[0]);
}


void parse_keymap(struct sim_arduboy_opts *opts, char *arg)
{
	int i=0;
	for (char *s = strtok(arg, ", "); s && i<BTN_COUNT; s = strtok(NULL, ", ")) {
		opts->key2btn[i] = atoi(s);
		i++;
	}
}


int parse_cmdline(int argc, char *argv[], struct sim_arduboy_opts *opts)
{
	int ch, ret = -1;

	/* set defaults */
	opts->gdb_port = 1234;
	opts->pixel_size = 2;
	opts->key2btn = default_key2btn;
	/* parse command line */
	while ((ch = getopt(argc, argv, "hdvk:g:p:")) != -1) {
		switch (ch) {
			case 'd':
				opts->debug = true;
				break;
			case 'g':
				opts->gdb_port = atoi(optarg);
				break;
			case 'p':
				opts->pixel_size = atoi(optarg);
				break;
			case 'v':
				opts->verbose++;
				break;
			case 'k':
				parse_keymap(opts, optarg);
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

	printf("Keymap: ");
	for (int i = 0; i < BTN_COUNT; i++) {
		printf("%d", opts.key2btn[i]);
		if (i < BTN_COUNT-1) {
			printf(",");
		}
	}
	printf("\n");

	ret = arduboy_avr_setup(&opts);
	if (!ret) {
		ret = arduboy_sdl_setup(&opts);
		if (!ret) {
			while (!ret) {
				ret = arduboy_sdl_loop();
				arduboy_avr_loop();
			}
			if (ret == -1) {
				/* successful exit */
				ret = 0;
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
