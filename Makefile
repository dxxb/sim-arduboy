
	# Copyright 2017 Delio Brignoli <brignoli.delio@gmail.com>

	# Arduboy board implementation using simavr.

	# This program is free software: you can redistribute it and/or modify
	# it under the terms of the GNU General Public License as published by
	# the Free Software Foundation, either version 3 of the License, or
	# (at your option) any later version.

	# This program is distributed in the hope that it will be useful,
	# but WITHOUT ANY WARRANTY; without even the implied warranty of
	# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	# GNU General Public License for more details.

	# You should have received a copy of the GNU General Public License
	# along with this program.  If not, see <http://www.gnu.org/licenses/>.

target = sim_arduboy_sdl
simavr-repo = ./simavr
simavr = ${simavr-repo}/simavr
simavr-parts = ${simavr-repo}/examples/parts

IPATH = .
IPATH += ${simavr-parts}
IPATH += ${simavr}/include
IPATH += ${simavr}/sim

VPATH = .
VPATH += ${simavr-repo}/examples/parts

LDFLAGS += -L${simavr}/${OBJ} -l SDL2

include ${simavr-repo}/examples/Makefile.opengl

all: obj libsimavr ${target}

libsimavr:
	make -C ${simavr} libsimavr

obj:
	mkdir -p ${OBJ}

include ${simavr-repo}/Makefile.common

board = ${OBJ}/${target}.elf

${board} : ${OBJ}/ssd1306_virt.o
${board} : ${OBJ}/ssd1306_gl.o
${board} : ${OBJ}/arduboy_sdl.o
${board} : ${OBJ}/arduboy_simavr.o
${board} : ${OBJ}/cli.o

${target}: ${board}
	@echo $@ done

clean: clean-${OBJ}
	make -C ${simavr-repo} clean
	rm -rf *.hex *.a ${target} *.vcd .*.swo .*.swp .*.swm .*.swn

