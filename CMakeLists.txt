# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(Phobri64 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()


# Add executable. Default name is the project name, version 0.1

add_executable(Phobri64 
  src/Phobri64.c
  src/joybus.c 
  src/report.c
  src/usb.c
  src/usb_descriptors.c
  src/control.c
  src/stick.c
  src/read_hardware.c
  src/polyfit.c
)

pico_generate_pio_header(Phobri64 ${CMAKE_CURRENT_LIST_DIR}/src/joybus.pio)

pico_set_program_name(Phobri64 "Phobri64")
pico_set_program_version(Phobri64 "0.1")

add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/generated/joybus.pio.h
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/joybus.pio
	COMMAND Pioasm ${CMAKE_CURRENT_LIST_DIR}/joybus.pio ${CMAKE_CURRENT_LIST_DIR}/generated/joybus.pio.h
	)

# Add the standard library to the build
target_link_libraries(Phobri64
        pico_stdlib
        pico_bootrom
        pico_multicore
        hardware_pio
        hardware_spi
        tinyusb_device
        tinyusb_board)

# Add the standard include files to the build
target_include_directories(Phobri64 PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/include
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(Phobri64
        )

add_compile_options(-finline-functions)

pico_add_extra_outputs(Phobri64)