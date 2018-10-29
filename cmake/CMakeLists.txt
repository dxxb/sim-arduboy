cmake_minimum_required(VERSION 3.10)

project(sim_arduboy)

get_filename_component(PARENT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE SIMAVR_OBJ_DIRNAME)
string(STRIP ${SIMAVR_OBJ_DIRNAME} SIMAVR_OBJ_DIRNAME)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${PARENT_DIRECTORY}/simavr/simavr/cores
        ${PARENT_DIRECTORY}/simavr/examples/parts
        ${PARENT_DIRECTORY}/simavr/simavr/sim
        /usr/local/include
        ${SDL2_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIRS}
        )

file(GLOB ARDUBOY_EMU_SRCS
        "${PARENT_DIRECTORY}/src/*.c"
        "${PARENT_DIRECTORY}/simavr/examples/parts/ssd1306_virt.c")

add_custom_target(
        simavr_lib
        COMMAND make libsimavr
        WORKING_DIRECTORY ${PARENT_DIRECTORY}/simavr/simavr
)

add_executable(sim_arduboy ${ARDUBOY_EMU_SRCS})
add_dependencies(sim_arduboy simavr_lib)

target_link_libraries(sim_arduboy
        ${PARENT_DIRECTORY}/simavr/simavr/obj-${SIMAVR_OBJ_DIRNAME}/libsimavr.a
        ${SDL2_LIBRARIES}
        ${OPENGL_LIBRARIES}
        )
