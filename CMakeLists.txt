cmake_minimum_required(VERSION 3.13)

set(APP rp2040_cdc_msc)

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
    src/usb/usb_descriptors.c
    src/usb/msc_disk.c
    src/usb/usb_service.c
)

target_include_directories(${APP} PUBLIC
    src
    src/usb
)

pico_enable_stdio_uart(${APP} 1)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(${APP} PUBLIC pico_stdlib pico_unique_id tinyusb_device tinyusb_board)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${APP})

add_custom_target(pack_files
    COMMAND python3 ../pack_msc_files.py
)

add_custom_target(flash
    COMMAND openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program ${APP}.elf verify reset exit"
    DEPENDS ${APP}
)

add_dependencies(${APP} pack_files)
