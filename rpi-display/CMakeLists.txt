cmake_minimum_required(VERSION 3.28)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)
set(PIMORONI_PICO_PATH ${CMAKE_SOURCE_DIR}/pimoroni-pico)

project(rpi_display)

pico_sdk_init()

set(CMAKE_C_STANDARD 11)

set(OUTPUT_NAME rpi_display)

include_directories(${PIMORONI_PICO_PATH})
list(APPEND CMAKE_MODULE_PATH ${PIMORONI_PICO_PATH})

add_executable(${OUTPUT_NAME} main.c)
add_subdirectory(pimoroni-pico/common)
add_subdirectory(pimoroni-pico/drivers)
add_subdirectory(pimoroni-pico/libraries)

# Pull in pico libraries that we need
target_link_libraries(${OUTPUT_NAME} pico_stdlib hardware_spi hardware_pwm hardware_dma button pico_display_2 st7789 pico_graphics)

# create map/bin/hex file etc.
pico_add_extra_outputs(${OUTPUT_NAME})
