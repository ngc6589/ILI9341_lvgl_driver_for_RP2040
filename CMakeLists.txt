# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "C:/msys64/home/masahiro/pico/pico-sdk")

set(PICO_BOARD pico CACHE STRING "Board type")
set(PICO_DEOPTIMIZED_DEBUG=1)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(lv9dev C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(
    lv9dev
    lv9dev.c
    ili9341.h
    ili9341.c
    xpt2046.h
    xpt2046.c
    pio_spi.c
)

pico_set_program_name(lv9dev "lv9dev")
pico_set_program_version(lv9dev "0.1")

pico_enable_stdio_uart(lv9dev 1)
pico_enable_stdio_usb(lv9dev 0)


pico_generate_pio_header(lv9dev ${CMAKE_CURRENT_LIST_DIR}/spi.pio)

# Add the standard library to the build
target_link_libraries(lv9dev
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(lv9dev PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries (lv9dev 
    hardware_gpio
    hardware_spi
    hardware_dma
    hardware_pio
    hardware_pwm
    lvgl
)

pico_add_extra_outputs(lv9dev)

set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL 1)
set(LV_CONF_BUILD_DISABLE_EXAMPLES 1)
set(LV_CONF_BUILD_DISABLE_DEMOS 1)
add_subdirectory(lvgl)
# Specify path to own LVGL config header
# set(LV_CONF_PATH
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/lv_conf.h
#    CACHE STRING "" FORCE)