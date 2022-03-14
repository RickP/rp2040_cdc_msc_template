# rp2040_template

## Dependencies

- Install gcc arm and add the bin dir to your $PATH

- Install rp2040 sdk and add PICO_SDK_PATH to your environment (optional - if you don't do it sdk will be downloaded in build dir)

- Install openocd with rp2040 and picoprobe support from https://github.com/raspberrypi/openocd.git (for using a 2nd rp2040 board as a debugger - see https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf Appendix A) 

```
git clone git clone https://github.com/raspberrypi/openocd.git --recursive --branch picoprobe --depth=1
cd openocd
./bootstrap
./configure --disable-presto --disable-openjtag
make -j4
sudo make install
```

## Building

    mkdir build && cd build && cmake .. && make

## Flashing through an rp2040 configured as picoprobe 

    make flash
