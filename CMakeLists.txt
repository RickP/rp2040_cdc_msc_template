cmake_minimum_required(VERSION 3.13)

set(APP rp2040_project)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(${APP})

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(${APP}
    src/main.c
)

pico_enable_stdio_usb(${APP} 1)
pico_enable_stdio_usb(${APP} 0)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(${APP} pico_stdlib)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${APP})

add_custom_target(flash
    COMMAND openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program ${APP}.elf verify reset exit"
    DEPENDS ${APP}
)