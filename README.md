# rp2040 CDC&MSC project template

This is a basic template for a Pi Pico C-SDK project. 

It provides the added funtionality of packing it's own sources when you compile it and makes them available as a read-only FAT16 mass storage usb drive.
You can also add up to 14 more files to the folder 'usb_files' that will be available on the disk. Don't exceed your flash memory size (2MB on the Pi Pico).

It also outputs printf messages over a USB serial port in addition to the mass storage device.

## Known issues

- Only 8 charater filenames (longer filenames are truncated)
- No subdirectories
- File creations dates are all 1/1/1970

## Dependencies

- Install gcc arm and add the bin dir to your $PATH

- You'll need a working Python3 installation

- Install rp2040 sdk and add PICO_SDK_PATH to your environment (optional - if you don't do it sdk will be downloaded in build dir)

## Building

    mkdir build && cd build && cmake .. && make
    
This will create a 'rp2040_cdc_msc.uf2' that can be uploaded to your Pico in bootloader mode over USB. 

## Flashing through an rp2040 configured as picoprobe 

If you have two Picos you can shorten develpments cycles by using one as a programmer. For this you have to install openocd with rp2040 and picoprobe support from https://github.com/raspberrypi/openocd.git (for using a 2nd rp2040 board as a debugger - see https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf Appendix A)

```
git clone git clone https://github.com/raspberrypi/openocd.git --recursive --branch picoprobe --depth=1
cd openocd
./bootstrap
./configure --disable-presto --disable-openjtag
make -j4
sudo make install
```

Then you can compile and flash with:

    make flash
