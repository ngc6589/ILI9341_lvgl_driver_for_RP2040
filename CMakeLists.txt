# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_DEOPTIMIZED_DEBUG=1)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "C:/msys64/home/masahiro/pico/pico-sdk")

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(lcdTest C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(
  lcdTest
  lcdTest.cpp
  ili9341.h
  ili9341.c
  xpt2046.h
  xpt2046.c
  pio_spi.c
)


pico_set_program_name(lcdTest "lcdTest")
pico_set_program_version(lcdTest "0.1")

pico_enable_stdio_uart(lcdTest 1)
pico_enable_stdio_usb(lcdTest 0)

pico_generate_pio_header(lcdTest ${CMAKE_CURRENT_LIST_DIR}/spi.pio)

# Add the standard library to the build
target_link_libraries(lcdTest
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(lcdTest PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(
        lcdTest 
        hardware_gpio
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_pwm
        lvgl
        )

pico_add_extra_outputs(lcdTest)

set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL 1)
set(LV_CONF_BUILD_DISABLE_EXAMPLES 1)
set(LV_CONF_BUILD_DISABLE_DEMOS 1)
add_subdirectory(lvgl)
