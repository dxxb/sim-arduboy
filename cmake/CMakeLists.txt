cmake_minimum_required(VERSION 3.10)
project(arduboy-emulator)

execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE SIMAVR_OBJ_DIRNAME)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../simavr/simavr/cores
					${CMAKE_CURRENT_LIST_DIR}/../simavr/examples/parts
					${CMAKE_CURRENT_LIST_DIR}/../simavr/simavr/sim
					/usr/local/include
					${SDL2_INCLUDE_DIRS})

file(GLOB ARDUBOY_EMU_SRCS
	"${CMAKE_CURRENT_LIST_DIR}/../src/*.c"
	"${CMAKE_CURRENT_LIST_DIR}/../simavr/examples/parts/ssd1306_virt.c")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_custom_target(
   simavr_lib
   COMMAND make libsimavr
   WORKING_DIRECTORY ../simavr/simavr
)

link_directories(../simavr/simavr/obj-${SIMAVR_OBJ_DIRNAME})

add_executable(arduboy-emulator ${ARDUBOY_EMU_SRCS})
add_dependencies(arduboy-emulator simavr_lib)
target_link_libraries(arduboy-emulator 
			simavr
			${SDL2_LIBRARIES} 
			${OPENGL_LIBRARIES})
