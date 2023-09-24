cmake_minimum_required(VERSION 3.13)

# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(N64_Remapper C CXX ASM)

pico_sdk_init()

add_subdirectory(lib/Zenith_Lib)

include_directories(
	include
)

add_executable(${PROJECT_NAME}
	src/joybus_cons.c
	src/main.c
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0")

target_compile_definitions(${PROJECT_NAME}
	PRIVATE
	PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
	#PICO_DEFAULT_BOOT_STAGE2=boot2_generic_03h
	#PICO_FLASH_SPI_CLKDIV=8
)

# Enable debugging features when running debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()


target_link_libraries(${PROJECT_NAME}
PRIVATE
zenith_lib
pico_stdlib
pico_bootrom
pico_multicore
pico_platform
pico_rand

hardware_gpio
hardware_adc
hardware_flash
hardware_pio
hardware_dma
hardware_pwm
hardware_spi
hardware_i2c
hardware_sync
hardware_timer
hardware_watchdog

tinyusb_device
tinyusb_board
)

pico_add_extra_outputs(${PROJECT_NAME})
