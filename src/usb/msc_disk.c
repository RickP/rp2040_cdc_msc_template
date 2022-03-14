/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"
#include "content.h"

// whether host does safe-eject
static bool ejected = false;

enum
{
  DISK_BLOCK_NUM  = 8384, // 8KB is the smallest size that windows allow to mount
  DISK_BLOCK_SIZE = 512
};

#define FS_HEADER { \
0Xeb, 0X3c, 0X90, 0X6d, 0X6b, 0X66, 0X73, 0X2e, 0X66, 0X61, 0X74, 0X00, 0X02, 0X01, 0X01, 0X00, 0X01, 0X10, 0X00, 0Xc0, \
0X20, 0Xf8, 0X21, 0X00, 0X20, 0X00, 0X02, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X80, 0X00, 0X29, 0Xcb, \
0Xd7, 0X0c, 0X21, 0X20, 0X20, 0X20, 0X20, 0X20, 0X20, 0X20, 0X20, 0X20, 0X20, 0X20, 0X46, 0X41, 0X54, 0X31, 0X36, 0X20, \
0X20, 0X20, 0X0e, 0X1f, 0Xbe, 0X5b, 0X7c, 0Xac, 0X22, 0Xc0, 0X74, 0X0b, 0X56, 0Xb4, 0X0e, 0Xbb, 0X07, 0X00, 0Xcd, 0X10, \
0X5e, 0Xeb, 0Xf0, 0X32, 0Xe4, 0Xcd, 0X16, 0Xcd, 0X19, 0Xeb, 0Xfe, 0X54, 0X68, 0X69, 0X73, 0X20, 0X69, 0X73, 0X20, 0X6e, \
0X6f, 0X74, 0X20, 0X61, 0X20, 0X62, 0X6f, 0X6f, 0X74, 0X61, 0X62, 0X6c, 0X65, 0X20, 0X64, 0X69, 0X73, 0X6b, 0X2e, 0X20, \
0X20, 0X50, 0X6c, 0X65, 0X61, 0X73, 0X65, 0X20, 0X69, 0X6e, 0X73, 0X65, 0X72, 0X74, 0X20, 0X61, 0X20, 0X62, 0X6f, 0X6f, \
0X74, 0X61, 0X62, 0X6c, 0X65, 0X20, 0X66, 0X6c, 0X6f, 0X70, 0X70, 0X79, 0X20, 0X61, 0X6e, 0X64, 0X0d, 0X0a, 0X70, 0X72, \
0X65, 0X73, 0X73, 0X20, 0X61, 0X6e, 0X79, 0X20, 0X6b, 0X65, 0X79, 0X20, 0X74, 0X6f, 0X20, 0X74, 0X72, 0X79, 0X20, 0X61, \
0X67, 0X61, 0X69, 0X6e, 0X20, 0X2e, 0X2e, 0X2e, 0X20, 0X0d, 0X0a, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, \
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X55, 0Xaa, \
}

const uint8_t fs_header[512] = FS_HEADER;
const uint8_t fs_content[][512] = FS_CONTENT;
const uint32_t fs_size[] = FS_SIZE;
const char *fs_names[][9] = FS_NAMES;
const char *fs_label = FS_LABEL;

#define FAT_SIZE 33
#define NUM_FILES sizeof(fs_size)/4

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
  (void) lun;

  const char vid[] = "Rick";
  const char pid[] = "Shut Up And Die!";
  const char rev[] = "1.0";

  memcpy(vendor_id  , vid, strlen(vid));
  memcpy(product_id , pid, strlen(pid));
  memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
  (void) lun;

  // RAM disk is ready until ejected
  if (ejected) {
    // Additional Sense 3A-00 is NOT_FOUND
    tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
    return false;
  }

  return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
  (void) lun;

  *block_count = DISK_BLOCK_NUM;
  *block_size  = DISK_BLOCK_SIZE;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
  (void) lun;
  (void) power_condition;

  if ( load_eject )
  {
    if (start)
    {
      // load disk storage
    }else
    {
      // unload disk storage
      ejected = true;
    }
  }

  return true;
}

static void get_num_blocks(uint32_t* fs_blocks) {
    for (uint8_t i; i<NUM_FILES; i++) {
        fs_blocks[i] = fs_size[i]/512;
        if (fs_size[i] % 512 != 0) fs_blocks[i]++;
    }
}

static bool check_last_sector(uint32_t sector_num, uint32_t *fs_blocks) {
    uint16_t last_block = 0;
    for (uint8_t i=0; i < NUM_FILES; i++) {
        last_block += fs_blocks[i];
        if (last_block == sector_num-2) {
            return true;
        }
    }
    return false;
}

static void fat_table(uint32_t fat_sector, uint8_t* buffer, uint32_t bufsize) {

    const uint8_t fat_start[4] = {0xF8, 0xFF, 0xFF, 0xFF};
    uint16_t skip = 0;
    uint32_t sector_num;
    uint32_t fs_blocks[NUM_FILES];
    get_num_blocks(fs_blocks);
    uint32_t last_block = 0;
    for (uint8_t i=0; i < NUM_FILES; i++) {
        last_block += fs_blocks[i];
    }

    if (fat_sector == 0) {
        buffer[skip++] = 0xF8;
        buffer[skip++] = 0xFF;
        buffer[skip++] = 0xFF;
        buffer[skip++] = 0xFF;
    }

    for (uint16_t i = skip; i < bufsize-1; i += 2) {
        sector_num = (fat_sector*512+i)/2 + 1;
        if (sector_num-1 > last_block+1) {
            buffer[i] = 0x00;
            buffer[i+1] = 0x00;
        } else if (check_last_sector(sector_num, fs_blocks)) {
            buffer[i] = 0xFF;
            buffer[i+1] = 0xFF;
        } else {
            buffer[i] = sector_num & 0xFF;
            buffer[i+1] = sector_num >> 8;
        }
    }
}

static void fs_directory(uint8_t* buffer, uint32_t bufsize) {

    uint32_t fs_blocks[NUM_FILES];
    get_num_blocks(fs_blocks);

    uint8_t directory_list[512] = {0X00};
    uint8_t date_fields[13] = {0X00};
    uint16_t pos = 0;
    // Root directory label
    memcpy(directory_list, fs_label, 11);
    pos += 11;
    directory_list[pos++] = 0x08;
    for (uint8_t i = 0; i<20; i++) {
        directory_list[pos++] = 0x00;
    }

    for (uint8_t i=0; i<NUM_FILES; i++) {
      // Set name
      memcpy(directory_list+pos, fs_names[i][0], 8);
      pos += 8;
      memcpy(directory_list+pos, fs_names[i][1], 3);
      pos += 3;
      // Set file flags
      directory_list[pos++] = 0x20;
      directory_list[pos++] = 0x18;
      // Set date stuff
      memcpy(directory_list+pos, date_fields, 13);
      pos += 13;
      // Set start cluster
      uint16_t start_sector = 0x02;
      for (uint8_t j=0; j < i; j++) {
        start_sector += fs_blocks[j];
      }
      directory_list[pos++] = start_sector & 0xFF;
      directory_list[pos++] = start_sector >> 8;
      // Set file size
      directory_list[pos++] = fs_size[i] & 0xFF;
      directory_list[pos++] = fs_size[i] >> 8;
      directory_list[pos++] = fs_size[i] >> 16;
      directory_list[pos++] = fs_size[i] >> 24;
    }

    memcpy(buffer, directory_list, bufsize);
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
// offset will always be zero as the buffer size is equal to the sector size (x 512 bytes)
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
  (void) lun;

  // out of ramdisk
  if ( lba >= DISK_BLOCK_NUM ) return -1;

  if (lba == 0) {
    memcpy(buffer, fs_header, bufsize);
    // Replace label in predefined header
    memcpy(buffer+43, fs_label, 11);
  } else if (lba < FAT_SIZE+1) {
    fat_table(lba-1, buffer, bufsize);
  } else if (lba == FAT_SIZE+1) {
    fs_directory(buffer, bufsize);
  } else {
    memcpy(buffer, fs_content[lba-(FAT_SIZE+2)], bufsize);
  }

  return (int32_t) bufsize;
}

bool tud_msc_is_writable_cb (uint8_t lun)
{
  (void) lun;

  return false;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
  (void) lun;

  // out of ramdisk
  if ( lba >= DISK_BLOCK_NUM ) return -1;

  (void) lba; (void) offset; (void) buffer;

  return (int32_t) bufsize;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
  // read10 & write10 has their own callback and MUST not be handled here

  void const* response = NULL;
  int32_t resplen = 0;

  // most scsi handled is input
  bool in_xfer = true;

  switch (scsi_cmd[0])
  {
    default:
      // Set Sense = Invalid Command Operation
      tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

      // negative means error -> tinyusb could stall and/or response with failed status
      resplen = -1;
    break;
  }

  // return resplen must not larger than bufsize
  if ( resplen > bufsize ) resplen = bufsize;

  if ( response && (resplen > 0) )
  {
    if(in_xfer)
    {
      memcpy(buffer, response, (size_t) resplen);
    }else
    {
      // SCSI output
    }
  }

  return (int32_t) resplen;
}
